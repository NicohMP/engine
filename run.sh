#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

NEEDS_SETUP=0

if [ ! -x ".venv/bin/python" ]; then
  NEEDS_SETUP=1
fi

if [ ! -x ".venv/bin/fastapi" ]; then
  NEEDS_SETUP=1
fi

if [ ! -d "web/node_modules" ]; then
  NEEDS_SETUP=1
fi

if [ ! -f "web/.env.local" ]; then
  NEEDS_SETUP=1
fi

if ! ls build/engine_cpp*.so >/dev/null 2>&1; then
  NEEDS_SETUP=1
fi

if [ "$NEEDS_SETUP" -eq 1 ]; then
  echo "Project is not fully set up. Running setup first..."
  ./scripts/setup.sh
fi

./scripts/start.sh
