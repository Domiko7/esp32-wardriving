# esp32-wardrive

Wardriving rig built on an ESP32. It reads position from a serial GPS module, scans for nearby WiFi networks, and uploads batches of (BSSID, SSID, RSSI, lat/lon, timestamp) to a small Flask server that keeps a SQLite table of access points.

## Hardware

- ESP32 dev board
- Serial GPS module (NMEA, wired to `Serial2`)

Default UART pins are set in `include/settings.h` (`RXD`, `TXD`) — change them to match your wiring.

## How it works

The firmware runs a 40-second cycle:

1. For the first 40 seconds, it scans WiFi every 4 seconds and appends the results to an in-memory batch, tagging each network with the current GPS fix.
2. Once the 40 seconds are up, it connects to WiFi, POSTs the whole batch to the server as JSON, clears it, and starts over.

GPS is parsed continuously in the background using TinyGPSPlus so the latest fix is always available when a scan happens.

## Firmware setup

Edit `include/settings.h` before building:

```c
#define WIFI_SSID "your-network"
#define WIFI_PASS "your-password"
#define ENDPOINT "http://your-server:port/log"
#define API_KEY "your-api-key"
#define RXD 16
#define TXD 17
```

`API_KEY` must match the `WARDRIVE_API_KEY` the server is started with (see below) — it's sent as an `X-API-Key` header on every upload.

Build and flash with PlatformIO:

```bash
pio run
pio run -t upload
pio device monitor
```

## Server

A single-file Flask app in `server/server.py` that stores discovered access points in SQLite, keeping only the strongest RSSI seen per BSSID along with its position.

```bash
cd server
export WARDRIVE_API_KEY="your-api-key"
python3 server.py
```

The server refuses to start without `WARDRIVE_API_KEY` set, and both endpoints reject requests whose `X-API-Key` header doesn't match it.

Endpoints:

- `POST /log` — accepts a JSON array of networks (`bssid`, `ssid`, `rssi`, `lat`, `lon`, `time`) and upserts them into `access_points`.
- `GET /data` — returns all stored access points as JSON.

Requires Flask (`pip install flask`); no other dependencies.

## License

AGPL-3.0, see `LICENSE`.
