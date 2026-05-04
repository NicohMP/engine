from fastapi import FastAPI
import os

app = FastAPI(title="Trading Engine API")  # Create API application project


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
    }
