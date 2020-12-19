//****************************************************************************
// @Module        MultiCAN Controller 
// @Filename      CAN.c
// @Project       Bootloader2.dav
//----------------------------------------------------------------------------
// @Controller    Infineon TC1791-512F200
//
// @Compiler      Hightec GNU 3.4.7.4
//
// @Codegenerator 1.0
//
// @Description   This file contains functions that use the CAN module.
//
//----------------------------------------------------------------------------
// @Date          12/18/2020 09:11:19
//
//****************************************************************************

// USER CODE BEGIN (CAN_General,1)

// USER CODE END



//****************************************************************************
// @Project Includes
//****************************************************************************

#include "MAIN.h"

// USER CODE BEGIN (CAN_General,2)

// USER CODE END


//****************************************************************************
// @Macros
//****************************************************************************

#define SetListCommand(Value) CAN_PANCTR.reg = Value; while (CAN_PANCTR.bits.BUSY);


// time mark entry
#define SetTME(NR, ARBM, IENRECF1, IENRECF0, IENTRAF1, IENTRAF0, INP, TMV) \
                    CAN_SENTRY[NR].uwEntry = 0x10000000 | ((uword)(ARBM) << 24) | \
                    ((uword)(IENRECF1) << 23) | ((uword)(IENRECF0) << 22) | \
                    ((uword)(IENTRAF1) << 21) | ((uword)(IENTRAF0) << 20) | \
                    ((uword)(INP) << 16) | ((uword)(TMV))

// interrupt control entry
#define SetICE(NR, IENRECF1, IENRECF0, IENTRAF1, IENTRAF0, INP, MCYCLE, CYCLE) \
                    CAN_SENTRY[NR].uwEntry = 0x20000000 | \
                    ((uword)(IENRECF1) << 23) | ((uword)(IENRECF0) << 22) | \
                    ((uword)(IENTRAF1) << 21) | ((uword)(IENTRAF0) << 20) | \
                    ((uword)(INP) << 16) | \
                    ((uword)(MCYCLE) << 8) | ((uword)(CYCLE))

// arbitration entry
#define SetARBE(NR, ARBM, MCYCLE, CYCLE) \
                    CAN_SENTRY[NR].uwEntry = 0x30000000 | ((uword)(ARBM) << 24) | \
                    ((uword)(MCYCLE) << 8) | ((uword)(CYCLE))

// transmit control entry
#define SetTCE(NR, ALTMSG, TREN, TCEMSGNR, MCYCLE, CYCLE) \
                    CAN_SENTRY[NR].uwEntry = 0x40000000 | ((uword)(ALTMSG) << 25) | \
                    ((uword)(TREN) << 24) | ((uword)(TCEMSGNR) << 16) | \
                    ((uword)(MCYCLE) << 8) | ((uword)(CYCLE))

// receive control entry
#define SetRCE(NR, CHEN, RCEMSGNR, MCYCLE, CYCLE) \
                    CAN_SENTRY[NR].uwEntry = 0x50000000 | \
                    ((uword)(CHEN) << 24) | ((uword)(RCEMSGNR) << 16) | \
                    ((uword)(MCYCLE) << 8) | ((uword)(CYCLE))

// reference message entry
#define SetRME(NR, GM, TMV) \
                    CAN_SENTRY[NR].uwEntry = 0x60000000 | \
                    ((uword)(GM) << 27) | ((uword)(TMV))

// basic cycle end entry
#define SetBCE(NR, GM, TMV) \
                    CAN_SENTRY[NR].uwEntry = 0x70000000 | \
                    ((uword)(GM) << 27) | ((uword)(TMV))

// end of scheduler memory entry
#define SetEOS(NR)  CAN_SENTRY[NR].uwEntry = 0x80000000

// USER CODE BEGIN (CAN_General,3)

// USER CODE END


//****************************************************************************
// @Defines
//****************************************************************************

// Structure for a single MultiCAN object
// A total of 128 such object structures exists

struct stCanObj 
{
  uword  uwMOFCR;    // Function Control Register
  uword  uwMOFGPR;   // FIFO/Gateway Pointer Register
  uword  uwMOIPR;    // Interrupt Pointer Register
  uword  uwMOAMR;    // Acceptance Mask Register
  ubyte  ubData[8];  // Message Data 0..7
  uword  uwMOAR;     // Arbitration Register
  uword  uwMOCTR;    // Control Register
};

#define CAN_HWOBJ ((struct stCanObj volatile *) 0xF0005000)


// Structure for a single TTCAN scheduler entry
// A total of 128 scheduler entries exists

struct stCanSEntry
{
  uword  uwEntry;    // scheduler entry
};

#define CAN_SENTRY ((struct stCanSEntry volatile *) 0xF0007E00)

// USER CODE BEGIN (CAN_General,4)

// USER CODE END


//****************************************************************************
// @Typedefs
//****************************************************************************

// USER CODE BEGIN (CAN_General,5)

// USER CODE END


//****************************************************************************
// @Imported Global Variables
//****************************************************************************

// USER CODE BEGIN (CAN_General,6)

// USER CODE END


//****************************************************************************
// @Global Variables
//****************************************************************************

static ubyte aubFIFOWritePtr[128];
static ubyte aubFIFOReadPtr[128];

// USER CODE BEGIN (CAN_General,7)

// USER CODE END


//****************************************************************************
// @External Prototypes
//****************************************************************************

// USER CODE BEGIN (CAN_General,8)

// USER CODE END


//****************************************************************************
// @Prototypes Of Local Functions
//****************************************************************************

// USER CODE BEGIN (CAN_General,9)

// USER CODE END


//****************************************************************************
// @Function      void CAN_vInit(void) 
//
//----------------------------------------------------------------------------
// @Description   This is the initialization function of the CAN function 
//                library. It is assumed that the SFRs used by this library 
//                are in their reset state. 
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

// USER CODE BEGIN (Init,1)

// USER CODE END

void CAN_vInit(void)
{
ubyte i;

  volatile unsigned int uwTemp;

  // USER CODE BEGIN (Init,2)

  // USER CODE END

  ///  -----------------------------------------------------------------------
  ///  Configuration of the Module Clock:
  ///  -----------------------------------------------------------------------

  ///  - the CAN module is not stopped during sleep mode
  ///  - normal divider mode is selected
  ///  - required CAN module clock is 80.00 MHz
  ///  - real CAN module clock is 100.00 MHz

  MAIN_vResetENDINIT();
  CAN_CLC.reg    = 0x00000008;   // load clock control register
  uwTemp         = CAN_CLC.reg;  // dummy read to avoid pipeline effects
  while ((CAN_CLC.reg & 0x00000002 )== 0x00000002);  //wait until module is enabled
  CAN_FDR.reg    = 0x000043FF;   // load fractional divider register
  uwTemp         = CAN_FDR.reg;  // dummy read to avoid pipeline effects
  MAIN_vSetENDINIT();

  //   - wait until Panel has finished the initialisation
  while (CAN_PANCTR.bits.BUSY);

  ///  -----------------------------------------------------------------------
  ///  Configuration of CAN Node 0:
  ///  -----------------------------------------------------------------------

  ///  General Configuration of the Node 0:
  ///  - set INIT and CCE

  CAN_NCR0.reg   = 0x00000041;   // load node 0 control register
  CAN_NIPR0.reg  = 0x00000000;   // load node 0 interrupt pointer register

  ///  Configuration of the Node 0 Error Counter:
  ///  - the error warning threshold value (warning level) is 96

  CAN_NECNT0.reg  = 0x00600000;  // load node 0 error counter register


  CAN_NPCR0.reg  = 0x00000000;   // load node 0 port control register

  ///  Configuration of the used CAN Port Pins:
  ///  - P6.8 is used as  CAN node 0 input signal 1 ( RXDCAN0)
  ///  - the pull-up device is assigned
  ///  - output driver characteristic: strong driver, sharp edge

  P6_IOCR8.reg   = (P6_IOCR8.reg & ~0x000000F0) | 0x00000020; // load control 
                                                              // register

  ///  - P6.9 is used as  CAN node 0 output signal 1 ( TXDCAN0)
  ///  - the push/pull function is activated
  ///  - output driver characteristic: strong driver, sharp edge

  P6_IOCR8.reg   = (P6_IOCR8.reg & ~0x0000F000) | 0x00009000; // load control 
                                                              // register

  ///  Configuration of the Node 0 Baud Rate:
  ///  - required baud rate = 500000.000 baud
  ///  - real baud rate     = 500000.000 baud
  ///  - sample point       = 80.00 %
  ///  - there are 7 time quanta before sample point
  ///  - there are 2 time quanta after sample point
  ///  - the (re)synchronization jump width is 2 time quanta

  CAN_NBTR0.reg  = 0x00001653;   // load  node 0 bit timing register

  ///  Configuration of the Frame Counter:
  ///  - Frame Counter Mode: the counter is incremented upon the reception 
  ///    and transmission of frames
  ///  - frame counter: 0x0000

  CAN_NFCR0.reg  = 0x00000000;   // load  node 0 frame counter register

  ///  Configuration of the TTCAN Functionality:
  ///  - the TTCAN functionality is disabled
  ///  -----------------------------------------------------------------------
  ///  Configuration of CAN Node 1:
  ///  -----------------------------------------------------------------------

  ///  General Configuration of the Node 1:
  ///  - set INIT and CCE

  CAN_NCR1.reg   = 0x00000041;   // load node 1 control register

  ///  -----------------------------------------------------------------------
  ///  Configuration of CAN Node 2:
  ///  -----------------------------------------------------------------------

  ///  General Configuration of the Node 2:
  ///  - set INIT and CCE

  CAN_NCR2.reg   = 0x00000041;   // load node 2 control register

  ///  -----------------------------------------------------------------------
  ///  Configuration of CAN Node 3:
  ///  -----------------------------------------------------------------------

  ///  General Configuration of the Node 3:
  ///  - set INIT and CCE

  CAN_NCR3.reg   = 0x00000041;   // load node 3 control register

  //   -----------------------------------------------------------------------
  //   Configuration of the Scheduler Entries:
  //   -----------------------------------------------------------------------

  // USER CODE BEGIN (Scheduler,1)

  // USER CODE END

  ///  -----------------------------------------------------------------------
  ///  Configuration of the CAN Message Object List Structure:
  ///  -----------------------------------------------------------------------
  ///  Allocate MOs for list 1:
  SetListCommand(0x01000002);  // MO0 for list 1
  SetListCommand(0x01010002);  // MO1 for list 1

  ///  -----------------------------------------------------------------------
  ///  Configuration of the CAN Message Objects 0 - 127:
  ///  -----------------------------------------------------------------------
  ///  -----------------------------------------------------------------------
  ///  Configuration of Message Object 0:
  ///  MO 0 is named as : Message Object 0
  ///  -----------------------------------------------------------------------

  ///  General Configuration of the Message Object 0 :
  ///  - message object 0 is valid
  ///  - message object is used as receive object
  ///  - this message object is assigned to list 1 (node 0)

  CAN_MOCTR0.reg  = 0x00A00000;  // load MO0 control register

  ///  - this object is a STANDARD MESSAGE OBJECT
  ///  - 8 valid data bytes

  CAN_MOFCR0.reg  = 0x08000000;  // load MO0 function control register


  CAN_MOFGPR0.reg  = 0x00000000; // load MO0 FIFO/gateway pointer register

  ///  - accept reception of standard and extended frames
  ///  - acceptance mask 29-bit: 0x1FFFFFFF
  ///  - acceptance mask 11-bit: 0x7FF

  CAN_MOAMR0.reg  = 0x1FFFFFFF;  // load MO0 acceptance mask register

  ///  - priority class 3; transmit acceptance filtering is based on the list 
  ///    order (like class 1)
  ///  - standard 11-bit identifier
  ///  - identifier 11-bit:      0x300

  CAN_MOAR0.reg  = 0xCC000000;   // load MO0 arbitration register

  ///  - use message pending register 0 bit position 0

  CAN_MOIPR0.reg  = 0x00000000;  // load MO0 interrupt pointer register
  CAN_MOCTR0.reg  = 0x00200000;  // set MSGVAL

  ///  -----------------------------------------------------------------------
  ///  Configuration of Message Object 1:
  ///  MO 1 is named as : Message Object 1
  ///  -----------------------------------------------------------------------

  ///  General Configuration of the Message Object 1 :
  ///  - message object 1 is valid
  ///  - message object is used as transmit object
  ///  - this message object is assigned to list 1 (node 0)

  CAN_MOCTR1.reg  = 0x0EA80000;  // load MO1 control register

  ///  - this object is a STANDARD MESSAGE OBJECT
  ///  - 8 valid data bytes

  CAN_MOFCR1.reg  = 0x08000000;  // load MO1 function control register


  CAN_MOFGPR1.reg  = 0x00000000; // load MO1 FIFO/gateway pointer register

  ///  - only accept receive frames with matching IDE bit
  ///  - acceptance mask 11-bit: 0x7FF

  CAN_MOAMR1.reg  = 0x3FFFFFFF;  // load MO1 acceptance mask register

  ///  - priority class 3; transmit acceptance filtering is based on the list 
  ///    order (like class 1)
  ///  - standard 11-bit identifier
  ///  - identifier 11-bit:      0x400

  CAN_MOAR1.reg  = 0xD0000000;   // load MO1 arbitration register

  CAN_MODATAL1.reg  = 0x00000000; // load MO1 data register low
  CAN_MODATAH1.reg  = 0x00000000; // load MO1 data register high

  ///  - use message pending register 0 bit position 1

  CAN_MOIPR1.reg  = 0x00000100;  // load MO1 interrupt pointer register
  CAN_MOCTR1.reg  = 0x00200000;  // set MSGVAL

  for (i = 0; i < 127; i++)
  {
    aubFIFOWritePtr[i] = (CAN_HWOBJ[i].uwMOFGPR & 0x000000ff);
    aubFIFOReadPtr[i]  = (CAN_HWOBJ[i].uwMOFGPR & 0x000000ff);
  }

  //   -----------------------------------------------------------------------
  //   Start the CAN Nodes:
  //   -----------------------------------------------------------------------
  CAN_NCR0.reg  &= ~0x00000041;  // reset INIT and CCE


  // USER CODE BEGIN (Init,3)

  // USER CODE END

} //  End of function CAN_vInit


//****************************************************************************
// @Function      void CAN_vGetMsgObj(ubyte ubObjNr, CAN_SWObj *pstObj) 
//
//----------------------------------------------------------------------------
// @Description   This function fills the forwarded SW message object with 
//                the content of the chosen HW message object.
//                
//                The structure of the SW message object is defined in the 
//                header file CAN.h (see CAN_SWObj).
//
//----------------------------------------------------------------------------
// @Returnvalue   None
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object to be read (0-127)
// @Parameters    *pstObj: 
//                Pointer on a message object to be filled by this function
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (GetMsgObj,1)

// USER CODE END

void CAN_vGetMsgObj(ubyte ubObjNr, CAN_SWObj *pstObj)
{
  ubyte i;

  // get DLC
  pstObj->usMOCfg  = (CAN_HWOBJ[ubObjNr].uwMOFCR & 0x0f000000) >> 24;
  for(i = 0; i < pstObj->usMOCfg; i++)
  {
    pstObj->ubData[i] = CAN_HWOBJ[ubObjNr].ubData[i];
  }

  pstObj->usMOCfg  = (pstObj->usMOCfg << 4);    // shift DLC
  if(CAN_HWOBJ[ubObjNr].uwMOCTR & 0x00000800)   // if transmit object
  {
    pstObj->usMOCfg  = pstObj->usMOCfg | 0x08;  // set DIR
  }

  if(CAN_HWOBJ[ubObjNr].uwMOAR & 0x20000000)    // extended identifier
  {
    pstObj->uwID     = CAN_HWOBJ[ubObjNr].uwMOAR & 0x1fffffff;
    pstObj->uwMask   = CAN_HWOBJ[ubObjNr].uwMOAMR & 0x1fffffff;
    pstObj->usMOCfg  = pstObj->usMOCfg | 0x04;  // set IDE
  }
  else                                          // standard identifier 
  {
    pstObj->uwID   = (CAN_HWOBJ[ubObjNr].uwMOAR & 0x1fffffff) >> 18;
    pstObj->uwMask = (CAN_HWOBJ[ubObjNr].uwMOAMR & 0x1fffffff) >> 18;
  }

  pstObj->usCounter = (CAN_HWOBJ[ubObjNr].uwMOIPR & 0xffff0000) >> 16;

} //  End of function CAN_vGetMsgObj


//****************************************************************************
// @Function      ubyte CAN_ubRequestMsgObj(ubyte ubObjNr) 
//
//----------------------------------------------------------------------------
// @Description   If a TRANSMIT OBJECT is to be reconfigured it must first be 
//                accessed. The access to the transmit object is exclusive. 
//                This function checks whether the choosen message object is 
//                still executing a transmit request, or if the object can be 
//                accessed exclusively.
//                After the message object is reserved, it can be 
//                reconfigured by using the function CAN_vConfigMsgObj or 
//                CAN_vLoadData.
//                Both functions enable access to the object for the CAN 
//                controller. 
//                By calling the function CAN_vTransmit transfering of data 
//                is started.
//
//----------------------------------------------------------------------------
// @Returnvalue   0 message object is busy (a transfer is active), else 1
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object (0-127)
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (RequestMsgObj,1)

// USER CODE END

ubyte CAN_ubRequestMsgObj(ubyte ubObjNr)
{
  ubyte ubReturn;

  ubReturn = 0;
  if((CAN_HWOBJ[ubObjNr].uwMOCTR & 0x00000100) == 0x00000000)  // if reset TXRQ 
  {
    CAN_HWOBJ[ubObjNr].uwMOCTR = 0x00000020;                   // reset MSGVAL 
    ubReturn = 1;
  }
  return(ubReturn);

} //  End of function CAN_ubRequestMsgObj


//****************************************************************************
// @Function      ubyte CAN_ubNewData(ubyte ubObjNr) 
//
//----------------------------------------------------------------------------
// @Description   This function checks whether the selected RECEIVE OBJECT 
//                has received a new message. If so the function returns the 
//                value '1'.
//
//----------------------------------------------------------------------------
// @Returnvalue   1 the message object has received a new message, else 0
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object (0-127)
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (NewData,1)

// USER CODE END

ubyte CAN_ubNewData(ubyte ubObjNr)
{
  ubyte ubReturn;

  ubReturn = 0;
  if((CAN_HWOBJ[ubObjNr].uwMOCTR & 0x00000008))    // if NEWDAT 
  {
    ubReturn = 1;
  }
  return(ubReturn);

} //  End of function CAN_ubNewData


//****************************************************************************
// @Function      void CAN_vTransmit(ubyte ubObjNr) 
//
//----------------------------------------------------------------------------
// @Description   This function triggers the CAN controller to send the 
//                selected message.
//                If the selected message object is a TRANSMIT OBJECT then 
//                this function triggers the sending of a data frame. If 
//                however the selected message object is a RECEIVE OBJECT 
//                this function triggers the sending of a remote frame.
//
//----------------------------------------------------------------------------
// @Returnvalue   None
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object (0-127)
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (Transmit,1)

// USER CODE END

void CAN_vTransmit(ubyte ubObjNr)
{
  CAN_HWOBJ[ubObjNr].uwMOCTR = 0x07000000;  // set TXRQ,TXEN0,TXEN1

} //  End of function CAN_vTransmit


//****************************************************************************
// @Function      void CAN_vConfigMsgObj(ubyte ubObjNr, CAN_SWObj *pstObj) 
//
//----------------------------------------------------------------------------
// @Description   This function sets up the message objects. This includes 
//                the 8 data bytes, the identifier (11- or 29-bit), the 
//                acceptance mask (11- or 29-bit), the data number (0-8 
//                bytes), the frame counter value and the EDE-bit (standard 
//                or extended identifier).  The direction bit (DIR) can not 
//                be changed. 
//                The message is not sent; for this the function 
//                CAN_vTransmit must be called.
//                
//                The structure of the SW message object is defined in the 
//                header file CAN.h (see CAN_SWObj).
//
//----------------------------------------------------------------------------
// @Returnvalue   None
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object to be configured (0-127)
// @Parameters    *pstObj: 
//                Pointer on a message object
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (ConfigMsgObj,1)

// USER CODE END

void CAN_vConfigMsgObj(ubyte ubObjNr, CAN_SWObj *pstObj)
{
  ubyte i;

  CAN_HWOBJ[ubObjNr].uwMOCTR = 0x00000020;     // reset MSGVAL

  if(pstObj->usMOCfg & 0x0004)                 // extended identifier
  {
    CAN_HWOBJ[ubObjNr].uwMOAR  &= ~0x3fffffff;
    CAN_HWOBJ[ubObjNr].uwMOAR  |= 0x20000000 | pstObj->uwID ;
    CAN_HWOBJ[ubObjNr].uwMOAMR &= ~0x1fffffff;
    CAN_HWOBJ[ubObjNr].uwMOAMR |= pstObj->uwMask ;
  }
  else                                         // standard identifier
  {
    CAN_HWOBJ[ubObjNr].uwMOAR  &= ~0x3fffffff;
    CAN_HWOBJ[ubObjNr].uwMOAR  |= pstObj->uwID << 18 ;
    CAN_HWOBJ[ubObjNr].uwMOAMR &= ~0x1fffffff;
    CAN_HWOBJ[ubObjNr].uwMOAMR |= pstObj->uwMask << 18 ;
  }

  CAN_HWOBJ[ubObjNr].uwMOIPR &= ~0xffff0000;
  CAN_HWOBJ[ubObjNr].uwMOIPR |= pstObj->usCounter << 16;

  CAN_HWOBJ[ubObjNr].uwMOFCR  = (CAN_HWOBJ[ubObjNr].uwMOFCR & ~0x0f000000) | ((pstObj->usMOCfg & 0x00f0) << 20);

  if(CAN_HWOBJ[ubObjNr].uwMOCTR & 0x00000800)  // if transmit direction
  {
    for(i = 0; i < (pstObj->usMOCfg & 0x00f0) >> 4; i++)
    {
      CAN_HWOBJ[ubObjNr].ubData[i] = pstObj->ubData[i];
    }
    CAN_HWOBJ[ubObjNr].uwMOCTR  = 0x06280040;  // set NEWDAT, reset RTSEL, 
  }                                            // set MSGVAL, set TXEN0, TXEN1
  else                                         // if receive direction
  {
    CAN_HWOBJ[ubObjNr].uwMOCTR  = 0x00200040;  // reset RTSEL, set MSGVAL
  }

} //  End of function CAN_vConfigMsgObj


//****************************************************************************
// @Function      void CAN_vLoadData(ubyte ubObjNr, ubyte *pubData) 
//
//----------------------------------------------------------------------------
// @Description   If a hardware TRANSMIT OBJECT has to be loaded with data 
//                but not with a new identifier, this function may be used 
//                instead of the function CAN_vConfigMsgObj. The message 
//                object should be accessed by calling the function 
//                CAN_ubRequestMsgObj before calling this function. This 
//                prevents the CAN controller from working with invalid data.
//
//----------------------------------------------------------------------------
// @Returnvalue   None
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object to be configured (0-127)
// @Parameters    *pubData: 
//                Pointer on a data buffer
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (LoadData,1)

// USER CODE END

void CAN_vLoadData(ubyte ubObjNr, ubyte *pubData)
{
  ubyte i;

  CAN_HWOBJ[ubObjNr].uwMOCTR = 0x00080000;       // set NEWDAT

  for(i = 0; i < (CAN_HWOBJ[ubObjNr].uwMOFCR & 0x0f000000) >> 24; i++)
  {
    CAN_HWOBJ[ubObjNr].ubData[i] = *(pubData++);
  }

  CAN_HWOBJ[ubObjNr].uwMOCTR = 0x00200040;       // reset RTSEL, set MSGVAL

} //  End of function CAN_vLoadData


//****************************************************************************
// @Function      ubyte CAN_ubMsgLost(ubyte ubObjNr) 
//
//----------------------------------------------------------------------------
// @Description   If a RECEIVE OBJECT receives new data before the old object 
//                has been read, the old object is lost. The CAN controller 
//                indicates this by setting the message lost bit (MSGLST). 
//                This function returns the status of this bit. 
//                
//                Note:
//                This function resets the message lost bit (MSGLST).
//
//----------------------------------------------------------------------------
// @Returnvalue   1 the message object has lost a message, else 0
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object (0-127)
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (MsgLost,1)

// USER CODE END

ubyte CAN_ubMsgLost(ubyte ubObjNr)
{
  ubyte ubReturn;

  ubReturn = 0;
  if(CAN_HWOBJ[ubObjNr].uwMOCTR & 0x00000010)  // if set MSGLST 
  {
    CAN_HWOBJ[ubObjNr].uwMOCTR = 0x00000010;   // reset MSGLST 
    ubReturn = 1;
  }
  return(ubReturn);

} //  End of function CAN_ubMsgLost


//****************************************************************************
// @Function      ubyte CAN_ubDelMsgObj(ubyte ubObjNr) 
//
//----------------------------------------------------------------------------
// @Description   This function marks the selected message object as not 
//                valid. This means that this object cannot be sent or 
//                receive data. If the selected object is busy (meaning the 
//                object is transmitting a message or has received a new 
//                message) this function returns the value "0" and the object 
//                is not deleted.
//
//----------------------------------------------------------------------------
// @Returnvalue   1 the message object was deleted, else 0
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object (0-127)
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (DelMsgObj,1)

// USER CODE END

ubyte CAN_ubDelMsgObj(ubyte ubObjNr)
{
  ubyte ubReturn;

  ubReturn = 0;
  if(!(CAN_HWOBJ[ubObjNr].uwMOCTR & 0x00000108)) // if set TXRQ or NEWDAT
  {
    CAN_HWOBJ[ubObjNr].uwMOCTR = 0x00000020;     // reset MSGVAL
    ubReturn = 1;
  }
  return(ubReturn);

} //  End of function CAN_ubDelMsgObj


//****************************************************************************
// @Function      void CAN_vReleaseObj(ubyte ubObjNr) 
//
//----------------------------------------------------------------------------
// @Description   This function resets the NEWDAT flag of the selected 
//                RECEIVE OBJECT, so that the CAN controller have access to 
//                it. This function must be called if the function 
//                CAN_ubNewData detects, that new data are present in the 
//                message object and the actual data have been read by 
//                calling the function CAN_vGetMsgObj. 
//
//----------------------------------------------------------------------------
// @Returnvalue   None
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object (0-127)
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (ReleaseObj,1)

// USER CODE END

void CAN_vReleaseObj(ubyte ubObjNr)
{
  CAN_HWOBJ[ubObjNr].uwMOCTR = 0x00000008;     // reset NEWDAT

} //  End of function CAN_vReleaseObj


//****************************************************************************
// @Function      void CAN_vSetMSGVAL(ubyte ubObjNr) 
//
//----------------------------------------------------------------------------
// @Description   This function sets the MSGVAL flag of the selected object. 
//                This is only necessary if the single data transfer mode 
//                (SDT) for the selected object is enabled. If SDT is set to 
//                '1', the CAN controller automatically resets bit MSGVAL 
//                after receiving or tranmission of a frame.
//
//----------------------------------------------------------------------------
// @Returnvalue   None
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the message object (0-127)
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (SetMSGVAL,1)

// USER CODE END

void CAN_vSetMSGVAL(ubyte ubObjNr)
{
  CAN_HWOBJ[ubObjNr].uwMOCTR = 0x00200000;     // set MSGVAL

} //  End of function CAN_vSetMSGVAL


//****************************************************************************
// @Function      ubyte CAN_ubWriteFIFO(ubyte ubObjNr, CAN_SWObj *pstObj) 
//
//----------------------------------------------------------------------------
// @Description   This function sets up the next free TRANSMIT message object 
//                which is part of a FIFO. This includes the 8 data bytes, 
//                the identifier (11- or 29-bit) and the data number (0-8 
//                bytes). The direction bit (DIR) and the EDE-bit can not be 
//                changed. The acceptance mask register and the Frame Counter 
//                remains unchanged. This function checks whether the choosen 
//                message object is still executing a transmit request, or if 
//                the object can be accessed exclusively. 
//                The structure of the SW message object is defined in the 
//                header file CAN.h (see CAN_SWObj).
//                Note: 
//                This function can only used for TRANSMIT objects which are 
//                configured for FIFO base functionality. 
//
//----------------------------------------------------------------------------
// @Returnvalue   0: message object is busy (a transfer is active); 1: the 
//                message object was configured and the transmite is 
//                requested; 2: this is not a FIFO base object
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the FIFO base object
// @Parameters    *pstObj: 
//                Pointer on a message object
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (WriteFIFO,1)

// USER CODE END

ubyte CAN_ubWriteFIFO(ubyte ubObjNr, CAN_SWObj *pstObj)
{
  ubyte i,j;
  ubyte ubReturn;

  ubReturn = 2;

  if((CAN_HWOBJ[ubObjNr].uwMOFCR & 0x0000000f) == 0x00000002)  // if transmit FIFO base object 
  {
    j = aubFIFOWritePtr[ubObjNr];

    ubReturn = 0;
    if((CAN_HWOBJ[j].uwMOCTR & 0x00000100) == 0x00000000) // if reset TXRQ 
    {
      if(j == (CAN_HWOBJ[ubObjNr].uwMOFGPR & 0x0000ff00) >> 8)  // top MO in a list
      {
        // WritePtr = BOT of the base object
        aubFIFOWritePtr[ubObjNr] = (CAN_HWOBJ[ubObjNr].uwMOFGPR & 0x000000ff);
      }
      else
      {
        // WritePtr = PNEXT of the current selected slave
        aubFIFOWritePtr[ubObjNr] = (CAN_HWOBJ[j].uwMOCTR & 0xff000000) >> 24;
      }

      CAN_HWOBJ[j].uwMOCTR = 0x00000008;                  // reset NEWDAT 

      if(CAN_HWOBJ[j].uwMOAR & 0x20000000)                // extended identifier
      {
        CAN_HWOBJ[j].uwMOAR  &= ~0x3fffffff;
        CAN_HWOBJ[j].uwMOAR  |= 0x20000000 | pstObj->uwID;
      }
      else                                                // if standard identifier
      {
        CAN_HWOBJ[j].uwMOAR  &= ~0x3fffffff;
        CAN_HWOBJ[j].uwMOAR  |= pstObj->uwID << 18 ;
      }

      CAN_HWOBJ[j].uwMOFCR  = (CAN_HWOBJ[j].uwMOFCR & ~0x0f000000) | ((pstObj->usMOCfg & 0x00f0) << 20);

      for(i = 0; i < (pstObj->usMOCfg & 0x00f0) >> 4; i++)
      {
        CAN_HWOBJ[j].ubData[i] = pstObj->ubData[i];
      }
      CAN_HWOBJ[j].uwMOCTR  = 0x01280000;                 // set TXRQ, NEWDAT, MSGVAL 
                                              
      ubReturn = 1;
    }
  }
  return(ubReturn);

} //  End of function CAN_ubWriteFIFO


//****************************************************************************
// @Function      ubyte CAN_ubReadFIFO(ubyte ubObjNr, CAN_SWObj *pstObj) 
//
//----------------------------------------------------------------------------
// @Description   This function reads the next RECEIVE message object which 
//                is part of a FIFO. It checks whether the selected RECEIVE 
//                OBJECT has received a new message. If so the forwarded SW 
//                message object is filled with the content of the HW message 
//                object and the functions returns the value "1". The 
//                structure of the SW message object is defined in the header 
//                file CAN.h (see CAN_SWObj).
//                Note: 
//                This function can only used for RECEIVE objects which are 
//                configured for FIFO base functionality. 
//                Be sure that no interrupt is enabled for the FIFO objects. 
//
//----------------------------------------------------------------------------
// @Returnvalue   0: the message object has not received a new message; 1: 
//                the message object has received a new message; 2: this is 
//                not a FIFO base object; 3: a previous message was lost; 4: 
//                the received message is corrupted
//
//----------------------------------------------------------------------------
// @Parameters    ubObjNr: 
//                Number of the FIFO base object
// @Parameters    *pstObj: 
//                Pointer on a message object to be filled by this function
//
//----------------------------------------------------------------------------
// @Date          12/18/2020
//
//****************************************************************************

// USER CODE BEGIN (ReadFIFO,1)

// USER CODE END

ubyte CAN_ubReadFIFO(ubyte ubObjNr, CAN_SWObj *pstObj)
{
  ubyte i,j;
  ubyte ubReturn;

  ubReturn = 2;

  if((CAN_HWOBJ[ubObjNr].uwMOFCR & 0x0000000f) == 0x00000001)  // if receive FIFO base object 
  {
    j = aubFIFOReadPtr[ubObjNr];

    ubReturn = 0;
    if(CAN_HWOBJ[j].uwMOCTR & 0x00000008)                 // if NEWDAT 
    {
      CAN_HWOBJ[j].uwMOCTR = 0x00000008;                  // clear NEWDAT

      if(j == (CAN_HWOBJ[ubObjNr].uwMOFGPR & 0x0000ff00) >> 8)  // top MO in a list
      {
        // ReadPtr = BOT of the base object
        aubFIFOReadPtr[ubObjNr] = (CAN_HWOBJ[ubObjNr].uwMOFGPR & 0x000000ff);
      }
      else
      {
        // ReadPtr = PNEXT of the current selected slave
        aubFIFOReadPtr[ubObjNr] = (CAN_HWOBJ[j].uwMOCTR & 0xff000000) >> 24;
      }

      // check if the previous message was lost 
      if(CAN_HWOBJ[j].uwMOCTR & 0x00000010)               // if set MSGLST 
      {
        CAN_HWOBJ[j].uwMOCTR = 0x00000010;                // reset MSGLST 
        return(3);
      }

      pstObj->usMOCfg  = (CAN_HWOBJ[j].uwMOFCR & 0x0f000000) >> 24;
      for(i = 0; i < pstObj->usMOCfg; i++)
      {
        pstObj->ubData[i] = CAN_HWOBJ[j].ubData[i];
      }

      pstObj->usMOCfg  = (pstObj->usMOCfg << 4);
      if(CAN_HWOBJ[j].uwMOCTR & 0x00000800)               // transmit object
      {
        pstObj->usMOCfg  = pstObj->usMOCfg | 0x08;
      }

      if(CAN_HWOBJ[j].uwMOAR & 0x20000000)                // extended identifier
      {
        pstObj->uwID     = CAN_HWOBJ[j].uwMOAR & 0x1fffffff;
        pstObj->usMOCfg  = pstObj->usMOCfg | 0x04;
      }
      else                                                // standard identifier 
      {
        pstObj->uwID   = (CAN_HWOBJ[j].uwMOAR & 0x1fffffff) >> 18;
      }

      pstObj->usCounter = (CAN_HWOBJ[j].uwMOIPR & 0xffff0000) >> 16;

      // check if the message was corrupted 
      if(CAN_HWOBJ[j].uwMOCTR & 0x00000008)               // if NEWDAT 
      {
        CAN_HWOBJ[j].uwMOCTR = 0x00000008;                // clear NEWDAT
        return(4);
      }
      ubReturn = 1;
    }
  }
  return(ubReturn);

} //  End of function CAN_ubReadFIFO



// USER CODE BEGIN (CAN_General,10)

// USER CODE END

