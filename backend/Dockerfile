# Start from an official python image
FROM python:3.12-slim 

# app/ is the working is the working directory
WORKDIR /app

# Copy the dependency file requirements.txt in the docker container
COPY requirements.txt .

# run pip install with the requirements to get all the necessary libraries
RUN pip install --no-cache-dir -r requirements.txt

# Copy the backend code in the docker image
COPY backend/app ./app

# Default command run when the container starts. Here, it runs the FastAPI server.
CMD ["fastapi", "run", "app/main.py", "--host", "0.0.0.0", "--port", "8000"]
