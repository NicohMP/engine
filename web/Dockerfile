FROM node:22-slim

WORKDIR /app

COPY web/package*.json ./
RUN npm install

COPY web ./

ENV VITE_API_URL=http://127.0.0.1:8000

CMD ["npm", "run", "dev", "--", "--host", "0.0.0.0"]
