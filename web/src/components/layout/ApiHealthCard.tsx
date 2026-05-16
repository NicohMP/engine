import { useState } from "react";

type HealthResponse = {
  status: string;
};

function ApiHealthCard() {
  const [status, setStatus] = useState<string>("not checked");
  const [error, setError] = useState<string | null>(null);

  async function checkHealth() {
    setStatus("checking...");
    setError(null);

    try {
      const response = await fetch(`${import.meta.env.VITE_API_URL}/health`);

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}`);
      }

      const data: HealthResponse = await response.json();
      setStatus(data.status);
    } catch (err) {
      setStatus("failed");
      setError(err instanceof Error ? err.message : "Unknown error");
    }
  }

  return (
    <section className="status-card" aria-label="API health">
      <div>
        <p className="section-kicker">Backend</p>
        <h2>API Health</h2>
      </div>

      <div className="status-actions">
        <span className={`status-pill status-${status.replace(" ", "-")}`}>
          {status}
        </span>

        <button className="secondary-button" onClick={checkHealth}>
          Check API Health
        </button>
      </div>

      {error && <p className="error-message">Error: {error}</p>}
    </section>
  );
}

export default ApiHealthCard;
