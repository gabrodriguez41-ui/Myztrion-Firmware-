#!/usr/bin/python3  
#-*- coding: utf-8 -*-

# --- Path tweak so this example can run from the repo without installing ---
import sys, pathlib
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[2] / 'myztrion' / 'python'))


import time

import Myztrion
rp = Myztrion.Myztrion()

# optimum settings to control a small servo (190 Hz) 
# typically the timing range is 0.8 .. 2.2 ms pulse width, but YMMV

rp.pwm_configure_pair(gpio=14, wrap_value=65535, clkdiv=50, clkdiv_int_frac=0)
#rp.pwm_set_value(gpio=14, value=65534) # minimum position
rp.pwm_set_value(gpio=14, value=0) # minimum position

gpio = 15
rp.pwm_configure_pair(gpio=gpio, wrap_value=65535, clkdiv=50, clkdiv_int_frac=0)
rp.pwm_set_value(gpio=gpio, value=100) # minimum position


