#!/usr/bin/env python

import sys

eventPtr = int(sys.argv[1])

normalEventsStart = 0x6e2
bit = eventPtr & 0xf
byte = (0xfffe & (eventPtr >> 3)) + normalEventsStart

print(f'Event: {eventPtr} byte: {byte:x} bit: {bit}')

