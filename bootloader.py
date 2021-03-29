import cmd
import crc_bruteforce
import can
from can import Message
import lz4.block
import math
from tqdm import tqdm
import struct
import time
import pigpio
import subprocess
from udsoncan.connections import IsoTPSocketConnection
import socket

TWISTER_PATH = (
    "../Simos18_SBOOT/twister"
)  # This is the path to the "twister" binary from https://github.com/bri3d/Simos18_SBOOT
SEED_START = (
    "1D00000"
)  # This is the starting value for the expected timer value range for the Seed/Key calculation.

sector_map_tc1791 = {  # Sector lengths for PMEM routines
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
    15: 0x40000,
}


def bits(byte):
    bit_arr = [
        (byte >> 7) & 1,
        (byte >> 6) & 1,
        (byte >> 5) & 1,
        (byte >> 4) & 1,
        (byte >> 3) & 1,
        (byte >> 2) & 1,
        (byte >> 1) & 1,
        (byte) & 1,
    ]
    bit_arr.reverse()
    return bit_arr


def print_success_failure(data):
    if data[0] is 0xA0:
        print("Success")
    else:
        print("Failure! " + data.hex())


def get_key_from_seed(seed_data):
    p = subprocess.run(
        [TWISTER_PATH, SEED_START, seed_data, "1"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    output_data = p.stdout.decode("us-ascii")
    return output_data


can_interface = "can0"
bus = can.interface.Bus(can_interface, bustype="socketcan")
pi = pigpio.pi()
pi.set_mode(23, pigpio.OUTPUT)
pi.set_pull_up_down(23, pigpio.PUD_UP)


def get_isotp_conn():
    conn = IsoTPSocketConnection(
        "can0", rxid=0x7E8, txid=0x7E0, params={"tx_padding": 0x55}
    )
    conn.tpsock.set_opts(txpad=0x55)
    conn.open()
    return conn


def sboot_pwm():
    import time
    import wavePWM

    GPIO = [12, 13]

    if not pi.connected:
        exit(0)

    pwm = wavePWM.PWM(pi)  # Use default frequency

    pwm.set_frequency(3210)
    cl = pwm.get_cycle_length()
    pwm.set_pulse_start_in_micros(13, cl / 1)
    pwm.set_pulse_length_in_micros(13, cl / 2)

    pwm.set_pulse_start_in_micros(12, 3 * cl / 4)
    pwm.set_pulse_length_in_micros(12, cl / 4)
    pwm.update()
    return pwm


def reset_ecu():
    pi.write(23, 0)
    time.sleep(0.01)
    pi.write(23, 1)


def sboot_getseed():
    conn = get_isotp_conn()
    print("Sending 0x30 to elevate SBOOT shell status...")
    conn.send(bytes([0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]))
    print_success_failure(conn.wait_frame())
    time.sleep(1)
    print("Sending 0x54 Generate Seed...")
    conn.send(bytes([0x54]))
    data = conn.wait_frame()
    print_success_failure(data)
    data = data[9:]
    conn.close()
    return data


def sboot_sendkey(key_data):
    conn = get_isotp_conn()
    send_data = bytearray([0x65])
    send_data.extend(key_data)
    print("Sending 0x65 Security Access with Key...")
    conn.send(send_data)
    print_success_failure(conn.wait_frame())
    conn.close()


def sboot_crc_reset(crc_start_address):
    prepare_upload_bsl()
    conn = get_isotp_conn()
    print("Setting initial CRC to 0x0...")
    send_data = bytes([0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
    conn.send(send_data)
    print_success_failure(conn.wait_frame())
    print("Setting expected CRC to 0x0...")
    send_data = bytes([0x78, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00])
    conn.send(send_data)
    print_success_failure(conn.wait_frame())
    print("Setting start CRC range count to 1...")
    send_data = bytes([0x78, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00])
    conn.send(send_data)
    print_success_failure(conn.wait_frame())
    print(
        "Setting start CRC start address to boot passwords at "
        + crc_start_address.hex()
        + "..."
    )
    send_data = bytearray([0x78, 0x00, 0x00, 0x00, 0x0C])
    send_data.extend(int.from_bytes(crc_start_address, "big").to_bytes(4, "little"))
    conn.send(send_data)
    print_success_failure(conn.wait_frame())
    print("Setting start CRC end address to a valid area at 0xb0010130...")
    send_data = bytes([0x78, 0x00, 0x00, 0x00, 0x10, 0x30, 0x01, 0x01, 0xB0])
    conn.send(send_data)
    print_success_failure(conn.wait_frame())
    print("Uploading valid part number for part correlation validator...")
    send_data = bytes(
        [
            0x78,
            0x00,
            0x00,
            0x00,
            0x14,
            0x4E,
            0x42,
            0x30,
            0xD1,
            0x00,
            0x00,
            0x53,
            0x43,
            0x38,
            0x34,
            0x30,
            0x2D,
            0x31,
            0x30,
            0x32,
            0x36,
            0x31,
            0x39,
            0x39,
            0x31,
            0x41,
            0x41,
            0x2D,
            0x2D,
            0x2D,
            0x2D,
            0x2D,
            0x2D,
        ]
    )
    conn.send(send_data)
    print_success_failure(conn.wait_frame())
    print("Starting Validator and rebooting into BSL...")
    conn.send(bytes([0x79]))
    time.sleep(0.0005)
    upload_bsl(True)
    crc_address = int.from_bytes(read_byte(0xD0010770 .to_bytes(4, "big")), "little")
    print("CRC Address Reached: ")
    print(hex(crc_address))
    crc_data = int.from_bytes(read_byte(0xD0010778 .to_bytes(4, "big")), "little")
    print("CRC32 Current Value: ")
    print(hex(crc_data))
    conn.close()
    return (crc_address, crc_data)


def sboot_shell():
    print("Setting up PWM waveforms...")
    pwm = sboot_pwm()
    time.sleep(1)
    print("Resetting ECU into Supplier Bootloader...")
    reset_ecu()
    bus.send(Message(data=[0x59, 0x45], arbitration_id=0x7E0, is_extended_id=False))
    print("Sending 59 45...")
    bus.send(Message(data=[0x6B], arbitration_id=0x7E0, is_extended_id=False))
    stage2 = False
    while True:
        if stage2 is True:
            bus.send(Message(data=[0x6B], arbitration_id=0x7E0, is_extended_id=False))
            print("Sending 6B...")
        message = bus.recv(0.01)
        print(message)
        if (
            message is not None
            and message.arbitration_id == 0x7E8
            and message.data[0] == 0xA0
        ):
            print("Got A0 message")
            if stage2:
                print("Switching to IsoTP Socket...")
                pwm.cancel()
                return sboot_getseed()
            print("Sending 6B...")
            stage2 = True
        if message is not None and message.arbitration_id == 0x0A7:
            print("FAILURE")
            pwm.cancel()
            return False


def sboot_login():
    sboot_seed = sboot_shell()
    print("Calculating key for seed: ")
    print(sboot_seed.hex())
    key = get_key_from_seed(sboot_seed.hex()[0:8])
    print("Key calculated : ")
    print(key)
    sboot_sendkey(bytearray.fromhex(key))


def extract_boot_passwords():
    addresses = map(
        lambda x: bytearray.fromhex(x), ["8001420C", "80014210", "80014214", "80014218"]
    )
    crcs = []
    for address in addresses:
        sboot_login()
        end_address, crc = sboot_crc_reset(address)
        print(address.hex() + " - " + hex(end_address) + " -> " + hex(crc))
        crcs.append(hex(crc))
    boot_passwords = crc_bruteforce.calculate_passwords(crcs)
    print(boot_passwords.hex())


# Enter REPL


def prepare_upload_bsl():
    # Pin 24 -> BOOT_CFG pin, pulled to GND to enable BSL mode.
    print("Resetting ECU into HWCFG BSL Mode...")
    pi.set_mode(24, pigpio.OUTPUT)
    pi.set_pull_up_down(24, pigpio.PUD_DOWN)
    pi.write(24, 0)


def upload_bsl(skip_prep=False):
    if skip_prep == False:
        prepare_upload_bsl()
    reset_ecu()
    time.sleep(0.1)
    pi.set_mode(24, pigpio.INPUT)
    pi.set_pull_up_down(24, pigpio.PUD_OFF)

    print("Sending BSL initialization message...")
    # send bootloader.bin to CAN BSL in Tricore
    bootloader_data = open("bootloader.bin", "rb").read()
    data = [
        0x55,
        0x55,
        0x00,
        0x01,
    ]  # 0x55 0x55 bit sync, 0x100 CAN ID for ACK (copied directly to MOAR register, so lower 2 bits are discarded, this will yield actual 0x40 CAN ID)
    data += struct.pack("<H", math.ceil(len(bootloader_data) / 8))
    data += [0x0, 0x3]  # 0x300 CAN ID for Data -> 0xC0 after right shift
    init_message = Message(
        is_extended_id=False, dlc=8, arbitration_id=0x100, data=data
    )  # 0x55 0x55 = magic for init, 0x00 0x1 = 0x100 CAN ID, 0x1 0x0 = 1 packet data, 0x00, 0x3 = 0x300 transfer data can id
    success = False
    bus.send(init_message)
    while success == False:
        message = bus.recv(0.5)
        if message is not None and not message.is_error_frame:
            if message.arbitration_id == 0x40:
                success = True
    print("Sending BSL data...")
    for block_base_address in tqdm(
        range(0, len(bootloader_data), 8), unit_scale=True, unit="blocks"
    ):
        block_end = min(len(bootloader_data), block_base_address + 8)
        message = Message(
            is_extended_id=False,
            dlc=8,
            arbitration_id=0xC0,
            data=bootloader_data[block_base_address:block_end],
        )
        bus.send(message, timeout=5)
        time.sleep(0.001)
    print("Device jumping into BSL... Draining receive queue...")
    while bus.recv(0.01) is not None:
        pass


def read_device_id():
    message = Message(
        is_extended_id=False,
        dlc=8,
        arbitration_id=0x300,
        data=[0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
    )
    bus.send(message)
    device_id = bytearray()
    message = bus.recv()
    if message.data[0] == 0x1:
        device_id += message.data[2:8]
    message = bus.recv()
    if message.data[0] == 0x1 and message.data[1] == 0x1:
        device_id += message.data[2:8]
    return device_id


def read_byte(byte_specifier):
    data = bytearray([0x02])
    data += byte_specifier
    data += bytearray([0x0, 0x0, 0x0])
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
    bus.send(message)
    byte_data = bytearray()
    message = bus.recv()
    if message.data[0] == 0x2:
        byte_data += message.data[1:5]
    return byte_data


def write_byte(addr, value):
    data = bytearray([0x03])
    data += addr
    data += bytearray([0x0, 0x0, 0x0])
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
    bus.send(message)
    byte_data = bytearray()
    message = bus.recv()
    if message.data[0] != 0x3:
        return False
    data = bytearray([0x03])
    data += value
    data += bytearray([0x0, 0x0, 0x0])
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
    bus.send(message)
    message = bus.recv()
    if message.data[0] != 0x3:
        return False
    else:
        return True


def send_passwords(pw1, pw2, ucb=0, read_write=0x8):
    data = bytearray([0x04])
    data += pw1
    data += bytearray([read_write, ucb, 0x0])
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
    bus.send(message)
    byte_data = bytearray()
    message = bus.recv()
    print(message)
    data = bytearray([0x04])
    data += pw2
    data += bytearray([0x0, 0x0, 0x0])
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
    bus.send(message)
    message = bus.recv()
    print(message)
    data = bytearray([0x04])
    data += pw1
    data += bytearray([read_write, ucb, 0x1])
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
    bus.send(message)
    byte_data = bytearray()
    message = bus.recv()
    print(message)
    data = bytearray([0x04])
    data += pw2
    data += bytearray([0x0, 0x0, 0x0])
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
    bus.send(message)
    message = bus.recv()
    print(message)


def erase_sector(address):
    data = bytearray([0x05])
    data += address
    data += bytearray([0, 0, 0])
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
    bus.send(message)
    message = bus.recv()


def print_enabled_disabled(string, value):
    enabled_or_disabled = "ENABLED" if value > 0 else "DISABLED"
    print(string + " " + enabled_or_disabled)


def print_sector_status(string, procon_sector_status):
    current_address = 0
    for sector_number in sector_map_tc1791:
        protection_status = procon_sector_status[sector_number]
        if sector_number > 9:
            protection_status = procon_sector_status[
                math.ceil(
                    sector_number - (sector_number % 2) - (sector_number - 10) / 2
                )
            ]
        if protection_status > 0:
            print(
                string
                + "Sector "
                + str(sector_number)
                + " "
                + hex(current_address)
                + ":"
                + hex((current_address + sector_map_tc1791[sector_number]))
                + " : "
                + "ENABLED"
            )

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
    print_enabled_disabled(
        pmem_string + " Read Protection Installation: ", flash_status[2]
    )
    print_enabled_disabled(pmem_string + " Read Protection Inhibit: ", flash_status[3])
    print_enabled_disabled(pmem_string + " Write Protection User 0: ", flash_status[5])
    print_enabled_disabled(pmem_string + " Write Protection User 1: ", flash_status[6])
    print_enabled_disabled(pmem_string + " OTP Installation: ", flash_status[7])

    flash_status_write = bits(fsr_value[3])
    print_enabled_disabled(
        pmem_string + " Write Protection User 0 Inhibit: ", flash_status_write[1]
    )
    print_enabled_disabled(
        pmem_string + " Write Protection User 1 Inhibit: ", flash_status_write[2]
    )

    protection_status = bits(fcon_value[2])
    print_enabled_disabled(pmem_string + " Read Protection: ", protection_status[0])
    print_enabled_disabled(
        pmem_string + " Disable Code Fetch from Flash Memory: ", protection_status[1]
    )
    print_enabled_disabled(
        pmem_string + " Disable Any Data Fetch from Flash: ", protection_status[2]
    )
    print_enabled_disabled(
        pmem_string + " Disable Data Fetch from DMA Controller: ", protection_status[4]
    )
    print_enabled_disabled(
        pmem_string + " Disable Data Fetch from PCP Controller: ", protection_status[5]
    )
    print_enabled_disabled(
        pmem_string + " Disable Data Fetch from SHE Controller: ", protection_status[6]
    )
    procon0_sector_status = bits(procon0_value[0]) + bits(procon0_value[1])
    print_sector_status(pmem_string + " USR0 Read Protection ", procon0_sector_status)
    procon1_sector_status = bits(procon1_value[0]) + bits(procon1_value[1])
    print_sector_status(pmem_string + " USR1 Write Protection ", procon1_sector_status)
    procon2_sector_status = bits(procon2_value[0]) + bits(procon2_value[1])
    print_sector_status(pmem_string + " USR2 OTP Protection ", procon2_sector_status)


def read_bytes_file(base_addr, size, filename):
    output_file = open(filename, "wb")
    for current_address in tqdm(
        range(base_addr, base_addr + size, 4), unit_scale=True, unit="block"
    ):
        bytes = read_byte(struct.pack(">I", current_address))
        output_file.write(bytes)
    output_file.close()


def read_compressed(address, size, filename):
    output_file = open(filename, "wb")
    data = bytearray([0x07])
    data += address
    data += size
    message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
    bus.send(message)
    total_size_remaining = int.from_bytes(size, "big")
    t = tqdm(total=total_size_remaining, unit="B")
    while total_size_remaining > 0:
        message = bus.recv()
        compressed_size = size_remaining = int.from_bytes(message.data[5:8], "big")
        # print("Waiting for compressed data of size: " + hex(size_remaining))
        data = bytearray()
        sequence = 1
        while size_remaining > 0:
            message = bus.recv()
            new_sequence = message.data[1]
            if sequence != new_sequence:
                print("Sequencing error! " + hex(new_sequence) + hex(sequence))
                t.close()
                output_file.close()
                return
            sequence += 1
            sequence = sequence & 0xFF
            data += message.data[2:8]
            size_remaining -= 6
        decompressed_data = lz4.block.decompress(data[:compressed_size], 4096)
        decompressed_size = len(decompressed_data)
        t.update(decompressed_size)
        total_size_remaining -= decompressed_size
        output_file.write(decompressed_data)
        data = bytearray([0x07, 0xAC])  # send an ACk packet
        message = Message(is_extended_id=False, dlc=8, arbitration_id=0x300, data=data)
        bus.send(message)
    output_file.close()
    t.close()


class BootloaderRepl(cmd.Cmd):
    intro = "Welcome to Tricore BSL. Type help or ? to list commands, you are likely looking for upload to start.\n"
    prompt = "(BSL) "
    file = None

    def do_upload(self, arg):
        "Upload BSL to device"
        upload_bsl()

    def do_deviceid(self, arg):
        "Read the Tricore Device ID from 0xD0000000 to 0xD000000C"
        device_id = read_device_id()
        if len(device_id) > 1:
            print(device_id.hex())
        else:
            print("Failed to retrieve Device ID")

    def do_readaddr(self, arg):
        "readaddr <addr> : Read 32 bits from an arbitrary address"
        byte_specifier = bytearray.fromhex(arg)
        byte = read_byte(byte_specifier)
        print(byte.hex())

    def do_writeaddr(self, arg):
        "writeaddr <addr> <data> : Write 32 bits to an arbitrary address"
        args = arg.split()
        byte_specifier = bytearray.fromhex(args[0])
        data_specifier = bytearray.fromhex(args[1])
        is_success = write_byte(byte_specifier, data_specifier)
        if is_success:
            print("Wrote " + args[1] + " to " + args[0])
        else:
            print("Failed to write value.")

    def do_flashinfo(self, arg):
        "Read flash information including PMEM protection status"
        PMU_BASE_ADDRS = {0: 0xF8001000, 1: 0xF8003000}

        for pmu_num in PMU_BASE_ADDRS:
            read_flash_properties(pmu_num, PMU_BASE_ADDRS[pmu_num])

    def do_dumpmaskrom(self, arg):
        "Dump the Tricore Mask ROM to maskrom.bin"
        read_bytes_file(0xAFFFC000, 0x4000, "maskrom.bin")

    def do_dumpmem(self, arg):
        "dumpmem <addr> <size> <filename>: Dump <addr> to <filename> with <size> bytes"
        args = arg.split()
        read_bytes_file(int(args[0], 16), int(args[1], 16), args[2])

    def do_sboot(self, arg):
        "Reset into SBOOT Command Shell, execute Seed/Key process"
        sboot_login()

    def do_sboot_sendkey(self, arg):
        "sboot_sendkey <keydata>: Send Key Data to SBOOT Command Shell"
        args = arg.split()
        key_data = bytearray.fromhex(args[0])
        sboot_sendkey(key_data)

    def do_sboot_crc_reset(self, arg):
        "sboot_crc_reset <address>: Configure SBOOT with CRC header pointed to <address>, reboot"
        args = arg.split()
        password_address = bytearray.fromhex(args[0])
        sboot_crc_reset(password_address)

    def do_send_read_passwords(self, arg):
        "send_read_passwords <pw1> <pw2>: unlock Flash using passwords"
        args = arg.split()
        pw1 = int.from_bytes(bytearray.fromhex(args[0]), "big").to_bytes(4, "little")
        pw2 = int.from_bytes(bytearray.fromhex(args[1]), "big").to_bytes(4, "little")
        send_passwords(pw1, pw2)

    def do_send_write_passwords(self, arg):
        "send_write_passwords <pw1> <pw2>: unlock Flash using passwords"
        args = arg.split()
        pw1 = int.from_bytes(bytearray.fromhex(args[0]), "big").to_bytes(4, "little")
        pw2 = int.from_bytes(bytearray.fromhex(args[1]), "big").to_bytes(4, "little")
        send_passwords(pw1, pw2, read_write=0x05, ucb=1)

    def do_erase_sector(self, arg):
        "erase_sector <addr> : Erase sector beginning with address"
        byte_specifier = bytearray.fromhex(arg)
        erase_sector(byte_specifier)

    def do_extract_boot_passwords(self, arg):
        "extract_boot_passwords : Extract Simos18 boot passwords using SBoot exploit chain. Requires 'crchack' in ../crchack and 'twister' in ../Simos18_SBOOT"
        extract_boot_passwords()

    def do_compressed_read(self, arg):
        "compressed_read <addr> <length> <filename>: read data using LZ4 compression (fast, hopefully)"
        args = arg.split()
        byte_specifier = bytearray.fromhex(args[0])
        length_specifier = bytearray.fromhex(args[1])
        filename = args[2]
        is_success = read_compressed(byte_specifier, length_specifier, filename)

    def do_reset():
        "reset: reset ECU"
        reset_ecu()

    def do_bye(self, arg):
        "Exit"
        return True


def parse(arg):
    "Convert a series of zero or more numbers to an argument tuple"
    return tuple(map(int, arg.split()))


BootloaderRepl().cmdloop()
