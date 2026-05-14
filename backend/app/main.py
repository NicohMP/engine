import os
from pathlib import Path
import sys

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field
from supabase import create_client

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "build"))

from ml.run_random_backtest import run_random_backtest

app = FastAPI(title="Trading Engine API")  # Create API application project

# Middle ware allowing vite to talk to api backend (i think)
app.add_middleware(
    CORSMiddleware,
    allow_origins=[
        "http://localhost:5173",
        "http://127.0.0.1:5173",
    ],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


class SimulationRunCreate(BaseModel):
    name: str = Field(min_length=1)
    dataset_id: str = Field(min_length=1)
    symbols: list[str] = Field(min_length=1)
    initial_cash: float = Field(gt=0)


@app.get("/")  #
def root():
    return {"name": "Trading Engine API", "status": "ok"}


@app.get("/health")
def health_check():
    return {"status": "ok"}


@app.get("/config")
def config_check():
    return {
        "app_env": os.getenv("APP_ENV"),
        "supabase_url_configured": os.getenv("SUPABASE_URL")
        not in (None, "", "not-configured"),
        "supabase_anon_key_configured": os.getenv("SUPABASE_ANON_KEY")
        not in (None, "", "not-configured"),
        "supabase_service_role_key_configured": os.getenv("SUPABASE_SERVICE_ROLE_KEY")
        not in (None, "", "not-configured"),
    }


def supabase_is_configured() -> bool:
    return os.getenv("SUPABASE_URL") not in (None, "", "not-configured") and (
        os.getenv("SUPABASE_SERVICE_ROLE_KEY") not in (None, "", "not-configured")
        or os.getenv("SUPABASE_ANON_KEY") not in (None, "", "not-configured")
    )


def get_supabase_client():
    key = os.getenv("SUPABASE_SERVICE_ROLE_KEY") or os.getenv("SUPABASE_ANON_KEY")

    if os.getenv("SUPABASE_URL") in (None, "", "not-configured") or key in (
        None,
        "",
        "not-configured",
    ):
        raise HTTPException(status_code=503, detail="Supabase is not configured")

    return create_client(os.environ["SUPABASE_URL"], key)


def raise_supabase_error(exc: Exception):
    raise HTTPException(
        status_code=502, detail=f"Supabase request failed: {exc}"
    ) from exc


@app.get("/supabase/health")
def supabase_health():
    if not supabase_is_configured():
        return {
            "configured": False,
            "reachable": False,
        }

    client = get_supabase_client()

    try:
        client.table("simulation_runs").select("id").limit(1).execute()
    except Exception as exc:
        raise HTTPException(
            status_code=502,
            detail=f"Supabase configured, but simulation_runs is not reachable: {exc}",
        ) from exc

    return {
        "configured": True,
        "reachable": True,
        "table": "simulation_runs",
    }


@app.get("/simulation-runs")
def list_simulation_runs():
    client = get_supabase_client()

    try:
        response = (
            client.table("simulation_runs")
            .select("id, created_at, name, dataset_id, symbols, initial_cash")
            .order("created_at", desc=True)
            .limit(20)
            .execute()
        )
    except Exception as exc:
        raise_supabase_error(exc)

    return {"items": response.data}


@app.post("/simulation-runs", status_code=201)
def create_simulation_run(run: SimulationRunCreate):
    client = get_supabase_client()

    try:
        response = client.table("simulation_runs").insert(run.model_dump()).execute()
    except Exception as exc:
        raise_supabase_error(exc)

    return response.data[0]


# Random backtest request model
class RandomBacktestRequest(BaseModel):
    tickers: list[str] = Field(default_factory=lambda: ["AAPL", "AMZN"], min_length=1)
    start: str = "2026-01-01"
    end: str = "2026-05-01"
    target_notional: float = Field(default=1000.0, gt=0)
    initial_cash: float = Field(default=100000.0, gt=0)
    seed: int = 42


# Endpoint for random backtest request
@app.post("/backtests/random")
def run_random_backtest_endpoint(request: RandomBacktestRequest):
    try:
        return run_random_backtest(
            tickers=request.tickers,
            start=request.start,
            end=request.end,
            target_notional=request.target_notional,
            initial_cash=request.initial_cash,
            seed=request.seed,
        )
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc)) from exc
