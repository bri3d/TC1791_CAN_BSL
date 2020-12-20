import cmd, sys
import can
from can import Message
import math
from tqdm import tqdm
import struct
import time

sector_map_tc1791 = { # Sector lengths for PMEM routines
    0: 0x4000,
    1: 0x4000,
    2: 0x4000,
    3: 0x4000,
    4: 0x4000,
    5: 0x4000,
    6: 0x4000,
    7: 0x4000,
    8: 0x20000,
    9: 0x40000,
    10: 0x40000,
    11: 0x40000,
    12: 0x40000,
    13: 0x40000,
    14: 0x40000,
    15: 0x40000
}

def bits(byte):
    bit_arr = [(byte >> 7) & 1,
            (byte >> 6) & 1,
            (byte >> 5) & 1,
            (byte >> 4) & 1,
            (byte >> 3) & 1,
            (byte >> 2) & 1,
            (byte >> 1) & 1,
            (byte) & 1]
    bit_arr.reverse()
    return bit_arr

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

def print_enabled_disabled(string, value):
    enabled_or_disabled = "ENABLED" if value > 0 else "DISABLED"
    print(string + " " + enabled_or_disabled)

def print_sector_status(string, procon_sector_status):
    current_address = 0
    for sector_number in sector_map_tc1791:
        protection_status = procon_sector_status[sector_number]
        if sector_number > 9:
            protection_status = procon_sector_status[math.ceil(sector_number - (sector_number % 2) - (sector_number - 10) / 2)]
        if protection_status > 0:                
            print(string + "Sector " + str(sector_number) + " " + hex(current_address) + ":" + hex((current_address+sector_map_tc1791[sector_number])) + " : " + "ENABLED")
            
        current_address += sector_map_tc1791[sector_number]

def read_flash_properties(flash_num, pmu_base_addr):
    FSR = 0x1010
    FCON = 0x1014
    PROCON0 = 0x1020
    PROCON1 = 0x1024
    PROCON2 = 0x1028
    fsr_value = read_byte(struct.pack(">I", pmu_base_addr + FSR))
    fcon_value = read_byte(struct.pack(">I", pmu_base_addr + FCON))
    procon0_value = read_byte(struct.pack(">I", pmu_base_addr + PROCON0))
    procon1_value = read_byte(struct.pack(">I", pmu_base_addr + PROCON1))
    procon2_value = read_byte(struct.pack(">I", pmu_base_addr + PROCON2))
    pmem_string = "PMEM" + str(flash_num)
    flash_status = bits(fsr_value[2])
    print_enabled_disabled(pmem_string + " Protection Installation: ", flash_status[0])
    print_enabled_disabled(pmem_string + " Read Protection Installation: ", flash_status[2])
    print_enabled_disabled(pmem_string + " Read Protection Inhibit: ", flash_status[3])
    print_enabled_disabled(pmem_string + " Write Protection User 0: ", flash_status[5])
    print_enabled_disabled(pmem_string + " Write Protection User 1: ", flash_status[6])
    print_enabled_disabled(pmem_string + " OTP Installation: ", flash_status[7])
    protection_status = bits(fcon_value[2])
    print_enabled_disabled(pmem_string + " Read Protection: ", protection_status[0])
    print_enabled_disabled(pmem_string + " Disable Code Fetch from Flash Memory: ", protection_status[1])
    print_enabled_disabled(pmem_string + " Disable Any Data Fetch from Flash: ", protection_status[2])
    print_enabled_disabled(pmem_string + " Disable Data Fetch from DMA Controller: ", protection_status[4])
    print_enabled_disabled(pmem_string + " Disable Data Fetch from PCP Controller: ", protection_status[5])
    print_enabled_disabled(pmem_string + " Disable Data Fetch from SHE Controller: ", protection_status[6])
    procon0_sector_status = bits(procon0_value[0]) + bits(procon0_value[1])
    print_sector_status(pmem_string + " USR0 Read Protection ", procon0_sector_status)
    procon1_sector_status = bits(procon1_value[0]) + bits(procon1_value[1])
    print_sector_status(pmem_string + " USR1 Write Protection ", procon1_sector_status)
    procon2_sector_status = bits(procon2_value[0]) + bits(procon2_value[1])
    print_sector_status(pmem_string + " USR2 OTP Protection ", procon2_sector_status)


class BootloaderRepl(cmd.Cmd):
    intro = 'Welcome to Tricore BSL. Type help or ? to list commands, you are likely looking for upload to start.\n'
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

    def do_flashinfo(self, arg):
        'Read flash information including PMEM protection status'
        PMU_BASE_ADDRS = {
            0:0xF8001000,
            1:0xF8003000
        }

        for pmu_num in PMU_BASE_ADDRS:
            read_flash_properties(pmu_num, PMU_BASE_ADDRS[pmu_num]) 

    def do_bye(self, arg):
        'Exit'
        return True

def parse(arg):
    'Convert a series of zero or more numbers to an argument tuple'
    return tuple(map(int, arg.split()))

BootloaderRepl().cmdloop()




