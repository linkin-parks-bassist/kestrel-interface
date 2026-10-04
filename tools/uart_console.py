#!/usr/bin/env python3
"""Keep one UART connection open; pass stdin commands and stream/log output."""
import argparse
import os
import select
import sys
import serial

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--port', required=True)
parser.add_argument('--baud', type=int, default=115200)
parser.add_argument('--log', required=True)
args = parser.parse_args()
# Opening can still reset this adapter/board. Do it once per HIL session.
uart = serial.Serial(port=None, baudrate=args.baud, timeout=0)
uart.dtr = False
uart.rts = False
uart.port = args.port
with open(args.log, 'xb') as log:
    uart.open()
    try:
        while True:
            ready, _, _ = select.select([uart.fileno(), sys.stdin.fileno()], [], [])
            if uart.fileno() in ready:
                data = uart.read(4096)
                if data:
                    log.write(data)
                    log.flush()
                    sys.stdout.buffer.write(data)
                    sys.stdout.buffer.flush()
            if sys.stdin.fileno() in ready:
                data = os.read(sys.stdin.fileno(), 4096)
                if not data:
                    break
                uart.write(data.replace(b'\n', b'\r'))
    finally:
        uart.close()
