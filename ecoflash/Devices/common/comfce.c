/*
 * comfce.c
 *
 * Communication Functions - source file
 *
 * Author: Filip Kinovic
 * Version: 2.1
 * Date: 30.06.2016
*/

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "comfce.h"
#include "list.h"
//#include "timer.h"
//#include "useful.h"
#include "memwatch.h"



//**** Global variables **************************************/

//direct logging
BOOL COM_log_enable = FALSE;
TCHAR COM_log_path[MAX_PATH] = TEXT("");
const char *COM_log_prognamea = "";
//
static DWORD g_log_count = 0;



//**** Function Definitions **********************************/

//-----------------------------------------------
//*** function inits libraries
BOOL COM_Init(void)
{
	//return(ETH_InitiateWSA());
	return true;
}

//-----------------------------------------------
//*** function deinits libraries
BOOL COM_DeInit(void)
{
	//return(ETH_DeinitiateWSA());
	return true;
}

//-----------------------------------------------
//setup
DWORD COM_Reset(COMH *hC);
DWORD COM_Setup(COMH *hC, COM_MODE mode, COM_LOGFORMAT format, unsigned int wbufsize, unsigned rbufsize);


//-----------------------------------------------
//*** function creates data buffers according to sizes
DWORD COM_CreateBuffers(COMH *hC)
{
	unsigned int n;

	//test handle
	assert(hC);

	//test for dynamic buffers
	if (hC->alloc && (hC->sendbufsize > COM_MIN_WBUF_LEN || hC->rcptbufsize > COM_MIN_RBUF_LEN)) {
		//allocate dynamic buffers

		//check sizes
		if (hC->sendbufsize < COM_MIN_WBUF_LEN)
			hC->sendbufsize = COM_MIN_WBUF_LEN;
		if (hC->rcptbufsize < COM_MIN_RBUF_LEN)
			hC->rcptbufsize = COM_MIN_RBUF_LEN;

		n = (hC->sendbufsize+hC->rcptbufsize+4)*sizeof(unsigned char);

		//allocate buffers
		hC->sendbuf = malloc(n);
		if (hC->sendbuf == NULL)
			return(ERROR_OUTOFMEMORY);

		//clear memory
		memset(hC->sendbuf, 0, n);

		//rcpt pointer
		hC->rcptbuf = hC->sendbuf+hC->sendbufsize;
	}
	else {
		//use static buffers
		hC->alloc = 0;
		hC->sendbuf = hC->sendbuff;
		hC->sendbufsize = COM_MIN_WBUF_LEN;
		hC->rcptbuf = hC->rcptbuff;
		hC->rcptbufsize = COM_MIN_RBUF_LEN;
	}

	return(NO_ERROR);
}

//-----------------------------------------------
//*** function destroyes data buffers
DWORD COM_DestroyBuffers(COMH *hC)
{
	//test handle
	assert(hC);

	//test for dynamic buffers
	if (hC->alloc) {
		if (hC->sendbuf) {
			free((void *)hC->sendbuf);
		}
	}

	hC->sendbuf = NULL;
	hC->rcptbuf = NULL;

	return(NO_ERROR);
}

//-----------------------------------------------
//*** function opens and configure connection
DWORD COM_OpenAndConf(COMH *hC)
{
	HANDLE h;
	DWORD ret;
#ifdef COM_MEMLOGGING
	char s[100] = "";
#endif

	//test handle
	assert(hC);

	//open connection
	switch (hC->mode) {
		default:
		case COM_MODE_RS232:
			h = RS232_OpenConfPortNo(hC->ser_pars.portno,
															 hC->ser_pars.baudrate,
															 hC->ser_pars.databits,
															 hC->ser_pars.parity,
															 hC->ser_pars.stopbits,
															 hC->ser_pars.handshake,
															 hC->ser_pars.insize,
															 hC->ser_pars.outsize);
			break;
		/*case COM_MODE_ETHERNET:
			h = ETH_OpenTCPSocket(hC->eth_pars.ip,
														hC->eth_pars.port,
														hC->eth_pars.cn_timeout);
			break;*/
	}
	if (h == NULL)
		ret = GetLastError();
	else
		ret = NO_ERROR;

	//create userid
	switch (hC->mode) {
		default:
		case COM_MODE_RS232:
			__sprintf(hC->userid, TEXT("COM%d"), hC->ser_pars.portno);
#ifdef COM_MEMLOGGING
#ifdef UNICODE
			sprintf(s, "COM%d/%d/%d/%ls/%ls",
#else
			sprintf(s, "COM%d/%d/%d/%s/%s",
#endif
								hC->ser_pars.portno,
								hC->ser_pars.baudrate,
								hC->ser_pars.databits,
								RS232_GetParityAbbrString(hC->ser_pars.parity),
								RS232_GetStopbitString(hC->ser_pars.stopbits));
#endif
			break;
		/*case COM_MODE_ETHERNET:
			__sprintf(hC->userid, TEXT("%s:%d"), hC->eth_pars.ip, hC->eth_pars.port);
#ifdef COM_MEMLOGGING
#ifdef UNICODE
			sprintf(s, "%ls:%d", hC->eth_pars.ip, hC->eth_pars.port);
#else
			sprintf(s, "%s:%d", hC->eth_pars.ip, hC->eth_pars.port);
#endif
#endif
			break;*/
		case COM_MODE_USERID:
			__sprintf(hC->userid, TEXT("USB"));
			break;
	}

	//log
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_OPEN, ret, s, strlen(s));
#endif

	if (ret != NO_ERROR)
		return(ret);

	hC->hComm = h;

	//create buffers
	ret = COM_CreateBuffers(hC);
	if (ret != NO_ERROR) {
		COM_Close(hC);
	}

	return(ret);
}

//-----------------------------------------------
//*** function closes connection (if opened)
BOOL COM_Close(COMH *hC)
{
	BOOL bret = FALSE;

	//test handle
	assert(hC);

	if (hC->hComm) {
		//destory buffers
		COM_DestroyBuffers(hC);

		//close connection
		switch (hC->mode) {
			default:
			case COM_MODE_RS232:
				bret = RS232_ClosePort(hC->hComm);
				break;
			/*case COM_MODE_ETHERNET:
				bret = ETH_CloseTCPSocket(hC->hComm);
				break;*/
		}

		hC->hComm = NULL;

		//log
#ifdef COM_MEMLOGGING
		COM_MemLogStore(hC, COM_MLOGMODE_CLOSE, bret ? NO_ERROR : ERROR_FILE_INVALID, NULL, 0);
#endif

	}

	return(bret);
}

//-----------------------------------------------
//*** function checks opened connection
BOOL COM_IsOpened(COMH *hC)
{
	//test handle
	assert(hC);

	return(hC->hComm ? TRUE : FALSE);
}

//-----------------------------------------------
//function reopens connection
DWORD COM_ReOpen(COMH *hC)
{
	COM_Close(hC);
	return(COM_OpenAndConf(hC));
}

//----------------------------------------------------------

//-----------------------------------------------
//*** function writes data immediatelly (not logged)
DWORD COM_WriteRaw(COMH *hC, char *wbuf, unsigned int wlen, unsigned int *bwritten)
{
	//test handle
	assert(hC);

	//test mode
	switch(hC->mode) {
		default:
		case COM_MODE_RS232:
			return(RS232_Write(hC->hComm, wbuf, wlen, bwritten));
		/*case COM_MODE_ETHERNET:
			return(ETH_Write(hC->hComm, wbuf, wlen, bwritten));*/
	}
}

//-----------------------------------------------
//*** function reads data immediatelly (not logged, no timeout)
DWORD COM_IReadRaw(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread)
{
	//test handle
	assert(hC);

	//test mode
	switch(hC->mode) {
		default:
		case COM_MODE_RS232:
			return(RS232_ReadIm(hC->hComm, rbuf, rlen, bread));
		/*case COM_MODE_ETHERNET:
			return(ETH_ReadIm(hC->hComm, rbuf, rlen, bread));*/
	}
}

//-----------------------------------------------
//*** function checkes if any data to read
//note: 0=no data, -1=error, >0=data available
int COM_CanRead(COMH *hC)
{
	//test handle
	assert(hC);

	//test mode
	switch(hC->mode) {
		default:
		case COM_MODE_RS232:
			return(RS232_CanRead(hC->hComm));
		/*case COM_MODE_ETHERNET:
			return(ETH_CanRead(hC->hComm));*/
	}
}

//-----------------------------------------------
//*** function flushes buffer(s)
DWORD COM_Flush(COMH *hC)
{
	//test handle
	assert(hC);

	//test mode
	switch(hC->mode) {
		default:
		case COM_MODE_RS232:
			return(RS232_Flush(hC->hComm));
		/*case COM_MODE_ETHERNET:
			return(ETH_Flush(hC->hComm));*/
	}
}


//-------------- Raw (not logged) ----------------------------------------------------------------

//-----------------------------------------------
//*** function reads data (not logged)
//note: ms_timeout=0 -> immediate read
DWORD COM_ReadRaw(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread, unsigned int ms_timeout)
{
	DWORD ret;
	double t0;
	unsigned int i = 0, n;

	t0 = timer_GetTime();		//time reference
	while (1) {
		//immediate read
		ret = COM_IReadRaw(hC, rbuf+i, rlen-i, &n);
		if (ret != NO_ERROR)
			break;	//error
		//read size
		i += n;
		if (i >= rlen)
			break;
		//timeout
		if (ms_timeout == 0)
			break;		//no timeout
		if ((timer_GetTime()-t0) < (double)ms_timeout)
			COM_Sleep();
		else {
			ret = ERROR_TIMEOUT;		//error
			break;
		}
	}

	//bytes read
	if (bread)
		*bread = i;

	return(ret);
}

//-----------------------------------------------
//*** function reads terminated data by terminal character (not logged)
DWORD COM_TerminatedReadRaw(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread, unsigned int ms_timeout)
{
	DWORD ret;
	double t0;
	unsigned int i = 0, n;
	char *pt;

	//test handles
	assert(hC);

	t0 = timer_GetTime();		//time reference
	while (1) {
		//immediate read
		ret = COM_IReadRaw(hC, rbuf+i, rlen-i, &n);
		if (ret != NO_ERROR)
			break;	//error
		//terminated char (from backward)
		pt = memrchr(rbuf+i, hC->terminal_char, n);
		if (pt) {
			i = pt+1-rbuf;
			break;
		}
		//read size
		i += n;
		if (i >= rlen)
			break;
		//timeout
		if ((timer_GetTime()-t0) < (double)ms_timeout)
			COM_Sleep();
		else {
			ret = ERROR_TIMEOUT;		//error
			break;
		}
	}

	//bytes read
	if (bread)
		*bread = i;

	return(ret);
}

//-----------------------------------------------
//*** function reads to empty reception (not logged)
DWORD COM_EmptyReadRaw(COMH *hC, char *rbuf, unsigned int rlen)
{
	DWORD ret = NO_ERROR;
	unsigned int n = 0;

	//flush
	COM_Flush(hC);

	while (COM_CanRead(hC)) {
		//read
		ret = COM_IReadRaw(hC, rbuf, rlen, &n);
		if (ret != NO_ERROR)
			break;
	}

	return(ret);
}



//-----------------------------------------------
//*** function writes data (logged)
DWORD COM_Write(COMH *hC, char *wbuf, unsigned int wlen, unsigned int *bwritten)
{
	DWORD ret;
	unsigned int n = 0;

	//write
	ret = COM_WriteRaw(hC, wbuf, wlen, &n);
	if (bwritten)
		*bwritten = n;

	//error
	COM_SetError(hC, ret);

	//log
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_WRITE, ret, wbuf, n);
#endif
#ifdef COM_LOGGING
	COM_LogData(hC, ret, wbuf, n, NULL, 0);
#endif
	return(ret);
}

//-----------------------------------------------
//*** function reads data (logged)
//note: ms_timeout=0 -> immediate read
DWORD COM_Read(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread, unsigned int ms_timeout)
{
	DWORD ret;
	unsigned int nr = 0;

	//read
	ret = COM_ReadRaw(hC, rbuf, rlen, &nr, ms_timeout);
	//bytes read
	if (bread)
		*bread = nr;

	//error
	COM_SetError(hC, ret);

	//log
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_READ, ret, rbuf, nr);
#endif
#ifdef COM_LOGGING
	COM_LogData(hC, ret, NULL, 0, rbuf, nr);
#endif

	return(ret);
}

//-----------------------------------------------
//*** function reads terminated data by terminal character (logged)
DWORD COM_TerminatedRead(COMH *hC, char *rbuf, unsigned int rlen, unsigned int *bread, unsigned int ms_timeout)
{
	DWORD ret;
	unsigned int nr = 0;

	ret = COM_TerminatedReadRaw(hC, rbuf, rlen, &nr, ms_timeout);

	//error
	COM_SetError(hC, ret);

	//log
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_READ, ret, rbuf, nr);
#endif
#ifdef COM_LOGGING
	COM_LogData(hC, ret, NULL, 0, rbuf, nr);
#endif

	if (bread)
		*bread = nr;

	return(ret);
}

//-----------------------------------------------
//*** function writes and reads by terminal character (logged)
DWORD COM_WriteAndRead(COMH *hC, char *wbuf, unsigned int wlen,  char *rbuf, unsigned int rlen, unsigned int *bread, unsigned int ms_timeout)
{
	DWORD ret;
	unsigned int nr = 0;
	unsigned int nw = 0;

	//flush fifos
	COM_Flush(hC);

	//write
	ret = COM_WriteRaw(hC, wbuf, wlen, &nw);
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_WRITE, ret, wbuf, nw);
#endif
	if (ret == NO_ERROR) {
		//terminated read
		ret = COM_TerminatedReadRaw(hC, rbuf, rlen, &nr, ms_timeout);
#ifdef COM_MEMLOGGING
		COM_MemLogStore(hC, COM_MLOGMODE_READ, ret, rbuf, nr);
#endif
	}

	//error
	COM_SetError(hC, ret);

	//log
#ifdef COM_LOGGING
	COM_LogData(hC, ret, wbuf, nw, rbuf, nr);
#endif

	if (bread)
		*bread = nr;

	return(ret);
}

//----- Using internal buffers -------------------------------

//-----------------------------------------------
//*** function empty reception using internal buffer
DWORD COM_Empty(COMH *hC)
{
	DWORD ret = NO_ERROR;
	unsigned int n = 0;

	//test pointer
	assert(hC);

	//flush
	COM_Flush(hC);

	double t0 = timer_GetTime();		//time reference
	while (COM_CanRead(hC) && (timer_GetTime()-t0) < (double)hC->timeout) {
		//read
		ret = COM_IReadRaw(hC, hC->rcptbuf, hC->rcptbufsize, &n);
		if (ret != NO_ERROR)
			break;
	}
	return(ret);
}

//-----------------------------------------------
//*** function writes using internal buffers (terminal character, logged)
DWORD COM_Send(COMH *hC, unsigned int wlen)
{
	DWORD ret;
	unsigned int nw = 0;

	//test pointer
	assert(hC);

	//write string
	ret = COM_WriteRaw(hC, hC->sendbuf, wlen, &nw);
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_WRITE, ret, hC->sendbuf, nw);
#endif

	//error
	COM_SetError(hC, ret);

	//log
#ifdef COM_LOGGING
	COM_LogData(hC, ret, hC->sendbuf, nw, NULL, 0);
#endif

	return(ret);
}

//-----------------------------------------------
//*** function writes and reads using internal buffers, flushes buffers (terminal character, logged)
DWORD COM_SendAndRcpt(COMH *hC, unsigned int wlen, unsigned int *bread)
{
	DWORD ret;
	unsigned int nw = 0, nr = 0;

	//test pointer
	assert(hC);

	//flush buffers
	COM_Flush(hC);

	//write string
	ret = COM_WriteRaw(hC, hC->sendbuf, wlen, &nw);
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_WRITE, ret, hC->sendbuf, nw);
#endif
	if (ret == NO_ERROR) {
		//terminated read
		ret = COM_TerminatedReadRaw(hC, hC->rcptbuf, hC->rcptbufsize, &nr, hC->timeout);
#ifdef COM_MEMLOGGING
		COM_MemLogStore(hC, COM_MLOGMODE_READ, ret, hC->rcptbuf, nr);
#endif
	}

	//error
	COM_SetError(hC, ret);

	//log
#ifdef COM_LOGGING
	COM_LogData(hC, ret, hC->sendbuf, nw, hC->rcptbuf, nr);
#endif

	if (bread)
		*bread = nr;

	return(ret);
}

//-----------------------------------------------
//*** function writes string using internal buffers (terminal character, logged)
DWORD COM_SendStr(COMH *hC)
{
	DWORD ret;
	unsigned int nw = 0;

	//test pointer
	assert(hC);

	//write string
	ret = COM_WriteRaw(hC, hC->sendbuf, strlen(hC->sendbuf), &nw);
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_WRITE, ret, hC->sendbuf, nw);
#endif

	//error
	COM_SetError(hC, ret);

	//log
#ifdef COM_LOGGING
	COM_LogData(hC, ret, hC->sendbuf, nw, NULL, 0);
#endif

	return(ret);
}

//-----------------------------------------------
//*** function writes and reads string using internal buffers, flushes buffers (terminal character, logged)
//note: buffers are not flushed if nothing to send
DWORD COM_SendAndRcptStr(COMH *hC)
{
	DWORD ret;
	unsigned int nw = 0, nr = 0;

	//test pointer
	assert(hC);

	//check if something to send
	nw = strlen((char *)hC->sendbuf);
	if (nw) {
		//flush buffers
		COM_Flush(hC);
	}

	//write string
	ret = COM_WriteRaw(hC, hC->sendbuf, nw, &nw);
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_WRITE, ret, hC->sendbuf, nw);
#endif
	if (ret == NO_ERROR) {
		*hC->rcptbuf = '\0';	//reset string
		//terminated read
		ret = COM_TerminatedReadRaw(hC, hC->rcptbuf, hC->rcptbufsize, &nr, hC->timeout);
		//add null character to being string
		hC->rcptbuf[nr] = '\0';
#ifdef COM_MEMLOGGING
		COM_MemLogStore(hC, COM_MLOGMODE_READ, ret, hC->rcptbuf, nr);
#endif
	}

	//error
	COM_SetError(hC, ret);

	//log
#ifdef COM_LOGGING
	COM_LogData(hC, ret, hC->sendbuf, nw, hC->rcptbuf, nr);
#endif

	return(ret);
}

//*** function writes and reads string using internal buffers, flushes buffers (eliminate terminal character, logged)
//note: buffers are not flushed if nothing to send
DWORD COM_SendAndRcptStr2(COMH *hC)
{
	DWORD ret;
	unsigned int nw = 0, nr = 0;

	//test pointer
	assert(hC);

	//check if something to send
	nw = strlen((char *)hC->sendbuf);
	if (nw) {
		//flush buffers
		COM_Flush(hC);
	}

	//write string
	ret = COM_WriteRaw(hC, hC->sendbuf, nw, &nw);
#ifdef COM_MEMLOGGING
	COM_MemLogStore(hC, COM_MLOGMODE_WRITE, ret, hC->sendbuf, nw);
#endif
	if (ret == NO_ERROR) {
		*hC->rcptbuf = '\0';	//reset string
		//terminated read
		ret = COM_TerminatedReadRaw(hC, hC->rcptbuf, hC->rcptbufsize, &nr, hC->timeout);
		//add null character to being string
		hC->rcptbuf[nr] = '\0';
#ifdef COM_MEMLOGGING
		COM_MemLogStore(hC, COM_MLOGMODE_READ, ret, hC->rcptbuf, nr);
#endif
	}

	//error
	COM_SetError(hC, ret);

	//log
#ifdef COM_LOGGING
	COM_LogData(hC, ret, hC->sendbuf, nw, hC->rcptbuf, nr);
#endif

	if (nr && hC->rcptbuf[nr-1] == hC->terminal_char) {
		hC->rcptbuf[nr-1] = '\0';
	}

	return(ret);
}

//-----------------------------------------------
//*** function reads to empty reception using internal buffer (not logged)
DWORD COM_EmptyRcptRaw(COMH *hC)
{
	//test pointer
	assert(hC);

	return(COM_EmptyReadRaw(hC, hC->rcptbuf, hC->rcptbufsize));
}



//------------------------------------------------------------------------------

//-----------------------------------------------
//*** function logs comunication to log file (using hC->log_format)
void COM_LogData(COMH *hC, DWORD ret, char *wbuf, unsigned int wlen, char *rbuf, unsigned int rlen)
{
	FILE *fw = NULL;
	SYSTEMTIME st;
	char *xstr;

	//test handle
	assert(hC);

	//is logging enabled
	if (!COM_log_enable)
		return;

	if (g_log_count == 0) {
		g_log_count++;
		//do first work (begin file)

		//file head
		if ((fw = __fopen(COM_log_path, TEXT("w"))) != NULL) {
			fprintf(fw, "Communication log file");
			if (COM_log_prognamea && *COM_log_prognamea) {
				fprintf(fw, " created by %s", COM_log_prognamea);
			}
			fprintf(fw, "\n\n");

			fprintf(fw, "no.\terror\ttime & date\tport\tsend command\tresponse\n");
			fclose(fw);
		}
	}

	if ((fw = __fopen(COM_log_path, TEXT("a"))) != NULL) {		//add to file
		GetLocalTime(&st);
		fprintf(fw, "%ld\t%s%ld\t%02d:%02d:%02d.%03d %02d.%02d.%04d\t",
						g_log_count++,
						ret != 0 ? "*" : "",
						ret,
						st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, st.wDay, st.wMonth, st.wYear);
		switch (hC->mode) {
			default:
			case COM_MODE_RS232:
				fprintf(fw, "COM%d/%d\t", hC->ser_pars.portno, hC->ser_pars.baudrate);
				break;
			case COM_MODE_ETHERNET:
				fprintf(fw, CSTR_S2 ":%d\t", ETH_MAX_IP_LEN, hC->eth_pars.ip, hC->eth_pars.port);
				break;
			case COM_MODE_USERID:
				fprintf(fw, CSTR_S2 "\t", COM_MAX_USERID_LEN, hC->userid);
				break;
		}

		//C-string
		if (hC->log_format == COM_LOGFORMAT_TEXT) {
			//written
			if (wbuf && wlen) {
				xstr = NULL;
				StringToCString((char *)wbuf, wlen, &xstr);
				if (xstr) {
					fprintf(fw, "%s", xstr);
					free((void *)xstr);
				}
			}
			fprintf(fw, "\t");
			//read
			if (rbuf && rlen) {
				xstr = NULL;
				StringToCString((char *)rbuf, rlen, &xstr);
				if (xstr) {
					fprintf(fw, "%s", xstr);
					free((void *)xstr);
				}
			}
		}
		//HEX-string
		else {
			//written
			if (wbuf && wlen) {
				xstr = NULL;
				StringToHexString((char *)wbuf, wlen, &xstr);
				if (xstr) {
					fprintf(fw, "%s", xstr);
					free((void *)xstr);
				}
			}
			fprintf(fw, "\t");
			//read
			if (rbuf && rlen) {
				xstr = NULL;
				StringToHexString((char *)rbuf, rlen, &xstr);
				if (xstr) {
					fprintf(fw, "%s", xstr);
					free((void *)xstr);
				}
			}
		}
		fprintf(fw, "\n");
	}

#ifdef COM_COMMDEBUG
	QueryPerformanceCounter(&t1);
	QueryPerformanceFrequency(&freq);
	fprintf(fw, "dt [ms]: %lf\n", (double)(t1.QuadPart-t0.QuadPart)*1000.0/(double)freq.QuadPart);
#endif

	if (fw)
		fclose(fw);

}

//--------- LOGGING --------------------------------------------

//declare functions
BOOL COM_MemLogFPrintItem(FILE *f, COMMLOGITEM *pitem, int c_format);


//global log variables
static COMMLOG g_commlog = {NULL, 0, 0};

/* function processes IPROC function for working with TCHAR strings */
//IPTYPE_CREATE: iparam -> pointer to lparam of item, lparam -> LPARAM
//IPTYPE_DELETE: iparam -> pointer to lparam of item, lparam -> 0
//IPTYPE_PRINT: iparam -> pointer to lparam of item, lparam -> FILE *
//IPTYPE_COMPARE: iparam -> pointer to lparam of item, lparam of other item -> LPARAM
BOOL COM_MemLogIProcParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	COMMLOGITEM *pitem;

	if (iparam) {
		switch (type) {
			//--- processed after ITEM creation
			case IPTYPE_CREATE:
				if (lparam) {
					//pointer na item
					pitem = (COMMLOGITEM *)lparam;
					//check content
					if (pitem->size && pitem->data == NULL)
						return(FALSE);
					//allocate memory
					*iparam = (LPARAM)malloc(sizeof(COMMLOGITEM)+pitem->size);
					if (*iparam == 0)
						return(FALSE);
					//copy data
					memcpy((void *)*iparam, pitem, sizeof(COMMLOGITEM));
					if (pitem->size) {
						memcpy((void *)(((char *)*iparam)+sizeof(COMMLOGITEM)), pitem->data, pitem->size*sizeof(char));
						((COMMLOGITEM *)*iparam)->data = ((char *)*iparam)+sizeof(COMMLOGITEM);
					}
					else {
						((COMMLOGITEM *)*iparam)->data = NULL;
					}
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
				if (lparam) {
					if (*iparam) {
						pitem = (COMMLOGITEM *)*iparam;
						//print item (c-format)
						COM_MemLogFPrintItem((FILE *)lparam, (COMMLOGITEM *)*iparam, TRUE);
					}
					else
						fprintf((FILE *)lparam, "(null)");
				}
				break;
			//--- processed comparing items
			case IPTYPE_COMPARE:
				return(0);
		}
	}
	return(TRUE);
}


/* function create log-list and enable logging */
BOOL COM_MemLogCreate(void)
{
	//destroy if already exist
	COM_MemLogDestroy();

	//create new log-list
	g_commlog.list = LIST_Create(COM_MemLogIProcParam);
	g_commlog.enable = TRUE;		//enable
	return(g_commlog.list ? TRUE : FALSE);
}

/* function destroy log-list */
BOOL COM_MemLogDestroy(void)
{
	g_commlog.idx = 0;
	if (LIST_Discard(g_commlog.list)) {
		g_commlog.list = NULL;
		return(TRUE);
	}
	else
		return(FALSE);
}

/* function reset log content */
BOOL COM_MemLogReset(int reset_counter)
{
	BOOL bret;

	//remove all items
	bret = LIST_RemoveAll(g_commlog.list);
	//reset counter
	if (reset_counter) {
		g_commlog.idx = 0;
	}
	return(bret);
}

/* finction enables/disables logging */
BOOL COM_MemLogEnable(BOOL enable)
{
	g_commlog.enable = enable;
	return(TRUE);
}


/* function stores actual data to log-list */
BOOL COM_MemLogStore(COMH *hC, COM_MLOGMODE mode, DWORD ret, char *buf, unsigned int size)
{
	COMMLOGITEM item;

	if (g_commlog.enable) {
		item.index = g_commlog.idx++;		//increment index
		item.mode = mode;
		item.result = ret;
		lstrcpyn(item.portname, hC->userid, COM_MAX_USERID_LEN);
		GetLocalTime(&item.st);
		item.size = size;
		item.data = buf;
		return(LIST_AppendItem(g_commlog.list, (LPARAM)&item));
	}
	return(FALSE);
}

//---
/* function print item to file */
BOOL COM_MemLogFPrintItem(FILE *f, COMMLOGITEM *pitem, int c_format)
{
	DWORD i;
	char *pdata;

	//index <tab> result <tab> time date <tab> port <tab> mode <tab> size <tab>
#ifdef UNICODE
	fprintf(f, "%05lu\t%ld\t%02d:%02d:%02d.%03d\t%-20ls\t%c\t%lu\t",
#else
	fprintf(f, "%05lu\t%ld\t%02d:%02d:%02d.%03d\t%-20s\t%c\t%lu\t",
#endif
		pitem->index,
		pitem->result,
		pitem->st.wHour, pitem->st.wMinute, pitem->st.wSecond, pitem->st.wMilliseconds,
		//pitem->st.wDay, pitem->st.wMonth, pitem->st.wYear,
		pitem->portname,
		pitem->mode,
		pitem->size);
	//data (text invalid characters --> c-characters)
	i = pitem->size;
	pdata = pitem->data;

	//format
	if (c_format || (pitem->mode != COM_MLOGMODE_READ && pitem->mode != COM_MLOGMODE_WRITE)) {
		//C-format
		while (i) {
			//check byte
			switch (*pdata) {
				case '\a': fprintf(f, "\\a"); break;
				case '\b': fprintf(f, "\\b"); break;
				case '\f': fprintf(f, "\\f"); break;
				case '\n': fprintf(f, "\\n"); break;
				case '\r': fprintf(f, "\\r"); break;
				case '\t': fprintf(f, "\\t"); break;
				case '\v': fprintf(f, "\\v"); break;
				case '\'': fprintf(f, "\\\'"); break;
				case '\"': fprintf(f, "\\\""); break;
				case '\\': fprintf(f, "\\\\"); break;
				default:
					if (*pdata < 0x20)
						fprintf(f, "\\x%02X", *pdata);
					else
						fputc(*pdata, f);
					break;
			}
			i--;
			pdata++;
		}
	}
	else {
		//HEX-format
		while (i) {
			fprintf(f, "%02X", (unsigned char)*pdata);
			i--;
			pdata++;
		}
	}

	return(TRUE);
}

/* function print log-list to file */
BOOL COM_MemLogWriteToFile(const TCHAR *filename, const TCHAR *appname, int c_format)
{
	FILE *fw = NULL;
	HITEM hitem;
	COMMLOGITEM *pitem;
	unsigned int cnt[18] = {0};

	//file head
	if ((fw = __fopen(filename, TEXT("w"))) != NULL) {
		fprintf(fw, "#Communication log file");
		if (appname) {
#ifdef UNICODE
			fprintf(fw, " created by %ls", appname);
#else
			fprintf(fw, " created by %s", appname);
#endif
		}
		fprintf(fw, "\n#\n");

		//index <tab> result <tab> time date <tab> port <tab> mode <tab> size <tab>
		fprintf(fw, "#no.\terror\t%-12s\t%-20s\tmode\tsize\tdata%s\n", "time", "port", c_format ? " (C-format)" : " (HEX)");

		//through items
		hitem = LIST_GetItem(g_commlog.list, LPOS_FIRST, 0);
		while (hitem && LIST_GetItemValue(hitem, (void *)&pitem)) {
			//print item (hex-format)
			COM_MemLogFPrintItem(fw, pitem, c_format);
			//EOL
			fputc('\n', fw);

			//calculate statistics
			switch (pitem->mode) {
				case COM_MLOGMODE_OPEN: 	cnt[0]++; cnt[1] += pitem->size; break;
				case COM_MLOGMODE_CLOSE: 	cnt[2]++; cnt[3] += pitem->size; break;
				case COM_MLOGMODE_SETUP: 	cnt[4]++; cnt[5] += pitem->size; break;
				case COM_MLOGMODE_READ: 	cnt[6]++; cnt[7] += pitem->size; break;
				case COM_MLOGMODE_WRITE: 	cnt[8]++; cnt[9] += pitem->size; break;
				case COM_MLOGMODE_INFO: 	cnt[10]++; cnt[11] += pitem->size; break;
				//case COM_MLOGMODE_CLEAR: 	cnt[12]++; cnt[13] += pitem->size; break;
				//case COM_MLOGMODE_BREAK: 	cnt[14]++; cnt[15] += pitem->size; break;
				//case COM_MLOGMODE_PARAM: 	cnt[16]++; cnt[17] += pitem->size; break;
				default: break;
			}

			//next item
			hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);
		}

		//some statistics
		fprintf(fw, "#\n#Staticstics\n");
		fprintf(fw, "# Opened  : %u (%uB)\n", 	cnt[0], cnt[1]);
		fprintf(fw, "# Closed  : %u (%uB)\n", 		cnt[2], cnt[3]);
		fprintf(fw, "# Setup   : %u (%uB)\n", 		cnt[4], cnt[5]);
		fprintf(fw, "# Read    : %u (%uB)\n", 		cnt[6], cnt[7]);
		fprintf(fw, "# Written : %u (%uB)\n", 	cnt[8], cnt[9]);
		fprintf(fw, "# Info    : %u (%uB)\n", 		cnt[10], cnt[11]);

		fclose(fw);

		return(TRUE);
	}
	return(FALSE);
}

//*** return static list
HLIST COM_GetLogList()
{
	return(g_commlog.list);
}

//------------------------------------------------------------------
//*** function matches response string by compare-mask
//MASK:
// '\x7F' -> any character
// 'z' -> text characters (A-Z)
// 't' -> boolean characters (T, F)
// '+' -> sign characters (+ or -)
// '1' -> decimal characters (0-1)
// '2' -> decimal characters (0-2)
// '3' -> decimal characters (0-3)
// '4' -> decimal characters (0-4)
// '5' -> decimal characters (0-5)
// '6' -> decimal characters (0-6)
// '7' -> decimal characters (0-7)
// '8' -> decimal characters (0-8)
// '9' -> decimal characters (0-9)
// 'a' -> hexadecimal characters (0-9, A)
// 'b' -> hexadecimal characters (0-9, A-B)
// 'c' -> hexadecimal characters (0-9, A-C)
// 'd' -> hexadecimal characters (0-9, A-D)
// 'e' -> hexadecimal characters (0-9, A-E)
// 'f' -> hexadecimal characters (0-9, A-F)
// '\x8X' -> rest is OK
// other -> should be the same
//return: TRUE -> match, FALSE -> doesn't match
BOOL COM_MathResponse2(const char *rcpt, const char *mask)
{
	BOOL done = TRUE;
	char *pr, *pc;

	if (rcpt == NULL || mask == NULL)
		return(FALSE);

	//compare strings
	pr = (char *)rcpt;
	pc = (char *)mask;
	while (done && *pr && *pc) {
		//rest is OK
		if (*pc >= '\x80')
			break;

		//test other mask characters
		switch (*pc) {
			//any character
			case '\x7F':
				break;
			//text characters (A-Z)
			case 'z':
				if (*pr < 'A' || *pr > 'Z')
					done = FALSE;		//not match
				break;
			//boolean characters (T, F)
			case 't':
				if (*pr != 'T' && *pr != 'F')
					done = FALSE;		//not match
				break;
			//sign characters (+,-)
			case '+':
				if (*pr != '+' && *pr != '-')
					done = FALSE;		//not match
				break;
			//decimal characters 0 up to max number
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if ((*pr < '0' || *pr > *pc))
					done = FALSE;		//not match
				break;
			//hexadecimal characters 0 up to max number
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				if ((*pr < '0' || *pr > '9') && (*pr < 'A' || *pr > (*pc-'a'+'A')))
					done = FALSE;		//not match
				break;
			//exact match
			default:
				if (*pr != *pc)
					done = FALSE;		//not match
				break;
		}
		pr++;
		pc++;
	}
	//test null character for length match
	if (*pc == '\0' && *pr != '\0')
		done = FALSE;

	return(done);
}




