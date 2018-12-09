#!/bin/bash
sox -t raw -c 1 -e signed-integer -b 16 -r 8000 output.pcm output.wav
