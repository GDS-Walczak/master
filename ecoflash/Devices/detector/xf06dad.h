/*
 * xf06dad.h
 *
 * F06DAD device - source file
 *
 * Author: Filip Kinovic
 * Version: 1.9
 * Date: 08.03.2017
*/

#ifndef _XF06DAD_H_
#define _XF06DAD_H_

#include <windows.h>
#include "../../chdevice.h"
#include "../../mygctrls.h"


/* constants */
#define XF06DAD_DEF_HOST TEXT("192.168.91.60")
#define XF06DAD_DEF_PORT 10001
#define XF06DAD_CONNECT_TIMEOUT 3500		//3.5s



#ifdef __cplusplus
extern "C" {
#endif

/* constants */
#define _XF06DAD_ST_IDLE 0
#define _XF06DAD_ST_INIT 1
#define _XF06DAD_ST_MEAS 2
#define _XF06DAD_ST_ZERO 3
#define _XF06DAD_ST_SABS 4
#define _XF06DAD_ST_SINT 5
#define _XF06DAD_ST_UMOD 6
#define _XF06DAD_ST_SELF 7
#define _XF06DAD_ST_FMOD 8
#define _XF06DAD_ST_ASCA 9

//propeties
#define _XF06DAD_MODEL 0
#define _XF06DAD_LAMP 1
#define _XF06DAD_WLA 2
#define _XF06DAD_WLB 3
#define _XF06DAD_WLC 4
#define _XF06DAD_WLD 5
#define _XF06DAD_HALF 6
#define _XF06DAD_FILTER 7
#define _XF06DAD_STAT 8
#define _XF06DAD_ERROR 9
#define _XF06DAD_LIFE 10
#define _XF06DAD_CLAM 11
#define _XF06DAD_ABSA 12
#define _XF06DAD_ABSB 13
#define _XF06DAD_ABSC 14
#define _XF06DAD_ABSD 15
#define _XF06DAD_INTA 16
#define _XF06DAD_INTB 17
#define _XF06DAD_INTC 18
#define _XF06DAD_INTD 19
#define _XF06DAD_ABSW 20
#define _XF06DAD_DFCE 21
#define _XF06DAD_DTHR 22
#define _XF06DAD_INJZERO 23
#define _XF06DAD_ALAMP 24
//virgil
#define _XF06DAD_ARATIO 25
#define _XF06DAD_AOFSA 26
#define _XF06DAD_AOFSB 27
#define _XF06DAD_AOFSC 28
#define _XF06DAD_AOFSD 29
#define _XF06DAD_BRIGHT 30
//
#define _XF06DAD_WLSCANRANGE 31
#define _XF06DAD_TEXTBY 32
#define _XF06DAD_GRIDBY 33
#define _XF06DAD_TEMP_WLRANGE 34
//#define _XF06DAD_TEMP_CONF 35
#define _XF06DAD_TEMP_EABS 38
#define _XF06DAD_TEMP_EINT 39
#define _XF06DAD_TEMP_EABSW 40
#define	_XF06DAD_TEMP_COMBUFF 41
#define	_XF06DAD_TEMP_SCANDATA 42
#define	_XF06DAD_TEMP_SCANSIZE 43
#define	_XF06DAD_TEMP_FIRSTSIG 44
#define	_XF06DAD_TEMP_CRR 45
#define	_XF06DAD_TEMP_MODEL 46
#define _XF06DAD_TEMP_DISPLAY 47
//
#define _XF06DAD_INIT_LAMP 48
#define _XF06DAD_INIT_MABS 49
#define _XF06DAD_INIT_AZERO 50
#define _XF06DAD_INIT_AZERODELAY 51
#define _XF06DAD_DEIN_LAMP 52
#define _XF06DAD_TEMP_SPARS 53
#define _XF06DAD_TEMP_ONLINE 54
//
#define _XF06DAD_NP (_XF06DAD_TEMP_ONLINE+1)		//number of prop.
//oper. propeties
#define _XF06DAD_AUTOZERO 0
#define _XF06DAD_BEEP 1
#define _XF06DAD_NOP (_XF06DAD_BEEP+1)		//number of oper prop.

//models
#define XF06DAD_MAXMODELS 9
#define XF06DAD_MODELA 0
#define XF06DAD_MODELB 1
#define XF06DAD_MODELC 2
#define XF06DAD_MODELD 3
#define XF06DAD_MODELE 4
#define XF06DAD_MODELF 5
#define XF06DAD_MODELG 6
#define XF06DAD_MODELH 7
#define XF06DAD_MODELI 8
#define XF06DAD_MODELA_CHAR 'A'
#define XF06DAD_MODELB_CHAR 'B'
#define XF06DAD_MODELC_CHAR 'C'
#define XF06DAD_MODELD_CHAR 'D'
#define XF06DAD_MODELE_CHAR 'E'
#define XF06DAD_MODELF_CHAR 'F'
#define XF06DAD_MODELG_CHAR 'G'
#define XF06DAD_MODELH_CHAR 'H'
#define XF06DAD_MODELI_CHAR 'I'
//baudrates
#define XF06DAD_MODELA_BAUD 38400		//A,B,C,E
#define XF06DAD_MODELD_BAUD 19200		//D,F
#define XF06DAD_MODELH_BAUD 57600		//H

#define _XF06DAD_MAXCHANNELS (_XF06DAD_INTA-_XF06DAD_ABSA)
#define _XF06DAD_MAXCHANSA _XF06DAD_MAXCHANNELS
#define _XF06DAD_MAXCHANSB 1
#define _XF06DAD_MAXCHANSD 2

#define _XF06DAD_MAXWCHANSA 0
#define _XF06DAD_MAXWCHANSG 1

#define XF06DAD_ACHAR 'A'
#define XF06DAD_WCHAR 'W'

#define XF06DAD_TMODEL_F06 0
#define XF06DAD_TMODEL_F10 1
#define XF06DAD_TMODEL_C640 2
#define XF06DAD_TMODEL_F12 3

#define _XF06DAD_DTHR_DEF 1.0		//1mV (100 x10uV)
#define _XF06DAD_DTHR_MIN 0.5		//0.5mV (50 x10uV)
#define _XF06DAD_DTHR_MAX 999.99		//999.99mV (99999 x10uV)
#define _XF06DAD_DFCE_IDX_ADIVB 6

#define _XF06DAD_SCANMODE_ABS 0
#define _XF06DAD_SCANMODE_INT 1
#define _XF06DAD_SCANMODE_IDX 2
#define _XF06DAD_SCANMODE_SABS 3

#define _XF06DAD_SUBSMODE_ABS 0
#define _XF06DAD_SUBSMODE_INT 1
#define _XF06DAD_SUBSMODE_SABS 2

//buffers
#define XF06DAD_MAX_SENDLEN 1024
#define XF06DAD_MAX_RCPTLEN 8192

#define XF06DAD_MAX_CRRSIZE 5000

#define _XF06DAD_DEF_ALAMP 0		//off
#define _XF06DAD_DEF_ARATIO 4		//1AU/V
#define _XF06DAD_DEF_AOFST 0.0
#define _XF06DAD_MIN_AOFST -250.0
#define _XF06DAD_MAX_AOFST 250.0
#define _XF06DAD_PREC_AOFST 2
#define _XF06DAD_DEF_BRIGHT 3	//50%


/* type definitions */
typedef struct {
	double wl[_XF06DAD_MAXCHANNELS];
	double aofs[_XF06DAD_MAXCHANNELS];
	//
	unsigned int _count;
	unsigned int _rest;
	unsigned int _err;
	int _st;
} F06DAD_SPARS;


/* function declarations */
DWORD F06DAD_CreateDevParams(HDEVICE, void *);

//
BOOL F06DAD_GetDeviceInfo(HDEVICE, DEVICEINFO *);
DWORD F06DAD_OpenDevice(HDEVICE hD);
DWORD F06DAD_CloseDevice(HDEVICE hD);
DWORD F06DAD_Search(HDEVICE hD, HLIST hlist);
BOOL F06DAD_TestID(HDEVICE, TCHAR *, TCHAR *, TCHAR *, TCHAR *);
DWORD F06DAD_GetStatus(HDEVICE hD, DWORD *status);
BOOL F06DAD_Validate(HDEVICE hD, DWORD pidx);
//
DWORD F06DAD_ReadProp(HDEVICE, WORD);
DWORD F06DAD_WriteProp(HDEVICE, WORD, D_VAL);
DWORD F06DAD_PreSetProp(HDEVICE, WORD, D_VAL);
//
DWORD F06DAD_InitDetector(HDEVICE, D_STAT *);
DWORD F06DAD_PostInitDetector(HDEVICE, D_STAT *);
//
BOOL F06DAD_TestABC(const char *str, unsigned n, unsigned offset, char beginchar);
BOOL F06DAD_TestXYZ(const char *str, unsigned n, unsigned offset, char beginchar);
unsigned F06DAD_GetModelByChar(int c);
unsigned F06DAD_GetModel(HDEVICE hD);
unsigned F06DAD_GetWavelengthByModel(int model);
unsigned F06DAD_GetChannelsByModel(int model);
unsigned F06DAD_GetBaudrateByModel(int model);
unsigned F06DAD_GetSTimingByModel(int model);
unsigned F06DAD_GetWChannelsByModel(int model);
int F06DAD_IsModel800(int model);
int F06DAD_IsModelSubScan(int model);
int F06DAD_GetFWVersion(HDEVICE hD);
#define F06DAD_IsVersionWithDfce() ((F06DAD_GetFWVersion()%100) >= 45)
int F06DAD_IsDisplay(HDEVICE hD);
//
DWORD F06DAD_GetLamp(HDEVICE, int, WORD, BOOL *, double *time);
DWORD F06DAD_SetLamp(HDEVICE, DWORD, BOOL);
DWORD F06DAD_GetWavelength(HDEVICE, int, WORD, double *, double *time);
DWORD F06DAD_PreSetWavelength(HDEVICE hD, DWORD, double);
DWORD F06DAD_SetWavelength(HDEVICE hD, DWORD, double);
DWORD F06DAD_GetHalfWidth(HDEVICE, int, unsigned *, double *time);
DWORD F06DAD_SetHalfWidth(HDEVICE hD, unsigned);
DWORD F06DAD_GetFilter(HDEVICE, int, unsigned *, double *time);
DWORD F06DAD_SetFilter(HDEVICE hD, unsigned);
DWORD F06DAD_GetWavelengthRange(HDEVICE, int, WORD, unsigned *, double *time);
#define F06DAD_GetWavelengthMeasRange(hD, how, u, t) F06DAD_GetWavelengthRange(hD, how, 0, u, t)
#define F06DAD_GetWavelengthScanRange(hD, how, u, t) F06DAD_GetWavelengthRange(hD, how, 1, u, t)
DWORD F06DAD_SetWavelengthScanRange(HDEVICE, unsigned);
DWORD F06DAD_GetAbs(HDEVICE, int, WORD, double *, double *time, D_DHQUEUE *dhq);
DWORD F06DAD_GetIntensity(HDEVICE, int, WORD, double *, double *time);		//???
DWORD F06DAD_GetWaveLength(HDEVICE, int, WORD, double *, double *time);
DWORD F06DAD_GetLampLife(HDEVICE, int, WORD, double *, double *time);
DWORD F06DAD_GetState(HDEVICE, int, unsigned *, double *time);
DWORD F06DAD_GetError(HDEVICE hD, int how, unsigned *uval, double *time);
//
BOOL F06DAD_SetScanMode(HDEVICE hD, int mode);
int F06DAD_GetDataScanMode(HDEVICE hD);
DWORD F06DAD_GetScan(HDEVICE hD, int how, DWORD *nn, double **xx, double **yy, char **ee);
//
DWORD F06DAD_StartSScan(HDEVICE hD);
DWORD F06DAD_StopSScan(HDEVICE hD);
DWORD F06DAD_GetSScan(HDEVICE hD, DWORD *nn, double **xx, double **yy, char **ee);
F06DAD_SPARS *F06DAD_GetSScanParams(HDEVICE hD);
DWORD F06DAD_MakeSScanAutozero(HDEVICE hD);
//
DWORD F06DAD_GetDFunction(HDEVICE hD, unsigned int *uval, double *time);
DWORD F06DAD_SetDFunction(HDEVICE hD, unsigned int uval);
DWORD F06DAD_GetDThreshold(HDEVICE hD, double *dval, double *time);
DWORD F06DAD_SetDThreshold(HDEVICE hD, double dval);
//
DWORD F06DAD_GetAutoLamp(HDEVICE hD, unsigned int *uval, double *time);
DWORD F06DAD_SetAutoLamp(HDEVICE hD, unsigned int uval);
//virgil
DWORD F06DAD_GetAnalogRatio(HDEVICE hD, unsigned int *uval, double *time);
DWORD F06DAD_SetAnalogRatio(HDEVICE hD, unsigned int uval);
DWORD F06DAD_GetAnalogOffset(HDEVICE hD, int how, WORD idx, double *dval, double *time);
DWORD F06DAD_PreSetAnalogOffset(HDEVICE hD, DWORD idx, double dval);
DWORD F06DAD_SetAnalogOffset(HDEVICE hD, DWORD idx, double dval);
DWORD F06DAD_GetBrightness(HDEVICE hD, unsigned int *uval, double *time);
DWORD F06DAD_SetBrightness(HDEVICE hD, unsigned int uval);
//
DWORD F06DAD_MakeOper(HDEVICE, WORD);
//
DWORD F06DAD_SetAbsMode(HDEVICE);
DWORD F06DAD_SetTestMode(HDEVICE);
DWORD F06DAD_RunAutozero(HDEVICE);
DWORD F06DAD_SubscribeMode(HDEVICE, int, BOOL);
DWORD F06DAD_MakeBeep(HDEVICE, unsigned);
//
BOOL CALLBACK F06DAD_SetupParamDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK F06DAD_SetupInitDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK F06DAD_OnLineDlgProc(HWND, UINT, WPARAM, LPARAM);
HWND F06DAD_CreateScanDialog(HINSTANCE, HWND, DLGPROC, LPARAM);
BOOL CALLBACK F06DAD_ScanDlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL F06DAD_CreateScanExportPopup(HWND hwnd, int x, int y, void *param);
BOOL F06DAD_DlgSetFilePath(HINSTANCE hinst, HWND hwnd, TCHAR *filepath, DWORD size);
DWORD F06DAD_ExportScan(HWND hwnd, TCHAR *filename, DWORD count, double *x, double *y, char *e, DWORD flags);
DWORD F06DAD_ExportToEMF(HWND hGA, TCHAR *filename, int picw, int pich);

//---------
BOOL F06DAD_ReportItemProcParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam);
DWORD F06DAD_GetReport(HDEVICE hD);
BOOL F06DAD_CreateCRFile(HLIST hreport, const TCHAR *filename);
BOOL CALLBACK F06DAD_ReportDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);
BOOL F06DAD_FillRichEditFromFile(HWND hwnd, LPCTSTR pszFile);

//
unsigned int F06DAD_DecodeErrorA(const char *str);
int F06DAD_PrintError(unsigned int err, int bytes, TCHAR *str);
DWORD F06DAD_GetTopError(DWORD errs);
const TCHAR *F06DAD_GetErrorDescription(int err_no);
const TCHAR *F06DAD_GetStatusDescription(int status);
BOOL F06DAD_ErrorsDecodeBox(HWND hwnd, unsigned int err_code, const TCHAR *title, const TCHAR *descr_fce(int));
//
BOOL F06DAD_Special(HDEVICE hD, DSPECIAL specidx, LPARAM lparam);




#ifdef __cplusplus
}
#endif

#endif	/* end of _XF06DAD_H_ */

