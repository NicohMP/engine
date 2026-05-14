const API_URL = import.meta.env.VITE_API_URL;

fetch(`${API_URL}/health`)

export async function apiFetch(path: string, options?: RequestInit) {
  const response = await fetch(`${API_URL}${path}`, options);

  if (!response.ok) {
    throw new Error(`HTTP ${response.status}`);
  }

  return response;
}
