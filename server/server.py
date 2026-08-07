import json
import sqlite3
import time
from http.server import BaseHTTPRequestHandler, HTTPServer

DB_PATH = "sensors.db"
LAST_WRITE_INTERVAL = 60  # sec
_last_write = 0


def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute("PRAGMA journal_mode=WAL;")
    conn.execute("""
            CREATE TABLE IF NOT EXISTS readings (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                ts REAL,
                node TEXT,
                temp REAL,
                hum REAL
            )
        """)
    conn.commit()
    conn.close()


class Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        try:
            length = int(self.headers["Content-Length"])
            data = json.loads(self.rfile.read(length))

            # display
            print("\033[H\033[J", end="")
            print(f"ESP2 - Temp: {data['esp2_temp']}°C | Hum: {data['esp2_hum']}%")
            print(f"ESP3 - Temp: {data['esp3_temp']}°C | Hum: {data['esp3_hum']}%")

            global _last_write
            now = time.time()

            # write to the database
            if now - _last_write >= LAST_WRITE_INTERVAL:
                _last_write = now

                conn = sqlite3.connect(DB_PATH)
                conn.execute(
                    "INSERT INTO readings (ts, node, temp, hum) VALUES (?, ?, ?, ?)",
                    (now, "esp2", data["esp2_temp"], data["esp2_hum"]),
                )

                conn.execute(
                    "INSERT INTO readings (ts, node, temp, hum) VALUES (?, ?, ?, ?)",
                    (now, "esp3", data["esp3_temp"], data["esp3_hum"]),
                )
                conn.commit()
                conn.close()

            self.send_response(200)
        except Exception as e:
            print(f"Error: {e}")
            self.send_response(400)
        finally:
            self.end_headers()

    def log_message(self, format, *args):
        pass


if __name__ == "__main__":
    init_db()
    HTTPServer(("0.0.0.0", 8080), Handler).serve_forever()
