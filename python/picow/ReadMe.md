# Circuit python   

I accidently bought SH1106 instead of SSH1306 because it was cheaper.     
SH1106 https://www.amazon.com.be/dp/B07V9SLQ6W      
SH1106 https://docs.circuitpython.org/projects/displayio_sh1106/en/latest/     

The following code is not optimised at all.
It is my first draft of sending from Godot UDP state of the SSH1306 state to a real SH1106 linked to a Pico W.   
Frame rate is very low. In this version.   


``` python

import wifi
import socketpool
import board
import busio
import displayio
import adafruit_displayio_sh1106

# ====================== WIFI ======================
ssid = "YOUR WIFI NAME"
password = "YOUR PASSWORD"

print("Connecting to WiFi...")
wifi.radio.connect(ssid, password)
pool = socketpool.SocketPool(wifi.radio)   
print("Connected:", wifi.radio.ipv4_address)

# ====================== DISPLAY ======================
displayio.release_displays()

i2c = busio.I2C(board.GP5, board.GP4, frequency=800_000)
display_bus = displayio.I2CDisplay(i2c, device_address=0x3C)

WIDTH = 128
HEIGHT = 64

display = adafruit_displayio_sh1106.SH1106(
    display_bus,
    width=WIDTH,
    height=HEIGHT,
    colstart=2,
    rotation=0
)

# Bitmap
bitmap = displayio.Bitmap(WIDTH, HEIGHT, 2)
palette = displayio.Palette(2)
palette[0] = 0x000000
palette[1] = 0xFFFFFF

tile_grid = displayio.TileGrid(bitmap, pixel_shader=palette)
group = displayio.Group()
group.append(tile_grid)
display.root_group = group

print("Display ready")

# ====================== UDP ======================
PORT = 3615
sock = pool.socket(pool.AF_INET, pool.SOCK_DGRAM)
sock.bind(("0.0.0.0", PORT))
sock.settimeout(0.01)
bitmap.fill(1)

print(f"Listening for 128x64 frames on UDP port {PORT}...")

data_current = bytearray(1024)
data_previous = bytearray(1024)


## KEEP DONT REMOVE CODE IN GODOT to send.
# func set_next_push_byte_as_array_bool(array:Array[bool]) -> void:
# 	next_byte_array = array
# 	next_byte_array_packed = PackedByteArray()
# 	for i in range(0, next_byte_array.size(), 8):
# 		var byte_value: int = 0
# 		for j in range(8):
# 			if i + j < next_byte_array.size() and next_byte_array[i + j]:
# 				byte_value |= (1 << (7 - j))
# 		next_byte_array_packed.append(byte_value)
## KEEP DONT REMOVE CODE IN GODOT

while True:
    try:
        nbytes, addr = sock.recvfrom_into(data_current)
    except OSError:
        continue
    
    if nbytes == 1024:
        
        for byte_index in range(1024):
            for bit_index in range(8):
                i = (byte_index << 3) | bit_index
                row = i // WIDTH
                col = i % WIDTH
                # has_bit_changed = (data_current[byte_index] & (1 << (7 - bit_index))) != (data_previous[byte_index] & (1 << (7 - bit_index)))   
                # if has_bit_changed:
                #     bitmap[col, row] = (data_current[byte_index] >> (7 - bit_index)) & 1
                    
                bitmap[col, row] = (data_current[byte_index] >> (7 - bit_index)) & 1

        for i in range(1024):
            data_previous[i] = data_current[i]

    elif nbytes > 0:
        print("Bad packet size:", nbytes)


```


``` gdscript

class_name SSD1306UdpSendToSingleTarget
extends Node

@export var target_ip: String = "192.168.178.122"
@export var target_port: int = 3615
@export var use_timer_send: bool = true
@export var send_interval_ms: int = 500

var next_byte_array: Array[bool] = []
var next_byte_array_packed: PackedByteArray = PackedByteArray()

func _ready() -> void:
	setup_udp()
	if use_timer_send:
		var timer: Timer = Timer.new()
		timer.wait_time = send_interval_ms / 1000.0
		timer.connect("timeout", Callable(self, "send_byte_in_memory"))
		add_child(timer)
		timer.start()

func set_next_push_byte_as_array_bool(array:Array[bool]) -> void:
	next_byte_array = array
	next_byte_array_packed = PackedByteArray()
	for i in range(0, next_byte_array.size(), 8):
		var byte_value: int = 0
		for j in range(8):
			if i + j < next_byte_array.size() and next_byte_array[i + j]:
				byte_value |= (1 << (7 - j))
		next_byte_array_packed.append(byte_value)

var udp_packet := PacketPeerUDP.new()

func setup_udp():
	udp_packet.set_dest_address(target_ip, target_port)

func send_byte_in_memory():
	send_byte_in_memory_pack(next_byte_array_packed)

func send_byte_in_memory_pack(pack: PackedByteArray) -> void:
	udp_packet.put_packet(pack)
	#print(pack.size())

```
