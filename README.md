# TC1791_CAN_BSL
CAN Bootstrap Loader (BSL) for Tricore AudoMAX (TC1791 and friends)

# Background
By setting the HWCFG register on Tricore processors to a specific value, the Mask ROM / Boot ROM in the CPU will enter a serial-based or CAN-based Bootstrap Loader.

On AudoMAX, this Bootstrap Loader copies bytes to the beginning of Scratchpad RAM (C0000000) and jumps directly to execution from SPRAM.

Unfortunately, when the BSL is invoked, flash memory is locked by the Tricore user passwords. A mechanism for extracting these passwords exists for various ECUs, including Simos18, but the mechanism exists so far only in commercial tools.

Such extraction is outside of the scope of this BSL code, which only intends to provide unlock/read/write/erase and arbitrary code execution primitives once the passwords are known. If the ECU is not locked by the Immobilizer and is functioning correctly, Simos18 boot passwords can be extracted using the "Write Without Erase" exploit documented [here](https://github.com/bri3d/VW_Flash/blob/master/docs.md) combined with a simple arbitrary read primitive attached to a CAN handler. The passwords are located at 0x8001420C in the OTP area of flash.

# Documentation

Nice pictures of HWCFG/PORT0 test points to attach to coming soon :)

# Current tools:

[bootloader.py](bootloader.py) : This tool uploads "bootloader.bin" into an ECU in Bootstrap Loader mode.
[bootloader](bootloader) : COMING SOON - This directory contains a project intended for us with the HiTec Tricore Free Toolchain (GCC) wich will produce a bootstrap loader binary containing some basic command primitives. 
