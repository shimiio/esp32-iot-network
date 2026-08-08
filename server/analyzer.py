import sqlite3
import time

import requests

DB_PATH = "sensors.db"

ANALYSIS_INTERVAL = 300  # seconds, adjust as needed
OLLAMA_URL = "http://localhost:11434/api/generate"
MODEL = "llama3.2"

SYSTEM_PROMPT = """You are a monitoring assistant for a temperature and humidity
sensor network with two nodes (esp2, esp3). You will be given recent sensor
readings. Briefly (2-3 sentences) report whether there are any anomalies, sharp
changes, or suspicious differences between the two sensors. If everything looks
normal, say so in one short line. Do not invent data beyond what is given."""


def get_recent_readings(seconds=300):
    conn = sqlite3.connect(DB_PATH)
    since = time.time() - seconds
    rows = conn.execute(
        "SELECT node, temp, hum, ts FROM readings WHERE ts > ? ORDER BY ts", (since,)
    ).fetchall()
    conn.close()
    return rows


def run_analysis():
    rows = get_recent_readings(seconds=ANALYSIS_INTERVAL)
    if not rows:
        print("No data in this window, skipping.")
        return

    prompt = f"Sensor readings from the last {ANALYSIS_INTERVAL} seconds: {rows}"

    response = requests.post(
        OLLAMA_URL,
        json={
            "model": MODEL,
            "system": SYSTEM_PROMPT,
            "prompt": prompt,
            "stream": False,
        },
    )

    result = response.json()["response"]
    print(f"[{time.strftime('%H:%M:%S')}] {result}")


if __name__ == "__main__":
    while True:
        run_analysis()
        time.sleep(ANALYSIS_INTERVAL)
