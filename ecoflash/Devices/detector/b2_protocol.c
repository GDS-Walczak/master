/*
 * b2_protocol.c
 *
 * B2 Protocol - source file
 *
 * Author: Filip Kinovic
 * Version: 1.4
 * Date: 19.05.2016
*/

#define WINVER 0x0400
#define _WIN32_IE 0x0500

#define B2PROT_LOG_ENABLE
#ifdef DEBUG
	//#define B2PROT_SIMUL
#endif

#include <windows.h>
#include <assert.h>
#include <process.h>
#include "../../crc32.h"
#include "../../charconv.h"
#include "../../useful.h"
//#include "../../diag.h"
#include "b2_protocol.h"


/* extern variables */
//logging
extern BOOL COM_log_enable;
extern TCHAR COM_log_path[MAX_PATH];

/* global variables */
static CRC32_ST g_crcst = {0};



//----------- Common -----------------------

/* calculate data stmcrc  */
unsigned int B2PROT_crccalc(unsigned char *buf, unsigned int size)
{
	return(crc32_b2_block(&g_crcst, buf, size));
}


//*** function empties reception stream
DWORD B2PROT_EmptyRead(COMH *hC, B2PROT_PACKET *pac)
{
	//test pointers
	if (hC == NULL || pac == NULL)
		return(ERROR_INVALID_HANDLE);

	//empty data
	return(COM_EmptyReadRaw(hC, (char *)pac, sizeof(B2PROT_PACKET)));
}

//*** function writes command
DWORD B2PROT_WriteCommand(COMH *hC, B2PROT_PACKET *pac, unsigned short var, unsigned short cmd, unsigned short data_len)
{
	unsigned int n, crc, *pval;
	DWORD ret = NO_ERROR;

	//test pointers
	if (hC == NULL || pac == NULL)
		return(ERROR_INVALID_HANDLE);

	//create packet
	pac->head.pre = B2PROT_PRECMD;
	pac->head.var = var;
	pac->head.size = data_len;
	pac->head.cmd = cmd;
	//cal. crc
	crc = B2PROT_crccalc((unsigned char *)pac, sizeof(B2PROT_PACHEAD)+data_len);
	pval = (unsigned int*)(pac->data + data_len);
	*pval = crc;
	data_len += sizeof(B2PROT_PACHEAD) + B2PROT_CRC_BYTES;

	//send
	ret = COM_WriteRaw(hC, (char *)pac, data_len, &n);
	if (ret != NO_ERROR) {
		ret = ERROR_TIMEOUT;
	}

#ifdef B2PROT_LOG_ENABLE
	if (COM_log_enable)
		COM_LogData(hC, ret, (char *)pac, n, NULL, 0);
#endif

	return(NO_ERROR);
}

//*** function reads command response
DWORD B2PROT_ReadCommand(COMH *hC, B2PROT_PACKET *pac, unsigned short *data_len, unsigned int timeout)
{
	unsigned int i, n, dn, crc;
	unsigned char *pbuf, *pt;
	DWORD ret;
	double t0, t;

	//test pointers
	if (hC == NULL || pac == NULL)
		return(ERROR_INVALID_HANDLE);

	//wait for response
	ret = ERROR_TIMEOUT;
	n = 0;	//total bytes
	dn = 0;	//data size
	pbuf = (unsigned char *)pac;
	t0 = timer_GetTime();
	while (((t = timer_GetTime())-t0) < (double)timeout) {
		//test if any data
		i = COM_CanRead(hC);
		if (i < 0)
			break;	//error
		//any data
		if (i > 0) {
			//read available data
			if (COM_IReadRaw(hC, (char *)pbuf+n, sizeof(B2PROT_PACKET)-n, &i) != NO_ERROR || i == 0)
				return(ERROR_TIMEOUT);
			n += i;
			//test for complete frame
			if (n >= B2PROT_BYTES(dn)) {
				//check parameters
				if (pac->head.pre != B2PROT_PRERCV) {
					//search for new beginning
					pt = memchr(pbuf+1, B2PROT_SEARCHCHAR0, n-1);
					if (pt) {
						i = pt-pbuf;
#ifdef B2PROT_LOG_ENABLE
						if (COM_log_enable)
							COM_LogData(hC, ERROR_INVALID_BLOCK, NULL, 0, (char *)pbuf, i);
#endif
						memcpy(pbuf, pt, n-i);
						n -= i;
					}
					else {
#ifdef B2PROT_LOG_ENABLE
						if (COM_log_enable)
							COM_LogData(hC, ERROR_INVALID_BLOCK, NULL, 0, (char *)pbuf, n);
#endif
						n = 0;
					}
					if (n < B2PROT_BYTES(dn))
						continue;	//not enough data
				}

				//check max. size
				dn = pac->head.size;
				if (dn > B2PROT_DATA_BYTES) {
					ret = ERROR_INVALID_BLOCK;
					break;
				}
				//check for complete data
				if (n < B2PROT_BYTES(dn))
					continue;	//not enough data

				ret = NO_ERROR;
				break;
			}
		}
		else
			Sleep(2);
	}

#ifdef B2PROT_LOG_ENABLE
	if (COM_log_enable)
		COM_LogData(hC, ret, NULL, 0, (char *)pac, n);
#endif

	//test for block error
	if (ret != NO_ERROR) {
		return(ret);
	}

	//cal. crc
	crc = B2PROT_crccalc((unsigned char *)pac, sizeof(B2PROT_PACHEAD)+dn);
	pbuf = pac->data + dn;
	//check crc
	n = *(unsigned int *)pbuf;
	if (n != crc)
		return(ERROR_CRC);

	if (data_len)
		*data_len = dn;

	return(NO_ERROR);
}

/** @brief Check response packet
 *
 * @param pac B2PROT_PACKET*
 * @param max_len unsigned short
 * @return int (0=ok, 1=small size, -1=invalid packet)
 *
 */
int B2PROT_CheckReplyPacket(B2PROT_PACKET *pac, unsigned short pac_len, unsigned short max_head_size)
{
	unsigned short n;
	unsigned int crc, *pval;

	if (pac == NULL)
		return(B2PROT_RET_INVALID);

	//min. size
	if (pac_len < B2PROT_MIN_BYTES)
		return(B2PROT_RET_SIZE);
	//test preamble
	if (pac->head.pre != B2PROT_PRERCV)
		return(B2PROT_RET_INVALID);
	//test size
	n = pac->head.size;
	if (n > max_head_size)	//too big packet
		return(B2PROT_RET_INVALID);
	if ((n+sizeof(B2PROT_PACHEAD)+B2PROT_CRC_BYTES) > pac_len)
		return(B2PROT_RET_SIZE);
	//get crc
	pval = (unsigned int*)(pac->data + n);
	//test crc
	crc = B2PROT_crccalc((unsigned char *)pac, sizeof(B2PROT_PACHEAD)+n);
	if (*pval != crc)
		return(B2PROT_RET_INVALID);

	//ok
	return(B2PROT_RET_OK);
}

//-------------------------------------------------------------

//*** create big protocol (should be run before openning)
BOOL B2PROT_Create(COMH *hC)
{

	if (hC == NULL)
		return(FALSE);

	//--- init CRC (is static) ---
	if (!crc32_isvalid(&g_crcst)) {
		crc32_create(&g_crcst, NULL);
		crc32_generate_byname(&g_crcst, CRC32_NAME_STM32);
	}

	//logging
	hC->log_format = COM_LOGFORMAT_BINARY;

	//--- setup buffers ---
	hC->alloc = TRUE;
	hC->sendbufsize = sizeof(B2PROT_PACKET);
	hC->rcptbufsize = sizeof(B2PROT_PACKET);

	//parameters
	hC->hProt = NULL;
	hC->SimulProc = NULL;


	return(TRUE);
}

//*** destroy big protocol (should be run after closing)
BOOL B2PROT_Destroy(COMH *hC)
{

	return(TRUE);
}

//*** get poiter to write buffer
char *B2PROT_GetWriteBuf(COMH *hC)
{
	if (hC == NULL)
		return(NULL);
	return(hC->sendbuf + sizeof(B2PROT_PACHEAD));
}

//*** get poiter to read buffer
char *B2PROT_GetReadBuf(COMH *hC)
{
	if (hC == NULL)
		return(NULL);
	return(hC->rcptbuf + sizeof(B2PROT_PACHEAD));
}

//*** send command and waits for response
DWORD B2PROT_CmdSendAndRcpt(COMH *hC, unsigned short cmd, unsigned short wlen, unsigned short *rlen)
{
	DWORD ret;
#ifdef B2PROT_SIMUL
	B2PROT_PACKET *pac;
	unsigned int crc;
	unsigned short n;
	unsigned int *pval;
#endif

	//handle
	if (hC == NULL || hC->rcptbuf == NULL)
		return(ERROR_INVALID_HANDLE);

	//simulation
#ifdef B2PROT_SIMUL
	if (hC->SimulProc) {
		//create w-data
		pac = (B2PROT_PACKET *)hC->sendbuf;
		pac->head.pre = B2PROT_PRECMD;
		pac->head.var = hC->var;
		pac->head.size = wlen;
		pac->head.cmd = cmd;
		//cal. crc
		crc = B2PROT_crccalc((unsigned char *)pac, sizeof(B2PROT_PACHEAD)+wlen);
		pval = (unsigned int*)(pac->data + wlen);
		*pval = crc;
		n = sizeof(B2PROT_PACHEAD) + wlen + B2PROT_CRC_BYTES;
#ifdef B2PROT_LOG_ENABLE
		if (COM_log_enable)
			COM_LogData(hC, NO_ERROR, (char *)pac, n, NULL, 0);
#endif
		//simulate data response
		ret = ((B2PROT_SIMULPROC)hC->SimulProc)(hC, cmd, wlen, &n);
		if (rlen)
			*rlen = n;

		//create r-data
		pac = (B2PROT_PACKET *)hC->rcptbuf;
		pac->head.pre = B2PROT_PRERCV;
		pac->head.var = hC->var;
		pac->head.size = n;
		if (ret == NO_ERROR)
			pac->head.cmd = cmd;
		else
			pac->head.cmd = ret;
		//cal. crc
		crc = B2PROT_crccalc((unsigned char *)pac, sizeof(B2PROT_PACHEAD)+n);
		pval = (unsigned int*)(pac->data + n);
		*pval = crc;
		n += sizeof(B2PROT_PACHEAD) + B2PROT_CRC_BYTES;
#ifdef B2PROT_LOG_ENABLE
		if (COM_log_enable)
			COM_LogData(hC, ret, NULL, 0, (char *)pac, n);
#endif

		hC->errs = 0;	//ok

		if (ret != NO_ERROR)
			return(ERROR_INVALID_DATA);
		return(NO_ERROR);
	}
#endif

	//empty data
	ret = B2PROT_EmptyRead(hC, (B2PROT_PACKET *)hC->rcptbuf);

	//write command
	ret = B2PROT_WriteCommand(hC, (B2PROT_PACKET *)hC->sendbuf, hC->var, cmd, wlen);
	if (ret != NO_ERROR) {
		hC->errs++;
		return(ret);
	}

	//read command
	ret = B2PROT_ReadCommand(hC, (B2PROT_PACKET *)hC->rcptbuf, rlen, hC->timeout);
	if (ret != NO_ERROR) {
		hC->errs++;
		return(ret);
	}
	hC->errs = 0;	//ok

	//test var
	if (hC->var != ((B2PROT_PACHEAD *)hC->rcptbuf)->var)
		return(ERROR_INVALID_BLOCK);
	if (cmd != ((B2PROT_PACHEAD *)hC->rcptbuf)->cmd) {
		switch (((B2PROT_PACHEAD *)hC->rcptbuf)->cmd) {
			default:
			case B2PROT_ERROR_COMMAND: return(ERROR_INVALID_DATA);
			case B2PROT_ERROR_BUSY: return(ERROR_BUSY);
			case B2PROT_ERROR_SIZE: return(ERROR_INVALID_DATA);
			case B2PROT_ERROR_VALUE: return(ERROR_INVALID_DATA);
			case B2PROT_ERROR_MODE: return(ERROR_INVALID_LEVEL);
			case B2PROT_ERROR_PENDING: return(ERROR_IO_PENDING);
			case B2PROT_ERROR_FAILED: return(ERROR_INVALID_OPERATION);
		}
	}

	return(NO_ERROR);
}

//*** send only command
DWORD B2PROT_CmdSendOnly(COMH *hC, unsigned short cmd)
{
	DWORD ret;

	//send command
	ret = B2PROT_CmdSendAndRcpt(hC, cmd, 0, NULL);
	if (ret == ERROR_INVALID_DATA)
		ret = 1;	//bad response
	return(ret);
}

/* setup simul function */
BOOL B2PROT_SetSimulFunction(COMH *hC, B2PROT_SIMULPROC simul_fce)
{
	//handle
	if (hC == NULL)
		return(FALSE);

	hC->SimulProc = (COM_SIMULCE)simul_fce;
	return(TRUE);
}


//------------------------------------------------------------
#define B2PROT_ERVIN_TIMEOUT 3000	//3.0s
#define B2PROT_ERVIN_BROADCAST_PORT 51455
#define B2PROT_ERVIN_BIND_PORT 51454
#define B2PROT_ERVIN_TCPBRIDGE_PORT 10001
#define B2PROT_ERVIN_TCPECHO_PORT 10011

#define B2PROT_ERVIN_CMD_BCINFO 0x0900

#define CMD_ST_ERVIN_NAME_LEN 8
typedef struct {
  char name[CMD_ST_ERVIN_NAME_LEN];
  uint16_t fw_ver;
  uint16_t fw_year;
  uint8_t fw_month;
  uint8_t fw_day;
  uint16_t rev;
  uint8_t uid[12];
  uint8_t res[4];
} CMD_ST_ERVININFO;   //32B

typedef struct {
  uint8_t mode;
  uint8_t dir;
  uint8_t locked;
  uint8_t res[5];
} CMD_ST_DIR; //8B

typedef struct {
	uint8_t netlink;  //0=down, 1=up
	uint8_t netstate;  //-1=not established, 0=static, 1=DHCP, 2=AutoIP
	uint8_t res1[2];
	uint8_t ip[4];
	uint8_t netmask[4];
	uint8_t gateway[4];
  uint8_t mac[6];
	uint16_t enflags;  //flags
  uint8_t res2[8];
} CMD_ST_ETHINFO;   //32B

typedef struct {
  uint8_t link;    //0=down, 1=up (connected)
  uint8_t baud;    //0=1200, 1=2400, 2=4800, 3=9600, 4=19200, 5=38400, 6=57600, 7=115200, 8=230400, 9=460800, 10=921600, 11=1000000, ...
  uint8_t spec;    //parity (none, even, odd), stopbits (1, 2)
  uint8_t res;     //reserve
} CMD_ST_UARTINFO;  //4B

typedef struct {
  uint8_t link;    //0=down, 1=up (connected)
  uint8_t vcpstate;
  uint8_t res[2];     //reserve
} CMD_ST_USBINFO;  //4B

#define CMD_ST_DEV_NAME_LEN 16
#define CMD_ST_DEV_MODEL_LEN 4
#define CMD_ST_DEV_OEM_LEN 16
#define CMD_ST_DEV_SN_LEN 8
#define CMD_ST_DEV_SDATA_LEN 12
typedef struct {
  char name[CMD_ST_DEV_NAME_LEN];
  char model[CMD_ST_DEV_MODEL_LEN];
  char oem[CMD_ST_DEV_OEM_LEN];
  char sn[CMD_ST_DEV_SN_LEN];
  uint16_t fw_ver;
  uint16_t fw_year;
  uint8_t fw_month;
  uint8_t fw_day;
  uint16_t res;
  uint8_t sdata[CMD_ST_DEV_SDATA_LEN];
} CMD_ST_DEV;   //72B

typedef struct {
  uint8_t status;
  uint8_t res;
  uint16_t lssub_period;
  uint32_t errors;
  uint32_t warnings;
  uint32_t flags;
} STATUS; //16B

//broadcast info
typedef struct {
  //ervin-info (32B)
  CMD_ST_ERVININFO ervin;
  //ervin-status (16B)
  STATUS status;
  //com-direction (8B)
  CMD_ST_DIR dir;
  //eth-info (32B)
  CMD_ST_ETHINFO ethinfo;
  //usb-info (4B)
  CMD_ST_USBINFO usbinfo;
  //uart-info (4B)
  CMD_ST_UARTINFO uartinfo;
  //device (64B)
  CMD_ST_DEV dev;
  //reserve (32B)
  uint8_t bcres[32];
} CMD_ST_BCINFO; //broadcast info (192B)

typedef struct {
	HLIST hlist;
	char ip[COM_MAX_IP_LEN];
	char *ervin_name;
} B2PROT_ST_LISTIP;

unsigned __stdcall B2PROT_SearchERVINLocalTh(void *param)
{
	HLIST hlist;
	SOCKET s;
	int n, addr_len;
	//int err;
	fd_set readset;
	struct sockaddr_in src_addr;
	struct timeval timeout = {0};
	char id[512], *ip;
	double t0;
	B2PROT_PACKET *pac;
	D_DEV_SEARCH dev_search;
	TCHAR *pt;
	char *ervin_name = NULL;
	//unsigned int found = 0;

	//test param
	if (param == NULL) {
		_endthreadex(0);
		return(0);
	}
	//test pointers
	hlist = ((B2PROT_ST_LISTIP*)param)->hlist;
	ip = 	((B2PROT_ST_LISTIP*)param)->ip;
	ervin_name = ((B2PROT_ST_LISTIP*)param)->ervin_name;

	if (hlist == NULL || ip == NULL) {
		_endthreadex(0);
		return(0);
	}

	//ervin name
	if (ervin_name == NULL) {
		ervin_name = "ERVIN5";
	}

	//--- SEARCH by local address
	while (1) {
		//create socket
		s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (s == INVALID_SOCKET) {
			//err = WSAGetLastError();
			closesocket(s);
			break;		//connection error
		}

		//enable broadcast
		if (ETH_SetUDPSocketBroadcastEnabled(s, 1) == 0) {
			//err = WSAGetLastError();
			closesocket(s);
			break;		//connection error
		}

		//bind to local ip
		if (ETH_BindUDPSocketToIP(s, ip, B2PROT_ERVIN_BIND_PORT) == 0) {
//		if (ETH_BindUDPSocketToAny(s, B2PROT_ERVIN_BIND_PORT)) {
			//err = WSAGetLastError();
			closesocket(s);
			break;		//connection error
		}


		//send broadcast data (FW+MAC query)
		pac = (B2PROT_PACKET*)id;
		pac->head.pre = B2PROT_PRECMD;
		pac->head.var = 0;
		pac->head.size = 0;
		pac->head.cmd = B2PROT_ERVIN_CMD_BCINFO;		//BCINFO
		unsigned int* pcrc = (unsigned int*)(pac->data+pac->head.size);
		*pcrc = B2PROT_crccalc((unsigned char*)pac, sizeof(B2PROT_PACHEAD));
		if (ETH_SendToBroadcast(s, id, B2PROT_BYTES(0), B2PROT_ERVIN_BROADCAST_PORT) == 0) {
			//err = WSAGetLastError();
			closesocket(s);
			break;		//connection error
		}

		//wait for responses
		n = 0;
		t0 = timer_GetTime();
		while((timer_GetTime()-t0) < (double)B2PROT_ERVIN_TIMEOUT) {
			FD_ZERO(&readset);
			FD_SET(s, &readset);

			if (select(sizeof(readset), &readset, NULL, NULL, &timeout) < 0) {
				break;
			}

			if (FD_ISSET(s, &readset)) {
				addr_len = sizeof(src_addr);		//init. size!
				n = recvfrom(s, id, sizeof(id), 0, (struct sockaddr *)&src_addr, &addr_len);

				//--- test for ERVIN_BCINFO response
				pac = (B2PROT_PACKET*)id;
				if (B2PROT_CheckReplyPacket(pac, n, B2PROT_MAX_PACKET_SIZE) == B2PROT_RET_OK &&
					pac->head.size >= sizeof(CMD_ST_BCINFO) &&
					strcmp((char*)pac->data, ervin_name) == 0) {

					CMD_ST_BCINFO *bcinfo = (CMD_ST_BCINFO*)pac->data;

					//zero search structure
					memset(&dev_search, 0, sizeof(D_DEV_SEARCH));

					//eth.
					dev_search.mode = COM_MODE_ETHERNET;
					dev_search.portno = 1;

					//get ip -> convert IP to string
					conv_utf8_to_str(inet_ntoa(src_addr.sin_addr), dev_search.ip, COM_MAX_IP_LEN);

					//MAC -> conver MAC to string
					__sprintf(dev_search.spec_id, TEXT("%02X%02X%02X%02X%02X%02X"),
										bcinfo->ethinfo.mac[0],
										bcinfo->ethinfo.mac[1],
										bcinfo->ethinfo.mac[2],
										bcinfo->ethinfo.mac[3],
										bcinfo->ethinfo.mac[4],
										bcinfo->ethinfo.mac[5]);

					//set origin (name+model)
					pt = dev_search.name;
					conv_utf8_to_str(bcinfo->dev.name, pt, sizeof(bcinfo->dev.name));
					//+model
					pt += __strlen(pt);
					*pt = '-';
					pt++;
					conv_utf8_to_str(bcinfo->dev.model, pt, sizeof(bcinfo->dev.model));
					//set name (oem+model)
					pt = dev_search.id;
					conv_utf8_to_str(bcinfo->dev.oem, pt, sizeof(bcinfo->dev.oem));
					//+model
					pt += __strlen(pt);
					*pt = '-';
					pt++;
					conv_utf8_to_str(bcinfo->dev.model, pt, sizeof(bcinfo->dev.model));
					pt += __strlen(pt)+1;
					//type
					__strcpy(pt, TEXT("n/a"));
					pt += __strlen(pt)+1;
					//fw
					__sprintf(pt, TEXT("%d"), bcinfo->dev.fw_ver);
					pt += __strlen(pt)+1;
					//sn
					conv_utf8_to_str(bcinfo->dev.sn, pt, sizeof(bcinfo->dev.sn));
					pt += __strlen(pt)+1;
					*pt = '\0';

					//add to list
					LIST_AppendItem(hlist, (LPARAM)&dev_search);
					n++;
				}
			}
			else
				Sleep(100);
		}

		//close socket
		closesocket(s);

		break;	//only once
	}

	_endthreadex(0);
	return(0);
}


//*** function processes search for ERVIN device
//wild_filter: using general wild characters '*' and '?'
//test_id: open communication and test ID (IP backup)
unsigned int B2PROT_SearchERVINx(HDEVICE hD, HLIST hlist, TCHAR *wild_filter, int test_id, char *ervin_name)
{
	//D_DEVICE *pD;
	HLIST l_hlist;
	HITEM hitem;
	D_DEV_SEARCH *psearch;
	int valid;
	char *ip;
	TCHAR s_ip[COM_MAX_IP_LEN];
	unsigned int found = 0, idx;
	if (hD == NULL || hlist == NULL)
		return(0);

	//--- init CRC (is static) ---
	if (!crc32_isvalid(&g_crcst)) {
		crc32_create(&g_crcst, NULL);
		crc32_generate_byname(&g_crcst, CRC32_NAME_STM32);
	}

	//create local list
	l_hlist = LIST_Create(D_ListIProcDevSearchParam);
	if (l_hlist == NULL) {
		return(ERROR_OUTOFMEMORY);
	}

	//--- SEARCH ---
	//go through local addresses
	#define B2PROT_ERVIN_MAX_LADDRS 6
	HANDLE thread[B2PROT_ERVIN_MAX_LADDRS];
	B2PROT_ST_LISTIP listip[B2PROT_ERVIN_MAX_LADDRS];
	unsigned int id;

	//run threads
	for (idx=0; (ip = ETH_ListLocalIP(idx)) != NULL && idx<B2PROT_ERVIN_MAX_LADDRS; idx++) {
		listip[idx].hlist = l_hlist;
		strncpy(listip[idx].ip, ip, COM_MAX_IP_LEN);
		listip[idx].ervin_name = ervin_name;
		thread[idx] = (HANDLE)_beginthreadex(NULL, 0, B2PROT_SearchERVINLocalTh, (void*)&listip[idx], 0, &id);
	}
	//wait for threads
	WaitForMultipleObjects(idx, thread, TRUE, 2*B2PROT_ERVIN_TIMEOUT);
	while (idx) {
		idx--;
		CloseHandle(thread[idx]);
	}

	//through local list
	idx = LIST_GetSize(l_hlist);
	hitem = LIST_GetItem(l_hlist, LPOS_FIRST, 0);
	found = 0;
	while (hitem) {
		psearch = NULL;
		if (LIST_GetItemValue(hitem, (void *)&psearch) && psearch) {
			valid = 0;
			//test filter (using wild characters)
			if (wild_filter == NULL || WildMatch(wild_filter, psearch->name) == MATCH) {

				//test TCP port
				psearch->status = D_SEARCHSTATUS_OFF;
				if (1) {
					if (ETH_TestTCPConnection(psearch->ip, ((D_DEVICE *)hD)->com.eth_pars.port, 350))
						psearch->status = D_SEARCHSTATUS_ONLINE;
					else if (ETH_TestTCPConnection(psearch->ip, B2PROT_ERVIN_TCPECHO_PORT, 350))
						psearch->status = D_SEARCHSTATUS_USED;
					else
						psearch->status = D_SEARCHSTATUS_UNREACH;
				}

				//test ID
				if (test_id) {
					//store parameters (ip)
					__strncpy(s_ip, ((D_DEVICE *)hD)->com.eth_pars.ip, COM_MAX_IP_LEN);
					__strncpy(((D_DEVICE *)hD)->com.eth_pars.ip, psearch->ip, COM_MAX_IP_LEN);

					//open device
					if (D_OpenDevice(hD) == NO_ERROR) {
						//test id
						valid = D_TestID(hD, NULL, NULL, NULL, NULL);
						if (valid && ((D_DEVICE*)hD)->id) {
							memcpy(psearch->id, ((D_DEVICE*)hD)->id, sizeof(TCHAR)*D_MAX_ID_LEN);
						}

						//close device
						D_CloseDevice(hD);
					}
					//restore parameters
					__strncpy(((D_DEVICE *)hD)->com.eth_pars.ip, s_ip, COM_MAX_IP_LEN);
				}
				else
					valid = 1;

				if (valid) {
					//add to search list
					LIST_AppendItem(hlist, (LPARAM)psearch);
					found++;
				}
			}
		}
		hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);
	}

	//destroy local list
	LIST_Discard(l_hlist);

	return(found);
}



//---------- GENERAL ECx2000 COMMANDS ---------

#ifdef XXX

/** @brief Read advanced specification
 *
 * @param hD HDEVICE
 * @param pidx WORD
 * @param uspec TCHAR*
 * @param max_len int
 * @return DWORD
 *
 */
DWORD B2PROT_ReadAdvanceSpec(HDEVICE hD, WORD pidx, TCHAR *aspec, int max_len)
{
	D_DEVICE *pD;
	unsigned short rlen;
	B2PROT_ST_ASPEC *as;
	DWORD ret;
	int i, len;
	D_DEV_PROP *prop = NULL;
	TCHAR *ptext;

	//check pointers
	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//prop by index
	if (pidx > 0 && pidx < pD->n_p && pD->p[pidx].val.m.buf) {
		prop = &pD->p[pidx];
	}

	while (1) {
		//send command
		ret = B2PROT_CmdSendAndRcpt(&pD->com, B2PROT_CMD_GET_ASPEC, 0, &rlen);
		if (ret != NO_ERROR)
			break;

		//check info data
		if (rlen < sizeof(B2PROT_ST_ASPEC)) {
			ret = ERROR_INVALID_DATA;
			break;
		}
		as = (B2PROT_ST_ASPEC *)B2PROT_GetReadBuf(&pD->com);
		//endianness

		//store data
		if (prop) {
				len = prop->val.m.size/sizeof(TCHAR);
				ptext = (TCHAR *)prop->val.m.buf;
				for (i=0; i<len && i<B2PROT_ST_ASPEC_LEN; i++) {
					ptext[i] = (TCHAR)as->data[i];
				}
		}

		//copy data out
		if (aspec) {
				for (i=0; i<max_len && i<B2PROT_ST_ASPEC_LEN; i++) {
					aspec[i] = (TCHAR)as->data[i];
				}
		}

		break;
	}
	if (ret == ERROR_INVALID_DATA)
		ret = 1;	//bad response

	return(ret);
}

/** @brief Read user specification
 *
 * @param hD HDEVICE
 * @param pidx WORD
 * @param uspec TCHAR*
 * @param max_len int
 * @return DWORD
 *
 */
DWORD B2PROT_ReadUserSpec(HDEVICE hD, WORD pidx, TCHAR *uspec, int max_len)
{
	D_DEVICE *pD;
	unsigned short rlen;
	B2PROT_ST_USPEC *us;
	DWORD ret;
	int i, len;
	D_DEV_PROP *prop = NULL;
	TCHAR *ptext;

	//check pointers
	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//prop by index
	if (pidx > 0 && pidx < pD->n_p && pD->p[pidx].val.m.buf) {
		prop = &pD->p[pidx];
	}

	while (1) {
		//send command
		ret = B2PROT_CmdSendAndRcpt(&pD->com, B2PROT_CMD_GET_USPEC, 0, &rlen);
		if (ret != NO_ERROR)
			break;

		//check info data
		if (rlen < sizeof(B2PROT_ST_USPEC)) {
			ret = ERROR_INVALID_DATA;
			break;
		}
		us = (B2PROT_ST_USPEC *)B2PROT_GetReadBuf(&pD->com);
		//endianness

		//store data
		if (prop) {
				len = prop->val.m.size/sizeof(TCHAR);
				ptext = (TCHAR *)prop->val.m.buf;
				for (i=0; i<len && i<B2PROT_ST_USPEC_LEN; i++) {
					ptext[i] = (TCHAR)us->data[i];
				}
		}

		//copy data out
		if (uspec) {
				for (i=0; i<max_len && i<B2PROT_ST_USPEC_LEN; i++) {
					uspec[i] = (TCHAR)us->data[i];
				}
		}

		break;
	}
	if (ret == ERROR_INVALID_DATA)
		ret = 1;	//bad response

	return(ret);
}

/** @brief Write user's specification
 *
 * @param hD HDEVICE
 * @param pidx WORD
 * @param uspec TCHAR*
 * @param max_len int
 * @return DWORD
 *
 */
DWORD B2PROT_WriteUserSpec(HDEVICE hD, WORD pidx, TCHAR *uspec, int max_len)
{
	D_DEVICE *pD;
	B2PROT_ST_USPEC *us;
	DWORD ret;
	int i, len;
	D_DEV_PROP *prop = NULL;
	TCHAR *ptext;

	//check pointers
	if (hD == NULL || (uspec == NULL && max_len != 0))
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//prop by index
	if (pidx > 0 && pidx < pD->n_p) {
		prop = &pD->p[pidx];
	}

	//prepare data
	us = (B2PROT_ST_USPEC *)B2PROT_GetWriteBuf(&pD->com);
	//set value
	for (i=0; i<max_len && i<B2PROT_ST_USPEC_LEN; i++) {
		us->data[i] = (char)(uspec[i] & 0xFF);
	}
	//endianness

	//send command
	ret = B2PROT_CmdSendAndRcpt(&pD->com, B2PROT_CMD_SET_USPEC, sizeof(B2PROT_ST_USPEC), NULL);
	if (ret == ERROR_INVALID_DATA) {
		ret = 1;	//bad response
	}
	else if (ret == NO_ERROR && prop) {
		//store data
		if (prop) {
				len = prop->val.m.size/sizeof(TCHAR);
				ptext = (TCHAR *)prop->val.m.buf;
				for (i=0; i<len && i<B2PROT_ST_USPEC_LEN; i++) {
					ptext[i] = (TCHAR)us->data[i];
				}
		}
	}

	return(ret);
}


BOOL CALLBACK B2PROT_SetupUserSpecDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);

/** @brief Setup user's specification text
 *
 * @param hwnd HWND
 * @param hD HDEVICE
 * @param pidx WORD
 * @return DWORD
 *
 */
DWORD B2PROT_SetupUserSpec(HWND hdlg, HDEVICE hD, WORD pidx)
{
	D_DEVICE *pD;
	DWORD ret;
	TCHAR uspec[B2PROT_ST_USPEC_LEN];
	int sel;

	//check pointers
	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//check communication
	if (COM_IsOpened(&pD->com) || pD->ufce.pOpen == NULL || pD->ufce.pTestID == NULL || pD->ufce.pClose == NULL)
		return(ERROR_INVALID_HANDLE_STATE);

	//load actual com. parameters
	SendMessage(GetParent(hdlg), WM_USER+115, 0, 0);		//WMU_SETINTER

	//open communication
	ret = pD->ufce.pOpen(hD);
	if (ret != NO_ERROR)
		return(ret);

	while (1) {
		//test ID
		if (!pD->ufce.pTestID(hD, NULL, NULL, NULL, NULL)) {
			ret = ERROR_FILE_NOT_FOUND;
			break;
		}

		//read uspec
		ret = B2PROT_ReadUserSpec(hD, -1, uspec, B2PROT_ST_USPEC_LEN);
		if (ret != NO_ERROR)
			break;

		//show setup dialog
		sel = CreateSimpleDialogBoxIndirect((HINSTANCE)GetWindowLong(hdlg, GWLP_HINSTANCE),
						hdlg, 0, D_("Setup user's specification"),
						0, 0, D_PA_GW(1), D_PA_GH(3),		//???
						B2PROT_SetupUserSpecDlgProc, (LPARAM)uspec);

		if (sel == 1) {
			//write uspec
			ret = B2PROT_WriteUserSpec(hD, -1, uspec, B2PROT_ST_USPEC_LEN);
			if (ret != NO_ERROR)
				break;
		}

		break;
	}

	//close communication
	pD->ufce.pClose(hD);

	return(ret);
}

//*** function processes ready dialog - device on-line configuration
BOOL CALLBACK B2PROT_SetupUserSpecDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	TCHAR *uspec = NULL;
	HWND hctl;
	int i;

	if (msg == WM_INITDIALOG) {
		if (lparam == 0)
			EndDialog(hdlg, -1);		//error

		//store handle
		uspec = (TCHAR *)lparam;
		SetWindowLong(hdlg, GWLP_USERDATA, (LONG)uspec);		//store handle !!!!!

		//set window size
		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = D_PA_GW(1)+D_PA_S1;
		rc.bottom = D_PA_GH(3)+D_PA_S1;
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		MoveWindow(hdlg, 0, 0, rc.right-rc.left, rc.bottom-rc.top, TRUE);

		i = 0;
		//--- groupbox
		hctl = CreateWindow(CLASS_BUTTON, D_("User's specification"),
												WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
												D_PA_COLGP(0), D_PA_ROWGP(i), D_PA_GW(1), D_PA_GH(3),
												hdlg, (HMENU)IDC_SSTATIC, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

		i = 2;
		//set button
		hctl = CreateWindow(CLASS_BUTTON, D_("Set"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												D_PA_COLST(0)+D_PA_GWIN(1)-D_PA_S2-2*D_PA_WB, D_PA_CTOF2(i), D_PA_WB, D_PA_HOF,
												hdlg, (HMENU)IDOK, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//cancel button
		hctl = CreateWindow(CLASS_BUTTON, D_("Cancel"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												D_PA_COLST(0)+D_PA_GWIN(1)-D_PA_WB, D_PA_CTOF2(i), D_PA_WB, D_PA_HOF,
												hdlg, (HMENU)IDCANCEL, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

		//editbox
		i = 0;
		hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""),
													WS_CHILD | WS_VISIBLE | WS_TABSTOP,
													D_PA_COLST(0), D_PA_ROWVAL(i), D_PA_GWIN(1), D_PA_H, hdlg, (HMENU)11, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		SendMessage(hctl, EM_SETLIMITTEXT, B2PROT_ST_USPEC_LEN, 0);
		SetWindowText(hctl, uspec);

		//scale window
		ScaleParentWindowByScreen(hdlg);

		//center window
		CenterParentWindow(hdlg);
	}
	else {
		uspec = (HLIST)GetWindowLong(hdlg, GWLP_USERDATA);		//get device handle

		//process rest messages
		switch(msg) {

			case WM_COMMAND:
				if (lparam) {
					if (HIWORD(wparam) == BN_CLICKED) {
						switch(LOWORD(wparam)) {
							case IDOK:
								//get text
								GetWindowText(GetDlgItem(hdlg, 11), uspec, B2PROT_ST_USPEC_LEN);
								SendMessage(hdlg, WM_CLOSE, 0, 1);		//close message (set)
								return TRUE;
							case IDCANCEL:
								SendMessage(hdlg, WM_CLOSE, 0, 0);		//close message (cancel)
								return TRUE;
							default:
								break;
						}
					}
				}
				break;

			case WM_CLOSE:
				EndDialog(hdlg, lparam);		//cancel
				return TRUE;

			case WM_DESTROY:


				break;
		}
	}
	return FALSE;
}

//---------------------------
//*** read log partial data
DWORD B2PROT_CmdReadLogPartData(HDEVICE hD, HLIST hlog, int min_items, int start)
{
	D_DEVICE *pD;
	unsigned short rlen;
	B2PROT_ST_LOGSIZE *psize;
	B2PROT_ST_LOGDATA *pdata;
	DWORD ret, i, k;

	//check pointers
	if (hD == NULL || hlog == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//min items
	if (min_items < 0)
		min_items = 1;

	if (start)
		LIST_RemoveAll(hlog);		//clear list

	//prepare data
	psize = (B2PROT_ST_LOGSIZE *)B2PROT_GetWriteBuf(&pD->com);
	//set values
	start = LIST_GetSize(hlog);
	psize->index = start;
	psize->length = B2PROT_LOGITEM_MAX_SIZE;
	//endianness
	psize->index = _htons(psize->index);
	psize->length = _htons(psize->length);

	while (1) {
		//send command
		ret = B2PROT_CmdSendAndRcpt(&pD->com, B2PROT_CMD_GET_LOGDATA, sizeof(B2PROT_ST_LOGSIZE), &rlen);
		if (ret != NO_ERROR)
			break;

		//check info data
		if (rlen < B2PROT_LOGDATA_MINBYTES) {
			ret = ERROR_INVALID_DATA;
			break;
		}
		pdata = (B2PROT_ST_LOGDATA *)B2PROT_GetReadBuf(&pD->com);
		//endianness
		pdata->size.index = _ntohs(pdata->size.index);
		pdata->size.length = _ntohs(pdata->size.length);

		//test index and length
		if (pdata->size.index != start || pdata->size.length*sizeof(B2PROT_ST_LOGITEM) != (rlen - sizeof(pdata->size))) {
			ret = ERROR_INVALID_DATA;
			break;
		}

		//item endianness + add data
		for (i=0; i<pdata->size.length; i++) {
			pdata->item[i].rec_id = _ntohs(pdata->item[i].rec_id);
			pdata->item[i].age_time = ntohl(pdata->item[i].age_time);
			k = ntohl(pdata->item[i].errors.dwCode[0]);
			pdata->item[i].errors.dwCode[0] = ntohl(pdata->item[i].errors.dwCode[1]);
			pdata->item[i].errors.dwCode[1] = k;
			k = ntohl(pdata->item[i].warnings.dwCode[0]);
			pdata->item[i].warnings.dwCode[0] = ntohl(pdata->item[i].warnings.dwCode[1]);
			pdata->item[i].warnings.dwCode[1] = k;
			pdata->item[i].events.dwCode = ntohl(pdata->item[i].events.dwCode);

			//append item
			if (!LIST_AppendItem(hlog, (LPARAM)&pdata->item[i])) {
				ret = ERROR_OUTOFMEMORY;
				break;
			}
		}

		if (pdata->size.length < min_items) {
			ret = 2;		//no more data
			break;
		}

		break;
	}
	if (ret == ERROR_INVALID_DATA)
		ret = 1;	//bad response
	else {
		//sort list
		LIST_BubbleSort(hlog, LIST_SORTMODE_ASCEND);
	}

	return(ret);
}

//---------------------------
//*** read component log partial data
DWORD B2PROT_CmdReadCmpPartData(HDEVICE hD, HLIST hcmp, int min_items, int start)
{
	D_DEVICE *pD;
	unsigned short rlen;
	B2PROT_ST_CMPSIZE *psize;
	B2PROT_ST_CMPDATA *pdata;
	DWORD ret, i;

	//check pointers
	if (hD == NULL || hcmp == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//min items
	if (min_items < 0)
		min_items = 1;

	if (start)
		LIST_RemoveAll(hcmp);		//clear list

	//prepare data
	psize = (B2PROT_ST_CMPSIZE *)B2PROT_GetWriteBuf(&pD->com);
	//set values
	start = LIST_GetSize(hcmp);
	psize->index = start;
	psize->length = B2PROT_CMPITEM_MAX_SIZE;
	//endianness
	psize->index = _htons(psize->index);
	psize->length = _htons(psize->length);

	while (1) {
		//send command
		ret = B2PROT_CmdSendAndRcpt(&pD->com, B2PROT_CMD_GET_CMPDATA, sizeof(B2PROT_ST_CMPSIZE), &rlen);
		if (ret != NO_ERROR)
			break;

		//check info data
		if (rlen < B2PROT_CMPDATA_MINBYTES) {
			ret = ERROR_INVALID_DATA;
			break;
		}
		pdata = (B2PROT_ST_CMPDATA *)B2PROT_GetReadBuf(&pD->com);
		//endianness
		pdata->size.index = _ntohs(pdata->size.index);
		pdata->size.length = _ntohs(pdata->size.length);

		//test index and length
		if (pdata->size.index != start || pdata->size.length*sizeof(B2PROT_ST_CMPITEM) != (rlen - sizeof(pdata->size))) {
			ret = ERROR_INVALID_DATA;
			break;
		}

		//item endianness + add data
		for (i=0; i<pdata->size.length; i++) {
			//endian
			pdata->item[i].rec_id = _ntohs(pdata->item[i].rec_id);
			pdata->item[i].oper = _ntohs(pdata->item[i].oper);
			pdata->item[i].dev_time = ntohl(pdata->item[i].dev_time);

			//append item
			if (!LIST_AppendItem(hcmp, (LPARAM)&pdata->item[i])) {
				ret = ERROR_OUTOFMEMORY;
				break;
			}
		}

		if (pdata->size.length < min_items) {
			ret = 2;		//no more data
			break;
		}

		break;
	}
	if (ret == ERROR_INVALID_DATA)
		ret = 1;	//bad response
	else {
		//sort list
		LIST_BubbleSort(hcmp, LIST_SORTMODE_ASCEND);
	}

	return(ret);
}


/* stop error sound */
DWORD B2PROT_CmdStopSound(HDEVICE hD)
{
	D_DEVICE *pD;
	DWORD ret;

	//check pointers
	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	ret = B2PROT_CmdSendAndRcpt(&pD->com, B2PROT_CMD_STOP_SOUND, 0, NULL);
	if (ret == ERROR_INVALID_DATA)
		ret = 1;	//bad response
	return(ret);
}

/* invoke beep */
DWORD B2PROT_CmdMakeBeep(HDEVICE hD, unsigned int uval)
{
	D_DEVICE *pD;
	B2PROT_ST_BEEP *pp;
	DWORD ret;

	//check pointers
	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//prepare data
	pp = (B2PROT_ST_BEEP *)B2PROT_GetWriteBuf(&pD->com);
	//set value
	pp->index = (unsigned char)uval;
	//endianness

	//send command
	ret = B2PROT_CmdSendAndRcpt(&pD->com, B2PROT_CMD_MAKE_BEEP, sizeof(B2PROT_ST_BEEP), NULL);
	if (ret == ERROR_INVALID_DATA)
		ret = 1;	//bad response
	return(ret);
}
#endif

//------------------------

//*** function processes IPROC function for working with strings
//IPTYPE_CREATE: iparam -> pointer to lparam of item, lparam -> LPARAM
//IPTYPE_DELETE: iparam -> pointer to lparam of item, lparam -> 0
//IPTYPE_PRINT: iparam -> pointer to lparam of item, lparam -> FILE *
BOOL B2PROT_LogItemProcParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	B2PROT_ST_LOGITEM *pitem, *pitem2;

	if (iparam) {
		switch (type) {
			//--- processed after ITEM creation
			case IPTYPE_CREATE:
				if (lparam) {
					//allocation
					*iparam = (LPARAM)malloc(sizeof(B2PROT_ST_LOGITEM));
					if (*iparam == 0)
						return(FALSE);		//out of memory
					//copy data
					memcpy((B2PROT_ST_LOGITEM *)*iparam, (B2PROT_ST_LOGITEM *)lparam, sizeof(B2PROT_ST_LOGITEM));
				}
				else
					*iparam = 0;
				break;
			//--- processed before ITEM deletion
			case IPTYPE_DELETE:
				if (*iparam)
					free((void *)*iparam);
				*iparam = 0;
				break;
			//--- processed at ITEM print
			case IPTYPE_PRINT:
				break;
			//--- processed at ITEM compare
			case IPTYPE_COMPARE:
				if (*iparam && lparam) {
					pitem = (B2PROT_ST_LOGITEM *)*iparam;
					pitem2 = (B2PROT_ST_LOGITEM *)lparam;
					//by time
					if (pitem->age_time_sec > pitem2->age_time_sec)
						return(1);
					else if (pitem->age_time_sec < pitem2->age_time_sec)
						return(-1);
					else {
						//by ms
						if (pitem->run_time_msec > pitem2->run_time_msec)
							return(1);
						else if (pitem->run_time_msec < pitem2->run_time_msec)
							return(-1);
						else {
							//by id
							if (pitem->rec_id > pitem2->rec_id)
								return(1);
							else if (pitem->rec_id < pitem2->rec_id)
								return(-1);
							else
								return(0);
						}
					}
					return(0);
				}
				return(0);		//don't sort
		}
	}
	return(TRUE);
}

//------------------------
//*** function processes IPROC function for working with strings
//IPTYPE_CREATE: iparam -> pointer to lparam of item, lparam -> LPARAM
//IPTYPE_DELETE: iparam -> pointer to lparam of item, lparam -> 0
//IPTYPE_PRINT: iparam -> pointer to lparam of item, lparam -> FILE *
BOOL B2PROT_CmpItemProcParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	B2PROT_ST_CMPITEM *pitem, *pitem2;

	if (iparam) {
		switch (type) {
			//--- processed after ITEM creation
			case IPTYPE_CREATE:
				if (lparam) {
					//allocation
					*iparam = (LPARAM)malloc(sizeof(B2PROT_ST_CMPITEM));
					if (*iparam == 0)
						return(FALSE);		//out of memory
					//copy data
					memcpy((B2PROT_ST_CMPITEM *)*iparam, (B2PROT_ST_CMPITEM *)lparam, sizeof(B2PROT_ST_CMPITEM));
				}
				else
					*iparam = 0;
				break;
			//--- processed before ITEM deletion
			case IPTYPE_DELETE:
				if (*iparam)
					free((void *)*iparam);
				*iparam = 0;
				break;
			//--- processed at ITEM print
			case IPTYPE_PRINT:
				break;
			//--- processed at ITEM compare
			case IPTYPE_COMPARE:
				if (*iparam && lparam) {
					pitem = (B2PROT_ST_CMPITEM *)*iparam;
					pitem2 = (B2PROT_ST_CMPITEM *)lparam;
					//by time
					if (pitem->age_time_sec > pitem2->age_time_sec)
						return(1);
					else if (pitem->age_time_sec < pitem2->age_time_sec)
						return(-1);
					else {
						//by id
						if (pitem->rec_id > pitem2->rec_id)
							return(1);
						else if (pitem->rec_id < pitem2->rec_id)
							return(-1);
						else
							return(0);
					}
					return(0);
				}
				return(0);		//don't sort
		}
	}
	return(TRUE);
}

/** @brief Create log list
 *
 * @return HLIST
 *
 */
HLIST B2PROT_CreateLogList(void)
{
	return(LIST_Create(B2PROT_LogItemProcParam));
}

/** @brief Create component log list
 *
 * @return HLIST
 *
 */
HLIST B2PROT_CreateCmpList(void)
{
	return(LIST_Create(B2PROT_CmpItemProcParam));
}

BOOL B2PROT_DestroyList(HLIST *hlist)
{
	if (hlist == NULL || *hlist == NULL)
		return(FALSE);

	LIST_Discard(*hlist);
	*hlist = NULL;

	return(TRUE);
}


//----------
//*** function create Info report RTF-file
BOOL B2PROT_CreateInfoFile(D_DEVICE *pD, HSTRING hs, const int *indexes)
{
	D_DEV_PROP *prop;
	DWORD i, param = 2, other = 0;
	const TCHAR *pstr;

	if (pD == NULL || hs == NULL || indexes == NULL)
		return(FALSE);

	//clear string
	HSTR_ClearString(hs);

	//--- create RTF header
	HSTR_AppendPrintf(hs,
						TEXT("{\\rtf1\\ansi\\ansicpg1250\\deff0\\deflang1029"		/* header */
						"{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Microsoft Sans Serif;}"		/* fonts */
						"{\\f1\\fmodern\\fprq1\\fcharset238 Courier New CE;}}\n"
						"{\\colortbl ;"		/* colors */
						"\\red250\\green0\\blue0;"
						"\\red0\\green250\\blue0;"
						"\\red0\\green0\\blue250;"
						"\\red100\\green100\\blue100;}\n"
						));

	//title
	B2PROT_TitleToRTF(hs, pD, TEXT("Device info list"), -1);

	//header
	HSTR_AppendPrintf(hs,
						TEXT("\\pard\\tx3000\\tx4000\\tx5000\\tx6000\\tx7000\\tx8000\\tx9000\\tx10000\\tx11000\n"
								 "\\fs18\n"));

	i = 0;
	while (indexes[i] != -1) {
		prop = &pD->p[indexes[i]];
		//showmem prop
		if (prop->guitype == DGUIT_SHOWMEM) {
			if (other || param == 1) {
				HSTR_AppendPrintf(hs, TEXT("\\par "));	//LF after other
			}
			HSTR_AppendPrintf(hs, TEXT("{\\b %s%s}\\par "), prop->label, *prop->suffix ? prop->suffix : TEXT(""));
			D_GuiRTFPrintShowMem(hs, prop->guidata, prop->val.m.buf, prop->val.m.size);
			HSTR_AppendPrintf(hs, TEXT("\\par "));		//LF after showmem
			other = 0;
			param = 0;
		}
		//normal property
		else {
				if (param) {
					if (param == 2)
						HSTR_AppendPrintf(hs, TEXT("{\\b %s}\\par "), TEXT("Parameters"));
					param = 1;
				}
				else if (!other) {
					HSTR_AppendPrintf(hs, TEXT("{\\b %s}\\par "), TEXT("Other"));
					other = 1;
				}
				//table item
				if (prop->guitype == DGUIT_TABLE && prop->type == DT_MEMORY && prop->guidata) {
					//print g-table
					TCHAR ttext[4096];
					if (D_GuiPrintTable(ttext, 4096, prop->guidata, prop->val.m.buf, prop->val.m.size, 1))
						pstr = ttext;
					else
						pstr = NULL;
				}
				else {
					pstr = D_GetGuiData(prop);
				}
				HSTR_AppendPrintf(hs, TEXT("%s%s: \\tab %s"), prop->label, *prop->suffix ? prop->suffix : TEXT(""), pstr ? pstr : TEXT(""));
				//unit
				pstr = D_GetUnitLabel(prop);
				if (pstr && *pstr) {
					HSTR_AppendPrintf(hs, TEXT(" %s"), pstr);
				}
				HSTR_AppendPrintf(hs, TEXT("\\par "));
		}

		i++;
	}

	//--- end RTF file
	HSTR_AppendPrintf(hs, TEXT("}"));

	return(TRUE);
}

//------------

BOOL B2PROT_LogSpecFunction(HSTRING hs, B2PROT_LOGSPECTYPE type, int par, B2PROT_ST_LOGITEM *pitem)
{
	assert(pitem);

	switch (type) {
		case B2PROT_LOGSPECTYPE_STATUS: HSTR_AppendPrintf(hs, TEXT("%d"), par); return(TRUE);
		case B2PROT_LOGSPECTYPE_ERROR: HSTR_AppendPrintf(hs, TEXT("%d"), par); return(TRUE);
		case B2PROT_LOGSPECTYPE_WARNING: HSTR_AppendPrintf(hs, TEXT("%d"), par); return(TRUE);
		case B2PROT_LOGSPECTYPE_LOGDESC: HSTR_AppendPrintf(hs, TEXT("f:%08Xh"), pitem->flags); return(TRUE);
		default: return(TRUE);
	}
}

//*** function create Log report RTF-file
BOOL B2PROT_CreateLogCmpFile(D_DEVICE *pD, HSTRING hs, HLIST hlog, HLIST hcmp, int options, B2PROT_LOGSPECFUNC fspec)
{
	HITEM hitem;
	B2PROT_ST_LOGITEM *logitem;
	B2PROT_ST_CMPITEM *cmpitem;
	HSTRING hsi = NULL;
	//double agetime;

	if (pD == NULL || hs == NULL)
		return(FALSE);

	//clear hstring
	HSTR_ClearString(hs);

	//hs
	hsi = HSTR_Create(NULL);

	//fce
	if (fspec == NULL)
		fspec = B2PROT_LogSpecFunction;

	//--- create RTF header
	HSTR_AppendPrintf(hs,
						TEXT("{\\rtf1\\ansi\\ansicpg1250\\deff0\\deflang1029"		/* header */
						"{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Microsoft Sans Serif;}"		/* fonts */
						"{\\f1\\fmodern\\fprq1\\fcharset238 Courier New CE;}}\n"
						"{\\colortbl ;"		/* colors */
						"\\red250\\green0\\blue0;"
						"\\red0\\green250\\blue0;"
						"\\red0\\green0\\blue250;"
						"\\red100\\green100\\blue100;}\n"
						));

	//--- log ---
	if (hlog) {
		//title
		B2PROT_TitleToRTF(hs, pD, TEXT("Device log list"), LIST_GetSize(hlog));

		//header
		HSTR_AppendPrintf(hs,
						 TEXT("\\pard\\tx1000\\tx2500\\tx4000\\tx5500\\tx7500\\tx9250\\tx11000\\tx12750\n"
									"\\fs18\n"
									"{\\ul\\b ID\\tab State\\tab Prev.\\tab Time [h]\\tab Date and Time\\tab Errors\\tab Warnings\\tab Events\\tab Details}\\par\n"
									"\\fs18\n"));

		hitem = LIST_GetItem(hlog, LPOS_FIRST, 0);
		int events = (options & 0x1) ? 10 : 0;
		while (hitem && LIST_GetItemValue(hitem, (LPARAM *)&logitem) && logitem) {

			HSTR_ClearString(hsi);
			//rec_id
			HSTR_AppendPrintf(hsi, TEXT("%d"), logitem->rec_id);
			//state
			HSTR_AppendPrintf(hsi, TEXT("\\tab "));
			fspec(hsi, B2PROT_LOGSPECTYPE_STATUS, logitem->state, logitem);
			//prev. state
			HSTR_AppendPrintf(hsi, TEXT("\\tab "));
			fspec(hsi, B2PROT_LOGSPECTYPE_STATUS, logitem->prev_state, logitem);
			//time
			HSTR_AppendPrintf(hsi, TEXT("\\tab %.5lf"), (double)logitem->age_time_sec/3600.0 + (double)logitem->run_time_msec/3600000.0);	//s->h + ms->h
			//rtc
			HSTR_AppendPrintf(hsi, TEXT("\\tab %u.%u.%u  %02u:%02u:%02u"),
												logitem->rtc_day, logitem->rtc_mon, logitem->rtc_yea+2000,		//rtc
												logitem->rtc_hrs, logitem->rtc_min, logitem->rtc_sec);		//s->h
			//errors
			HSTR_AppendPrintf(hsi, TEXT("\\tab "));
			B2PROT_ErrorsToRTF(hsi, &logitem->errors, 10);
			//warnings
			HSTR_AppendPrintf(hsi, TEXT("\\tab "));
			B2PROT_ErrorsToRTF(hsi, &logitem->warnings, 10);
			//events
			HSTR_AppendPrintf(hsi, TEXT("\\tab "));
			B2PROT_EventsToRTF(hsi, &logitem->events, events);
			//details (special text)
			HSTR_AppendPrintf(hsi, TEXT("\\tab "));
			fspec(hsi, B2PROT_LOGSPECTYPE_LOGDESC, 0, logitem);

			//add item line
			HSTR_AppendPrintf(hs, HSTR_GetStringPtr(hsi));
			HSTR_AppendPrintf(hs, TEXT(" \\par\n"));

			hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);	//next
		}
		HSTR_AppendPrintf(hs, TEXT("\\par\n"));
	}

	//--- cmp ---
	if (hcmp) {
		//title
		B2PROT_TitleToRTF(hs, hlog == NULL ? pD : NULL, TEXT("Device component log list"), LIST_GetSize(hcmp));

		//header
		HSTR_AppendPrintf(hs,
							TEXT("\\pard\\tx1000\\tx2300\\tx4000\\tx6000\\tx8000\\tx10000\n"
									"\\fs18\n"
									"{\\ul\\b ID\\tab Operation\\tab Time [h]\\tab Date and Time\\tab Text}\\par\n"
									"\\fs18\n"));

		hitem = LIST_GetItem(hcmp, LPOS_FIRST, 0);
		while (hitem && LIST_GetItemValue(hitem, (LPARAM *)&cmpitem) && cmpitem) {

			HSTR_AppendPrintf(hs, TEXT("%d\\tab %d\\tab %.5lf\\tab %u.%u.%u  %02u:%02u:%02u"),
								cmpitem->rec_id,
								cmpitem->operation,
								(double)cmpitem->age_time_sec / 3600.0,		//s->h
								cmpitem->rtc_day, cmpitem->rtc_mon, cmpitem->rtc_yea+2000,		//rtc
								cmpitem->rtc_hrs, cmpitem->rtc_min, cmpitem->rtc_sec);		//s->h
			//text
#ifdef UNICODE
			HSTR_AppendPrintf(hs, TEXT("\\tab %S \\par\n"), cmpitem->text);
#else
			HSTR_AppendPrintf(hs, TEXT("\\tab %s \\par\n"), cmpitem->text);
#endif

			hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);	//next
		}
		HSTR_AppendPrintf(hs, TEXT("\\par\n"));
	}

	//--- end RTF file
	HSTR_AppendPrintf(hs, TEXT("}"));

	HSTR_Destroy(hsi);

	return(TRUE);
}

//--------------------------------------
/* show RTF title info */
BOOL B2PROT_TitleToRTF(HSTRING hs, D_DEVICE *pD, TCHAR *title, int count)
{
	TCHAR *pname = NULL, *pmodel = NULL, *psn = NULL, *pfw = NULL;
	TCHAR *cstr_empty = TEXT("");

	if (hs == NULL)
		return(FALSE);

	//title
	if (title) {
		HSTR_AppendPrintf(hs, TEXT("{\\qc\\fs20\\b %s \\par\\par}\n"), title);
	}

	//device info
	if (pD) {
		pname = GetSubString(pD->id, 0);
		pmodel = GetSubString(pD->id, 1);
		pfw = GetSubString(pD->id, 2);
		psn = GetSubString(pD->id, 3);

		HSTR_AppendPrintf(hs, TEXT("\\fs18\n"));
		HSTR_AppendPrintf(hs, TEXT("\\pard\\tx2300\n"));
		HSTR_AppendPrintf(hs, TEXT("{\\b\\i Device Name:}\\tab %s-%s\\par\n"), pname ? pname : cstr_empty, pmodel ? pmodel : cstr_empty);
		HSTR_AppendPrintf(hs, TEXT("{\\b\\i Serial Number:}\\tab %s\\par\n"), psn ? psn : cstr_empty);
		HSTR_AppendPrintf(hs, TEXT("{\\b\\i Firmware Version:}\\tab %s\\par\n"), pfw ? pfw : cstr_empty);
	}

	if (count >= 0) {
		HSTR_AppendPrintf(hs, TEXT("{\\b\\i Total Items:}\\tab %d\\par\n"), count);
	}
	HSTR_AppendPrintf(hs, TEXT("\\par\n"));

	return(TRUE);
}

//--------------------------------------
//*** decode error/warning codes to file (return number of errors)
int B2PROT_ErrorsToRTF(HSTRING hs, B2PROT_ERRVAL *err_val, unsigned int max_errors)
{
	unsigned int code, i, err_no, n = 0;

	if (hs == NULL || err_val == NULL)
		return(-1);
	//
	//through dwords
	for (i=0; i<B2PROT_ERROR_DWORDS; i++) {
		code = err_val->dwCode[i];
		err_no = 32*i + 0;		//0, 32
		while (code) {
			if (code & 0x1) {
				if (n < max_errors) {
					HSTR_AppendPrintf(hs, TEXT("%s%d"), n ? TEXT(",") : TEXT(""), err_no);
				}
				n++;
			}
			code >>= 1;
			err_no++;
		}
	}

	//no error
	if (n == 0) {
		HSTR_AppendPrintf(hs, TEXT("None"));
	}
	else if (n >= max_errors) {
		HSTR_AppendPrintf(hs, TEXT(",..."));
	}

	return(n);
}

//--------------------------------------
//*** decode events code to file (return number of errors)
int B2PROT_EventsToRTF(HSTRING hs, B2PROT_EVENTVAL *event_val, unsigned int max_events)
{
	unsigned int code, i, err_no, n = 0;

	if (hs == NULL || event_val == NULL)
		return(-1);

	//no event list
	if (max_events == 0)	 {
		HSTR_AppendPrintf(hs, TEXT("0x%08X"), event_val->dwCode);
		return(n);
	}

	//
	//through dwords
	for (i=0; i<B2PROT_EVENT_DWORDS; i++) {
		code = event_val->dwCode;
		err_no = 32*i + 0;		//0, 32
		while (code) {
			if (code & 0x1) {
				if (n < max_events) {
					HSTR_AppendPrintf(hs, TEXT("%s%d"), n ? TEXT(",") : TEXT(""), err_no);
				}
				n++;
			}
			code >>= 1;
			err_no++;
		}
	}

	//no events
	if (n == 0) {
		HSTR_AppendPrintf(hs, TEXT("None"));
	}
	else if (n >= max_events) {
		HSTR_AppendPrintf(hs, TEXT(",..."));
	}

	return(n);
}


//----------------------------

/** @brief Check is demand error is active
 *
 * @param err_val B2PROT_ERRVAL*
 * @param error_no int
 * @return BOOL
 *
 */
BOOL B2PROT_IsErrorNo(B2PROT_ERRVAL *err_val, int error_no)
{
	uint32_t idx, mask;

	if (err_val) {
		idx = error_no / 32;
		mask = 1 << (error_no % 32);

		if (idx < B2PROT_ERROR_DWORDS && (err_val->dwCode[idx] & mask)) {
			return(TRUE);
		}
	}
	return(FALSE);
}


//*** Decode error/warning codes to number string (return number of errors)
/** @brief
 *
 * @param err_val B2PROT_ERRVAL*
 * @param str TCHAR*
 * @param max_codes unsigned int
 * @return int
 *
 */
int B2PROT_DecodeErrorNumbers(B2PROT_ERRVAL *err_val, TCHAR *str, unsigned int max_codes)
{
	TCHAR *pstr;
	unsigned int code, i, err_no, n = 0;

	if (err_val == NULL)
		return(-1);
	//
	pstr = str;

	//through bytes
	for (i=B2PROT_ERROR_DWORDS; i>0; i--) {
		code = err_val->dwCode[i-1];
		err_no = 32*i - 1;		//63, 31
		while (code) {
			if (code & 0x80000000) {
				if (pstr && n < max_codes) {
					if (n) {
						*(pstr++) = ',';
						*pstr = '\0';
					}
					__sprintf(pstr, TEXT("%d"), err_no);
					pstr += 2;
				}
				n++;
			}
			code <<= 1;
			err_no--;
		}
	}

	//no error
	if (n == 0) {
		lstrcpy(pstr, D_("None"));
	}
	else if (n >= max_codes) {
		lstrcat(pstr, TEXT(",..."));
	}

	return(n);
}

/** @brief Decode errors/warnings to message box
 *
 * @param hwnd HWND
 * @param err_val B2PROT_ERRVAL*
 * @return BOOL
 *
 */
BOOL B2PROT_ErrorsDecodeBox(HWND hwnd, B2PROT_ERRVAL *err_val, const TCHAR *title, const TCHAR *descr_fce(int))
{
	unsigned int code, i, err_no, len, n = 0, total_len;
	const TCHAR *ptext;
	TCHAR *pstr;

	if (err_val == NULL || descr_fce == NULL)
		return(FALSE);
	//
	total_len = 128;
	pstr = (TCHAR *)malloc(total_len*sizeof(TCHAR));
	if (pstr == NULL)
		return(FALSE);
	*pstr = '\0';

	//through bytes
	for (i=B2PROT_ERROR_DWORDS; i>0; i--) {
		code = err_val->dwCode[i-1];
		err_no = 32*i - 1;		//63, 31
		while (code) {
			if (code & 0x80000000) {
				ptext = descr_fce(err_no);
				len = ptext ? __strlen(ptext) : 0;
				if (len) {
					total_len += len+20;
					pstr = (TCHAR *)realloc(pstr, total_len*sizeof(TCHAR));
					if (pstr == NULL)
						return(FALSE);
					//add
					__sprintf(pstr, TEXT("%s%d - %s\r\n"), pstr, err_no, ptext);
				}

				n++;
			}
			code <<= 1;
			err_no--;
		}
	}

	//no error
	if (n == 0) {
		__strcpy(pstr, D_("None"));
	}

	//message box
	MessageBox(hwnd, pstr, title, MB_OK);

	//free memory
	free((void *)pstr);

	return(TRUE);
}

/** @brief Return top (highest) error
 *
 * @param err_val B2PROT_ERRVAL*
 * @return int
 *
 */
int B2PROT_GetTopError(B2PROT_ERRVAL *err_val)
{
	unsigned int code, i, err_no;

	//through bytes
	for (i=B2PROT_ERROR_DWORDS; i>0; i--) {
		code = err_val->dwCode[i-1];
		err_no = 32*i - 1;		//63, 31
		while (code) {
			if (code & 0x80000000) {
				return(err_no);
			}
			code <<= 1;
			err_no--;
		}
	}
	return(-1);
}

/** @brief Decode errors/warnings to message box
 *
 * @param hs HSTRING
 * @param err_val B2PROT_ERRVAL*
 * @param descr_fce
 * @return unsigned int Number of errors
 *
 */
unsigned int B2PROT_AppendDecodedErrors(HSTRING hs, B2PROT_ERRVAL *err_val, const TCHAR *descr_fce(int))
{
	unsigned int code, i, err_no, n = 0;

	if (hs == NULL || err_val == NULL)
		return(-1);
	//
	//through dwords
	for (i=0; i<B2PROT_ERROR_DWORDS; i++) {
		code = err_val->dwCode[i];
		err_no = 32*i + 0;		//0, 32
		while (code) {
			if (code & 0x1) {
				HSTR_AppendPrintf(hs, TEXT("%sERR%02d:%s"), HSTR_GetStringLength(hs) ? TEXT(", ") : TEXT(""), err_no, descr_fce(err_no));
				n++;
			}
			code >>= 1;
			err_no++;
		}
	}
	return(n);
}

//-------------------

/** @brief Read user specification
 *
 * @param hD HDEVICE
 * @param pidx WORD
 * @param uspec TCHAR*
 * @param max_len int
 * @return DWORD
 *
 */
DWORD B2PROT_ReadUserSpec(HDEVICE hD, WORD pidx, TCHAR *uspec, int max_len)
{
	D_DEVICE *pD;
	unsigned short rlen;
	B2PROT_ST_USPEC *us;
	DWORD ret;
	int i, len;
	D_DEV_PROP *prop = NULL;
	TCHAR *ptext;

	//check pointers
	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//prop by index
	if (pidx > 0 && pidx < pD->n_p && pD->p[pidx].val.m.buf) {
		prop = &pD->p[pidx];
	}

	while (1) {
		//send command
		ret = B2PROT_CmdSendAndRcpt(&pD->com, B2PROT_CMD_GET_USPEC, 0, &rlen);
		if (ret != NO_ERROR)
			break;

		//check info data
		if (rlen < sizeof(B2PROT_ST_USPEC)) {
			ret = ERROR_INVALID_DATA;
			break;
		}
		us = (B2PROT_ST_USPEC *)B2PROT_GetReadBuf(&pD->com);
		//endianness

		//store data
		if (prop) {
				len = prop->val.m.size/sizeof(TCHAR);
				ptext = (TCHAR *)prop->val.m.buf;
				for (i=0; i<len && i<B2PROT_ST_USPEC_LEN; i++) {
					ptext[i] = (TCHAR)us->data[i];
				}
		}

		//copy data out
		if (uspec) {
				for (i=0; i<max_len && i<B2PROT_ST_USPEC_LEN; i++) {
					uspec[i] = (TCHAR)us->data[i];
				}
		}

		break;
	}
	if (ret == ERROR_INVALID_DATA)
		ret = 1;	//bad response

	return(ret);
}

/** @brief Write user's specification
 *
 * @param hD HDEVICE
 * @param pidx WORD
 * @param uspec TCHAR*
 * @param max_len int
 * @return DWORD
 *
 */
DWORD B2PROT_WriteUserSpec(HDEVICE hD, WORD pidx, TCHAR *uspec, int max_len)
{
	D_DEVICE *pD;
	B2PROT_ST_USPEC *us;
	DWORD ret;
	int i, len;
	D_DEV_PROP *prop = NULL;
	TCHAR *ptext;

	//check pointers
	if (hD == NULL || (uspec == NULL && max_len != 0))
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//prop by index
	if (pidx > 0 && pidx < pD->n_p) {
		prop = &pD->p[pidx];
	}

	//prepare data
	us = (B2PROT_ST_USPEC *)B2PROT_GetWriteBuf(&pD->com);
	//set value
	for (i=0; i<max_len && i<B2PROT_ST_USPEC_LEN; i++) {
		us->data[i] = (char)(uspec[i] & 0xFF);
	}
	//endianness

	//send command
	ret = B2PROT_CmdSendAndRcpt(&pD->com, B2PROT_CMD_SET_USPEC, sizeof(B2PROT_ST_USPEC), NULL);
	if (ret == ERROR_INVALID_DATA) {
		ret = 1;	//bad response
	}
	else if (ret == NO_ERROR && prop) {
		//store data
		if (prop) {
				len = prop->val.m.size/sizeof(TCHAR);
				ptext = (TCHAR *)prop->val.m.buf;
				for (i=0; i<len && i<B2PROT_ST_USPEC_LEN; i++) {
					ptext[i] = (TCHAR)us->data[i];
				}
		}
	}

	return(ret);
}


BOOL CALLBACK B2PROT_SetupUserSpecDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);

/** @brief Setup user's specification text
 *
 * @param hwnd HWND
 * @param hD HDEVICE
 * @param pidx WORD
 * @return DWORD
 *
 */
DWORD B2PROT_SetupUserSpec(HWND hdlg, HDEVICE hD, WORD pidx)
{
	D_DEVICE *pD;
	DWORD ret;
	TCHAR uspec[B2PROT_ST_USPEC_LEN];
	int sel;

	//check pointers
	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//check communication
	if (COM_IsOpened(&pD->com) || pD->ufce.pOpen == NULL || pD->ufce.pTestID == NULL || pD->ufce.pClose == NULL)
		return(ERROR_INVALID_HANDLE_STATE);

	//load actual com. parameters
	SendMessage(GetParent(hdlg), WM_USER+115, 0, 0);		//WMU_SETINTER

	//open communication
	ret = pD->ufce.pOpen(hD);
	if (ret != NO_ERROR)
		return(ret);

	while (1) {
		//test ID
		if (!pD->ufce.pTestID(hD, NULL, NULL, NULL, NULL)) {
			ret = ERROR_FILE_NOT_FOUND;
			break;
		}

		//read uspec
		ret = B2PROT_ReadUserSpec(hD, -1, uspec, B2PROT_ST_USPEC_LEN);
		if (ret != NO_ERROR)
			break;

		//show setup dialog
		sel = CreateSimpleDialogBoxIndirect((HINSTANCE)GetWindowLong(hdlg, GWLP_HINSTANCE),
						hdlg, 0, D_("Setup user's specification"),
						0, 0, D_PA_GW(1), D_PA_GH(3),		//???
						B2PROT_SetupUserSpecDlgProc, (LPARAM)uspec);

		if (sel == 1) {
			//write uspec
			ret = B2PROT_WriteUserSpec(hD, -1, uspec, B2PROT_ST_USPEC_LEN);
			if (ret != NO_ERROR)
				break;
		}

		break;
	}

	//close communication
	pD->ufce.pClose(hD);

	return(ret);
}

//*** function processes ready dialog - device on-line configuration
BOOL CALLBACK B2PROT_SetupUserSpecDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	TCHAR *uspec = NULL;
	HWND hctl;
	int i;

	if (msg == WM_INITDIALOG) {
		if (lparam == 0)
			EndDialog(hdlg, -1);		//error

		//store handle
		uspec = (TCHAR *)lparam;
		SetWindowLong(hdlg, GWLP_USERDATA, (LONG)uspec);		//store handle !!!!!

		//set window size
		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = D_PA_GW(1)+D_PA_S1;
		rc.bottom = D_PA_GH(3)+D_PA_S1;
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		MoveWindow(hdlg, 0, 0, rc.right-rc.left, rc.bottom-rc.top, TRUE);

		i = 0;
		//--- groupbox
		hctl = CreateWindow(CLASS_BUTTON, D_("User's specification"),
												WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
												D_PA_COLGP(0), D_PA_ROWGP(i), D_PA_GW(1), D_PA_GH(3),
												hdlg, (HMENU)IDC_SSTATIC, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

		i = 2;
		//set button
		hctl = CreateWindow(CLASS_BUTTON, D_("Set"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												D_PA_COLST(0)+D_PA_GWIN(1)-D_PA_S2-2*D_PA_WB, D_PA_CTOF2(i), D_PA_WB, D_PA_HOF,
												hdlg, (HMENU)IDOK, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//cancel button
		hctl = CreateWindow(CLASS_BUTTON, D_("Cancel"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												D_PA_COLST(0)+D_PA_GWIN(1)-D_PA_WB, D_PA_CTOF2(i), D_PA_WB, D_PA_HOF,
												hdlg, (HMENU)IDCANCEL, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

		//editbox
		i = 0;
		hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""),
													WS_CHILD | WS_VISIBLE | WS_TABSTOP,
													D_PA_COLST(0), D_PA_ROWVAL(i), D_PA_GWIN(1), D_PA_H, hdlg, (HMENU)11, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		SendMessage(hctl, EM_SETLIMITTEXT, B2PROT_ST_USPEC_LEN, 0);
		SetWindowText(hctl, uspec);

		//scale window
		ScaleParentWindowByScreen(hdlg);

		//center window
		CenterParentWindow(hdlg);
	}
	else {
		uspec = (TCHAR*)GetWindowLong(hdlg, GWLP_USERDATA);		//get device handle

		//process rest messages
		switch(msg) {

			case WM_COMMAND:
				if (lparam) {
					if (HIWORD(wparam) == BN_CLICKED) {
						switch(LOWORD(wparam)) {
							case IDOK:
								//get text
								GetWindowText(GetDlgItem(hdlg, 11), uspec, B2PROT_ST_USPEC_LEN);
								SendMessage(hdlg, WM_CLOSE, 0, 1);		//close message (set)
								return TRUE;
							case IDCANCEL:
								SendMessage(hdlg, WM_CLOSE, 0, 0);		//close message (cancel)
								return TRUE;
							default:
								break;
						}
					}
				}
				break;

			case WM_CLOSE:
				EndDialog(hdlg, lparam);		//cancel
				return TRUE;

			case WM_DESTROY:


				break;
		}
	}
	return FALSE;
}




