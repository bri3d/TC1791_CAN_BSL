# TC1791_CAN_BSL
CAN Bootstrap Loader (BSL) for Tricore AudoMAX (TC1791 and friends)

# Background
By setting the HWCFG register on Tricore processors to a specific value, the Mask ROM / Boot ROM in the CPU will enter a serial-based or CAN-based Bootstrap Loader.

On AudoMAX, this Bootstrap Loader copies bytes to the beginning of Scratchpad RAM (C0000000) and jumps directly to execution from SPRAM.

Unfortunately, when the BSL is invoked, flash memory is locked by the Tricore user passwords. A mechanism for extracting these passwords exists for various ECUs, including Simos18, and a partial implementation is contained here. In tandem with the documentation and `twister` tool available at https://github.com/bri3d/Simos18_SBOOT , a complete "boot mode" / "bench mode" / "boot read passwords" open-source functionality is available for Simos18.

Furthermore, if the ECU is not locked by the Immobilizer and is functioning correctly, Simos18 boot passwords can be extracted using the "Write Without Erase" exploit documented [here](https://github.com/bri3d/VW_Flash/blob/master/docs.md) combined with a simple arbitrary read primitive attached to a CAN handler. The passwords are located at 0x8001420C in the OTP area of flash.

# Internal Pins and PCB modifications

Perform these PCB modifications to a Simos18 main board to enter the BSL. A few notes:

* The yellow goo isn't bodge wires or something, it's potting sealant from the other side of the PCB that's been pushed through the vias. If you find it ugly it is easy to remove.
* To open the ECU is very easy. It's held together with a few bent metal tabs and black RTV sealant. Simply bend the metal tabs away and cut through the black RTV sealant and the cover will lift right off. To reinstall is just the opposite.
* If you have a nicer setup with pogo pins and pullup / pulldown capability, you're in luck!
* If you already have boot passwords, simply using the BSL is not timing critical, you can just pull down the CFG attached to the blue/purple wire when you would like to boot. If you do not, and you need to recover the passwords, you need to attach this blue/purple CFG wire and the CPU RST pin to two Pi GPIO pins. The other CFG modifications can be performed statically.

Make these connections statically:

![PCB1](Board1.jpg)
![PCB3](Board3.jpg)

Connect the following wire or probe to Raspberry Pi GPIO 24 (see https://pinout.xyz ) - this is the HWCFG we use to select "BSL Mode" dynamically. If you are not extracting passwords and just want BSL access, you can just tie this to GND to always enter BSL Mode.

![PCB2](Board2.jpg)

(optional, needed for password extraction only): Connect the following probe to Raspberry Pi GPIO 23 - this is the CPU RST pin we use to perform a reset exploit to infer boot passwords.

![RST](RST.jpg)

# Harness Pins

Connect the following pins of the left-hand connector when looking at the opened ECU: 

* + ~13V: Left harness connector pins 6, 50, 86 (6 is one of the large pins, the rest are small).
* GROUND: 1 (this is the upper-right large pin).
* CANH: 79, to CAN interface CanH
* CANL: 80, to CAN interface CanL
* "PWM1": 66 (only necessary for SBOOT), to a 5V level shifter attached to GPIO 12.
* "PWM2": 71 (only necessary for SBOOT), to a 5V level shifter attached to GPIO 13.

# Extra Setup for Password Extraction

* Clone https://github.com/bri3d/Simos18_SBOOT into a sibling directory and compile `twister.c` 
* Clone https://github.com/resilar/crchack into a sibling directory and compile it per instructions.

# Use

* configure and bring up your SocketCAN interface
* start pigpio: `sudo pigpiod`
* `python3 bootloader.py`
* `help` to see commands

# Current tools:

* [bootloader.py](bootloader.py) : This tool uploads "bootloader.bin" into an ECU in Bootstrap Loader mode.
* [bootloader](Bootloader_2) : This directory contains a project intended for us with the HiTec Tricore Free Toolchain (GCC) wich will produce a bootstrap loader binary containing some basic command primitives. It uses the basic TriBoard TC1791 iRAM linker presets from HiTec, with the DRAM memory map adjusted to not clobber the boot-time device id stored at D0000000 to D000000C. CANBus primitives were generated using DaVe V2.
