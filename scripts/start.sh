#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if [ ! -x ".venv/bin/fastapi" ]; then
  echo "Python environment is not ready. Run ./scripts/setup.sh first."
  exit 1
fi

if [ ! -d "web/node_modules" ]; then
  echo "Frontend dependencies are not installed. Run ./scripts/setup.sh first."
  exit 1
fi

cleanup() {
  echo
  echo "Stopping servers..."
  if [ -n "${BACKEND_PID:-}" ]; then
    kill "$BACKEND_PID" 2>/dev/null || true
  fi
  if [ -n "${FRONTEND_PID:-}" ]; then
    kill "$FRONTEND_PID" 2>/dev/null || true
  fi
}

trap cleanup EXIT INT TERM

echo "Starting FastAPI backend at http://127.0.0.1:8000 ..."
.venv/bin/fastapi dev backend/app/main.py --host 127.0.0.1 --port 8000 &
BACKEND_PID=$!

echo "Starting React frontend at http://127.0.0.1:5173 ..."
cd web
npm run dev -- --host 127.0.0.1 &
FRONTEND_PID=$!

sleep 2

echo
echo "App is starting."
echo "Open http://127.0.0.1:5173 in your browser."

if command -v open >/dev/null 2>&1; then
  open http://127.0.0.1:5173 || true
fi

wait
