#!/usr/bin/python
#
# Python3 local server for AP/STA index, to develop/debug the HTML locally without
# having to recompile SDB and use an ESP32 deployment.

import argparse
import json
import http.server

PARSER = argparse.ArgumentParser()
ARGS = None

HOST = "localhost"
PORT = 8080
AP_INDEX = "src/html/_mod_wifi_ap_index.html"
STA_INDEX = "src/html/_mod_wifi_sta_index.html"
INDEX = None

def parse():
    global ARGS, HOST, PORT, INDEX
    PARSER.add_argument("-p", "--port", default=PORT)
    PARSER.add_argument("--host", default=HOST)
    PARSER.add_argument("-m", "--mode", choices=["ap", "sta"], required=True)
    ARGS = PARSER.parse_args()
    PORT = int(ARGS.port)
    HOST = ARGS.host
    INDEX = AP_INDEX if ARGS.mode == "ap" else STA_INDEX


def serve():
    webServer = http.server.HTTPServer((HOST, PORT), LocalSdbServer)
    print("SDB Local Server started at http://%s:%s for %s" % (HOST, PORT, INDEX))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    print("SDB Local Server stopped.")


class LocalSdbServer(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        path = self.path
        if path == "/":
            # Serve the main index
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            with open(INDEX) as indexFile:
                self.wfile.write(bytes(indexFile.read(), "utf-8"))
        elif path == "/get":
            # This is the only dynamic request currently handled by SDB.
            # Response is JSON.
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            data = {
                "id": "Current SSID or blank",
                "pw": "Current Password or blank",
                "st": "AP status string",
                "ls": ["ESSID 1","OSSID\"2\"","ESSID\'3\'"],
            }
            self.wfile.write(bytes(json.dumps(data), "utf-8"))
        else:
            self.send_response(404)
    
    def do_POST(self):
        path = self.path
        if path == "/set":
            print("SDB POST headers:" + str(self.headers))
            # get the body for the POST
            content_length = int(self.headers.get('Content-Length'))
            post_body = self.rfile.read(content_length)
            print("SDB POST body:" + str(post_body))
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            data = {
                "st": "Connecting",
            }
            self.wfile.write(bytes(json.dumps(data), "utf-8"))
        else:
            self.send_response(404)

if __name__ == "__main__":
    parse()
    serve()
