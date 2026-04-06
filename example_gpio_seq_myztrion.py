#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
GPIO output sequence example for Myztrion.

Install:
    pip install myztrion

Run:
    python example_gpio_seq_myztrion.py
"""

import time
from myztrion.core import Myztrion  # <- paquete instalado desde PyPI

rp = Myztrion()

# The microsecond delay between bit pattern stages
delay_us = 1

while True:
    print(
        #         2. ...2 ...1 ...1 .1
        #         5. ...0 ...6 ...2 .098_7654_3210   = GPIO numbering
        rp.gpio_out_seq(
            0b0000_0010_0000_0000_0000_0000_0000_0011,      # bit mask
            0b0000_0010_0000_0000_0000_0000_0000_0011, 10,  #  0
            0b0000_0000_0000_0000_0000_0000_0000_0010, delay_us,  #  1
            0b0000_0000_0000_0000_0000_0000_0000_0001, delay_us,  #  2
            # ... (stages 3..15 opcionales)
        )
    )

