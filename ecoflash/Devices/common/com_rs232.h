/*
 * com_rs232.h
 *
 * Communication through serial port (RS232) - header file
 *
 * Author: Filip Kinovic
 * Version: 1.3
 * Date: 26.02.2016
*/


#ifndef _COM_RS232_H_
#define _COM_RS232_H_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/* constants */

#define RS232_MASK(idx) (1<<(idx))
#define RS232_MASK_ALL 0xFFFFFFFF

#define RS232_TERMINAL_CR '\r'
#define RS232_TERMINAL_LF '\n'

/* type definitions */

//baudrate
typedef enum {
	RS232_BAUDRATE_110 = CBR_110,
	RS232_BAUDRATE_300 = CBR_300,
	RS232_BAUDRATE_600 = CBR_600,
	RS232_BAUDRATE_1200 = CBR_1200,
	RS232_BAUDRATE_2400 = CBR_2400,
	RS232_BAUDRATE_4800 = CBR_4800,
	RS232_BAUDRATE_9600 = CBR_9600,
	RS232_BAUDRATE_19200 = CBR_19200,
	RS232_BAUDRATE_38400 = CBR_38400,
	RS232_BAUDRATE_57600 = CBR_57600,
	RS232_BAUDRATE_115200 = CBR_115200,
	RS232_BAUDRATE_230400 = 230400,
	RS232_BAUDRATE_250000 = 250000,
	RS232_BAUDRATE_460800 = 460800,
	RS232_BAUDRATE_921600 = 921600,
	RS232_BAUDRATE_1000000 = 1000000,
	RS232_BAUDRATE_1500000 = 1500000,
	RS232_BAUDRATE_3000000 = 3000000,
	RS232_BAUDRATE_3500000 = 3500000,
	RS232_BAUDRATE_4000000 = 4000000,
	RS232_BAUDRATE_5250000 = 5250000,
} RS232_BAUDRATE;
#define RS232_BAUDRATE_MAX 21
#define RS232_BAUDRATE_DEFAULT RS232_BAUDRATE_9600

//baudrate mask
#define RS232_M_BAUDRATE_110 RS232_MASK(0)
#define RS232_M_BAUDRATE_300 RS232_MASK(1)
#define RS232_M_BAUDRATE_600 RS232_MASK(2)
#define RS232_M_BAUDRATE_1200 RS232_MASK(3)
#define RS232_M_BAUDRATE_2400 RS232_MASK(4)
#define RS232_M_BAUDRATE_4800 RS232_MASK(5)
#define RS232_M_BAUDRATE_9600 RS232_MASK(6)
#define RS232_M_BAUDRATE_19200 RS232_MASK(7)
#define RS232_M_BAUDRATE_38400 RS232_MASK(8)
#define RS232_M_BAUDRATE_57600 RS232_MASK(9)
#define RS232_M_BAUDRATE_115200 RS232_MASK(10)
#define RS232_M_BAUDRATE_230400 RS232_MASK(11)
#define RS232_M_BAUDRATE_250000 RS232_MASK(12)
#define RS232_M_BAUDRATE_460800 RS232_MASK(13)
#define RS232_M_BAUDRATE_921600 RS232_MASK(14)
#define RS232_M_BAUDRATE_1000000 RS232_MASK(15)
#define RS232_M_BAUDRATE_1500000 RS232_MASK(16)
#define RS232_M_BAUDRATE_3000000 RS232_MASK(17)
#define RS232_M_BAUDRATE_3500000 RS232_MASK(18)
#define RS232_M_BAUDRATE_4000000 RS232_MASK(19)
#define RS232_M_BAUDRATE_5250000 RS232_MASK(20)

//databits
typedef enum {
	RS232_DATABITS_5 = 5,
	RS232_DATABITS_6 = 6,
	RS232_DATABITS_7 = 7,
	RS232_DATABITS_8 = 8,
} RS232_DATABITS;
#define RS232_DATABITS_MAX 4
#define RS232_DATABITS_DEFAULT RS232_DATABITS_8

#define RS232_M_DATABITS_5 RS232_MASK(0)
#define RS232_M_DATABITS_6 RS232_MASK(1)
#define RS232_M_DATABITS_7 RS232_MASK(2)
#define RS232_M_DATABITS_8 RS232_MASK(3)

//parity
typedef enum {
	RS232_PARITY_NONE = NOPARITY,
	RS232_PARITY_ODD = ODDPARITY,
	RS232_PARITY_EVEN = EVENPARITY,
	RS232_PARITY_MARK = MARKPARITY,
	RS232_PARITY_SPACE = SPACEPARITY,
} RS232_PARITY;
#define RS232_PARITY_MAX 5
#define RS232_PARITY_DEFAULT RS232_PARITY_NONE

#define RS232_M_PARITY_NONE RS232_MASK(0)
#define RS232_M_PARITY_ODD RS232_MASK(1)
#define RS232_M_PARITY_EVEN RS232_MASK(2)
#define RS232_M_PARITY_MARK RS232_MASK(3)
#define RS232_M_PARITY_SPACE RS232_MASK(4)

//stopbits
typedef enum {
	RS232_STOPBITS_1 = ONESTOPBIT,
	RS232_STOPBITS_2 = TWOSTOPBITS,
} RS232_STOPBITS;
#define RS232_STOPBITS_MAX 2
#define RS232_STOPBITS_DEFAULT RS232_STOPBITS_1

#define RS232_M_STOPBITS_1 RS232_MASK(0)
#define RS232_M_STOPBITS_2 RS232_MASK(1)

//handshake
typedef enum {
	RS232_HANDSHAKE_NONE = 0,
	RS232_HANDSHAKE_CTS_RTS = 1,
	RS232_HANDSHAKE_DSR_DTR = 2,
	RS232_HANDSHAKE_BOTH = 3,
} RS232_HANDSHAKE;
#define RS232_HANDSHAKE_MAX 4
#define RS232_HANDSHAKE_DEFAULT RS232_HANDSHAKE_NONE

#define RS232_M_HANDSHAKE_NONE RS232_MASK(0)
#define RS232_M_HANDSHAKE_CTS_RTS RS232_MASK(1)
#define RS232_M_HANDSHAKE_DSR_DTR RS232_MASK(2)
#define RS232_M_HANDSHAKE_BOTH RS232_MASK(3)

//in/out fifo size
#define RS232_INSIZE_DEFAULT 1024
#define RS232_OUTSIZE_DEFAULT 512

//---------
//rs232 parameters
typedef struct {
	int portno;
	RS232_BAUDRATE baudrate;
	RS232_DATABITS databits;
	RS232_PARITY parity;
	RS232_STOPBITS stopbits;
	RS232_HANDSHAKE handshake;
	unsigned int insize;
	unsigned int outsize;
} RS232_PARAMS;

//rs232 enable flags
typedef struct {
	unsigned int en_baudrate;
	unsigned int en_databits;
	unsigned int en_parity;
	unsigned int en_stopbits;
	unsigned int en_handshake;
} RS232_FLAGS;


/* function declarations */

//port manipulation
BOOL RS232_TestPort(const TCHAR *PortName);
BOOL RS232_TestPortNo(int PortNo);
//
HANDLE RS232_OpenPort(const TCHAR *PortName);
HANDLE RS232_OpenConfPortNo(int PortNo,
														RS232_BAUDRATE baudrate,
														RS232_DATABITS databits,
														RS232_PARITY parity,
														RS232_STOPBITS stopbits,
														RS232_HANDSHAKE handshake,
														unsigned int inbuf, unsigned int outbuf);
BOOL RS232_ClosePort(HANDLE hPort);
BOOL RS232_IsPortOpened(HANDLE hPort);

//configuration
DWORD RS232_SetPortConf(HANDLE hPort,
												RS232_BAUDRATE baudrate,
												RS232_DATABITS databits,
												RS232_PARITY parity,
												RS232_STOPBITS stopbits,
												RS232_HANDSHAKE handshake,
												unsigned int inbuf, unsigned int outbuf);
DWORD RS232_GetPortConf(HANDLE hPort,
												RS232_BAUDRATE *baudrate,
												RS232_DATABITS *databits,
												RS232_PARITY *parity,
												RS232_STOPBITS *stopbits,
												RS232_HANDSHAKE *handshake);
DWORD RS232_SetPortTimeout(HANDLE hPort, unsigned int ms_timeout);	//do not use!!!
DWORD RS232_GetPortTimeout(HANDLE hPort, unsigned int *ms_timeout);

//queues
DWORD RS232_GetBytesInQueue(HANDLE hPort, unsigned int *bytes);
DWORD RS232_Flush(HANDLE hPort);

//direct line control (no handshake on the line)
BOOL RS232_SetDTRLine(HANDLE hPort, BOOL state);
BOOL RS232_SetRTSLine(HANDLE hPort, BOOL state);

//read and write
DWORD RS232_Write(HANDLE hPort, char *wbuf, unsigned int wlen, unsigned int *bwritten);
DWORD RS232_ReadIm(HANDLE hPort, char *rbuf, unsigned int rlen, unsigned int *bread);
int RS232_CanRead(HANDLE hPort);		//0=no data, -1=error, >0=data available

/* listing comports (enumser.zip)  */



//useful
unsigned int RS232_GetBaudrateMask(RS232_BAUDRATE baudrate);
unsigned int RS232_GetDatabitsMask(RS232_DATABITS databits);
unsigned int RS232_GetParityMask(RS232_PARITY parity);
unsigned int RS232_GetStopbitsMask(RS232_STOPBITS stopbits);
unsigned int RS232_GetHandshakeMask(RS232_HANDSHAKE handshake);
//
unsigned int RS232_FIRSTMASK(unsigned int mask);
RS232_BAUDRATE RS232_GetBaudrateByMask(unsigned int en_mask);
RS232_DATABITS RS232_GetDatabitsByMask(unsigned int en_mask);
RS232_PARITY RS232_GetParityByMask(unsigned int en_mask);
RS232_STOPBITS RS232_GetStopbitsByMask(unsigned int en_mask);
RS232_HANDSHAKE RS232_GetHandshakeByMask(unsigned int en_mask);
//
BOOL RS232_SetFlags(RS232_PARAMS *pars, RS232_FLAGS *flags);
//
int RS232_IsMultipleMask(unsigned int en_mask);
int RS232_GetCBListIndex(unsigned int val_mask, unsigned int en_mask);
unsigned int RS232_GetCBListValMask(int idx, unsigned int en_mask);


//validation
BOOL RS232_ValidateBaudrate(unsigned int en_mask, RS232_BAUDRATE baudrate);
BOOL RS232_ValidateDatabits(unsigned int en_mask, RS232_DATABITS databits);
BOOL RS232_ValidateParity(unsigned int en_mask, RS232_PARITY parity);
BOOL RS232_ValidateStopbits(unsigned int en_mask, RS232_STOPBITS stopbits);
BOOL RS232_ValidateHandshake(unsigned int en_mask, RS232_HANDSHAKE handshake);

//strings
const TCHAR *RS232_GetParityString(RS232_PARITY parity);
const TCHAR *RS232_GetParityAbbrString(RS232_PARITY parity);
const TCHAR *RS232_GetStopbitString(RS232_STOPBITS stopbits);


//testing purpose
DWORD RS232_Testing(void);




#ifdef __cplusplus
}
#endif

#endif	/* end of _COM_RS232_H_ */
