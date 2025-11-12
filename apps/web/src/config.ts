/**
 * Server configuration based on environment
 * In dev mode: uses localhost:3000
 * In production: uses VITE_SERVER_URL or defaults to raspberrypi.local:3000
 */
export const getServerUrl = (): string => {
  if (import.meta.env.DEV) {
    return "http://localhost:3000";
  }

  return import.meta.env.VITE_SERVER_URL || "http://raspberrypi.local:3000";
};

export const getWebSocketUrl = (): string => {
  const serverUrl = getServerUrl();
  // Convert http:// to ws:// and https:// to wss://
  return serverUrl.replace(/^http/, "ws");
};
