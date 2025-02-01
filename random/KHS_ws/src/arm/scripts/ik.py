import sys
import serial

sys.path.append('~/KHS_ws/src/rcb4/rcb4')
from rcb4.armh7interface import ARMH7Interface

interface = ARMH7Interface()

ser = serial.Serial('/dev/ttyUSB0', baudrate=115200, timeout=1)

interface.auto_open()
interface.search_servo_ids()
print(servo_ids)