FROM python:3.12-slim

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

COPY requirements-docker.txt ./
RUN pip install --no-cache-dir -r requirements-docker.txt

COPY cpp ./cpp
COPY ml ./ml
COPY backend ./backend

RUN cmake -S cpp -B build -G Ninja \
    -DPython_EXECUTABLE="$(which python)" \
    -Dpybind11_DIR="$(python -m pybind11 --cmakedir)"
RUN cmake --build build -j

ENV PYTHONPATH=/app:/app/build

CMD ["fastapi", "run", "backend/app/main.py", "--host", "0.0.0.0", "--port", "8000"]
