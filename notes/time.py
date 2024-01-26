#!/usr/bin/env python

import sys

def print_time(time):
    seconds = time * 2
    minutes = seconds / 60
    hours = seconds / 3600
    days = hours / 24
    print (f"d {days} h {hours} m {minutes} s {seconds}")

if __name__ == "__main__":
    time = sys.argv[1]
    if time[:2] == "0x":
        time = int(time, 16)
    else:
        time = int(time)
    print_time(time)
