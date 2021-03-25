//****************************************************************************
// @Project Includes
//****************************************************************************

#include "MAIN.h"
#include "lz4.h"

//****************************************************************************
// @Defines
//****************************************************************************

#define RESET_INDICATOR     ((SCU_RSTSTAT.reg & ((uword)(0x0001001B))))
#define WATCHDOG_RESET      ((uword)0x00000008)
#define SOFTWARE_RESET      ((uword)0x00000010)
#define ESR0_RESET          ((uword)0x00000001)
#define ESR1_RESET          ((uword)0x00000002)
#define POWERON_RESET       ((uword)0x00010000)

//****************************************************************************
// @Global Variables
//****************************************************************************
volatile unsigned int DummyToForceRead;

//****************************************************************************
// @Function      void MAIN_vInit(void) 
//
//----------------------------------------------------------------------------
// @Description   This function initializes the microcontroller.
//
//----------------------------------------------------------------------------
// @Returnvalue   None
//
//----------------------------------------------------------------------------
// @Parameters    None
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

void MAIN_vInit(void) {
	///  -----------------------------------------------------------------------
	///  Clock System:
	///  -----------------------------------------------------------------------
	///  - external clock frequency: 20.00 MHz
	///  - input divider (PDIV): 2
	///  - PLL operation (VCOBYP = 0)
	///  - VCO range: 400 MHz - 500 MHz
	///  - feedback divider (NDIV): 40
	///  - the VCO output frequency is: 400.00 MHz
	///  - output divider (KDIV): 2
	///  - CPU clock: 200.00 MHz
	///  - the ratio fcpu /ffpi is  2 / 1
	///  - the ratio fcpu /fsri is  1 / 1
	///  - the ratio fcpu /fpcp is  1 / 1
	///  - system clock: 100.00 MHz

	/// Comparing with the Compiler settings
	if (((SCU_PLLCON0.reg & 0X1004E00) != 0X1004E00)
			|| ((SCU_PLLCON1.reg & 0X20001) != 0X20001)
			|| ((SCU_CCUCON0.reg & 0X1) != 0X1)) {

		//// - after a software reset PLL refuse to lock again unless oscillator is
		////   disconnected first

		MAIN_vResetENDINIT()
;
		SCU_PLLCON0.bits.VCOBYP = 0;  // reset VCO bypass
		SCU_PLLCON0.bits.SETFINDIS = 1; // disconnect OSC from PLL
		MAIN_vSetENDINIT();

		if (!SCU_PLLSTAT.bits.PWDSTAT) {
			MAIN_vResetENDINIT()
;
			SCU_CCUCON0.reg = 0x00000001; // set FPI,SRI and PCP dividers
			SCU_PLLCON0.bits.VCOBYP = 1; // set VCO bypass (goto Prescaler Mode)
			while (!SCU_PLLSTAT.bits.VCOBYST)
				; // wait for prescaler mode
			SCU_PLLCON0.reg = 0x01054E21; // set P,N divider, connect OSC
			SCU_PLLCON1.reg = 0x00020001; // set K1,K2 divider
			MAIN_vSetENDINIT();
			while (SCU_PLLSTAT.bits.VCOLOCK == 0)
				; // wait for LOCK
			MAIN_vResetENDINIT()
;
			SCU_PLLCON0.bits.VCOBYP = 0; // Reset VCO bypass (Leave Prescaler Mode)
			MAIN_vSetENDINIT();
		}
	}

	MAIN_vResetENDINIT()
;
	WDT_CON1.reg = 0X8; // Disable watchdog timer
	MAIN_vSetENDINIT();

	///  -----------------------------------------------------------------------
	///  Interrupt System:
	///  -----------------------------------------------------------------------
	///  - four arbitration cycles (max. 255 interrupt sources)
	///  - two clocks per arbitration cycle

	MTCR(0xFE2C, 0x00000000);      // load CPU interrupt control register
	ISYNC();

	///  -----------------------------------------------------------------------
	///  Peripheral Control Processor (PCP):
	///  -----------------------------------------------------------------------
	///  - stop the PCP internal clock when PCP is idle

	///  - use Full Context save area (R[0] - R[7])
	///  - start progam counter as left by last invocation
	///  - channel watchdog is disabled
	///  - maximum channel number checking is disabled

	MAIN_vResetENDINIT()
;
	PCP_CLC.reg = 0x00000000;   // load PCP clock control register
	PCP_CS.reg = 0x00000200;   // load PCP control and status register
	MAIN_vSetENDINIT();

	///  - four arbitration cycles (max. 255 PCP channels)
	///  - two clocks per arbitration cycle
	PCP_ICR.reg = 0x00000000;   // load PCP interrupt control register

	///  - the PCP warning mechanism is disabled
	PCP_ITR.reg = 0x00000000;   // load PCP interrupt threshold register

	///  - type of service of PCP node 4 is CPU interrupt
	PCP_SRC4.reg = 0x00001000;   // load service request control register 4

	///  - type of service of PCP node 5 is CPU interrupt
	PCP_SRC5.reg = 0x00001000;   // load service request control register 5

	///  - type of service of PCP node 6 is CPU interrupt
	PCP_SRC6.reg = 0x00001000;   // load service request control register 6

	///  - type of service of PCP node 7 is CPU interrupt
	PCP_SRC7.reg = 0x00001000;   // load service request control register 7

	///  - type of service of PCP node 8 is CPU interrupt
	PCP_SRC8.reg = 0x00001000;   // load service request control register 8

	///  -----------------------------------------------------------------------
	///  Configuration of the DMA Module Clock:
	///  -----------------------------------------------------------------------
	///  - enable the DMA module

	MAIN_vResetENDINIT()
;
	DMA_CLC.reg = 0x00000008;   // DMA clock control register
	DummyToForceRead = DMA_CLC.reg; // read it back to ensure it is read
	MAIN_vSetENDINIT();

	//   -----------------------------------------------------------------------
	//   Initialization of the Peripherals:
	//   -----------------------------------------------------------------------
	//   initializes the MultiCAN Controller
	CAN_vInit();

	///  -----------------------------------------------------------------------
	///  System Start Conditions:
	///  -----------------------------------------------------------------------

	//// - the CPU interrupt system is globally disabled
	DISABLE();
}

//****************************************************************************
// @Function      void MAIN_vWriteWDTCON0(uword uwValue) 
//
//----------------------------------------------------------------------------
// @Description   This function writes the parameter uwValue to the WDT_CON0 
//                register which is password protected. 
//
//----------------------------------------------------------------------------
// @Returnvalue   None
//
//----------------------------------------------------------------------------
// @Parameters    uwValue: 
//                Value for the WDTCON0 register
//

void MAIN_vWriteWDTCON0(uword uwValue) {
	uword uwDummy;

	uwDummy = WDT_CON0.reg;
	uwDummy |= 0x000000F0;       //  set HWPW1 = 1111b

	if (WDT_CON1.bits.DR) {
		uwDummy |= 0x00000008;     //  set HWPW0 = WDTDR
	} else {
		uwDummy &= ~0x00000008;     //  set HWPW0 = WDTDR
	}
	if (WDT_CON1.bits.IR) {
		uwDummy |= 0x00000004;     //  set HWPW0 = WDTIR
	} else {
		uwDummy &= ~0x00000004;     //  set HWPW0 = WDTIR
	}

	uwDummy &= ~0x00000002;       //  set WDTLCK = 0
	WDT_CON0.reg = uwDummy;          //  unlock access

	uwValue |= 0x000000F2;      //  set HWPW1 = 1111b and WDTLCK = 1
	uwValue &= ~0x0000000C;      //  set HWPW0 = 00b
	WDT_CON0.reg = uwValue;         //  write access and lock

}

struct bootloader_state boot;
struct password_cmd_state passwordCmdState;
struct write_page_state writePageState;
struct compress_read_state compressReadState;

void MAIN_sendCan(ubyte canData[]) {
	while (CAN_ubRequestMsgObj(1) != 1)
		;
	CAN_vLoadData(1, canData);
	CAN_vTransmit(1);
}

/*
 * 0x1: readDevice ID
 * Takes no arguments
 * Returns first 12 bytes of RAM, populated with Device ID from Configuration Sector by the BootROM.
 */
void MAIN_CMD_readDeviceId() {
	ubyte i;
	ubyte canData[8];
	ubyte *device_id = (ubyte *) 0xD0000000;
	canData[0] = 0x1;
	canData[1] = 0x0;
	for (i = 0; i < 6; i++) {
		canData[i + 2] = device_id[i];
	}
	MAIN_sendCan(canData);
	canData[1] = 0x1;
	for (i = 0; i < 6; i++) {
		canData[i + 2] = device_id[i + 6];
	}
	MAIN_sendCan(canData);
}

/*
 * 0x2: Read32.
 * Uses bytes 1-4 to set the address to read from.
 * Returns the value from performing a direct Tricore bus read to the read address. This can be RAM, a register, or an address in Flash if it is unlocked and in read mode.
 */
void MAIN_CMD_read32(CAN_SWObj *cur_msg) {
	uword address = (cur_msg->ubData[1] << 24) | (cur_msg->ubData[2] << 16)
			| (cur_msg->ubData[3] << 8) | cur_msg->ubData[4];
	uword address_value = *(uword *) address;
	ubyte canData[8];
	canData[0] = 0x2;
	canData[4] = (address_value >> 24) & 0xFF;
	canData[3] = (address_value >> 16) & 0xFF;
	canData[2] = (address_value >> 8) & 0xFF;
	canData[1] = address_value & 0xFF;
	canData[5] = 0xFF;
	canData[6] = 0xFF;
	canData[7] = 0xFF;
	MAIN_sendCan(canData);
}

/*
 * 0x3 : write32SetAddress
 * Uses bytes 1-4 to set the address pointer for a Write32 command.
 * Expects a subsequent 0x4 command with the data.
 * Places the read loop in a wait state to wait for the subsequent 0x4 command.
 */
void MAIN_CMD_write32SetAddress(CAN_SWObj *cur_msg) {
	uword address = (cur_msg->ubData[1] << 24) | (cur_msg->ubData[2] << 16)
			| (cur_msg->ubData[3] << 8) | cur_msg->ubData[4];
	ubyte canData[8];
	canData[0] = 0x3;
	canData[4] = (address >> 24) & 0xFF;
	canData[3] = (address >> 16) & 0xFF;
	canData[2] = (address >> 8) & 0xFF;
	canData[1] = address & 0xFF;
	canData[5] = 0xFF;
	canData[6] = 0xFF;
	canData[7] = 0xFF;
	MAIN_sendCan(canData);
	boot.address = address;
}
/*
 * 0x3 : write32SetData
 * Uses bytes 1-4 to set the data value for a Write32 command, and performs the write
 * Only called when read loop was already in a wait state to wait for the subsequent 0x4 command after write32SetAddress
 */
void MAIN_CMD_write32SetData(CAN_SWObj *cur_msg) {
	uword data = (cur_msg->ubData[1] << 24) | (cur_msg->ubData[2] << 16)
			| (cur_msg->ubData[3] << 8) | cur_msg->ubData[4];
	*(uword *) boot.address = data;
	ubyte canData[8];
	canData[0] = 0x3;
	canData[4] = (data >> 24) & 0xFF;
	canData[3] = (data >> 16) & 0xFF;
	canData[2] = (data >> 8) & 0xFF;
	canData[1] = data & 0xFF;
	canData[5] = 0xFF;
	canData[6] = 0xFF;
	canData[7] = 0xFF;
	MAIN_sendCan(canData);
}

/*
 * 0x4: unlockPasswordSetFirstPassword
 * Uses bytes 1-4 to set the first flash password value.
 * Uses byte 5 to set the "read" or "write" mode to be unlocked.
 * Uses byte 6 to set which UCB block the passwords are for.
 * Uses byte 7 to specify which flash controller to use.
 * Places the read loop in a wait state to wait for the next 0x5 command containing the second password.
 */
void MAIN_CMD_unlockPasswordSetFirstPassword(CAN_SWObj *cur_msg) {
	uword firstPassword = (cur_msg->ubData[1] << 24)
			| (cur_msg->ubData[2] << 16) | (cur_msg->ubData[3] << 8)
			| cur_msg->ubData[4];
	ubyte readOrWrite = cur_msg->ubData[5];
	ubyte whichUCB = cur_msg->ubData[6];
	ubyte whichController = cur_msg->ubData[7];
	ubyte canData[8];
	canData[0] = 0x4;
	canData[4] = (firstPassword >> 24) & 0xFF;
	canData[3] = (firstPassword >> 16) & 0xFF;
	canData[2] = (firstPassword >> 8) & 0xFF;
	canData[1] = firstPassword & 0xFF;
	canData[5] = 0xFF;
	canData[6] = 0xFF;
	canData[7] = 0xFF;
	MAIN_sendCan(canData);
	passwordCmdState.whichUCB = whichUCB;
	passwordCmdState.readOrWrite = readOrWrite;
	passwordCmdState.firstPassword = firstPassword;
	passwordCmdState.whichController = whichController;
}

/*
 * 0x4: unlockPassword
 * Uses bytes 1-4 to set the second flash password value.
 * Performs the unlock procedure.
 * Returns the flash status register content.
 */
void MAIN_CMD_unlockPassword(CAN_SWObj *cur_msg) {
	uword secondPassword = (cur_msg->ubData[1] << 24)
			| (cur_msg->ubData[2] << 16) | (cur_msg->ubData[3] << 8)
			| cur_msg->ubData[4];
	uword baseAddress =
			(passwordCmdState.whichController > 0) ? 0x80800000 : 0x80000000;
	FLASH_sendPasswords(passwordCmdState.readOrWrite, baseAddress,
			passwordCmdState.firstPassword, secondPassword,
			passwordCmdState.whichUCB);
	FLASHn_FSR_t *flashFSR =
			(passwordCmdState.whichController > 0) ? &FLASH1_FSR : &FLASH0_FSR;
	uword flashStatus = flashFSR->reg;
	ubyte canData[8];
	canData[0] = 0x4;
	canData[4] = (flashStatus >> 24) & 0xFF;
	canData[3] = (flashStatus >> 16) & 0xFF;
	canData[2] = (flashStatus >> 8) & 0xFF;
	canData[1] = flashStatus & 0xFF;
	canData[5] = 0xFF;
	canData[6] = 0xFF;
	canData[7] = 0xFF;
	MAIN_sendCan(canData);
}

/*
 * 0x5: eraseFlashSector
 * Uses bytes 1-4 to set the address to erase.
 * Erases the sector.
 * Returns the flash status register content.
 */
void MAIN_CMD_eraseFlashSector(CAN_SWObj *cur_msg) {
	uword address = (cur_msg->ubData[1] << 24) | (cur_msg->ubData[2] << 16)
			| (cur_msg->ubData[3] << 8) | cur_msg->ubData[4];
	uword baseAddress = (address >= 0x80800000) ? 0x80800000 : 0x80000000;
	uword whichController = (address >= 0x80800000);
	FLASHn_FSR_t *flashFSR = (whichController > 0) ? &FLASH1_FSR : &FLASH0_FSR;
	FLASH_eraseSector(flashFSR, baseAddress, address);
	uword flashStatus = flashFSR->reg;
	ubyte canData[8];
	canData[0] = 0x5;
	canData[4] = (flashStatus >> 24) & 0xFF;
	canData[3] = (flashStatus >> 16) & 0xFF;
	canData[2] = (flashStatus >> 8) & 0xFF;
	canData[1] = flashStatus & 0xFF;
	canData[5] = 0xFF;
	canData[6] = 0xFF;
	canData[7] = 0xFF;
	MAIN_sendCan(canData);
}

/*
 * 0x5: writeFlashPage
 * Uses bytes 1-4 to set the address to be written.
 * Configures the state of the BSL to await 256 bytes of page data to write to the Page.
 * Returns the address.
 */
void MAIN_CMD_writeFlashPage(CAN_SWObj *cur_msg) {
	uword address = (cur_msg->ubData[1] << 24) | (cur_msg->ubData[2] << 16)
			| (cur_msg->ubData[3] << 8) | cur_msg->ubData[4];
	writePageState.address = address;
	writePageState.cursor = 0;
	ubyte canData[8];
	canData[0] = 0x6;
	canData[4] = (address >> 24) & 0xFF;
	canData[3] = (address >> 16) & 0xFF;
	canData[2] = (address >> 8) & 0xFF;
	canData[1] = address & 0xFF;
	canData[5] = 0xFF;
	canData[6] = 0xFF;
	canData[7] = 0xFF;
	MAIN_sendCan(canData);
}

ubyte MAIN_CMD_writeFlashPageData(CAN_SWObj *cur_msg) {
	int i = 1;
	for (i = 1; i < 8; i++) {
		writePageState.assemblyBuffer[writePageState.cursor] =
				cur_msg->ubData[i];
		if (writePageState.cursor == 255) {
			break;
		}
		writePageState.cursor += 1;
		i += 1;
	}
	if (writePageState.cursor == 255) {
		uword baseAddress =
				(writePageState.address >= 0x80800000) ?
						0x80800000 : 0x80000000;
		uword whichController = (writePageState.address >= 0x80800000);
		FLASHn_FSR_t *flashFSR =
				(whichController > 0) ? &FLASH1_FSR : &FLASH0_FSR;
		FLASH_writePage(flashFSR, baseAddress, writePageState.address,
				writePageState.assemblyBuffer);
		ubyte canData[8];
		canData[0] = 0x6;
		canData[4] = 0x00;
		canData[3] = 0x00;
		canData[2] = 0x00;
		canData[1] = 0x00;
		canData[5] = 0xFF;
		canData[6] = 0xFF;
		canData[7] = 0xFF;
		MAIN_sendCan(canData);
		return 1;
	}
	return 0;
}

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void MAIN_CMD_readCompressed(CAN_SWObj *cur_msg) {
	uword address = (cur_msg->ubData[1] << 24) | (cur_msg->ubData[2] << 16)
			| (cur_msg->ubData[3] << 8) | cur_msg->ubData[4];
	uword length = (cur_msg->ubData[5] << 16) | (cur_msg->ubData[6] << 8)
			| cur_msg->ubData[7];
	compressReadState.address = address;
	compressReadState.length = length;
	compressReadState.cursor = 0;

	for (;;) {
		const char* inpPtr = (const char*) (compressReadState.address
				+ compressReadState.cursor);
		const int inpBytes = MIN(4096,
				compressReadState.length - compressReadState.cursor);
		compressReadState.cursor += inpBytes;
		if (0 == inpBytes) {
			break;
		}
		{
			char cmpBuf[LZ4_COMPRESSBOUND(4096)];
			const int cmpBytes = LZ4_compress_default(inpPtr, cmpBuf, inpBytes,
					LZ4_COMPRESSBOUND(4096));
			if (cmpBytes <= 0) {
				break;
			}
			ubyte canData[8];
			canData[0] = 0x7;
			canData[1] = ((uword)inpPtr >> 24) & 0xFF;
			canData[2] = ((uword)inpPtr >> 16) & 0xFF;
			canData[3] = ((uword)inpPtr >> 8) & 0xFF;
			canData[4] = (uword)inpPtr & 0xFF;
			canData[5] = (cmpBytes >> 16) & 0xFF;
			canData[6] = (cmpBytes >> 8) & 0xFF;
			canData[7] = cmpBytes & 0xFF;
			MAIN_sendCan(canData);
			ubyte canSeq = 0;
			int i = 0;
			for (i = 0; i < cmpBytes; i += 4) {
				canSeq++;
				canData[0] = 0x7;
				canData[1] = canSeq;
				canData[2] = cmpBuf[i];
				canData[3] = cmpBuf[i + 1];
				canData[4] = cmpBuf[i + 2];
				canData[5] = cmpBuf[i + 3];
				canData[6] = 0xFF;
				canData[7] = 0xFF;
				MAIN_sendCan(canData);
			}
		}
	}

}

void MAIN_processMessage(CAN_SWObj *cur_msg) {
	switch (boot.status) {
	case WAIT_COMMAND: {
		boot.command = cur_msg->ubData[0];
		switch (boot.command) {
		case READ_DEVICEID: // Read Device ID
		{
			MAIN_CMD_readDeviceId();
			break;
		}
		case READ_MEM32: // Read32 from Arbitrary Address
		{
			MAIN_CMD_read32(cur_msg);
			break;
		}
		case WRITE_MEM32: // Write32 to Arbitrary Address
		{
			MAIN_CMD_write32SetAddress(cur_msg);
			boot.status = WAIT_WRITE32_DATA;
			break;
		}
		case UNLOCK_PASSWORD: // Send UNLOCK command to Flash controller
			MAIN_CMD_unlockPasswordSetFirstPassword(cur_msg);
			boot.status = WAIT_PASSWORD_DATA;
			break;
		case ERASE_SECTOR:
			MAIN_CMD_eraseFlashSector(cur_msg);
			break;
		case WRITE_PAGE:
			MAIN_CMD_writeFlashPage(cur_msg);
			boot.status = WAIT_WRITEPAGE_DATA;
			break;
		case READ_COMPRESSED:
			MAIN_CMD_readCompressed(cur_msg);
			break;
		default: {
			break;
		}
		}
		break;
	}
	case WAIT_WRITE32_DATA: {
		boot.command = cur_msg->ubData[0];
		switch (boot.command) {
		case WRITE_MEM32: {
			MAIN_CMD_write32SetData(cur_msg);
			break;
		}
		default: {
			ubyte canData[8];
			canData[0] = 0x4F;
			canData[4] = 0xFF;
			canData[3] = 0xFF;
			canData[2] = 0xFF;
			canData[1] = 0xFF;
			canData[5] = 0xFF;
			canData[6] = 0xFF;
			canData[7] = 0xFF;
			MAIN_sendCan(canData);
			break;
		}
		}
		boot.address = 0;
		boot.status = WAIT_COMMAND;
		break;
	}
	case WAIT_PASSWORD_DATA: {
		boot.command = cur_msg->ubData[0];
		switch (boot.command) {
		case UNLOCK_PASSWORD: {
			MAIN_CMD_unlockPassword(cur_msg);
			break;
		}
		default: {
			ubyte canData[8];
			canData[0] = 0x5F;
			canData[4] = 0xFF;
			canData[3] = 0xFF;
			canData[2] = 0xFF;
			canData[1] = 0xFF;
			canData[5] = 0xFF;
			canData[6] = 0xFF;
			canData[7] = 0xFF;
			MAIN_sendCan(canData);
			break;
		}
		}
		boot.address = 0;
		boot.status = WAIT_COMMAND;
		break;
	}
	case WAIT_WRITEPAGE_DATA: {
		boot.command = cur_msg->ubData[0];
		switch (boot.command) {
		case WRITE_PAGE: {
			if (MAIN_CMD_writeFlashPageData(cur_msg) > 0) {
				boot.status = WAIT_COMMAND;
			}
			break;
		}
		default: {
			boot.status = WAIT_COMMAND;
			ubyte canData[8];
			canData[0] = 0x6F;
			canData[4] = 0xFF;
			canData[3] = 0xFF;
			canData[2] = 0xFF;
			canData[1] = 0xFF;
			canData[5] = 0xFF;
			canData[6] = 0xFF;
			canData[7] = 0xFF;
			MAIN_sendCan(canData);
			break;
		}
		}
		break;
	}
	default: {
		break;
	}
	}
}

sword main(void) {

	MAIN_vInit();
	CAN_SWObj cur_msg;
	memset(&boot, 0x00, sizeof(struct bootloader_state));
	do {
		if (CAN_ubNewData(0)) { // check for new message
			CAN_vGetMsgObj(0, &cur_msg); // copy and free to allow next msg into slot
			CAN_vReleaseObj(0);
			MAIN_processMessage(&cur_msg);
		}
	} while (1);
} //  End of function main
