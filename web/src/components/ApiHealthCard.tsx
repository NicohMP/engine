// useState is a React hook. It lets a component remember values between renders.
import { useState } from "react";

// TypeScript type describing the JSON returned by GET /health.
type HealthResponse = {
  status: string;
};

function ApiHealthCard(){
  
  // API health state. This controls what we display under "API Health".
  const [status, setStatus] = useState<string>("not checked");
  // Stores an error message if the health request fails. null means no error.
  const [error, setError] = useState<string | null>(null);


  // Calls the FastAPI GET /health endpoint and updates the UI with the result.
  async function checkHealth() {
      // Give immediate feedback to the user before the network request finishes.
      setStatus("checking...");

      // Clear any previous health error before making a new request.
      setError(null);

      try {
        // import.meta.env.VITE_API_URL comes from web/.env.local.
        const response = await fetch(`${import.meta.env.VITE_API_URL}/health`);

        // fetch only rejects on network errors. HTTP 500/404 still produce a response,
        // so we explicitly treat non-2xx responses as errors.
        if (!response.ok) {
          throw new Error(`HTTP ${response.status}`);
        }

        // Parse the JSON response and tell TypeScript it matches HealthResponse.
        const data: HealthResponse = await response.json();

        // Update React state, which causes the component to re-render.
        setStatus(data.status);
      } catch (err) {
        // If anything failed, show a simple error state in the UI.
        setStatus("failed");
        setError(err instanceof Error ? err.message : "Unknown error");
      }
    }
    // Return a section with a title api health, the result of the checkHealth : status, etc
    return (
    <section>
      <h2>API Health</h2>

      <button onClick={checkHealth}>Check API Health</button>

      <p>Status: {status}</p>

      {error && <p>Error: {error}</p>}
    </section>
  );

}

export default ApiHealthCard;
