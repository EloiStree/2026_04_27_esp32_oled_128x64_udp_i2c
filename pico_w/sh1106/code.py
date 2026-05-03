import wifi
import socketpool
import board
import busio
import displayio
import adafruit_displayio_sh1106

# ====================== WIFI ======================
ssid = "EloiStreeWifi2G"
password = ""

print("Connecting to WiFi...")
wifi.radio.connect(ssid, password)
pool = socketpool.SocketPool(wifi.radio)      # ← Must be defined BEFORE using it
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

# def get_x_y_lrtd(x, y)->bool:
#     index = y * WIDTH + x
#     byte_index = index >> 3
#     bit_index = index & 0x07
#     return (data[byte_index] >> bit_index) & 1

# def set_x_y_lrtd(x, y, value:bool):
#     global previous_data, new_data
#     index = y * WIDTH + x
#     byte_index = index >> 3
#     bit_index = index & 0x07
#     if value:
#         data[byte_index] |= (1 << bit_index)
#     else:
#         data[byte_index] &= ~(1 << bit_index)

# def set_1d_index_lrtd(index, value:bool):
#     global previous_data, new_data
#     byte_index = index >> 3
#     bit_index = index & 0x07
#     if value:
#         data[byte_index] |= (1 << bit_index)
#     else:
#         data[byte_index] &= ~(1 << bit_index)

# previous_data = bytearray(1024)
# new_data = bytearray(1024)
# def copy_current_to_previous():
#     global previous_data, new_data
#     for i in range(1024):
#         if data[i] != previous_data[i]:
#             new_data[i] = data[i]
#         previous_data[i] = data[i]


## KEEP DONT REMOVE CODE IN GODOT
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