import json
import os
from http.server import BaseHTTPRequestHandler, HTTPServer


class Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        length = int(self.headers["Content-Length"])
        data = json.loads(self.rfile.read(length))
        os.system("cls")
        print(f"ESP2 - Temp: {data['esp2_temp']}°C | Hum: {data['esp2_hum']}%")
        print(f"ESP3 - Temp: {data['esp3_temp']}°C | Hum: {data['esp3_hum']}%")
        self.send_response(2000)
        self.end_headers()

    def log_message(self, format, *args):
        pass


HTTPServer(("0.0.0.0", 8080), Handler).serve_forever()
