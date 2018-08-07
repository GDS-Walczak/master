/*
 * xchf122sc.h
 *
 * FC203B device - source file
 *
 * Author: Filip Kinovic
 * Version: 1.0
 * Date: 19.12.2008
*/

#ifndef _XFC203B_H_
#define _XFC203B_H_

#include <windows.h>
//#include "../common/chdevice.h"
#include "../common/commfce.h"

/* constants */

//models
#define FC203B_MODEL_203B 0
#define FC203B_MODEL_204 1

//special option
#define FC203B_SPECIAL_NONE 0
#define FC203B_SPECIAL_21LV 1


#ifdef __cplusplus
extern "C" {
#endif

/* type definitions */

/* function declarations */
DWORD FC203B_CreateDevParams(HDEVICE, void *);

//
BOOL FC203B_GetDeviceInfo(HDEVICE, DEVICEINFO *);
BOOL FC203B_TestID(HDEVICE, TCHAR *, TCHAR *, TCHAR *, TCHAR *);
BOOL FC203B_Validate(HDEVICE, DWORD);
DWORD FC203B_GetStatus(HDEVICE hD, DWORD *status);
//
DWORD FC203B_ReadProp(HDEVICE, WORD);
DWORD FC203B_WriteProp(HDEVICE, WORD, D_VAL);
//
DWORD FC203B_InitDetector(HDEVICE, D_STAT *);
DWORD FC203B_PostInitDetector(HDEVICE, D_STAT *);
//
DWORD FC203B_GetState(HDEVICE hD, int how, unsigned *uval, double *time);

DWORD FC203B_GetVialposition(HDEVICE hD, int how, unsigned *uval, double *time);
DWORD FC203B_SetVialposition(HDEVICE hD, unsigned uval);
DWORD FC203B_GetOutputs(HDEVICE hD, int how, unsigned *uval, double *time);
DWORD FC203B_SetOutputs(HDEVICE hD, unsigned uval);
//
DWORD FC203B_MakeOper(HDEVICE, WORD);
//
DWORD FC203B_SetRemoteControl(HDEVICE hD);
DWORD FC203B_SetLocalControl(HDEVICE hD);
DWORD FC203B_MakeBeep(HDEVICE hD, unsigned ms_duration);

//*** function gets FC vial range
DWORD FC203B_GetFCInfo(HDEVICE hD, D_FC_INFO *info);
DWORD FC203B_Start(HDEVICE hD, LPARAM vialpos);
DWORD FC203B_CollectWaste(HDEVICE hD, LPARAM c_w);
DWORD FC203B_Next(HDEVICE hD);
DWORD FC203B_Stop(HDEVICE hD);

//
BOOL CALLBACK FC203B_SetupParamDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK FC203B_SetupInitDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK FC203B_OnLineDlgProc(HWND, UINT, WPARAM, LPARAM);

//
DWORD FC203B_SendIC(CF_COMMH *hC, unsigned char adr, char cmd);
DWORD FC203B_SendBC(CF_COMMH *hC, unsigned char adr, const char *cmd);


#ifdef __cplusplus
}
#endif

#endif	/* end of _XFC203B_H_ */

