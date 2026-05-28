#!/usr/bin/env python3

import serial
import sys
import collections
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

matplotlib.use('TkAgg')	# switch backend if needed

print("python serial plot script started\n")

ser = serial.Serial(sys.argv[1], sys.argv[2])

data = collections.deque(maxlen=10000)

plt.ion()  # interactive plot updates
fig, ax = plt.subplots()
line, = ax.plot([], [], 'r-')
ax.set_xlim(0, 300)
ax.set_ylim(0, 1023)  # 10-bit values
ax.set_title("Received 10-bit values")
ax.set_xlabel("n")
ax.set_ylabel("x(n)")

try:
	while True:
		value = int.from_bytes(ser.read(2), byteorder='big', signed=False)

		if value > 1023:	# alignment error has happened
			throwaway = ser.read(1)
			continue

		data.append(value)
		line.set_xdata(np.arange(len(data)))
		line.set_ydata(data)

		if len(data) > 300:	# scroll x-axis
			ax.set_xlim(len(data) - 300, len(data))
		
		plt.pause(0.00001)
except KeyboardInterrupt:
	print("key esc")
finally:
	print("done")
	ser.close()
	plt.ioff()	# shutdown interactive plot
