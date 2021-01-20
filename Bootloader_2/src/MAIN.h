//****************************************************************************
// @Module        Project Settings
// @Filename      MAIN.h
// @Project       Bootloader2.dav
//----------------------------------------------------------------------------
// @Controller    Infineon TC1791-512F200
//
// @Compiler      Hightec GNU 3.4.7.4
//
// @Codegenerator 1.0
//
// @Description   This file contains all function prototypes and macros for 
//                the MAIN module.
//
//----------------------------------------------------------------------------
// @Date          12/18/2020 09:11:18
//
//****************************************************************************

// USER CODE BEGIN (MAIN_Header,1)

// USER CODE END



#ifndef _MAIN_H_
#define _MAIN_H_

//****************************************************************************
// @Project Includes
//****************************************************************************

// USER CODE BEGIN (MAIN_Header,2)

// USER CODE END


//****************************************************************************
// @Macros
//****************************************************************************

// USER CODE BEGIN (MAIN_Header,3)

// USER CODE END


//****************************************************************************
// @Defines
//****************************************************************************

// compiler dependent names of builtin functions
#define MTCR _mtcr
#define ISYNC _isync
#define DISABLE _disable
#define ENABLE _enable

// USER CODE BEGIN (MAIN_Header,4)

// USER CODE END




//****************************************************************************
// @Typedefs
//****************************************************************************

typedef unsigned char  ubyte;    // 1 byte unsigned; prefix: ub 
typedef signed char    sbyte;    // 1 byte signed;   prefix: sb 
typedef signed short   sshort;   // 2 byte signed;   prefix: ss 
typedef unsigned int   uword;    // 4 byte unsigned; prefix: uw 
typedef signed int     sword;    // 4 byte signed;   prefix: sw 

typedef volatile struct
{
  unsigned int    bit0      : 1;
  unsigned int    bit1      : 1;
  unsigned int    bit2      : 1;
  unsigned int    bit3      : 1;
  unsigned int    bit4      : 1;
  unsigned int    bit5      : 1;
  unsigned int    bit6      : 1;
  unsigned int    bit7      : 1;
  unsigned int    bit8      : 1;
  unsigned int    bit9      : 1;
  unsigned int    bit10     : 1;
  unsigned int    bit11     : 1;
  unsigned int    bit12     : 1;
  unsigned int    bit13     : 1;
  unsigned int    bit14     : 1;
  unsigned int    bit15     : 1;
  unsigned int    bit16     : 1;
  unsigned int    bit17     : 1;
  unsigned int    bit18     : 1;
  unsigned int    bit19     : 1;
  unsigned int    bit20     : 1;
  unsigned int    bit21     : 1;
  unsigned int    bit22     : 1;
  unsigned int    bit23     : 1;
  unsigned int    bit24     : 1;
  unsigned int    bit25     : 1;
  unsigned int    bit26     : 1;
  unsigned int    bit27     : 1;
  unsigned int    bit28     : 1;
  unsigned int    bit29     : 1;
  unsigned int    bit30     : 1;
  unsigned int    bit31     : 1;
}  T_Reg32;

// USER CODE BEGIN (MAIN_Header,5)

// USER CODE END


//****************************************************************************
// @Imported Global Variables
//****************************************************************************

// USER CODE BEGIN (MAIN_Header,6)

// USER CODE END


//****************************************************************************
// @Global Variables
//****************************************************************************

// USER CODE BEGIN (MAIN_Header,7)

// USER CODE END


//****************************************************************************
// @Prototypes Of Global Functions
//****************************************************************************

void MAIN_vWriteWDTCON0(uword uwValue);


// USER CODE BEGIN (MAIN_Header,8)

// USER CODE END


//****************************************************************************
// @Macro         MAIN_vSetENDINIT() 
//
//----------------------------------------------------------------------------
// @Description   This macro sets the EndInit bit, which controls access to 
//                system critical registers. Setting the EndInit bit locks 
//                all EndInit protected registers.
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

#define MAIN_vSetENDINIT() MAIN_vWriteWDTCON0(WDT_CON0.reg | 0x00000001)


//****************************************************************************
// @Macro         MAIN_vResetENDINIT() 
//
//----------------------------------------------------------------------------
// @Description   This macro clears the EndInit bit, which controls access to 
//                system critical registers. Clearing the EndInit bit unlocks 
//                all EndInit protected registers. Modifications of the 
//                EndInit bit are monitored by the watchdog timer such that 
//                after clearing EndInit, the watchdog timer enters a defined 
//                time-out mode; EndInit must be set again before the 
//                time-out expires.
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

#define MAIN_vResetENDINIT() MAIN_vWriteWDTCON0(WDT_CON0.reg & ~0x00000001);while (WDT_CON0.bits.ENDINIT != 0)


//****************************************************************************
// @Interrupt Vectors
//****************************************************************************


// USER CODE BEGIN (MAIN_Header,9)

// USER CODE END


//****************************************************************************
// @Project Includes
//****************************************************************************

#include  <TC1791.h> 
#include  <machine/intrinsics.h>
#include  <machine/cint.h>
#include  <string.h>
#include  <sys/types.h> 
#include  "CAN.h"
#include  "FLASH.h"

// USER CODE BEGIN (MAIN_Header,10)

// USER CODE END

enum CAN_COMMAND {
	READ_DEVICEID = 1,
	READ_MEM32 = 2,
	WRITE_MEM32 = 3,
	UNLOCK_PASSWORD = 4,
	ERASE_SECTOR = 5,
	WRITE_PAGE = 6
};

enum CURRENT_STATUS {
	WAIT_COMMAND = 0,
	WAIT_WRITE32_DATA = 1,
	WAIT_WRITEPAGE_DATA = 2,
	WAIT_PASSWORD_DATA = 3
};

struct bootloader_state {
	enum CURRENT_STATUS status;
	enum CAN_COMMAND command;
	uword address;
	ubyte len[2];
};

struct password_cmd_state {
	uword firstPassword;
	ubyte whichController;
	ubyte readOrWrite;
	ubyte whichUCB;
};

struct write_page_state {
	uword address;
	uword cursor;
	ubyte assemblyBuffer[256];
};


#endif  // ifndef _MAIN_H_
