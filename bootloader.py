import can
from can import Message
import math
from tqdm import tqdm
import struct
import time
bootloader_data = open("bootloader.bin", 'rb').read()
can_interface = 'can0'
bus = can.interface.Bus(can_interface, bustype='socketcan')
data = [0x55, 0x55, 0x00, 0x01] # 0x55 0x55 bit sync, 0x100 CAN ID for ACK (copied directly to MOAR register, so lower 2 bits are discarded, this will yield actual 0x40 CAN ID)
data += struct.pack("<H", math.ceil(len(bootloader_data) / 8))
data += [0x0, 0x3] # 0x300 CAN ID for Data -> 0xC0 after right shift
init_message = Message(is_extended_id=False, dlc=8, arbitration_id=0x100,data=data) # 0x55 0x55 = magic for init, 0x00 0x1 = 0x100 CAN ID, 0x1 0x0 = 1 packet data, 0x00, 0x3 = 0x300 transfer data can id
success = False
while(success == False):
    print(init_message)
    bus.send(init_message)
    message = bus.recv()
    print(message)
    if(not message.is_error_frame):
        if(message.arbitration_id == 0x40):
            success = True
for block_base_address in tqdm(range(0, len(bootloader_data), 8), unit_scale=True, unit="blocks"):
    block_end = min(len(bootloader_data), block_base_address+8)
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0xC0,data=bootloader_data[block_base_address:block_end])
    bus.send(message, timeout=5)
    time.sleep(.001)
while(True):
    message = bus.recv()
    print(message)
