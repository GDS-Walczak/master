/*
 * b2_protocol.h
 *
 * B2 Protocol - header file
 *
 * Author: Filip Kinovic
 * Version: 1.0
 * Date: 29.04.2015
*/

#ifndef _B2PROTOCOL_H_
#define _B2PROTOCOL_H_

#include <windows.h>
#include <stdint.h>
#include "../../chdevice.h"

/* constants */
#define B2PROT_CRC_BYTES 4
#define B2PROT_DATA_BYTES 16384
#define B2PROT_BYTES(n) (sizeof(B2PROT_PACHEAD)+B2PROT_CRC_BYTES+(n))
#define B2PROT_MIN_BYTES B2PROT_BYTES(0)
#define B2PROT_MAX_BYTES B2PROT_BYTES(B2PROT_DATA_BYTES)
#define B2PROT_MAX_PACKET_SIZE 4096

#define B2PROT_PRECMD 0xF1CD
#define B2PROT_PRERCV 0x98E1

#define B2PROT_PRECMD_1 (B2PROT_PRECMD & 0xFF)
#define B2PROT_PRECMD_2 (B2PROT_PRECMD >> 8)
#define B2PROT_PRERCV_1 (B2PROT_PRERCV & 0xFF)
#define B2PROT_PRERCV_2 (B2PROT_PRERCV >> 8)

#define B2PROT_SEARCHCHAR0 B2PROT_PRERCV_1
#define B2PROT_SEARCHCHAR1 B2PROT_PRERCV_2

/* errors */
#define B2PROT_ERROR_COMMAND 			0x8000
#define B2PROT_ERROR_BUSY 				0x8001
#define B2PROT_ERROR_SIZE 				0x8002
#define B2PROT_ERROR_VALUE 				0x8003
#define B2PROT_ERROR_MODE 				0x8004
#define B2PROT_ERROR_PENDING			0x8005
#define B2PROT_ERROR_FAILED 			0x8006
#define B2PROT_ERROR_LOCKED 			0x8007
//
#define B2PROT_ERROR_USER 				0x8100

/* general commands */
#define B2PROT_CMD_GET_INFO 				0x0000
#define B2PROT_CMD_GET_ASPEC				0x0004
#define B2PROT_CMD_SET_ASPEC				0x0005
#define B2PROT_CMD_GET_USPEC				0x0006
#define B2PROT_CMD_SET_USPEC				0x0007
#define B2PROT_CMD_GET_BRDCOUNT 		0x0010
#define B2PROT_CMD_GET_BRDINFO 			0x0012
#define B2PROT_CMD_GET_ETHCFG				0x0020
#define B2PROT_CMD_SET_ETHCFG 			0x0021
#define B2PROT_CMD_GET_ETHINFO			0x0022
#define B2PROT_CMD_GET_LOGCFG	 			0x0030
#define B2PROT_CMD_SET_LOGCFG	 			0x0031
#define B2PROT_CMD_GET_LOGCOUNT 		0x0032
#define B2PROT_CMD_GET_LOGDATA 			0x0034
#define B2PROT_CMD_GET_CMPCOUNT 		0x0036
#define B2PROT_CMD_GET_CMPDATA 			0x0038
#define B2PROT_CMD_GET_REPORTCOUNT 	0x0040
#define B2PROT_CMD_GET_REPORTDATA 	0x0042
#define B2PROT_CMD_MAKE_BEEP 				0x0051
#define B2PROT_CMD_STOP_SOUND 			0x0053

//function returns
#define B2PROT_RET_OK					0		//ok
#define B2PROT_RET_SIZE				1		//small size
#define B2PROT_RET_INVALID		-1	//invalid (failed)




#ifdef __cplusplus
extern "C" {
#endif

/* structures */

/* protocol constants */

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

/* packet*/
typedef struct {
	unsigned short pre;
	unsigned short var;
	unsigned short size;
	unsigned short cmd;
} B2PROT_PACHEAD;

typedef struct {
	B2PROT_PACHEAD head;
	unsigned char data[B2PROT_DATA_BYTES+B2PROT_CRC_BYTES];
} B2PROT_PACKET;

/* packet V2 (minor usage) */
typedef struct {
	unsigned int pre;
	unsigned short size;
	unsigned short cmd;
} B2PROT_PACHEAD_V2;

typedef struct {
	B2PROT_PACHEAD_V2 head;
	unsigned char data[B2PROT_DATA_BYTES+B2PROT_CRC_BYTES];
} B2PROT_PACKET_V2;

/* general command structures */
#define B2PROT_ST_INFO_NAME_LEN 16
#define B2PROT_ST_INFO_MODEL_LEN 4
#define B2PROT_ST_INFO_OEM_LEN 16
#define B2PROT_ST_INFO_SN_LEN 8
#define B2PROT_ST_INFO_SDATA_LEN 12
typedef struct {
  char name[B2PROT_ST_INFO_NAME_LEN];
  char model[B2PROT_ST_INFO_MODEL_LEN];
  char oem[B2PROT_ST_INFO_OEM_LEN];
  char sn[B2PROT_ST_INFO_SN_LEN];
  uint16_t fw_ver;
  uint16_t fw_year;
  uint8_t fw_month;
  uint8_t fw_day;
  uint16_t res;
  uint8_t sdata[B2PROT_ST_INFO_SDATA_LEN];
} B2PROT_ST_INFO;   //64B

#define B2PROT_ST_SPEC_LEN 20
#define B2PROT_ST_USPEC_LEN B2PROT_ST_SPEC_LEN
typedef struct {
  char data[B2PROT_ST_USPEC_LEN];
} B2PROT_ST_USPEC;   //20B

#define B2PROT_ST_ASPEC_LEN B2PROT_ST_SPEC_LEN
typedef struct {
  char data[B2PROT_ST_ASPEC_LEN];
} B2PROT_ST_ASPEC;   //20B

typedef struct {
	uint16_t count;		//count/index
	uint16_t res;
} B2PROT_ST_BRDCOUNT;  //4B

#define B2PROT_ST_BRDINFO_NAME_LEN 16
#define B2PROT_ST_BRDINFO_UID_LEN 28
#define B2PROT_ST_BRDINFO_SDATA_LEN 8
typedef struct {
	char name[B2PROT_ST_BRDINFO_NAME_LEN]; // Name text string
	char uid[B2PROT_ST_BRDINFO_UID_LEN]; // Unique ID text string
	uint16_t fw_ver; // Firmware version
	uint16_t fw_year; // Firmware date as year, month, day
	uint8_t fw_month;
	uint8_t fw_day;
	uint16_t revision; // Board revision
	char sdata[B2PROT_ST_BRDINFO_SDATA_LEN]; // Board Specific data (Reserve)
} B2PROT_ST_BRDINFO;   //60B

typedef struct {
/*
	uint8_t link;	//0=down, 1=up
	uint8_t conn;	//0=static, 1=dhcp, 2=autoip
	uint8_t tcp_conn;	//0=none, 1=established
	uint8_t reserve;
	uint8_t ip[4];
	uint8_t netmask[4];
	uint8_t gateway[4];
	uint8_t mac[18];
*/
	uint8_t netlink;  //0=down, 1=up
	uint8_t netstate;  //0=static, 1=DHCP, 2=AutoIP, -1=not established
	uint8_t res1[2];
	uint8_t ip[4];
	uint8_t netmask[4];
	uint8_t gateway[4];
  uint8_t mac[6];
	uint16_t enflags;  //flags
  uint8_t res2[8];
} B2PROT_ST_ETHINFO;   //32B

//error + warning
#define B2PROT_ERROR_BYTES 8
#define B2PROT_ERROR_DWORDS (B2PROT_ERROR_BYTES/4)
typedef union {
	//uint8_t bCode[B2PROT_ERROR_BYTES];
	uint32_t dwCode[B2PROT_ERROR_DWORDS];
} B2PROT_ERRVAL;

//event
#define B2PROT_EVENT_BYTES 4
#define B2PROT_EVENT_DWORDS (B2PROT_EVENT_BYTES/4)
typedef union {
	//uint8_t bCode[B2PROT_EVENT_BYTES];
	uint32_t dwCode;
} B2PROT_EVENTVAL;

//flags
#define B2PROT_FLAGS_BYTES 4
#define B2PROT_FLAGS_DWORDS (B2PROT_FLAGS_BYTES/4)
typedef union {
	//uint8_t bCode[B2PROT_FLAGS_BYTES];
	uint32_t dwCode;
} B2PROT_FLAGS;

//log
typedef struct {
	uint16_t count; 	// Number of registered records in log
	uint16_t res;
} B2PROT_ST_LOGCOUNT;

typedef struct {
	uint16_t index; 	// record start index
	uint16_t length; 	// number of records
} B2PROT_ST_LOGSIZE;

typedef struct {
	uint16_t rec_id; 			// Record identification number
	//
	uint8_t rtc_yea;			// RTC date/time (year without offset 2000)
	uint8_t rtc_mon;
	uint8_t rtc_day;
	uint8_t rtc_hrs;
	uint8_t rtc_min;
	uint8_t rtc_sec;
	//
	uint32_t age_time_sec; 		// age time
	uint32_t run_time_sec; 		// run time
	uint16_t run_time_msec;		// run time
	uint8_t prev_state; 	// Previous state of device
	uint8_t state; 				// Actual state of device
	//
	uint8_t pri_sta;                                // Status of primary lamp
	uint8_t sec_sta;                                // Status of secondary lamp
	uint8_t res[6];																	// Reserves
	//
	B2PROT_ERRVAL errors;
	B2PROT_ERRVAL warnings;
	B2PROT_EVENTVAL events;
	B2PROT_FLAGS flags;
} B2PROT_ST_LOGITEM;		//52B

#define B2PROT_LOGITEM_MAX_SIZE 8 // Limit number of records send one time
typedef struct {
	B2PROT_ST_LOGSIZE size;
	B2PROT_ST_LOGITEM item[B2PROT_LOGITEM_MAX_SIZE]; // Record logging structure array
} B2PROT_ST_LOGDATA;
#define B2PROT_LOGDATA_DATABYTES (sizeof(B2PROT_ST_LOGITEM)*B2PROT_LOGITEM_MAX_SIZE)
#define B2PROT_LOGDATA_MINBYTES (sizeof(B2PROT_ST_LOGDATA)-B2PROT_LOGDATA_DATABYTES)

//component log
typedef struct {
	uint16_t count; 	// Number of registered records in log
	uint16_t res;
} B2PROT_ST_CMPCOUNT;

typedef struct {
	uint16_t index; 	// record start index
	uint16_t length; 	// number of records
} B2PROT_ST_CMPSIZE;

#define B2PROT_CMPTEXT_LEN 164
typedef struct {
	uint16_t rec_id; 									// Record identification number
	//
	uint8_t rtc_yea;			// RTC date/time (year without offset 2000)
	uint8_t rtc_mon;
	uint8_t rtc_day;
	uint8_t rtc_hrs;
	uint8_t rtc_min;
	uint8_t rtc_sec;
	//
	uint32_t age_time_sec; 		// age time
	uint8_t res[10];
	uint16_t operation;							// Operation code
	char text[B2PROT_CMPTEXT_LEN]; 	// Text
} B2PROT_ST_CMPITEM;	//188B

#define B2PROT_CMPITEM_MAX_SIZE 4 // Limit number of records send one time
typedef struct {
	B2PROT_ST_CMPSIZE size;
	B2PROT_ST_CMPITEM item[B2PROT_CMPITEM_MAX_SIZE]; // Record logging structure array
} B2PROT_ST_CMPDATA;
#define B2PROT_CMPDATA_DATABYTES (sizeof(B2PROT_ST_CMPITEM)*B2PROT_CMPITEM_MAX_SIZE)
#define B2PROT_CMPDATA_MINBYTES (sizeof(B2PROT_ST_CMPDATA)-B2PROT_CMPDATA_DATABYTES)

typedef enum {
	B2PROT_BEEPIDX_STOP = 0,
	B2PROT_BEEPIDX_KEYS = 1,
	B2PROT_BEEPIDX_SHORT = 2,
	B2PROT_BEEPIDX_DONE = 3,
	B2PROT_BEEPIDX_FAILED = 4
} B2PROT_BEEPIDX;

typedef struct {
	uint16_t code;			//0=stop, 1=2400Hz/2ms, 2=2400Hz/50ms, 3=2400Hz/200ms, 4=550Hz/400ms
	uint16_t res;
} B2PROT_ST_BEEP;


#pragma pack(pop)   /* restore original alignment from stack */

//
typedef DWORD (*B2PROT_EMPTYPROC)(COMH *hC, B2PROT_PACKET *pac);
typedef DWORD (*B2PROT_WRITECMDPROC)(COMH *hC, B2PROT_PACKET *pac, unsigned short var, unsigned short cmd, unsigned short data_len);
typedef DWORD (*B2PROT_READCMDPROC)(COMH *hC, B2PROT_PACKET *pac, unsigned short *data_len, unsigned int timeout);
//
typedef DWORD (*B2PROT_SIMULPROC)(COMH *hC, unsigned short cmd, unsigned short wlen, unsigned short *rlen);

typedef enum {
	B2PROT_LOGSPECTYPE_STATUS = 0,
	B2PROT_LOGSPECTYPE_ERROR = 1,
	B2PROT_LOGSPECTYPE_WARNING = 2,
	B2PROT_LOGSPECTYPE_LOGDESC = 3
} B2PROT_LOGSPECTYPE;
typedef BOOL (*B2PROT_LOGSPECFUNC)(HSTRING hs, B2PROT_LOGSPECTYPE type, int par, B2PROT_ST_LOGITEM *pitem);
BOOL B2PROT_LogSpecFunction(HSTRING hs, B2PROT_LOGSPECTYPE type, int par, B2PROT_ST_LOGITEM *pitem);

typedef struct {
	unsigned short var;
	B2PROT_PACKET wdata;
	B2PROT_PACKET rdata;
	//communication fce
	B2PROT_EMPTYPROC EmptyReadProc;
	B2PROT_WRITECMDPROC WriteCommandProc;
	B2PROT_READCMDPROC ReadCommandProc;
	//users
	B2PROT_SIMULPROC simul_proc;
	unsigned int *param;
} B2PROTST;

/* internal functions */
unsigned int B2PROT_crccalc(unsigned char *buf, unsigned int size);
//
DWORD B2PROT_EmptyRead(COMH *hC, B2PROT_PACKET *pac);
DWORD B2PROT_WriteCommand(COMH *hC, B2PROT_PACKET *pac, unsigned short var, unsigned short cmd, unsigned short data_len);
DWORD B2PROT_ReadCommand(COMH *hC, B2PROT_PACKET *pac, unsigned short *data_len, unsigned int timeout);
int B2PROT_CheckReplyPacket(B2PROT_PACKET *pac, unsigned short max_len, unsigned short max_head_size);
unsigned int B2PROT_CalcCRC(unsigned char *buf, unsigned int len);

/* basic functions */
BOOL B2PROT_Create(COMH *hC);
BOOL B2PROT_Destroy(COMH *hC);
//
char *B2PROT_GetWriteBuf(COMH *hC);
char *B2PROT_GetReadBuf(COMH *hC);
DWORD B2PROT_CmdSendAndRcpt(COMH *hC, unsigned short cmd, unsigned short wlen, unsigned short *rlen);
DWORD B2PROT_CmdSendOnly(COMH *hC, unsigned short cmd);
//
BOOL B2PROT_SetSimulFunction(COMH *hC, B2PROT_SIMULPROC simul_fce);

//---
unsigned int B2PROT_SearchERVINx(HDEVICE hD, HLIST hlist, TCHAR *wild_filter, int test_id, char *ervin_name);
#define B2PROT_SearchERVIN(hD, hlist, wild_filter, test_id) B2PROT_SearchERVINx(hD, hlist, wild_filter, test_id, NULL)
#define B2PROT_SearchERVIN7(hD, hlist, wild_filter, test_id) B2PROT_SearchERVINx(hD, hlist, wild_filter, test_id, "ERVIN7")

/*

//swap functions

void B2PROT_rev16(void *val);
void B2PROT_rev32(void *val);


//commands
DWORD B2PROT_ReadAdvanceSpec(HDEVICE hD, WORD pidx, TCHAR *aspec, int max_len);
DWORD B2PROT_ReadUserSpec(HDEVICE hD, WORD pidx, TCHAR *uspec, int max_len);
DWORD B2PROT_WriteUserSpec(HDEVICE hD, WORD pidx, TCHAR *uspec, int max_len);
DWORD B2PROT_SetupUserSpec(HWND hdlg, HDEVICE hD, WORD pidx);

DWORD B2PROT_CmdReadLogPartData(HDEVICE hD, HLIST hlog, int min_items, int start);
DWORD B2PROT_CmdReadCmpPartData(HDEVICE hD, HLIST hcmp, int min_items, int start);

DWORD B2PROT_CmdStopSound(HDEVICE hD);
DWORD B2PROT_CmdMakeBeep(HDEVICE hD, unsigned int uval);
*/

//
HLIST B2PROT_CreateLogList(void);
HLIST B2PROT_CreateCmpList(void);
BOOL B2PROT_DestroyList(HLIST *hlist);

BOOL B2PROT_CreateInfoFile(D_DEVICE *pD, HSTRING hs, const int *indexes);
BOOL B2PROT_CreateLogFile(D_DEVICE *pD, HSTRING hs, HLIST hlist);
BOOL B2PROT_CreateCmpFile(D_DEVICE *pD, HSTRING hs, HLIST hlist);
BOOL B2PROT_CreateLogCmpFile(D_DEVICE *pD, HSTRING hs, HLIST hlog, HLIST hcmp, int options, B2PROT_LOGSPECFUNC fspec);
BOOL B2PROT_TitleToRTF(HSTRING hs, D_DEVICE *pD, TCHAR *title, int count);
int B2PROT_ErrorsToRTF(HSTRING hs, B2PROT_ERRVAL *err_val, unsigned int max_errors);
int B2PROT_EventsToRTF(HSTRING hs, B2PROT_EVENTVAL *event_val, unsigned int max_events);

BOOL B2PROT_IsErrorNo(B2PROT_ERRVAL *err_val, int error_no);
#define B2PROT_MAX_SHOWNERRS 5
int B2PROT_DecodeErrorNumbers(B2PROT_ERRVAL *err_val, TCHAR *str, unsigned int max_codes);
BOOL B2PROT_ErrorsDecodeBox(HWND hwnd, B2PROT_ERRVAL *err_val, const TCHAR *title, const TCHAR *descr_fce(int));
int B2PROT_GetTopError(B2PROT_ERRVAL *err_val);
unsigned int B2PROT_AppendDecodedErrors(HSTRING hs, B2PROT_ERRVAL *err_val, const TCHAR *descr_fce(int));

DWORD B2PROT_ReadUserSpec(HDEVICE hD, WORD pidx, TCHAR *uspec, int max_len);
DWORD B2PROT_WriteUserSpec(HDEVICE hD, WORD pidx, TCHAR *uspec, int max_len);
DWORD B2PROT_SetupUserSpec(HWND hdlg, HDEVICE hD, WORD pidx);



#define B2PROT_DLG_BW 100
#define B2PROT_DLG_BH 22


#ifdef __cplusplus
}
#endif

#endif	/* end of _B2PROTOCOL_H_ */


