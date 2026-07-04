from flask import Flask, request, jsonify
import json
import sqlite3
import os

app = Flask(__name__)

DB_FILE = "database.db"
API_KEY = os.environ.get("WARDRIVE_API_KEY")

if not API_KEY:
    raise RuntimeError("WARDRIVE_API_KEY environment variable is not set")

def require_api_key():
    if request.headers.get("X-API-Key") != API_KEY:
        return jsonify({"status": "error", "message": "invalid or missing API key"}), 401
    return None

def init_db():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS access_points (
            bssid TEXT PRIMARY KEY,
            ssid TEXT,
            max_rssi INTEGER,
            latitude REAL,
            longitude REAL,
            discovery_time INTEGER,
            last_update INTEGER
        )
    """)
    conn.commit()
    conn.close()

@app.route("/data", methods=["GET"])
def get_data():
    auth_error = require_api_key()
    if auth_error:
        return auth_error

    try:
        conn = sqlite3.connect(DB_FILE)
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM access_points")
        rows = cursor.fetchall()
        conn.close()
        
        return jsonify([dict(row) for row in rows]), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route("/log", methods=["POST"])
def log_data():
    auth_error = require_api_key()
    if auth_error:
        return auth_error

    try:
        network_batch = request.get_json()
        if not network_batch:
            return jsonify({"status": "bad_request", "message": "No JSON data received"}), 400
            
        print(f"\n[INFO] Received a batch of {len(network_batch)} networks!")
        
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        new_count, updated_count = 0, 0
        
        for net in network_batch:
            bssid = net.get("bssid")
            ssid = net.get("ssid", "[Hidden Network]")
            rssi = int(net.get("rssi", -100))
            lat = float(net.get("lat", 0.0))
            lon = float(net.get("lon", 0.0))
            timestamp = int(net.get("time", 0))
            
            cursor.execute("SELECT max_rssi FROM access_points WHERE bssid = ?", (bssid,))
            row = cursor.fetchone()
            
            if row is None:
                cursor.execute("""
                    INSERT INTO access_points (bssid, ssid, max_rssi, latitude, longitude, discovery_time, last_update)
                    VALUES (?, ?, ?, ?, ?, ?, ?)
                """, (bssid, ssid, rssi, lat, lon, timestamp, timestamp))
                new_count += 1
            else:
                old_rssi = row[0]
                if rssi > old_rssi:
                    cursor.execute("""
                        UPDATE access_points 
                        SET max_rssi = ?, latitude = ?, longitude = ?, last_update = ?
                        WHERE bssid = ?
                    """, (rssi, lat, lon, timestamp, bssid))
                    updated_count += 1
                else:
                    cursor.execute("""
                        UPDATE access_points SET last_update = ? WHERE bssid = ?
                    """, (timestamp, bssid))
        
        conn.commit()
        conn.close()
        
        print(f"[INFO] Batch Processed -> New: {new_count} | Updated Stronger Signal: {updated_count}")
        return jsonify({"status": "success"}), 200
        
    except Exception as e:
        print(f"[WARN] processing batch: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == "__main__":
    init_db()
    app.run(host="0.0.0.0", port=53104)