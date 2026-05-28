#!/usr/bin/env python3

import serial
import sys

print("python echo script started\n")
HEX = False

ser = serial.Serial(sys.argv[1], sys.argv[2])

try:
	while True:
		sys.stdout.write(ser.read(1).decode(encoding="ISO-8859-1"))
		sys.stdout.flush()
except KeyboardInterrupt:
	print("key esc")
finally:
	print("done")
	ser.close()