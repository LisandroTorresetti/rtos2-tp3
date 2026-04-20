import serial
import time
import random
import threading
import re
from collections import defaultdict

# ================= CONFIG =================
PORT = "/dev/ttyACM0"
BAUD = 115200

REQUEST_RATE = 40          # avg msgs/sec
JITTER = 0.3               # randomness factor
FRAME_SIZE = 256

# =========================================

ser = serial.Serial(PORT, BAUD, timeout=0.1)

stats = defaultdict(int)
lock = threading.Lock()

# regex for parsing messages
msg_regex = re.compile(r"\{([^}]+)\}")

def parse_message(msg):
    try:
        content = msg.strip()[1:-1]
        fields = dict()

        for part in content.split(","):
            if ":" in part:
                k, v = part.split(":")
                fields[k.strip()] = v.strip()

        return fields
    except:
        return None


def receiver():
    buffer = ""

    while True:
        try:
            data = ser.read(1024).decode(errors="ignore")
            if not data:
                continue

            buffer += data

            while "{" in buffer and "}" in buffer:
                start = buffer.find("{")
                end = buffer.find("}", start)

                if end == -1:
                    break

                raw = buffer[start:end+1]
                buffer = buffer[end+1:]

                msg = parse_message(raw)
                if not msg:
                    continue

                mtype = msg.get("type")

                with lock:
                    if mtype == "0":
                        stats["TICK"] += 1

                    elif mtype == "2":
                        stats["RESPONSE"] += 1

                    elif mtype == "1":
                        stats["ECHO_REQUEST_RX"] += 1

        except Exception as e:
            print("RX error:", e)


def sender():
    msg_id = 1

    while True:
        priority = random.randint(0, 1000)

        msg = f"{{type:1,id:{msg_id},priority:{priority}}}\n"

        ser.write(msg.encode())

        with lock:
            stats["REQUEST_SENT"] += 1

        msg_id += 1

        # ~40 msgs/sec with jitter
        base_delay = 1.0 / REQUEST_RATE
        jitter = random.uniform(-JITTER, JITTER) * base_delay
        time.sleep(max(0.001, base_delay + jitter))


def printer():
    while True:
        time.sleep(2)
        with lock:
            print("\n=== STATS ===")
            for k, v in stats.items():
                print(f"{k}: {v}")


# ================= MAIN =================

print("Starting UART test...")

t_rx = threading.Thread(target=receiver, daemon=True)
t_tx = threading.Thread(target=sender, daemon=True)
t_pr = threading.Thread(target=printer, daemon=True)

t_rx.start()
t_tx.start()
t_pr.start()

while True:
    time.sleep(1)