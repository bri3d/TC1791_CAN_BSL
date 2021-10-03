# TC1791_CAN_BSL
CAN Bootstrap Loader (BSL) for Tricore AudoMAX (TC1791 and friends)

# Background
By setting the HWCFG register on Tricore processors to a specific value, the Mask ROM / Boot ROM in the CPU will enter a serial-based or CAN-based Bootstrap Loader.

On AudoMAX, this Bootstrap Loader copies bytes to the beginning of Scratchpad RAM (C0000000) and jumps directly to execution from SPRAM.

Unfortunately, when the BSL is invoked, flash memory is locked by the Tricore user passwords. A mechanism for extracting these passwords exists for various ECUs, including Simos18, and a partial implementation is contained here. In tandem with the documentation and `twister` tool available at https://github.com/bri3d/Simos18_SBOOT , a complete "boot mode" / "bench mode" / "boot read passwords" open-source functionality is available for Simos18.

Furthermore, if the ECU is not locked by the Immobilizer and is functioning correctly, Simos18 boot passwords can be extracted using the "Write Without Erase" exploit documented [here](https://github.com/bri3d/VW_Flash/blob/master/docs.md) combined with a simple arbitrary read primitive attached to a CAN handler. The passwords are located at 0x8001420C in the OTP area of flash.

# Internal Pins and PCB modifications


* The yellow goo isn't bodge wires or something, it's potting sealant from the other side of the PCB that's been pushed through the vias. If you find it ugly it is easy to remove.
* To open the ECU is very easy. It's held together with a few bent metal tabs and black RTV sealant. Simply bend the metal tabs away and cut through the black RTV sealant and the cover will lift right off. To reinstall is just the opposite.
* If you have a nicer setup with pogo pins and pullup / pulldown capability, you're in luck!
* If you already have boot passwords, simply using the BSL is not timing critical, you can just pull down the CFG attached to the blue/purple wire when you would like to boot. If you do not, and you need to recover the passwords, you need to attach this blue/purple CFG wire and the CPU RST pin to two Pi GPIO pins. The other CFG modifications can be performed statically.

# Recommend bill of materials:

* All of these parts are common electronics parts. You may have most, or even all of them already. Starting from scratch, I estimate this setup could be constructed for around $100.

* 12-14V bench power supply. I usually supply 13.6V.
* Raspberry Pi 2, 3, or 4 (3B preferred).
* Seeeed Studios 2517/2518FD CAN Hat. 2515 CAN hats are not capable of reliably completing a read process, but may be useful for brick recovery.
* 20+ various lengths of wire. A breadboard wire kit will be enough.
* A resistor - 1K preferred, higher values may work as well.
* High speed (3.2khz capable) 3.3V to 5V level shifters. 2 level shifters are required - most cheap "I2C converter" boards on Amazon, eBay, Aliexpress will work. 
* Soldering equipment OR a "BDM frame" and probes. These test points and vias are very easy to solder with a steady hand, but if you do not wish to solder, a "BDM frame" and probes as found on Aliexpress or Amazon will also work.
* A mechanism for connecting to pins on the ECU connector. Even alligator clips will work in a pinch. I have had good luck with simple crimp-on female pin connectors (JST or the like) wrapped in heat shrink. Or, a "pigtail" style take-off ECU harness, often sold as "4H0906971A," which will need to be re-pinned as the two PWM connections are not usually connected at all.


# Hardware Setup

Make these connections:

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

![Connector Pinout](pinout_connector.jpg)

# Setup for Password Extraction

* Clone https://github.com/bri3d/Simos18_SBOOT into a sibling directory and compile `twister.c` 
* Clone https://github.com/resilar/crchack into a sibling directory and compile it per instructions.

# "Bench reading" a Simos18 ECU:

* Perform the above steps to configure the ECU for BSL mode. Ensure you have `../Simos18_SBOOT/twister` and `../crchack/crchack` compiled.
* In total, you should have seven connections to the ECU mainboard, via probes or solder - a jumper between two test points, two points connected via resistor to a third point (3V3), and two points connected directly to GPIO 23 and 24 on the Pi. 
* You should also have eight connections to the ECU connector: GPIO 12 should be attached to a 5V level shifter, then to pin 66 on the ECU connector. GPIO 13 should be attached to another 5V level shifter, then to pin 71 on the ECU connector. CanH to pin 79 and CanL to pin 80. Then, 3 power connections and 1 ground connection. 
* Ensure `pigpiod` is running: `sudo pigpiod`
* Ensure `can0` is up at bitrate `500000` and with the `txqueuelen` increased: `sudo ip link set can0 up type can bitrate 500000 && sudo ifconfig can0 txqueuelen 65536`
* Start `python3 bootloader.py` and run the following commands:

```
$ python3 bootloader.py 
Welcome to Tricore BSL. Type help or ? to list commands, you are likely looking for upload to start.

(BSL) extract_boot_passwords
[... , device will start 4 times to find 4 CRC values, should take ~2 minutes]
CRC32 Current Value: 
0xf427254f
80014218 - 0x80014318 -> 0xf427254f
abf425508513c27314e31d3542b92b1b # These are the boot passwords. The first 8 bytes are the Read passwords and the second 8 bytes are the Write passwords.
(BSL) send_read_passwords abf42550 8513c273 # <<< These passwords are the first 8 bytes from the previous line. 
(BSL) compressed_read AF000000 18000 PMU0_DFlash.bin
(BSL) compressed_read AF080000 18000 PMU1_Dflash.bin
(BSL) compressed_read 80000000 200000 PMU0_PFlash.bin
(BSL) compressed_read 80800000 100000 PMU1_PFlash.bin
```

Concatenating the PMU0 and PMU1 files will produce the "bench read" format preferred by some commercial tools. Unfortunately they aren't standard so if you are trying to use a commercial toolchain you may have to experiment with how to make the data line up. 

# Recovering a bricked Simos18 ECU:

* Since compressed writes are not yet implemented in the BSL, the easiest way to do this is simply to erase the first segment of Calibration, which will force the ECU into CBOOT.

```
$ python3 bootloader.py 
Welcome to Tricore BSL. Type help or ? to list commands, you are likely looking for upload to start.

(BSL) extract_boot_passwords
[... , device will start 4 times to find 4 CRC values, should take ~2 minutes]
CRC32 Current Value: 
0xf427254f
80014218 - 0x80014318 -> 0xf427254f
abf425508513c27314e31d3542b92b1b # These are the boot passwords. The first 8 bytes are the Read passwords and the second 8 bytes are the Write passwords.
(BSL) send_read_passwords abf42550 8513c273 # <<< These passwords are the first 8 bytes from the previous line. 
(BSL) send_write_passwords 14e31d35 42b92b1b
(BSL) erase_sector 80800000
(BSL) reset
```

Now use `VW_Flash` to reflash the ECU from CBOOT with whatever software you wanted. 

# Current tools:

* [bootloader.py](bootloader.py) : This tool uploads "bootloader.bin" into an ECU in Bootstrap Loader mode.
* [bootloader](Bootloader_2) : This directory contains a project intended for us with the HiTec Tricore Free Toolchain (GCC) wich will produce a bootstrap loader binary containing some basic command primitives. It uses the basic TriBoard TC1791 iRAM linker presets from HiTec, with the DRAM memory map adjusted to not clobber the boot-time device id stored at D0000000 to D000000C. CANBus primitives were generated using DaVe V2.
