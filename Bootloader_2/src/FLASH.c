/*
 * FLASH.c
 *
 *  Created on: Dec 29, 2020
 *      Author: b l
 */

#include "FLASH.h"

void FLASH_sendPasswords(enum FLASH_PROTECTION whichProtection, uword flashBaseAddress, uword password0, uword password1, uword whichUCB)
{
	// Reset flash controller status
	(*(volatile uword *) (flashBaseAddress + 0x5554)) = 0x00F5;

	// Send passwords to flash controller
	(*(volatile uword *) (flashBaseAddress + 0x5554)) = 0x00AA;
	(*(volatile uword *) (flashBaseAddress + 0xAAA8)) = 0x0055;
	(*(volatile uword *) (flashBaseAddress + 0x553C)) = whichUCB;
	(*(volatile uword *) (flashBaseAddress + 0xAAA8)) = password0;
	(*(volatile uword *) (flashBaseAddress + 0xAAA8)) = password1;
	(*(volatile uword *) (flashBaseAddress + 0x5558)) = whichProtection;

	__asm("nop");
	__asm("nop");
	__asm("nop");
}
