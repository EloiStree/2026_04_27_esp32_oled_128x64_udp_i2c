# Send a udp array of 1024 bytes to the ssd1306 display
# ip is 192.168.178.122


import socket
import time
#UDP_IP = "192.168.178.122"
UDP_IP = "192.168.137.176"
#UDP_IP = "127.0.0.1"
UDP_PORT = 3615

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP   


full_bytes_true = bytearray(1024)
full_bytes_false = bytearray(1024)
full_bytes_half_true = bytearray(1024)

for i in range(1024):
    full_bytes_true[i] = 255
    full_bytes_false[i] = 0

for i in range(1024):
    if i % 2 == 0:
        full_bytes_half_true[i] = 255
    else:
        full_bytes_half_true[i] = 0

def send_random_bytes():
    data = bytearray(1024)
    for i in range(1024):
        data[i] = i % 256   
    send_bytes(data)

def send_bytes(data):
    sock.sendto(data, (UDP_IP, UDP_PORT))


array = bytearray(1024)


while True:    
    # send_bytes(full_bytes_true)
    # time.sleep(1)
    # send_bytes(full_bytes_false)
    # time.sleep(1)
    # send_bytes(full_bytes_half_true)
    # time.sleep(1)
    # send_random_bytes()
    # time.sleep(1)


    for i in range(1024):
        for b in range(8):
            array[i] |= (1 << b)
            # if (i * 8 + b) % 2 == 0:
            #     array[i] |= (1 << b)
            # else:
            #     array[i] &= ~(1 << b)
            send_bytes(array)
            time.sleep(0.01)

