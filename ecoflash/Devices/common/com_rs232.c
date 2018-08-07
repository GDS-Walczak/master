/*
 * com_rs232.c
 *
 * Communication through serial port - source file
 *
 * Author: Filip Kinovic
 * Version: 2.2
 * Date: 26.02.2016
*/

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "com_rs232.h"
//#include "useful.h"
#include "memwatch.h"



//****** General Function for Port Opening and Configuration ******

//-----------------------------------------------
//*** function tests if demanded port is available (tries to open only)
BOOL RS232_TestPort(const TCHAR *PortName)
{
  HANDLE hPort = NULL;

  //open port
  hPort = CreateFile(PortName,
                     GENERIC_READ | GENERIC_WRITE,
                     0,
                     0,
                     OPEN_EXISTING,
       				       FILE_FLAG_OVERLAPPED,
       				       0);
  //test return handle
  if (hPort == INVALID_HANDLE_VALUE)
    return(FALSE);
  //close port
  CloseHandle(hPort);
  return(TRUE);
}

//-----------------------------------------------
//*** function tests if demanded port number is available (tries to open only)
BOOL RS232_TestPortNo(int PortNo)
{
	TCHAR ttext[32];

	//make com string
	__sprintf(ttext, TEXT("\\\\.\\COM%d"), PortNo);
	//open new port
	return(RS232_TestPort(ttext));
}

//-----------------------------------------------
//*** function opens demanded comport
HANDLE RS232_OpenPort(const TCHAR *PortName)
{
	HANDLE hPort;

  //open port
  hPort = CreateFile(PortName,
                      GENERIC_READ | GENERIC_WRITE,
                      0,
                      0,
                      OPEN_EXISTING,
        					    FILE_FLAG_OVERLAPPED,
        					    0);
  //test return handle
  if (hPort == INVALID_HANDLE_VALUE)
    return(NULL);
	return(hPort);
}

//-----------------------------------------------
//*** function opens and configure demanded port number
HANDLE RS232_OpenConfPortNo(int PortNo,
														RS232_BAUDRATE baudrate,
														RS232_DATABITS databits,
														RS232_PARITY parity,
														RS232_STOPBITS stopbits,
														RS232_HANDSHAKE handshake,
														unsigned int inbuf, unsigned int outbuf)
{
	TCHAR ttext[32];
	HANDLE hPort;

	//make com string
	__sprintf(ttext, TEXT("\\\\.\\COM%d"), PortNo);

	//open port
	hPort = RS232_OpenPort(ttext);
	if (hPort == NULL)
		return(NULL);

	//configure port
	if (RS232_SetPortConf(hPort, baudrate, databits, parity, stopbits, handshake, inbuf, outbuf) != NO_ERROR) {
		RS232_ClosePort(hPort);
		return(NULL);
	}
	//set timeout (ignore errors)
	RS232_SetPortTimeout(hPort, 0);		//no read timeout!!!
	//purge queues (ignore errors)
	RS232_Flush(hPort);

	//done
	return(hPort);
}

//-----------------------------------------------
//*** function closes port
BOOL RS232_ClosePort(HANDLE hPort)
{
	//test handle
	assert(hPort);

  //close handle
  return(CloseHandle(hPort));
}

//-----------------------------------------------
//*** function tests port handle
BOOL RS232_IsPortOpened(HANDLE hPort)
{
	return(hPort==NULL ? FALSE : TRUE);
}

//-----------------------------------------------
//*** function sets configuration of port (baudrate, ...)
DWORD RS232_SetPortConf(HANDLE hPort,
												RS232_BAUDRATE baudrate,
												RS232_DATABITS databits,
												RS232_PARITY parity,
												RS232_STOPBITS stopbits,
												RS232_HANDSHAKE handshake,
												unsigned int inbuf, unsigned int outbuf)
{
  DCB dcb;
  DWORD ret;

	//test handle
	assert(hPort);

  //set buffer size
	SetupComm(hPort, inbuf, outbuf);

  if (!GetCommState(hPort, &dcb))
    return(GetLastError());

	dcb.DCBlength = sizeof(DCB);
  dcb.BaudRate = baudrate;
  dcb.ByteSize = databits;
  dcb.Parity = parity;
  dcb.StopBits = stopbits;

	//
	dcb.fBinary = TRUE;
	//handshake
	switch (handshake) {
		default:
		case RS232_HANDSHAKE_NONE:
			dcb.fOutxCtsFlow = FALSE;
			dcb.fRtsControl = RTS_CONTROL_ENABLE;
			dcb.fOutxDsrFlow = FALSE;
			dcb.fDtrControl = DTR_CONTROL_ENABLE;
			break;
		case RS232_HANDSHAKE_CTS_RTS:
			dcb.fOutxCtsFlow = TRUE;
			dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
			dcb.fOutxDsrFlow = FALSE;
			dcb.fDtrControl = DTR_CONTROL_ENABLE;
			break;
		case RS232_HANDSHAKE_DSR_DTR:
			dcb.fOutxCtsFlow = FALSE;
			dcb.fRtsControl = RTS_CONTROL_ENABLE;
			dcb.fOutxDsrFlow = TRUE;
			dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
			break;
		case RS232_HANDSHAKE_BOTH:
			dcb.fOutxCtsFlow = TRUE;
			dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
			dcb.fOutxDsrFlow = TRUE;
			dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
			break;
	}
	dcb.fDsrSensitivity = 0;
	dcb.fTXContinueOnXoff = 0;
	dcb.fOutX = 0;
	dcb.fInX = 0;
	dcb.fErrorChar = 0;
	dcb.fNull = 0;
	dcb.fAbortOnError = 0;		//no break on error
	dcb.XonLim = 0;            // transmit XON threshold
	dcb.XoffLim = 0;            // transmit XOFF threshold
	dcb.XonChar = 0;             // Tx and Rx XON character
	dcb.XoffChar = 0;            // Tx and Rx XOFF character
	dcb.ErrorChar = 0;            // error replacement character
	dcb.EofChar = 0;              // end of input character
	dcb.EvtChar = 0;              // received event character

  if (!SetCommState(hPort, &dcb)) {
  	ret = GetLastError();
  	return(ret);
  }

  return(NO_ERROR);
}

//-----------------------------------------------
//*** function gets configuration of port (baudrate, ...)
DWORD RS232_GetPortConf(HANDLE hPort,
												RS232_BAUDRATE *baudrate,
												RS232_DATABITS *databits,
												RS232_PARITY *parity,
												RS232_STOPBITS *stopbits,
												RS232_HANDSHAKE *handshake)
{
  DCB dcb;
  BYTE hs;

	//test handle
	assert(hPort);

  if (!GetCommState(hPort, &dcb))
    return(GetLastError());

	//get baudrate
  if (baudrate)
    *baudrate = dcb.BaudRate;   //get baudrate
	//get number of databits (4-8)
  if (databits)
    *databits = dcb.ByteSize;
	//get parity (0-4; none, odd, even, mark, space)
  if (parity)
    *parity = dcb.Parity;
	//get number of stopbits (0-2; 1, 1.5, 2)
  if (stopbits)
    *stopbits = dcb.StopBits;
	//get handshake
	if (handshake) {
		hs = RS232_HANDSHAKE_NONE;
		if (dcb.fRtsControl == RTS_CONTROL_HANDSHAKE)
			hs |= RS232_HANDSHAKE_CTS_RTS;
		if (dcb.fDtrControl == DTR_CONTROL_HANDSHAKE)
			hs |= RS232_HANDSHAKE_DSR_DTR;
		*handshake = hs;
	}

  return(NO_ERROR);
}

//-----------------------------------------------
//*** function sets port timeout
DWORD RS232_SetPortTimeout(HANDLE hPort, unsigned int ms_timeout)
{
  COMMTIMEOUTS ltimeouts = {0};

	//test handle
	assert(hPort);

	if (ms_timeout == 0) {
		//no read timeout (read returns immediatelly)
		ltimeouts.ReadIntervalTimeout = MAXDWORD;
		ltimeouts.ReadTotalTimeoutMultiplier = 0;
		ltimeouts.ReadTotalTimeoutConstant = 0;
	}
	else {
		//set default read timeout
		ltimeouts.ReadIntervalTimeout = 20;
		ltimeouts.ReadTotalTimeoutMultiplier = 10;
		ltimeouts.ReadTotalTimeoutConstant = ms_timeout;
	}
	//default write timeout
  ltimeouts.WriteTotalTimeoutMultiplier = 10;
  ltimeouts.WriteTotalTimeoutConstant = 100;

  //set new timeouts
  if (!SetCommTimeouts(hPort, &ltimeouts)) {
    return(GetLastError());
  }

  return(NO_ERROR);
}

//-----------------------------------------------
//*** function gets timeout for read operation
DWORD RS232_GetReadTimeout(HANDLE hPort, unsigned int *ms_timeout)
{
  COMMTIMEOUTS ltimeouts;

	//test handle
	assert(hPort);

  //get actual timeouts
  if (!GetCommTimeouts(hPort, &ltimeouts)) {
    return(GetLastError());
  }
  //get read constant timeout
  if (ms_timeout) {
		if (ltimeouts.ReadIntervalTimeout == MAXDWORD &&
				ltimeouts.ReadTotalTimeoutMultiplier == 0 &&
				ltimeouts.ReadTotalTimeoutConstant == 0)
			*ms_timeout = 0;
		else
			*ms_timeout = ltimeouts.ReadTotalTimeoutConstant;
  }

  return(NO_ERROR);
}

//-----------------------------------------------
//*** function gets number of received bytes
DWORD RS232_GetBytesInQueue(HANDLE hPort, unsigned int *bytes)
{
  COMSTAT cStat = {0};
  DWORD lError = 0;

	//test handles
	assert(hPort);
	assert(bytes);

  if (!ClearCommError(hPort, &lError, &cStat))
    return(GetLastError());

	*bytes = cStat.cbInQue;

	if (lError)
		return(ERROR_INVALID_DATA);
  return(NO_ERROR);
}

//-----------------------------------------------
//*** function clear input & output buffer
DWORD RS232_Flush(HANDLE hPort)
{
	//test handle
	assert(hPort);

  PurgeComm(hPort, PURGE_TXCLEAR | PURGE_RXCLEAR);

  return(NO_ERROR);
}

//-----------------------------------------------
//function sets state of DTR line; TRUE -> set, FALSE -> clear
//note: need fDtrControl = DTR_CONTROL_ENABLE
BOOL RS232_SetDTRLine(HANDLE hPort, BOOL state)
{
	//test handle
	assert(hPort);

	return(EscapeCommFunction(hPort, state == TRUE ? SETDTR : CLRDTR));
}

//-----------------------------------------------
//function sets state of RTS line; TRUE -> set, FALSE -> clear
//note: need fDtrControl = RTS_CONTROL_ENABLE
BOOL RS232_SetRTSLine(HANDLE hPort, BOOL state)
{
	//test handle
	assert(hPort);

	return(EscapeCommFunction(hPort, state == TRUE ? SETRTS : CLRRTS));
}




//********************* Read and Write Operation ******************

//-----------------------------------------------------
//*** function writes buffer to port (return err and number of send bytes)
DWORD RS232_Write(HANDLE hPort, char *wbuf, unsigned int wlen, unsigned int *bwritten)
{
  OVERLAPPED osWrite = {0};
  DWORD dwWritten = 0;
  DWORD dwRes;
  DWORD lastError = NO_ERROR;

	//test handles
	assert(hPort);

  //test buffer & size
  if (wbuf && wlen) {

		//create write operation's OVERLAPPED structure event handle
		osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (osWrite.hEvent == NULL)
			return(GetLastError());    //error with handle
		//write
		if (!WriteFile(hPort, wbuf, wlen, &dwWritten, &osWrite)) {
			lastError = GetLastError();
			if (lastError != ERROR_IO_PENDING) {
				//WriteFile failed, but isn't delayed
				//??
			}
			else {
				//write is pending
				dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
				switch(dwRes) {
					//overlapped event signaled
					case WAIT_OBJECT_0:
						if (!GetOverlappedResult(hPort, &osWrite, &dwWritten, FALSE))
							lastError = GetLastError();
						else {
							if (dwWritten != wlen)
								//write operation timed out
								lastError = ERROR_TIMEOUT;
							else
								//write operation was successful
								lastError = NO_ERROR;
						}
						break;

					default:
						//error occured in WaitForSingleObject
						lastError = GetLastError();
						break;
				}
			}
		}
		else {
			//WriteFile completed immediatelly
			if (dwWritten != wlen)
				//timeout
				lastError = ERROR_TIMEOUT;
			else
				lastError = NO_ERROR;
		}


		CloseHandle(osWrite.hEvent);
  }

  //set number of written bytes
	if (bwritten)
		*bwritten = dwWritten;

  return(lastError);
}

//-----------------------------------------------------
//*** function reads bytes from port to buffer immediatelly (return err and number of read bytes)
DWORD RS232_ReadIm(HANDLE hPort, char *rbuf, unsigned int rlen, unsigned int *bread)
{
  DWORD dwRead = 0;
  DWORD lastError = NO_ERROR;

	//test handles
	assert(hPort);
	assert(bread);

  //test buffer & size
  if (rbuf && rlen && RS232_CanRead(hPort) > 0) {		//CanRead() is needed for Linux Wine environment!

		OVERLAPPED osRead = {0};
		//create write operation's OVERLAPPED structure event handle
		osRead.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (osRead.hEvent == NULL)
			return(GetLastError());    //error with handle
		//read
		if (!ReadFile(hPort, rbuf, rlen, &dwRead, &osRead)) {
			lastError = GetLastError();
			if (lastError != ERROR_IO_PENDING) {
				//ReadFile failed, but isn't delayed
				//??
			}
			else {
				//read is pending
				BOOL bRes;
				DWORD dwRes = WaitForSingleObject(osRead.hEvent, INFINITE);
				switch (dwRes) {
					//overlapped event signaled
					case WAIT_OBJECT_0:
						//something to do
						bRes = GetOverlappedResult(hPort, &osRead, &dwRead, FALSE);
						if (!bRes)
							lastError = GetLastError();
						else {
							//write operation was successful
							lastError = NO_ERROR;
						}
						break;

					default:
						//error occured in WaitForSingleObject
						lastError = GetLastError();
						break;
				}
			}
		}
		else {
			//ReadFile completed immediatelly
			lastError = NO_ERROR;
		}

		//close event
		CloseHandle(osRead.hEvent);
  }

  //set number of read bytes
  *bread = dwRead;

  return(lastError);
}

//-------------------------------------------------------------------
//*** function test if any data can be read ()
int RS232_CanRead(HANDLE hPort)
{
	unsigned int count = 0;

	if (RS232_GetBytesInQueue(hPort, &count) != NO_ERROR)
		return(-1);		//error
	return((int)count);		//number of bytes
}



//----------- Useful ------------------------------------------------

//-------------------------------------------------------------------
//*** function returns mask by baudrate
unsigned int RS232_GetBaudrateMask(RS232_BAUDRATE baudrate)
{
	switch (baudrate) {
		case RS232_BAUDRATE_110: return(RS232_M_BAUDRATE_110);
		case RS232_BAUDRATE_300: return(RS232_M_BAUDRATE_300);
		case RS232_BAUDRATE_600: return(RS232_M_BAUDRATE_600);
		case RS232_BAUDRATE_1200: return(RS232_M_BAUDRATE_1200);
		case RS232_BAUDRATE_2400: return(RS232_M_BAUDRATE_2400);
		case RS232_BAUDRATE_4800: return(RS232_M_BAUDRATE_4800);
		case RS232_BAUDRATE_9600: return(RS232_M_BAUDRATE_9600);
		case RS232_BAUDRATE_19200: return(RS232_M_BAUDRATE_19200);
		case RS232_BAUDRATE_38400: return(RS232_M_BAUDRATE_38400);
		case RS232_BAUDRATE_57600: return(RS232_M_BAUDRATE_57600);
		case RS232_BAUDRATE_115200: return(RS232_M_BAUDRATE_115200);
		case RS232_BAUDRATE_230400: return(RS232_M_BAUDRATE_230400);
		case RS232_BAUDRATE_250000: return(RS232_M_BAUDRATE_250000);
		case RS232_BAUDRATE_460800: return(RS232_M_BAUDRATE_460800);
		case RS232_BAUDRATE_921600: return(RS232_M_BAUDRATE_921600);
		case RS232_BAUDRATE_1000000: return(RS232_M_BAUDRATE_1000000);
		case RS232_BAUDRATE_1500000: return(RS232_M_BAUDRATE_1500000);
		case RS232_BAUDRATE_3000000: return(RS232_M_BAUDRATE_3000000);
		case RS232_BAUDRATE_3500000: return(RS232_M_BAUDRATE_3500000);
		case RS232_BAUDRATE_4000000: return(RS232_M_BAUDRATE_4000000);
		case RS232_BAUDRATE_5250000: return(RS232_M_BAUDRATE_5250000);
	}
	return(0);
}

//*** function returns mask by databits
unsigned int RS232_GetDatabitsMask(RS232_DATABITS databits)
{
	switch (databits) {
		case RS232_DATABITS_5: return(RS232_M_DATABITS_5);
		case RS232_DATABITS_6: return(RS232_M_DATABITS_6);
		case RS232_DATABITS_7: return(RS232_M_DATABITS_7);
		case RS232_DATABITS_8: return(RS232_M_DATABITS_8);
	}
	return(0);
}

//*** function returns mask by parity
unsigned int RS232_GetParityMask(RS232_PARITY parity)
{
	switch (parity) {
		case RS232_PARITY_NONE: return(RS232_M_PARITY_NONE);
		case RS232_PARITY_ODD: return(RS232_M_PARITY_ODD);
		case RS232_PARITY_EVEN: return(RS232_M_PARITY_EVEN);
		case RS232_PARITY_MARK: return(RS232_M_PARITY_MARK);
		case RS232_PARITY_SPACE: return(RS232_M_PARITY_SPACE);
	}
	return(0);
}

//*** function returns mask by stopbits
unsigned int RS232_GetStopbitsMask(RS232_STOPBITS stopbits)
{
	switch (stopbits) {
		case RS232_STOPBITS_1: return(RS232_M_STOPBITS_1);
		case RS232_STOPBITS_2: return(RS232_M_STOPBITS_2);
	}
	return(0);
}

//*** function returns mask by handshake
unsigned int RS232_GetHandshakeMask(RS232_HANDSHAKE handshake)
{
	switch (handshake) {
		case RS232_HANDSHAKE_NONE: return(RS232_M_HANDSHAKE_NONE);
		case RS232_HANDSHAKE_CTS_RTS: return(RS232_M_HANDSHAKE_CTS_RTS);
		case RS232_HANDSHAKE_DSR_DTR: return(RS232_M_HANDSHAKE_DSR_DTR);
		case RS232_HANDSHAKE_BOTH: return(RS232_M_HANDSHAKE_BOTH);
	}
	return(0);
}

//-------------------------------------------------------------------
//*** function returns first mask from mask
unsigned int RS232_FIRSTMASK(unsigned int mask)
{
	unsigned int idx = 0;

	while ((mask & 0x1) == 0) {
		idx++;
		mask >>= 1;
	}
	return(RS232_MASK(idx));
}

//*** function returns baudrate by mask
RS232_BAUDRATE RS232_GetBaudrateByMask(unsigned int en_mask)
{
	switch (RS232_FIRSTMASK(en_mask)) {
		case RS232_M_BAUDRATE_110: return(RS232_BAUDRATE_110);
		case RS232_M_BAUDRATE_300: return(RS232_BAUDRATE_300);
		case RS232_M_BAUDRATE_600: return(RS232_BAUDRATE_600);
		case RS232_M_BAUDRATE_1200: return(RS232_BAUDRATE_1200);
		case RS232_M_BAUDRATE_2400: return(RS232_BAUDRATE_2400);
		case RS232_M_BAUDRATE_4800: return(RS232_BAUDRATE_4800);
		default:
		case RS232_M_BAUDRATE_9600: return(RS232_BAUDRATE_9600);
		case RS232_M_BAUDRATE_19200: return(RS232_BAUDRATE_19200);
		case RS232_M_BAUDRATE_38400: return(RS232_BAUDRATE_38400);
		case RS232_M_BAUDRATE_57600: return(RS232_BAUDRATE_57600);
		case RS232_M_BAUDRATE_115200: return(RS232_BAUDRATE_115200);
		case RS232_M_BAUDRATE_230400: return(RS232_BAUDRATE_230400);
		case RS232_M_BAUDRATE_250000: return(RS232_BAUDRATE_250000);
		case RS232_M_BAUDRATE_460800: return(RS232_BAUDRATE_460800);
		case RS232_M_BAUDRATE_921600: return(RS232_BAUDRATE_921600);
		case RS232_M_BAUDRATE_1000000: return(RS232_BAUDRATE_1000000);
		case RS232_M_BAUDRATE_1500000: return(RS232_BAUDRATE_1500000);
		case RS232_M_BAUDRATE_3000000: return(RS232_BAUDRATE_3000000);
		case RS232_M_BAUDRATE_3500000: return(RS232_BAUDRATE_3500000);
		case RS232_M_BAUDRATE_4000000: return(RS232_BAUDRATE_4000000);
		case RS232_M_BAUDRATE_5250000: return(RS232_BAUDRATE_5250000);
	}
}

//*** function returns databits by mask
RS232_DATABITS RS232_GetDatabitsByMask(unsigned int en_mask)
{
	switch (RS232_FIRSTMASK(en_mask)) {
		case RS232_M_DATABITS_5: return(RS232_DATABITS_5);
		case RS232_M_DATABITS_6: return(RS232_DATABITS_6);
		case RS232_M_DATABITS_7: return(RS232_DATABITS_7);
		default:
		case RS232_M_DATABITS_8: return(RS232_DATABITS_8);
	}
}

//*** function returns parity by mask
RS232_PARITY RS232_GetParityByMask(unsigned int en_mask)
{
	switch (RS232_FIRSTMASK(en_mask)) {
		default:
		case RS232_M_PARITY_NONE: return(RS232_PARITY_NONE);
		case RS232_M_PARITY_ODD: return(RS232_PARITY_ODD);
		case RS232_M_PARITY_EVEN: return(RS232_PARITY_EVEN);
		case RS232_M_PARITY_MARK: return(RS232_PARITY_MARK);
		case RS232_M_PARITY_SPACE: return(RS232_PARITY_SPACE);
	}
}

//*** function returns stopbits by mask
RS232_STOPBITS RS232_GetStopbitsByMask(unsigned int en_mask)
{
	switch (RS232_FIRSTMASK(en_mask)) {
		default:
		case RS232_M_STOPBITS_1: return(RS232_STOPBITS_1);
		case RS232_M_STOPBITS_2: return(RS232_STOPBITS_2);
	}
}

//*** function returns handshake by mask
RS232_HANDSHAKE RS232_GetHandshakeByMask(unsigned int en_mask)
{
	switch (RS232_FIRSTMASK(en_mask)) {
		default:
		case RS232_M_HANDSHAKE_NONE: return(RS232_HANDSHAKE_NONE);
		case RS232_M_HANDSHAKE_CTS_RTS: return(RS232_HANDSHAKE_CTS_RTS);
		case RS232_M_HANDSHAKE_DSR_DTR: return(RS232_HANDSHAKE_DSR_DTR);
		case RS232_M_HANDSHAKE_BOTH: return(RS232_HANDSHAKE_BOTH);
	}
}

//-------------------------------------------------------------------
//*** function sets flags according to parameters
BOOL RS232_SetFlags(RS232_PARAMS *pars, RS232_FLAGS *flags)
{
	if (pars == NULL || flags == NULL)
		return(FALSE);

	flags->en_baudrate = RS232_GetBaudrateMask(pars->baudrate);
	flags->en_databits = RS232_GetDatabitsByMask(pars->databits);
	flags->en_parity = RS232_GetParityMask(pars->parity);
	flags->en_stopbits = RS232_GetStopbitsMask(pars->stopbits);
	flags->en_handshake = RS232_GetHandshakeMask(pars->handshake);

	return(TRUE);
}

//-------------------------------------------------------------------
//*** function counts set bits
int RS232_IsMultipleMask(unsigned int en_mask)
{
	int count;

	for (count=0; en_mask>0; en_mask>>=1) {
		if (en_mask & 0x1)
			count++;
	}
	return(count);
}

//*** function compares val-mask with enable-mask to get CB list index
int RS232_GetCBListIndex(unsigned int val_mask, unsigned int en_mask)
{
	int idx;

	for (idx=0; en_mask>0; en_mask>>=1,val_mask>>=1) {
		if (en_mask & 0x1) {
			if (val_mask & 0x1)
				return(idx);	//done
			idx++;
		}
	}
	return(0);
}

//*** function returns val-mask from CB list index and enable-mask
unsigned int RS232_GetCBListValMask(int idx, unsigned int en_mask)
{
	int sh = 0;

	while (en_mask) {
		if (en_mask & 0x1) {
			if (idx <= 0)
				break;
			idx--;
		}
		en_mask >>= 1;
		sh++;
	}
	return(RS232_MASK(sh));
}


//----------- Validation --------------------------------------------

//-------------------------------------------------------------------
//*** function validates baudrate by enable-mask
BOOL RS232_ValidateBaudrate(unsigned int en_mask, RS232_BAUDRATE baudrate)
{
	if (en_mask & RS232_GetBaudrateMask(baudrate))
		return(TRUE);
	return(FALSE);
}

//-------------------------------------------------------------------
//*** function validates databits by enable-mask
BOOL RS232_ValidateDatabits(unsigned int en_mask, RS232_DATABITS databits)
{
	if (en_mask & RS232_GetDatabitsMask(databits))
		return(TRUE);
	return(FALSE);
}

//-------------------------------------------------------------------
//*** function validates parity by enable-mask
BOOL RS232_ValidateParity(unsigned int en_mask, RS232_PARITY parity)
{
	if (en_mask & RS232_GetParityMask(parity))
		return(TRUE);
	return(FALSE);
}

//-------------------------------------------------------------------
//*** function validates stopbits by enable-mask
BOOL RS232_ValidateStopbits(unsigned int en_mask, RS232_STOPBITS stopbits)
{
	if (en_mask & RS232_GetStopbitsMask(stopbits))
		return(TRUE);
	return(FALSE);
}

//-------------------------------------------------------------------
//*** function validates handshake by enable-mask
BOOL RS232_ValidateHandshake(unsigned int en_mask, RS232_HANDSHAKE handshake)
{
	if (en_mask & RS232_GetHandshakeMask(handshake))
		return(TRUE);
	return(FALSE);
}


//----------- Strings -----------------------------------------------

const TCHAR *c_str_uknown = TEXT("?");

/* get parity string */
const TCHAR *RS232_GetParityString(RS232_PARITY parity)
{
	switch(parity) {
		case RS232_PARITY_NONE: return(TEXT("none"));
		case RS232_PARITY_ODD: return(TEXT("odd"));
		case RS232_PARITY_EVEN: return(TEXT("even"));
		case RS232_PARITY_MARK: return(TEXT("mark"));
		case RS232_PARITY_SPACE: return(TEXT("space"));
		default: return(c_str_uknown);
	}
}

/* get parity abbreviation string */
const TCHAR *RS232_GetParityAbbrString(RS232_PARITY parity)
{
	switch(parity) {
		case RS232_PARITY_NONE: return(TEXT("N"));
		case RS232_PARITY_ODD: return(TEXT("O"));
		case RS232_PARITY_EVEN: return(TEXT("E"));
		case RS232_PARITY_MARK: return(TEXT("M"));
		case RS232_PARITY_SPACE: return(TEXT("S"));
		default: return(c_str_uknown);
	}
}

/* get stopbits string */
const TCHAR *RS232_GetStopbitString(RS232_STOPBITS stopbits)
{
	switch(stopbits) {
		case RS232_STOPBITS_1: return(TEXT("1"));
		case RS232_STOPBITS_2: return(TEXT("2"));
		default: return(c_str_uknown);
	}
}


//----------- Testing -----------------------------------------------

//-------------------------------------------------------------------
//*** testing function
DWORD RS232_Testing(void)
{
  HANDLE hPort = NULL;
  DWORD ret;
  char rbuf[100];
  unsigned int len = 0;

  hPort = RS232_OpenConfPortNo(3, 9600, 8, NOPARITY, 0, 0, 512, 512);
  if (hPort == NULL)
    return(ERROR_INVALID_HANDLE);

	ret = RS232_IsPortOpened(hPort);

  ret = RS232_Write(hPort, "HELLO", 5, &len);

	ret = RS232_CanRead(hPort);

  ret = RS232_ReadIm(hPort, rbuf, 100, &len);

  RS232_ClosePort(hPort);

  return(ret);
}




