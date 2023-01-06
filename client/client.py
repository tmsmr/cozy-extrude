import serial

ser = serial.Serial('/dev/ttyACM0', 115200)

# request temperature
ser.write(b'|\x1A\x00\x1A')

# wait for response
ser.read_until(b'|')
ser.read_until(b'\x1A')
payload_len = int.from_bytes(ser.read(), byteorder='big', signed=False)
payload = b''
for i in range(payload_len):
    payload += ser.read()
print(int.from_bytes(payload, byteorder='big', signed=True)/100.0)

#print(len(res))
#print(res)
