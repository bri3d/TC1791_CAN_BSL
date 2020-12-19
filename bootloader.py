import cmd, sys
import can
from can import Message
import math
from tqdm import tqdm
import struct
import time

can_interface = 'can0'
bus = can.interface.Bus(can_interface, bustype='socketcan')

# Enter REPL

def upload_bsl():
    print("Sending BSL initialization message...")
    # send bootloader.bin to CAN BSL in Tricore
    bootloader_data = open("bootloader.bin", 'rb').read()
    data = [0x55, 0x55, 0x00, 0x01] # 0x55 0x55 bit sync, 0x100 CAN ID for ACK (copied directly to MOAR register, so lower 2 bits are discarded, this will yield actual 0x40 CAN ID)
    data += struct.pack("<H", math.ceil(len(bootloader_data) / 8))
    data += [0x0, 0x3] # 0x300 CAN ID for Data -> 0xC0 after right shift
    init_message = Message(is_extended_id=False, dlc=8, arbitration_id=0x100,data=data) # 0x55 0x55 = magic for init, 0x00 0x1 = 0x100 CAN ID, 0x1 0x0 = 1 packet data, 0x00, 0x3 = 0x300 transfer data can id
    success = False
    while(success == False):
        bus.send(init_message)
        message = bus.recv(0.5)
        if(message is not None and not message.is_error_frame):
            if(message.arbitration_id == 0x40):
                success = True
            else:
                print("Got unexpected CAN response, please check CFG pins...")
    print("Sending BSL data...")
    for block_base_address in tqdm(range(0, len(bootloader_data), 8), unit_scale=True, unit="blocks"):
        block_end = min(len(bootloader_data), block_base_address+8)
        message = Message(is_extended_id=False, dlc=8, arbitration_id=0xC0,data=bootloader_data[block_base_address:block_end])
        bus.send(message, timeout=5)
        time.sleep(.001)
    print("Device jumping into BSL...")

def read_device_id():
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300,data=[0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    bus.send(message)
    device_id = bytearray()
    message = bus.recv()
    if(message.data[0] == 0x1):
        device_id += message.data[2:8]
    message = bus.recv()
    if(message.data[0] == 0x1 and message.data[1] == 0x1):
        device_id += message.data[2:8]
    return device_id

def read_byte(byte_specifier):
    data = bytearray([0x02])
    data += byte_specifier
    data += bytearray([0x0,0x0,0x0])
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300,data=data)
    bus.send(message)
    byte_data = bytearray()
    message = bus.recv()
    if(message.data[0] == 0x2):
        byte_data += message.data[1:5]
    return byte_data

class BootloaderRepl(cmd.Cmd):
    intro = 'Welcome to Tricore BSL.   Type help or ? to list commands, you are likely looking for upload to start.\n'
    prompt = '(BSL) '
    file = None

    def do_upload(self, arg):
        'Upload BSL to device'
        upload_bsl()

    def do_deviceid(self, arg):
        'Read the Tricore Device ID from 0xD0000000 to 0xD000000C'
        device_id = read_device_id()
        if(len(device_id) > 1):
            print(device_id.hex())
        else:
            print("Failed to retrieve Device ID")
            
    def do_readaddr(self, arg):
        'Read 32 bits from an arbitrary address'
        byte_specifier = bytearray.fromhex(arg)
        byte = read_byte(byte_specifier)
        print(byte.hex())

    def do_bye(self, arg):
        'Exit'
        return True

def parse(arg):
    'Convert a series of zero or more numbers to an argument tuple'
    return tuple(map(int, arg.split()))

BootloaderRepl().cmdloop()




