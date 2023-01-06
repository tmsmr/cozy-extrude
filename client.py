import serial

ser = serial.Serial('/dev/ttyACM0', 115200)
ser.write(b'\x3A\x3A|')
res = ser.read_until(b'|')
print(len(res))
print(res)
