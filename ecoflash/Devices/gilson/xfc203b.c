/*
 * xfc203b.c
 *
 * FC203B device - source file
 *
 * Author: Filip Kinovic
 * Version: 2.3
 * Date: 29.02.2012
*/


#include <windows.h>
#include <math.h>
#include <process.h>
#include "xfc203b.h"

/* constants */
const TCHAR *fc203bname = TEXT("FC203B");
const TCHAR *fc203bver = TEXT("2.3");
const TCHAR *fc203bspec = DC_("Fraction collector");
const TCHAR *fc203bprod = TEXT("Gilson");
const TCHAR *fc203bdesc = TEXT("Supported models: FC203B and FC204.");

#ifdef DEBUG
	//#define FC203B_SIMUL
#endif


//propeties
#define _XFC203B_ADR 0
#define _XFC203B_VIALS 1
#define _XFC203B_VIALPOS 2
#define _XFC203B_CVALVE 3
//
#define _XFC203B_TEMP_SETVIAL 4
#define _XFC203B_TEMP_MOVING 5
#define _XFC203B_TEMP_CFG 6
#define _XFC203B_TEMP_MODEL 7
#define _XFC203B_TEMP_SPECIAL 8
//
#define _XFC203B_NP (_XFC203B_TEMP_SPECIAL+1)		//number of prop.
//oper. propeties
#define _XFC203B_START 0
#define _XFC203B_COLLECT 1
#define _XFC203B_WASTE 2
#define _XFC203B_NEXT 3
#define _XFC203B_STOP 4
#define _XFC203B_REMOTE 5
#define _XFC203B_LOCAL 6
#define _XFC203B_BEEP 7
//
#define _XFC203B_NOP (_XFC203B_BEEP+1)		//number of oper prop.

//--- constants ---
#define _XFC203B_MIN_TDELAY 20		//min. 20 ms
#define _XFC203B_MAX_CMDWAIT 5000		//5s

#define _XFC203B_CFLAG_IN_REMOTE 0x1
#define _XFC203B_CFLAG_DE_LOCALE 0x2

#define _XFC203B_MAX_OUTMASK 0x3

/* macros */
#ifdef FC203B_SIMUL
	extern BOOL COM_log_enable;
	DWORD FC203B_SimulIC(CF_COMMH *hC, unsigned char adr, char cmd);
	DWORD FC203B_SimulBC(CF_COMMH *hC, unsigned char adr, const char *cmd);
	#define FC203B__SendIC FC203B_SimulIC
	#define FC203B__SendBC FC203B_SimulBC
#else
	#define FC203B__SendIC FC203B_SendIC
	#define FC203B__SendBC FC203B_SendBC
#endif


/* global variables */

/* external variables */
//logging
extern BOOL COM_log_enable;

/* function definitions */


//*** function creates FC203B device parameters
DWORD FC203B_CreateDevParams(HDEVICE hD, void *param)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//set some comm. parameters
	pD->com.mode = COM_MODE_RS232;
	pD->com.log_format = COM_LOGFORMAT_TEXT;
	pD->com.com_flags.en_rs232 = 1;
	pD->com.timeout = 100;
	pD->com.com_flags.en_timeout = 1;
	pD->com.terminal_char = RS232_TERMINAL_CR;
	pD->com.repeat = 0;
	pD->com.errs = 0;
	//
	pD->com.ser_pars.baudrate = RS232_BAUDRATE_19200;
	pD->com.ser_pars.databits = RS232_DATABITS_8;
	pD->com.ser_pars.parity = RS232_PARITY_EVEN;
	pD->com.ser_pars.stopbits = RS232_STOPBITS_1;
	pD->com.ser_pars.handshake = RS232_HANDSHAKE_NONE;
	pD->com.ser_pars.insize = RS232_INSIZE_DEFAULT;
	pD->com.ser_pars.outsize = RS232_OUTSIZE_DEFAULT;
	//
	RS232_SetFlags(&pD->com.ser_pars, &pD->com.ser_flags);
	pD->com.ser_flags.en_baudrate |= RS232_M_BAUDRATE_19200|RS232_M_BAUDRATE_9600|RS232_M_BAUDRATE_4800;

	//model
	pD->model = D_MODEL_FRACCOLLECTOR;		//fraction collector
	//color
	pD->color = 0x000000FFL;		//red
	//delay
	pD->tmindelay = _XFC203B_MIN_TDELAY;		//store min. tdelay
	pD->tdelay = 100;		//measure time [ms]
	pD->tdelay2 = D_DEFAULT_CTRLTIME;		//control time [ms]

	//--- properties ---
	if (_XFC203B_NP) {
		if ((pD->p = (D_DEV_PROP *)malloc(_XFC203B_NP*sizeof(D_DEV_PROP))) == NULL)
			return(ERROR_OUTOFMEMORY);
		memset(pD->p, 0, _XFC203B_NP*sizeof(D_DEV_PROP));
		//set properties
		D_SetProperty(&pD->p[_XFC203B_ADR], DC_("Address"), NULL, NULL, DFLAG_CONF|DFLAG_MINMAX, DT_UINT);
		pD->p[_XFC203B_ADR].val.u = 6;		//default address
		pD->p[_XFC203B_ADR].min.u = 1;
		pD->p[_XFC203B_ADR].max.u = 32;
		D_SetProperty(&pD->p[_XFC203B_VIALS], DC_("Total vials"), NULL, NULL, DFLAG_CONF|DFLAG_MINMAX, DT_UINT);
		pD->p[_XFC203B_VIALS].val.u = 80;		//default max. vials
		pD->p[_XFC203B_VIALS].min.u = 10;
		pD->p[_XFC203B_VIALS].max.u = 10000;
		D_SetProperty(&pD->p[_XFC203B_VIALPOS], DC_("Vial position"), NULL, NULL, DFLAG_CTRL|DFLAG_RECV, DT_UINT);
		pD->p[_XFC203B_VIALPOS].guiflags = DGFLAG_EMPH;
		pD->p[_XFC203B_VIALPOS].guitype = DGUIT_READ;
		D_SetProperty(&pD->p[_XFC203B_CVALVE], DC_("Collection valve"), NULL, NULL, DFLAG_CTRL, DT_BOOL);
		pD->p[_XFC203B_CVALVE].guitype = DGUIT_READ;
		D_SetProperty(&pD->p[_XFC203B_TEMP_SPECIAL], DC_("Special option"), NULL, NULL, DFLAG_CONF, DT_UINT);
		pD->p[_XFC203B_TEMP_SPECIAL].val.u = 0;		//none
		pD->p[_XFC203B_TEMP_SPECIAL].guitype = DGUIT_COMBOBOX;
		pD->p[_XFC203B_TEMP_SPECIAL].guidata = DC_("None") TEXT("\0") DC_("21LV (Multiple)") TEXT("\0");
		//D_SetProperty(&pD->p[_XFC203B_OUTPUTS], DC_("Outputs"), NULL, NULL, DFLAG_CTRL, DT_UINT);
		D_SetProperty(&pD->p[_XFC203B_TEMP_SETVIAL], TEXT("t_vial"), NULL, NULL, 0, DT_UINT);
		D_SetProperty(&pD->p[_XFC203B_TEMP_MOVING], TEXT("t_moving"), NULL, NULL, 0, DT_BOOL);
		D_SetProperty(&pD->p[_XFC203B_TEMP_CFG], TEXT("t_cfg"), NULL, NULL, DFLAG_CONF, DT_UINT);
		pD->p[_XFC203B_TEMP_CFG].val.u = _XFC203B_CFLAG_IN_REMOTE | _XFC203B_CFLAG_DE_LOCALE;
		D_SetProperty(&pD->p[_XFC203B_TEMP_MODEL], TEXT("t_model"), NULL, NULL, DFLAG_STAT, DT_UINT);
	}
	pD->n_p = _XFC203B_NP;

	//--- oper. properties ---
	if (_XFC203B_NOP) {
		if ((pD->op = (D_DEV_OPERPROP *)malloc(_XFC203B_NOP*sizeof(D_DEV_OPERPROP))) == NULL)
			return(ERROR_OUTOFMEMORY);
		memset(pD->op, 0, _XFC203B_NOP*sizeof(D_DEV_OPERPROP));
		//set oper. properties
		D_SetOperProperty(&pD->op[_XFC203B_START], DC_("Set start"), 0);
		D_SetOperProperty(&pD->op[_XFC203B_COLLECT], DC_("Set collect"), 0);
		D_SetOperProperty(&pD->op[_XFC203B_WASTE], DC_("Set waste"), 0);
		D_SetOperProperty(&pD->op[_XFC203B_NEXT], DC_("Set next"), 0);
		D_SetOperProperty(&pD->op[_XFC203B_STOP], DC_("Set stop"), 0);
		D_SetOperProperty(&pD->op[_XFC203B_REMOTE], DC_("Set remote control"), 0);
		D_SetOperProperty(&pD->op[_XFC203B_LOCAL], DC_("Set local control"), 0);
		D_SetOperProperty(&pD->op[_XFC203B_BEEP], DC_("Make a beep"), 0);
	}
	pD->n_op = _XFC203B_NOP;

	//--- pointer proc. function ---
	//universal functions
	pD->ufce.pGetDeviceInfo = FC203B_GetDeviceInfo;		//done
	pD->ufce.pTestID = FC203B_TestID;		//done
	pD->ufce.pValidate = FC203B_Validate;
	pD->ufce.pGetStatus = NULL;
	pD->ufce.pSetOper = FC203B_MakeOper;
	pD->ufce.pInit = FC203B_InitDetector;
	pD->ufce.pPostInit = FC203B_PostInitDetector;
	pD->ufce.pReadProp = FC203B_ReadProp;
	pD->ufce.pWriteProp = FC203B_WriteProp;
	pD->ufce.pSetupParamDlgProc = FC203B_SetupParamDlgProc;
	pD->ufce.pSetupInitDlgProc = FC203B_SetupInitDlgProc;
	pD->ufce.pOnLineDlgProc = FC203B_OnLineDlgProc;
	//detector functions
	pD->detfce.pGetSignal = NULL;		//done
	//FC functions
	pD->fcfce.pGetFCInfo = FC203B_GetFCInfo;
	pD->fcfce.pStart = FC203B_Start;
	pD->fcfce.pCollectWaste = FC203B_CollectWaste;
	pD->fcfce.pNext = FC203B_Next;
	pD->fcfce.pStop = FC203B_Stop;

	//--- other operations ---
	pD->dis_meas_int = 1;		//disable measure interval

	return(NO_ERROR);
}

//--------------------- Universal Interface Functions -------------------------
//*** function gets device info (text pointers must not be freed)
BOOL FC203B_GetDeviceInfo(HDEVICE hd, DEVICEINFO *dinfo)
{
	if (dinfo) {
		dinfo->name = fc203bname;
		dinfo->ver = fc203bver;
		dinfo->spec = fc203bspec;
		dinfo->desc = fc203bdesc;
		dinfo->prod = fc203bprod;
		dinfo->model = D_MODEL_FRACCOLLECTOR;
		dinfo->filter = D_DEVFILTER_OTHER_CHROM;

		return TRUE;
	}
	return FALSE;
}

//*** function tests and returns device ID
BOOL FC203B_TestID(HDEVICE hD, TCHAR *name, TCHAR *type, TCHAR *sw, TCHAR *sn)
{
	D_DEVICE *pD;
	TCHAR *pid;
	char *ptext;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;


	//////// test ////////
#ifdef DEBUG
/*
	unsigned vial;
	FC203B_GetInputs(hD, 1, &vial, NULL);
	FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "KE\r");
	FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "KE\r");
	FC203B_GetVialposition(hD, 1, &vial, NULL);
	FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "T001\r");
	FC203B_GetVialposition(hD, 1, &vial, NULL);
	FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "T076\r");
	FC203B_GetVialposition(hD, 1, &vial, NULL);
	FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "KE\r");
	FC203B_GetVialposition(hD, 1, &vial, NULL);
	FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "T080\r");
	FC203B_GetVialposition(hD, 1, &vial, NULL);
	FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "T075\r");
	FC203B_GetVialposition(hD, 1, &vial, NULL);
	FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "T005\r");
	FC203B_GetVialposition(hD, 1, &vial, NULL);
	FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "KE\r");
*/

	FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, '*');
	FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, '?');
	FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, '%');
	FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, 'R');
	FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, 'r');
	FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, '0');
	FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, '0');
	FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, '9');

#endif

	//--- read ID ---
	if (FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, '%') == NO_ERROR) {
		//decode response
		if (FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, '%') == NO_ERROR &&
				(ptext = strchr(pD->com.rcptbuff+2, 'v')) > 0)	{
			*ptext = '\0';
			//test models
			if (strncmp(pD->com.rcptbuff+1, "203", 3) == 0)
				pD->p[_XFC203B_TEMP_MODEL].val.u = FC203B_MODEL_203B;
			else if (strncmp(pD->com.rcptbuff+1, "204", 3) == 0)
				pD->p[_XFC203B_TEMP_MODEL].val.u = FC203B_MODEL_204;
			else
				return(FALSE);		//not valid model

			//response OK
			pid = pD->id;

			//decode name
			__sprintf(pid, TEXT("FC%hs"), pD->com.rcptbuff+1);
			if (name)
				lstrcpy(name, pid);		//get name
			pid += lstrlen(pid)+1;

			//type (HW)
			lstrcpy(pid, D_STRQUESTION);
			if (type)
				lstrcpy(type, pid);
			pid += lstrlen(pid)+1;

			//software version (SW)
			if (ptext)
				w2strncpy(pid, ptext+1, 20);
			else
				lstrcpy(pid, D_STRQUESTION);
			if (sw)
				lstrcpy(sw, pid);
			pid += lstrlen(pid)+1;

			//decode serial number (SN)
			lstrcpy(pid, D_STRQUESTION);
			if (sn)
				lstrcpy(sn, pid);
			pid += lstrlen(pid)+1;
			*pid = '\0';

			return TRUE;
		}
	}
	*pD->id = '\0';		//no identification


	return FALSE;
}

//*** function validates device's prop. values
BOOL FC203B_Validate(HDEVICE hD, DWORD pidx)
{
	D_DEVICE *pD;
	BOOL valid = TRUE;

	if (hD) {
		pD = (D_DEVICE *)hD;

		if (pD->n_p && pD->p) {

		}
	}
	return(valid);
}

//*** function gets device status (initiate, ready, comm. error, error, ...)
DWORD FC203B_GetStatus(HDEVICE hD, DWORD *status)
{

	return(NO_ERROR);
}

//----------------------- Device Interface Functions -------------------------
//*** function reads property (to memory) by index
DWORD FC203B_ReadProp(HDEVICE hD, WORD pidx)
{
	D_DEVICE *pD;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->n_p && pD->p) {
			switch (pidx) {
				case _XFC203B_VIALPOS: return(FC203B_GetVialposition(hD, 1, NULL, NULL));
				//case _XFC203B_OUTPUTS: return(FC203B_GetOutputs(hD, 1, NULL, NULL));
			}
		}
	}
	return D_PERROR_NOTAVAILABLE;
}

//*** function writes property
DWORD FC203B_WriteProp(HDEVICE hD, WORD pidx, D_VAL val)
{
	D_DEVICE *pD;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->n_p && pD->p) {
			switch (pidx) {
				case _XFC203B_VIALPOS: return(FC203B_SetVialposition(hD, val.u));
				case _XFC203B_CVALVE: return(FC203B_CollectWaste(hD, val.b));
				//case _XFC203B_OUTPUTS: return(FC203B_SetOutputs(hD, val.u));
			}
		}
	}
	return D_PERROR_NOTAVAILABLE;
}

//*** function initiates detector (make it ready to work properly, switch lamp on, ...)
DWORD FC203B_InitDetector(HDEVICE hD, D_STAT *stat)
{
	D_DEVICE *pD;
	DWORD ret = NO_ERROR;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;
	if (pD->n_p == 0 || pD->p == NULL)
		return(ERROR_INVALID_HANDLE);		//bad device handle (bad initiation)

	while (1) {
		// 1) identify device
		D_SetState(0, D_("Identify device"), stat);
		if (!FC203B_TestID(hD, NULL, NULL, NULL, NULL))
			return(ERROR_FILE_INVALID);		//device not found

		// 2) remote
		if (pD->p[_XFC203B_TEMP_CFG].val.u & _XFC203B_CFLAG_IN_REMOTE) {
			ret = FC203B_SetRemoteControl(hD);
			if (ret != NO_ERROR)
				break;
		}

		// 3) setting parameters
		D_SetState(25, D_("Setting parameters"), stat);
		ret = FC203B_Stop(hD);		//stop FC
		if (ret != NO_ERROR)
			break;

		// 5) done
		D_SetState(100, D_("Done"), stat);

		break;
	}

	return(ret);
}

//*** function initiates detector when closing communication
DWORD FC203B_PostInitDetector(HDEVICE hD, D_STAT *stat)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;
	if (pD->n_p == 0 || pD->p == NULL)
		return(ERROR_INVALID_HANDLE);		//bad device handle (bad initiation)

	while (1) {



		// 1) set stop
		FC203B_Stop(hD);		//stop FC

		// 2) set locale
		if (pD->p[_XFC203B_TEMP_CFG].val.u & _XFC203B_CFLAG_DE_LOCALE) {
			FC203B_SetLocalControl(hD);
		}

		D_SetState(100, D_("Done"), stat);
		break;
	}

	return(NO_ERROR);
}

//-------------------------------------------------------------------
//*** function gets vial position
DWORD FC203B_GetVialposition(HDEVICE hD, int how, unsigned *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;
	int pos;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch(how) {
		default:
		case 0:		//get vial. pos. from ram
			ret = NO_ERROR;
			break;

		case 1:
			//--- check if head is stationary
			//--- check for y-axis
			t0 = timer_GetTime();
			ret = FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, 'Y');		//read Y position
			pD->p[_XFC203B_TEMP_MOVING].time = (t0 + timer_GetTime())/2.0;
			if (ret != NO_ERROR)
				break;		//error (timeout)
			//get head status
			if (*(pD->com.rcptbuff+1) == 'M') {
				pD->p[_XFC203B_TEMP_MOVING].val.b = TRUE;
				break;		//tube is not valid
			}
			//--- check for x-axis
			t0 = timer_GetTime();
			ret = FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, 'X');		//read X position
			pD->p[_XFC203B_TEMP_MOVING].time = (t0 + timer_GetTime())/2.0;
			if (ret != NO_ERROR)
				break;		//error (timeout)
			//get head status
			if (*(pD->com.rcptbuff+1) == 'M') {
				pD->p[_XFC203B_TEMP_MOVING].val.b = TRUE;
				break;		//tube is not valid
			}
			else
				pD->p[_XFC203B_TEMP_MOVING].val.b = FALSE;		//tube is not moving

			//--- read vial position
			t0 = timer_GetTime();
			//manage command
			ret = FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, 'T');
			//average abs. time between request and response
			pD->p[_XFC203B_VIALPOS].time = (t0 + timer_GetTime())/2.0;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			pos = atoi(pD->com.rcptbuff+1);
			if (pos) {
				//test spec. option
				switch (pD->p[_XFC203B_TEMP_SPECIAL].val.u) {
					default:
					//NONE
					case FC203B_SPECIAL_NONE: break;
					//21LV
					case FC203B_SPECIAL_21LV: pos = pos/4; break;		//recalculate vial position
				}
			}
			pD->p[_XFC203B_VIALPOS].val.u = pos;		//real position
			pD->p[_XFC203B_VIALPOS].lerr = ret;
			break;
	}

	//???
	if (uval)
		*uval = pD->p[_XFC203B_VIALPOS].val.u;
	if (time)
		*time = pD->p[_XFC203B_VIALPOS].time;

	return(ret);
}

//*** function sets vial position (0 -> drain)
DWORD FC203B_SetVialposition(HDEVICE hD, unsigned uval)
{
	D_DEVICE *pD;
	DWORD ret;
	char ttext[10];
	int pos;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//check range
	if (uval <= pD->p[_XFC203B_VIALS].val.u) {
		//manage command
		if (uval) {
			//test spec. option
			switch (pD->p[_XFC203B_TEMP_SPECIAL].val.u) {
				default:
				//NONE
				case FC203B_SPECIAL_NONE: pos = uval; break;
				//21LV
				case FC203B_SPECIAL_21LV: pos = uval*4; break;		//recalculate vial position
			}
			sprintf(ttext, "T%03u\r", pos);
		}
		else
			strcpy(ttext, "KE\r");		//home (drain) position
		ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, ttext);
		if (ret != NO_ERROR)
			return(ret);		//error (timeout)

		pD->p[_XFC203B_TEMP_SETVIAL].val.u = uval;
		//read real vial position
		FC203B_GetVialposition(hD, 1, NULL, NULL);
	}
	else
		ret = 1;		//invalid parameter

	return(ret);
}

//*** function gets outputs
/*
DWORD FC203B_GetOutputs(HDEVICE hD, int how, unsigned *uval, double *time)
{
	D_DEVICE *pD;
	unsigned i, cd;
	char *ptext;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch(how) {
		default:
		case 0:		//get inputs from ram
			ret = NO_ERROR;
			break;

		case 1:		//read inputs
			t0 = timer_GetTime();
			//manage command
			ret = FC203B__SendIC(&pD->com, pD->p[_XFC203B_ADR].val.u, '?');
			//average abs. time between request and response
			pD->p[_XFC203B_OUTPUTS].time = (t0 + timer_GetTime())/2.0;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode inputs
			i = 0;
			cd = 0;
			ptext = pD->com.rcptbuff+1;
			while (*ptext == 'C' || *ptext == 'D') {
				if (*ptext == 'C')
					cd |= 1<<i;
				ptext++;
				i++;
			}

			pD->p[_XFC203B_OUTPUTS].val.u = cd;
			pD->p[_XFC203B_OUTPUTS].lerr = ret;
			break;
	}

	//???
	if (uval)
		*uval = pD->p[_XFC203B_OUTPUTS].val.u;
	if (time)
		*time = pD->p[_XFC203B_OUTPUTS].time;

	return(ret);
}
*/

//*** function sets outputs
/*
DWORD FC203B_SetOutputs(HDEVICE hD, unsigned uval)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//check range
	uval &= 0x03;		//2 outputs only

	//manage command
	switch (uval) {
		case 0x0:
			ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "D12");
			break;
		case 0x1:
			ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "C1");
			ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "D2");
			break;
		case 0x2:
			ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "D1");
			ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "C2");
			break;
		case 0x3:
			ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, "C12");
			break;
	}
	return(ret);
}
*/

//----------------------------------------------------------
//*** function sets oper property by index
DWORD FC203B_MakeOper(HDEVICE hD, WORD oidx)
{
	D_DEVICE *pD;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->n_op && pD->op) {
			switch(oidx) {
				case _XFC203B_START: return(FC203B_Start(hD, 1));
				case _XFC203B_COLLECT: return(FC203B_CollectWaste(hD, 1));
				case _XFC203B_WASTE: return(FC203B_CollectWaste(hD, 0));
				case _XFC203B_NEXT: return(FC203B_Next(hD));
				case _XFC203B_STOP: return(FC203B_Stop(hD));
				case _XFC203B_REMOTE: return(FC203B_SetRemoteControl(hD));
				case _XFC203B_LOCAL: return(FC203B_SetLocalControl(hD));
				case _XFC203B_BEEP: return(FC203B_MakeBeep(hD, 8));
			}
		}
	}
	return NO_ERROR;
}

//----------------------------------------------------------
//*** function sets remote control
DWORD FC203B_SetRemoteControl(HDEVICE hD)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, pD->p[_XFC203B_TEMP_MODEL].val.u == FC203B_MODEL_204 ? "L1\r" : "L0\r");		//lock front panel
	return(ret);
}

//*** function sets local control
DWORD FC203B_SetLocalControl(HDEVICE hD)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, pD->p[_XFC203B_TEMP_MODEL].val.u == FC203B_MODEL_204 ? "L0\r" : "L1\r");		//unlock front panel
	return(ret);
}

//*** function makes beep, range 0--10s, duration at 0.1s (example: 10 => 1s)
DWORD FC203B_MakeBeep(HDEVICE hD, unsigned duration)
{
	D_DEVICE *pD;
	DWORD ret;
	char ttext[10];

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//check limits
	if (duration > 100)
		duration = 100;

	//manage command
	sprintf(ttext, "G%03u", duration);
	ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, ttext);		//make beep
	return(ret);
}

//---------------------
//*** function gets FC info
DWORD FC203B_GetFCInfo(HDEVICE hD, D_FC_INFO *info)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (info) {
		info->tube_from = 1;
		info->tube_to = pD->p[_XFC203B_VIALS].val.u;
		info->tube_step = 1;
		info->cw_valve = 1;
	}

	return(NO_ERROR);
}

//*** function inits FC before collection
DWORD FC203B_Start(HDEVICE hD, LPARAM vialpos)
{
	DWORD ret;

	//set waste
	ret = FC203B_CollectWaste(hD, 0);
	//set vial position
	ret = FC203B_SetVialposition(hD, vialpos);		//0->drain

	return(ret);
}

//*** function sets collect/waste valve
DWORD FC203B_CollectWaste(HDEVICE hD, LPARAM c_w)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	ret = FC203B__SendBC(&pD->com, pD->p[_XFC203B_ADR].val.u, c_w ? "V0\r" : "V1\r");		//V1=waste, V0=collect
	if (ret != NO_ERROR)
		return(ret);		//error (timeout)
	else {
		pD->p[_XFC203B_CVALVE].val.b = c_w;
	}

	return(ret);
}

//*** function sets next vial
DWORD FC203B_Next(HDEVICE hD)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//set vial position
	ret = FC203B_SetVialposition(hD, pD->p[_XFC203B_TEMP_SETVIAL].val.u+1);

	return(ret);
}

//*** function inits FC after collection
DWORD FC203B_Stop(HDEVICE hD)
{
	DWORD ret;

	//set waste
	ret = FC203B_CollectWaste(hD, 0);
	//set vial position
	ret = FC203B_SetVialposition(hD, 0);

	return(ret);
}


//--------------- Device Dialogs --------------------------------------------------
//*** function processes setup dialog - parameters subdialog
BOOL CALLBACK FC203B_SetupParamDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static HDEVICE hD = NULL;
	D_DEVICE *pD;
	int i;

	switch(msg) {
		case WM_INITDIALOG:
			if (lparam) {
				hD = (HDEVICE)lparam;
				pD = (D_DEVICE *)hD;

				//create & initiate parameters

				//group box
				i = 0;
				D_CreateParamGroup(hdlg, D_("Writable parameters"), 0, i, 1, 3);

				//address
				D_CreateParamCtrl(hdlg, _XFC203B_ADR, pD, D_CTRLSTYLE_PROP, 0, i++);
				//total vials
				D_CreateParamCtrl(hdlg, _XFC203B_VIALS, pD, D_CTRLSTYLE_PROP, 0, i++);
				//special
				D_CreateParamCtrl(hdlg, _XFC203B_TEMP_SPECIAL, pD, D_CTRLSTYLE_PROP, 0, i++);

      }
			break;

		case WMD_ENABLE:
			//disable special
			//EnableWindow(D_GETOLH_VAL(hdlg, _XFC203B_TEMP_SPECIAL), wparam);
			return TRUE;

	}
	return(D_RestSetupDlgProc(hdlg, msg, wparam, lparam, (D_DEVICE *)hD));
}

//*** function processes setup dialog - parameters subdialog
BOOL CALLBACK FC203B_SetupInitDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static HDEVICE hD = NULL;
	static D_DEVICE *pD;
	HWND hctl;
	unsigned i;

	switch(msg) {
		case WM_INITDIALOG:
			if (lparam) {
				hD = (HDEVICE)lparam;
				pD = (D_DEVICE *)hD;
				if (pD->n_p == 0 || pD->p == NULL)
					break;


				//--- init ---
				i = 0;
				D_CreateParamGroup(hdlg, D_("Start-up parameters"), 0, i, 1, 1);

				//set remote control
				hctl = CreateWindow(CLASS_BUTTON, D_("Set remote control"),
														WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
														18, 33, 150, 20, hdlg, (HMENU)1, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SendMessage(hctl, BM_SETCHECK, (WPARAM)(pD->p[_XFC203B_TEMP_CFG].val.u & _XFC203B_CFLAG_IN_REMOTE), 0);

				//--- deinit ---
				i = 0;
				D_CreateParamGroup(hdlg, D_("Stop parameters"), 1, i, 1, 1);

				//set local control
				hctl = CreateWindow(CLASS_BUTTON, D_("Set local control"),
														WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
														272, 33, 150, 20, hdlg, (HMENU)11, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SendMessage(hctl, BM_SETCHECK, (WPARAM)(pD->p[_XFC203B_TEMP_CFG].val.u & _XFC203B_CFLAG_DE_LOCALE), 0);

				ScaleChildWindowsByScreen(hdlg);		//scale window
			}
			break;

		case WMD_APPLY:
			//apply
			if (pD->n_p && pD->p) {
				i = 0;
				i |= 	(IsDlgButtonChecked(hdlg, 1) ? _XFC203B_CFLAG_IN_REMOTE : 0);		//remote at startup
				i |= 	(IsDlgButtonChecked(hdlg, 11) ? _XFC203B_CFLAG_DE_LOCALE : 0);		//locale at stop
				pD->p[_XFC203B_TEMP_CFG].val.u = i;
			}
			return TRUE;

		case WM_COMMAND:
			return TRUE;

		case WM_CLOSE:
			return TRUE;

		case WM_DESTROY:
			break;

	}
	return FALSE;
}

//*** function processes ready dialog - device on-line configuration
BOOL CALLBACK FC203B_OnLineDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	D_DEVICE *pD = NULL;
	TCHAR ttext[128];
	HWND hctl;
	int i;
	#define XFC203B_OBT_START (D_OL_IDUSER + 100)
	#define XFC203B_OBT_STOP (D_OL_IDUSER + 101)
	#define XFC203B_OBT_CW (D_OL_IDUSER + 102)
	#define XFC203B_OBT_NEXT (D_OL_IDUSER + 103)

	if (msg == WM_INITDIALOG) {
		SetWindowLong(hdlg, GWLP_USERDATA, (LONG)lparam);		//store device handle !!!!!
		if (lparam) {
			pD = (D_DEVICE *)lparam;
		}


    //create & initiate parameters

		//-------------- status controls -------------
		i = 0;
		D_CreateOnLineGroup(hdlg, D_("Status parameters"), i, 2);
		//vial position
		D_CreateOnLineCtrl(hdlg, _XFC203B_VIALPOS, pD, i++);
		//valve
		D_CreateOnLineCtrl(hdlg, _XFC203B_CVALVE, pD, i++);

		//-------------- oper. controls -------------
		i += 2;
		D_CreateOnLineOperCB(hdlg, pD, i);

		//-------------- control buttons ------------
		i += 3;
		D_CreateOnLineGroup(hdlg, D_("Manual control"), i, 2);
		D_CreateOnLineButton(hdlg, XFC203B_OBT_START, D_("START"), 0, 3, i);
		D_CreateOnLineButton(hdlg, XFC203B_OBT_STOP, D_("STOP"), 0, 3, i+1);
		D_CreateOnLineButton(hdlg, XFC203B_OBT_CW, D_("C/W"), 1, 3, i | D_OL_XHEIGHT_2);
		D_CreateOnLineButton(hdlg, XFC203B_OBT_NEXT, D_("NEXT"), 2, 3, i | D_OL_XHEIGHT_2);

    //read
		D_ReadFunction((HDEVICE)pD, FC203B_ReadProp, _XFC203B_VIALPOS, hdlg, D_OL_WM_UPDATE);
	}
	else {
		pD = (D_DEVICE *)GetWindowLong(hdlg, GWLP_USERDATA);		//get device handle

		//process rest messages
		switch(msg) {
			case D_OL_WM_UPDATE:
				if (pD && pD->n_p && pD->p) {

					switch (wparam) {
						case _XFC203B_VIALPOS:
							hctl = D_GETOLH_VAL(hdlg, wparam);
							D_TestUserData(hctl, (LONG)lparam);		//test & store response
							if (pD->p[_XFC203B_TEMP_MOVING].val.b)
								__strcpy(ttext, D_("Moving..."));		//moving -> tube is not valid
							else {
								if (pD->p[_XFC203B_VIALPOS].val.u)
									__sprintf(ttext, TEXT("%d"), pD->p[_XFC203B_VIALPOS].val.u);
								else
									__strcpy(ttext, D_("Drain"));
							}
							if (IsCtrlTextDiff(hctl, ttext))
								SetWindowText(hctl, ttext);
							return TRUE;
						case _XFC203B_CVALVE:
							hctl = D_GETOLH_VAL(hdlg, wparam);
							D_TestUserData(hctl, (LONG)lparam);		//test & store response
							__strcpy(ttext, pD->p[_XFC203B_CVALVE].val.b ? D_("Collect") : D_("Waste"));
							if (IsCtrlTextDiff(hctl, ttext))
								SetWindowText(hctl, ttext);
							return TRUE;
					}
				}
				break;

			case WM_COMMAND:
				if (lparam && pD) {
					//device properties
					if (pD->n_p && pD->p) {
						if (HIWORD(wparam) == BN_CLICKED && LOWORD(wparam) >= XFC203B_OBT_START) {
							switch (LOWORD(wparam)) {
								case XFC203B_OBT_START: i = _XFC203B_START; break;
								case XFC203B_OBT_STOP: i = _XFC203B_STOP; break;
								case XFC203B_OBT_CW: i = (pD->p[_XFC203B_CVALVE].val.b ? _XFC203B_WASTE : _XFC203B_COLLECT); break;
								case XFC203B_OBT_NEXT: i = _XFC203B_NEXT; break;
								default: i = -1; break;
							}
							if (i >= 0) {
								D_MakeOperFunction((HDEVICE)pD, FC203B_MakeOper, i, GetParent(hdlg), WMD_RESPONSE);
							}
							return(TRUE);
						}
					}
				}
				break;

			case WM_TIMER:
				if (pD && pD->n_p && pD->p) {
					switch ((pD->counter++)%4) {		//2*125=250 (2*250=500)
						case 0:
							//read vial position
							D_ReadFunction((HDEVICE)pD, FC203B_ReadProp, _XFC203B_VIALPOS, hdlg, D_OL_WM_UPDATE);
							break;
					}

					if (IsWindowVisible(hdlg)) {
						i = _XFC203B_CVALVE;
						SendMessage(hdlg, D_OL_WM_UPDATE, i, pD->p[i].lerr);

						return TRUE;
					}
				}
				break;

			case WM_DESTROY:
				break;
		}
	}
	return(D_RestDevDlgProc(hdlg, msg, wparam, lparam, pD));
}



//--------------- raw communication functions ------------------------

//*** function sends Immediate Command
DWORD FC203B_SendIC(CF_COMMH *hC, unsigned char adr, char cmd)
{
	unsigned char *pw, *pr;
	DWORD ret;
	int repeat = 2;

	if (hC == NULL)
		return(-1);		//error

	while (repeat--) {
		//purge buffers
		CF_PurgeBothQues(hC);

		//
		pw = (unsigned char *)hC->sendbuff;
		pr = (unsigned char *)hC->rcptbuff;

		//test address
		*pw = adr | 0x80;
		ret = CF_SendReadByte(hC, (char)*pw, (char *)pr);
		if (ret != NO_ERROR || *pr != *pw) {
			//try again
			continue;
		}
		pw++;
		pr++;

		//process command
		*pw = cmd;
		while ((pr-(unsigned char *)hC->rcptbuff) < COM_MIN_RBUF_LEN && (ret = CF_SendReadByte(hC, (char)*pw, (char *)pr)) == NO_ERROR) {
			pw++;
			//test received character
			if (*pr >= 0x80) {
				*pr &= 0x7F;		//remove MSb
				pr++;
				break;
			}

			pr++;
			*pw = 0x06;		//ACK
		}
		*pw = '\0';
		*pr = '\0';

		if (ret == NO_ERROR)
			break;
	}

	//count errors
	if (ret != NO_ERROR)
		hC->errs++;

	//logging
	if (COM_log_enable)
		COM_LogText(hC, ret);

	return(ret);
}

//*** function sends Buffered Command
DWORD FC203B_SendBC(CF_COMMH *hC, unsigned char adr, const char *cmd)
{
	unsigned char *pw, *pr;
	DWORD ret;
	double t0;

	if (hC == NULL || cmd == NULL)
		return(-1);		//error

	while (1) {
		//purge buffers
		CF_PurgeBothQues(hC);

		//
		pw = (unsigned char *)hC->sendbuff;
		pr = (unsigned char *)hC->rcptbuff;

		//test address
		*pw = adr | 0x80;
		ret = CF_SendReadByte(hC, (char)*pw, (char *)pr);
		if (ret != NO_ERROR || *pr != *pw) {
			//try again
			pw++;
			pr++;
			*pw = '\0';
			*pr = '\0';
			ret = 1;
			break;
		}
		pw++;
		pr++;

		//init command
		*pw = '\n';
		t0 = timer_GetTime();
		while ((timer_GetTime()-t0) < _XFC203B_MAX_CMDWAIT &&
					 (ret = CF_SendReadByte(hC, (char)*pw, (char *)pr)) == NO_ERROR && *pr == 0xA3) {
			Sleep(20);
		}
		if (*pw != *pr) {
			pw++;
			pr++;
			*pw = '\0';
			*pr = '\0';
			ret = 1;
			break;
		}
		pw++;
		pr++;

		//process command
		*pw = *cmd;
		while (*pw && (ret = CF_SendReadByte(hC, (char)*pw, (char *)pr)) == NO_ERROR) {
			//test received character
			if (*pr != *pw) {
				pw++;
				pr++;
				ret = 1;
				break;		//error
			}
			pw++;
			pr++;
			cmd++;
			*pw = *cmd;
		}
		*pw = '\0';
		*pr = '\0';

		break;		//run once only
	}

	//count errors
	if (ret != NO_ERROR)
		hC->errs++;

	//logging
	if (COM_log_enable)
		COM_LogText(hC, ret);

	return(ret);
}


//------------------ SIMULATION -----------------------------

#ifdef FC203B_SIMUL
static int g_tube = 0;

/* simulate immediate command */
DWORD FC203B_SimulIC(CF_COMMH *hC, unsigned char adr, char cmd)
{
	unsigned char *pw, *pr;
	DWORD ret;
	int i;

	if (hC == NULL)
		return(-1);		//error

	pw = (unsigned char *)hC->sendbuff;
	pr = (unsigned char *)hC->rcptbuff;

	//init buffers
	*(pw++) = adr | 0x80;
	*(pr++) = adr | 0x80;
	*(pw++) = cmd;

	//process command
	ret = 0;
	switch (cmd) {
		default: ret = 1; break;	//invalid response
		//identification
		case '%':
			*(pr++) = '2';
			*(pw++) = 0x6;
			*(pr++) = '0';
			*(pw++) = 0x6;
			*(pr++) = '3';
			*(pw++) = 0x6;
			*(pr++) = 'B';
			*(pw++) = 0x6;
			*(pr++) = 'v';
			*(pw++) = 0x6;
			*(pr++) = '1';
			*(pw++) = 0x6;
			*(pr++) = '.';
			*(pw++) = 0x6;
			*(pr++) = '0' | 0x80;
			break;
		//x,y-position
		case 'X':
		case 'Y':
			*(pr++) = 'S';
			*(pw++) = 0x6;
			*(pr++) = '0';
			*(pw++) = 0x6;
			*(pr++) = '0';
			*(pw++) = 0x6;
			*(pr++) = '0';
			*(pw++) = 0x6;
			*(pr++) = '0' | 0x80;
			break;
		//tube position
		case 'T':
			i = g_tube / 100;
			*(pr++) = i + '0';
			*(pw++) = 0x6;
			i = g_tube % 100;
			*(pr++) = (i / 10) + '0';
			*(pw++) = 0x6;
			*(pr++) = ((i % 10) + '0') | 0x80;
			break;
		//outputs
		case '?':
			*(pr++) = 'C';
			*(pw++) = 0x6;
			*(pr++) = 'C';
			*(pw++) = 0x6;
			*(pr++) = 'D';
			*(pw++) = 0x6;
			*(pr++) = 'D';
			*(pw++) = 0x6;
			*(pr++) = 'D';
			*(pw++) = 0x6;
			*(pr++) = 'D' | 0x80;
			break;
	}

	*pw = '\0';
	*pr = '\0';

	//log
#ifdef COM_LOGGING
	if (COM_log_enable)
		COM_LogText(hC, ret);
#endif

	pr--;
	if (*pr & 0x80)
		*pr &= 0x7F;

	return(ret);
}

/* simulate buffered command */
DWORD FC203B_SimulBC(CF_COMMH *hC, unsigned char adr, const char *cmd)
{
	unsigned char *pw, *pr;
	DWORD ret;

	if (hC == NULL || cmd == NULL)
		return(-1);		//error

	pw = (unsigned char *)hC->sendbuff;
	pr = (unsigned char *)hC->rcptbuff;

	//init buffers
	*(pw++) = adr | 0x80;
	*(pr++) = adr | 0x80;
	*(pw++) = '\n';
	*(pr++) = '\n';
	*(pw++) = *cmd;

	//process command
	ret = 0;
	switch (*cmd) {
		default: ret = 1; break;	//invalid response
		//set tube position
		case 'T':
			g_tube = atoi(cmd+1);
			break;
		//keys
		case 'K':
			if (*(cmd+1) == 'E')
				g_tube = 0;
			break;
		//ok commands
		case 'V':
		case 'L':
		case 'G':
		case 'C':
		case 'D':
			break;
	}

	if (ret == 0) {
		*(pr++) = *cmd;
		cmd++;
		while (*cmd) {
			*(pw++) = *cmd;
			*(pr++) = *cmd;
			cmd++;
		}
	}

	*pw = '\0';
	*pr = '\0';

	//log
#ifdef COM_LOGGING
	if (COM_log_enable)
		COM_LogText(hC, ret);
#endif

	return(ret);
}

#endif



