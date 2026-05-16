#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if ! command -v python3.12 >/dev/null 2>&1; then
  echo "Missing python3.12. Install Python 3.12, then rerun this script."
  exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
  echo "Missing cmake. Install CMake, then rerun this script."
  exit 1
fi

if ! command -v npm >/dev/null 2>&1; then
  echo "Missing npm. Install Node.js, then rerun this script."
  exit 1
fi

echo "Creating Python virtual environment..."
python3.12 -m venv .venv

echo "Installing Python dependencies..."
.venv/bin/pip install --upgrade pip
.venv/bin/pip install -r requirements.txt

echo "Configuring C++ build..."
cmake -S cpp -B build -DPython_EXECUTABLE="$ROOT_DIR/.venv/bin/python"

echo "Building C++ engine bindings..."
cmake --build build -j

echo "Installing frontend dependencies..."
cd web
npm install

echo "Writing frontend API config..."
cat > .env.local <<EOF
VITE_API_URL=http://127.0.0.1:8000
EOF

echo
echo "Setup complete."
echo "Run ./scripts/start.sh to launch the app."
