#ifndef WEB_H
#define WEB_H

// ── BondPay API-Only Mode ───────────────────────────────────────────
// No HTML served from ESP8266. Web UI runs locally on user's browser.
// ESP8266 only serves JSON API endpoints over WiFi STA connection.
//
// The web UI HTML file is separate and communicates via HTTP API.
// This header is kept for any shared web utilities if needed.

inline void sendCORS(ESP8266WebServer &srv) {
  srv.sendHeader("Access-Control-Allow-Origin", "*");
  srv.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  srv.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  srv.sendHeader("Access-Control-Allow-Private-Network", "true");
  srv.sendHeader("Cache-Control", "no-cache");
}

// Handle CORS preflight OPTIONS requests
inline void handleCORSPreflight(ESP8266WebServer &srv) {
  sendCORS(srv);
  srv.send(204, "", "");
}

#endif
