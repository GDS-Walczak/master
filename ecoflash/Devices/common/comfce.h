/*
 * comfce.h
 *
 * Communication Functions - header file
 *
 * Author: Filip Kinovic
 * Version: 2.1
 * Date: 30.06.2016
*/

#ifndef _COMFCE_H_
#define _COMFCE_H_

#include <windows.h>
#include "list.h"
#include "com_rs232.h"
//#include "com_eth.h"


#define COM_LOGGING
//memory logging
//#define COM_MEMLOGGING


/* constants */
#define COM_MAX_USERID_LEN 64
#define COM_MIN_WBUF_LEN 512
#define COM_MIN_RBUF_LEN 1024

#define COM_MAX_IP_LEN ETH_MAX_IP_LEN
#define COM_MAX_MACADDR_LEN ETH_MAX_MACADDR_LEN


#ifdef __cplusplus
extern "C" {
#endif

/* type definitions */

typedef enum {
	COM_MODE_RS232 = 0,
	//COM_MODE_ETHERNET = 1,
	COM_MODE_USERID = 10,
} COM_MODE;

typedef struct {
	unsigned int en_rs232 : 1;
	//unsigned int en_eth : 1;
	unsigned int en_user : 1;
	//
	unsigned int en_timeout;
} COM_FLAGS;

typedef enum {
	COM_LOGFORMAT_BINARY = 0,
	COM_LOGFORMAT_TEXT = 1,
} COM_LOGFORMAT;

//communication functions
typedef DWORD (*COM_IWRITEFCE)(HANDLE h, char *wbuf, unsigned int wlen, unsigned int *bwritten);
typedef DWORD (*COM_IREADFCE)(HANDLE h, char *rbuf, unsigned int rlen, unsigned int *bread);
typedef DWORD (*COM_CANREADFCE)(HANDLE h);
typedef DWORD (*COM_FLUSHFCE)(HANDLE h);
//
typedef DWORD (*COM_SIMULCE)(void);


//communication data type
typedef struct {
	//basic parameters (select periphery)
	COM_MODE mode;		//RS232, ethernet, ...
	//static buffers
	char sendbuff[COM_MIN_WBUF_LEN+4];
	char rcptbuff[COM_MIN_RBUF_LEN+4];
	//dynamic buffers
	int alloc;
	unsigned int sendbufsize;
	unsigned int rcptbufsize;
	char *sendbuf;
	char *rcptbuf;
	//general parameters
	unsigned int timeout;		//reception timeout (storable)
	int repeat;
	int terminal_char;
	COM_FLAGS com_flags;
	COM_LOGFORMAT log_format;		//0=binary (hex), 1=text (C-string)
	//rs232 parameters
	RS232_PARAMS ser_pars;		//storable
	RS232_FLAGS ser_flags;
	//ethernet parameters
	ETH_PARAMS eth_pars;				//storable
	ETH_FLAGS eth_flags;
	//user ID parameters
	TCHAR userid[COM_MAX_USERID_LEN];		//storable
	//---
	//handle (file handle, socket, ...)
	HANDLE hComm;		//connection handle (port handle, socket handle)
	//errors
	DWORD errs;		//timeout errors (no response)
  //protocol handle (data)
  HANDLE hProt;
  DWORD var;
  COM_SIMULCE SimulProc;
  LPARAM lparam;
} COMH;


/* macros */
#define COM_Sleep() Sleep(2)
//#define COM_Sleep() (void)0


#define COM_SetError(hC, ret) (((ret)==NO_ERROR) ? (hC)->errs = 0 : (hC)->errs++)


/* function declarations */

//init/deinit (WSA)
BOOL COM_Init(void);
BOOL COM_DeInit(void);

//setup
DWORD COM_Reset(COMH *hC);
DWORD COM_Setup(COMH *hC, COM_MODE mode, COM_LOGFORMAT format, unsigned int wbufsize, unsigned rbufsize);
//DWORD COM_SetTimeout(COMH *hC, unsigned int timeout, unsigned int old_timeout);

//buffers
DWORD COM_CreateBuffers(COMH *hC);
DWORD COM_DestroyBuffers(COMH *hC);

//open/close and configure connection
DWORD COM_OpenAndConf(COMH *hC);
BOOL COM_Close(COMH *hC);
BOOL COM_IsOpened(COMH *hC);
DWORD COM_ReOpen(COMH *hC);


//search ports (api)



//immediate read and write (Raw=not logged, not count errors)
DWORD COM_WriteRaw(COMH *hC, char *wbuf, unsigned int wlen, unsigned int *bwritten);
DWORD COM_IReadRaw(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread);
int COM_CanRead(COMH *hC);		//0=no data, -1=error, >0=data available
DWORD COM_Flush(COMH *hC);

//advanced read and write (Raw=not logged, not count errors)
DWORD COM_ReadRaw(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread, unsigned int ms_timeout);
DWORD COM_TerminatedReadRaw(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread, unsigned int ms_timeout);
DWORD COM_EmptyReadRaw(COMH *hC, char *rbuf, unsigned int rlen);

//advanced read and write (Raw=not logged, not count errors)
DWORD COM_Write(COMH *hC, char *wbuf, unsigned int wlen, unsigned int *bwritten);
DWORD COM_Read(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread, unsigned int ms_timeout);
#define COM_ReadT(hC, rbuf, rlen, bread) COM_Read(hC, rbuf, rlen, bread, (hC)->timeout)
DWORD COM_TerminatedRead(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread, unsigned int ms_timeout);

//read and write using internal buffers (Raw=not logged, not count errors)
DWORD COM_Empty(COMH *hC);
DWORD COM_Send(COMH *hC, unsigned int wlen);
DWORD COM_SendAndRcpt(COMH *hC, unsigned int wlen, unsigned int *bread);		//+flush
DWORD COM_SendStr(COMH *hC);
DWORD COM_SendAndRcptStr(COMH *hC);		//+flush when something to send
DWORD COM_SendAndRcptStr2(COMH *hC);	//+flush when something to send, eliminater terminal char.
DWORD COM_EmptyRcptRaw(COMH *hC);


//------- logging -------

//memory logging
typedef enum {
	COM_MLOGMODE_OPEN = 'o',
	COM_MLOGMODE_CLOSE = 'c',
	COM_MLOGMODE_SETUP = 's',
	COM_MLOGMODE_READ = 'r',
	COM_MLOGMODE_WRITE = 'w',
	COM_MLOGMODE_INFO = 'i',
	COM_MLOGMODE_CLEAR = 'e',
	COM_MLOGMODE_BREAK = 'b',
	COM_MLOGMODE_PARAM = 'p',
} COM_MLOGMODE;

typedef struct {
	DWORD index;
	TCHAR portname[COM_MAX_USERID_LEN];
	SYSTEMTIME st;
	DWORD result;
	COM_MLOGMODE mode;
	DWORD size;
	char *data;
} COMMLOGITEM;

typedef struct {
	HLIST list;
	DWORD idx;
	BOOL enable;
} COMMLOG;

BOOL COM_MemLogCreate(void);
BOOL COM_MemLogDestroy(void);
BOOL COM_MemLogReset(int reset_counter);
BOOL COM_MemLogEnable(BOOL enable);
BOOL COM_MemLogWriteToFile(const TCHAR *filename, const TCHAR *appname, int c_format);
BOOL COM_MemLogStore(COMH *hC, COM_MLOGMODE mode, DWORD ret, char *buf, unsigned int size);

HLIST COM_GetLogList();

//direct file logging
void COM_LogData(COMH *hC, DWORD ret, char *wbuf, unsigned int wlen, char *rbuf, unsigned int rlen);
#define COM_LogText(hc, ret) COM_LogData((hc), (ret), (hc)->sendbuff, strlen((hc)->sendbuff), (hc)->rcptbuff, strlen((hc)->rcptbuff))




//--------------
//testing purpose
DWORD COM_Testing(void);

//useful
BOOL COM_MathResponse2(const char *rcpt, const char *mask);





#ifdef __cplusplus
}
#endif

#endif		/* end of _COMFCE_H_ */

