#!/usr/bin/python
#
# Python3 local server for AP/STA index, to develop/debug the HTML locally without
# having to recompile SDB and use an ESP32 deployment.

import argparse
import http.server
import json
import urllib.parse

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
            if ARGS.mode == "ap":
                data = self.ap_get()
            else:
                data = self.sta_get()
            self.wfile.write(bytes(json.dumps(data), "utf-8"))
        elif path.startswith("/props"):
            if ARGS.mode == "ap":
                # this is only for STA mode
                self.send_response(404)
            else:
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.end_headers()
                data = self.sta_props()
                self.wfile.write(bytes(json.dumps(data), "utf-8"))
        else:
            self.send_response(404)

    def ap_get(self):
        data = {
            "id": "Current SSID or blank",
            "pw": "Current Password or blank",
            "st": "AP status string",
            "ls": ["ESSID 1","OSSID\"2\"","ESSID\'3\'"],
        }
        return data

    def sta_get(self):
        data = {
            "blocks": ["block0", "block1"],
            "sensors": ["tof0","tof1"],
            "servers": ["jmri", "mqtt"]
        }
        return data

    def sta_props(self):
        url = urllib.parse.urlparse(self.path)
        params = urllib.parse.parse_qs(url.query)
        type = params.get("t")
        name = params.get("n")
        if type: type = type[0]
        if name: name = name[0]
        data = {}

        if type == "block":
            if name in [ "block0", "block1" ]:
                data = {
                    "props": {
                        "bl.name.s":    {"l": "Name",               "v": name},
                        "bl.desc.s":    {"l": "Description",        "v": "Block"},
                        "bl.negate.b":  {"l": "Inverted Output",    "v": "0"},
                        "bl.sensor.s":  {"l": "Sensor Name",        "v": name.replace("block", "tof")},
                        "bl.jmname.s":  {"l": "JMRI Sensor Name",   "v": "NS752"},
                        "bl.mqtopic.s": {"l": "MQTT Topic",         "v": "/some/topic"},
                        "bl!state.b":   {"l": "Current State",      "v": "1"}
                    }
                }
        elif type == "sensor":
            if name in [ "tof0", "tof1" ]:
                data = {
                    "props": {
                        "sr.name.s":    {"l": "Name",               "v": name},
                        "sr.desc.s":    {"l": "Description",        "v": "Adafruit VL53L0X ToF"},
                        "sr.min.i":     {"l": "Min Threshold (mm)", "v": "0"},
                        "sr.max.i":     {"l": "Max Threshold (mm)", "v": "2000"},
                        "sr!value.i":   {"l": "Distance (mm)",      "v": "2000"}
                    }
                }
        elif type == "server":
            if name in [ "jmri", "mqtt" ]:
                data = {
                    "props": {
                        "sv.name.s":    {"l": "Name",           "v": name},
                        "sv.desc.s":    {"l": "Description",    "v": "JMRI or MQTT server"},
                        "sv.host.s":    {"l": "Server IP",      "v": "127.0.0.1"},
                        "sv.port.i":    {"l": "Server Port",    "v": "1234"},
                        "mq.channel.s": {"l": "MQTT Channel",   "v": "/something"}
                    }
                }

        return data

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
