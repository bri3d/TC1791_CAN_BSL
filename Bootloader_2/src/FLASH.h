/*
 * FLASH.h
 *
*/

#ifndef FLASH_H_
#define FLASH_H_

#include "MAIN.h"

enum FLASH_PROTECTION {
	READ_PROTECTION = 0x08,
	WRITE_PROTECTION = 0x05
};

void FLASH_sendPasswords(enum FLASH_PROTECTION whichProtection, uword flashBaseAddress, uword password0, uword password1, uword whichUCB);

#endif /* FLASH_H_ */
