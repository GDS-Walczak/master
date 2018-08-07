/*
 * device.h
 *
 * Device - source file
 *
 * Author: Filip Kinovic
 * Version: 3.3
 * Date: 26.05.2017
*/


#ifndef _CHDEVICE_H_
#define _CHDEVICE_H_

#include <windows.h>
#include "comfce.h"
#include "myqueue.h"
#include "simplexml.h"
#include "timer.h"
#include "usefulapi.h"
#include "chlanguage.h"
#include "chunits.h"
#include "list.h"
#include "memwatch.h"

/* constants */
//max com. ports
#define D_MIN_COMPORT 1
#define D_MAX_COMPORT 30
#define D_MAX_ADDDEVICES 16
#define D_MAX_CID_LEN 100
#define D_MAX_SID_LEN 100
#define D_MAX_ID_LEN 120
#define D_MAX_TEXT_LEN 128
#define D_MAX_SUFFIX_LEN 5
#define D_MAX_DEVNAME_LEN 64
#define D_MAX_PROPNAME_LEN 64
#define D_MAX_PROPSUFFIX_LEN 8
#define D_MAX_PROPQUAN_LEN 64
#define D_MAX_PROPUNIT_LEN 8
#define D_MAX_SIGNALNAME_LEN (D_MAX_PROPNAME_LEN+D_MAX_PROPSUFFIX_LEN+10)

//mesages
#define WMD_MAIN (WM_USER+0)
#define WMD_APPLY (WM_USER+1)
#define WMD_START (WM_USER+2)
#define WMD_STOP (WM_USER+3)
#define WMD_STATUS (WM_USER+4)
#define WMD_ENABLE (WM_USER+5)
#define WMD_RESPONSE (WM_USER+6)
#define WMD_SELECTDLG (WM_USER+7)
#define WMD_SYNC (WM_USER+8)
#define WMD_UPDATE (WM_USER+9)
#define WMD_TEST (WM_USER+10)
#define WMD_PAUSE (WM_USER+12)
#define WMD_RESUME (WM_USER+13)
#define WMD_SPECIAL (WM_USER+20)

//global messages
#define WMD_MEASURE (WM_USER+30)
#define WMD_MEASURES (WM_USER+32)
#define WMD_DEVERROR  (WM_USER+33)
//
#define WMD_EX_ERROR (WM_USER+40)
#define WMD_EX_START (WM_USER+41)
#define WMD_EX_STOP (WM_USER+42)
#define WMD_EX_MARK_E (WM_USER+43)	//external
#define WMD_EX_MARK_F (WM_USER+44)	//fraction
#define WMD_EX_MARK_R (WM_USER+45)	//recycle
#define WMD_EX_ZERO (WM_USER+46)
#define WMD_EX_BEEP (WM_USER+47)
#define WMD_EX_FCW (WM_USER+48)			//frac. collect/waste
#define WMD_EX_PAUSE (WM_USER+51)			//pause
#define WMD_EX_RESUME (WM_USER+52)			//resume
#define WMD_EX_SPECIAL (WM_USER+60)	//special
//
#define WMD_USER (WM_USER+80)


//models
#define D_MODEL_TYPE						0x00FFFFFFL
#define D_MODEL_NOAUTO					0x01000000L		//no autodetection
#define D_MODEL_DEMO						0x02000000L		//do not use comport
#define D_MODEL_CALIB						0x04000000L		//calibration device

#define D_MODEL_UNKNOWN 				0x000000L
#define D_MODEL_DETECTOR 				0x000001L
#define D_MODEL_PUMP 						0x000002L
#define D_MODEL_AUTOSAMPLER 		0x000004L
#define D_MODEL_FRACCOLLECTOR 	0x000008L
#define D_MODEL_THERMOOVEN 			0x000010L
#define D_MODEL_STARTMARK				0x000020L		//starter & marker
#define D_MODEL_THERMOMETER			0x000040L		//thermometer
#define D_MODEL_OUTPUT  				0x000080L		//output device
#define D_MODEL_VALVE	  				0x000100L		//valve device
#define D_MODEL_COLUMN  				0x000200L		//chrom. column
#define D_MODEL_MULTIMETER			0x000400L		//multimeter
#define D_MODEL_SOURCE					0x000800L		//source of voltage/current
#define D_MODEL_FLOWMETER				0x001000L		//flowmeter
#define D_MODEL_CONVERTER				0x002000L		//converter
#define D_MODEL_COUNTER					0x004000L		//counter
#define D_MODEL_BALANCE					0x008000L		//balance
#define D_MODEL_SYSDEVICE				0x010000L		//system device

#define D_MODEL_MULTIPLE 				0x400000L		//complex device
#define D_MODEL_OTHER						0x800000L		//other device

//colors

//device flags
#define D_FLAG_AUTOZERO					0x00000001		//device can autozero

//property flags
#define DFLAG_SAVE							0x00000001		//property is saved to ch-file
#define DFLAG_MET								0x00000002		//property is saved to method-file
#define DFLAG_CONF							0x00000003		//property is used for configuration (value is saved to ch-file)
#define DFLAG_STAT							0x00000005		//property is used for status (value is saved to ch-file)
#define DFLAG_CTRL							0x00000008		//property is available for control
#define DFLAG_QUEUE							0x00000010		//property with data queue
#define DFLAG_GRAD							0x00000020		//property is default for control gradient control
#define DFLAG_READONLY					0x00000040		//property is readonly
#define DFLAG_CTRLREAD					0x00000048		//property is readonly for control
#define DFLAG_SEND							0x00000080		//property is used for preparing device (send-value)
#define DFLAG_MEAS							0x00000100		//property is measureable
#define DFLAG_PROC							0x00000200		//property is processed
#define DFLAG_MEASPROC					0x00000300		//property is measureable and processed
#define DFLAG_NOSIGNAL					0x00000400		//property is default not signaled
#define DFLAG_RECV							0x00000800		//property is used for preparing device (receive-value)
#define DFLAG_MIN								0x00001000		//property is limited to minimum value
#define DFLAG_MAX								0x00002000		//property is limited to maximum value
#define DFLAG_MINMAX						(DFLAG_MIN|DFLAG_MAX)		//property is limited to range (min, max)
#define DFLAG_MEM								0x00004000		//property is mem. variable only (do not copy)
#define DFLAG_ZER								0x00008000		//property is can be zero (if out of MIN-MAX range)
#define DFLAG_MINMAXZER					(DFLAG_MIN|DFLAG_MAX|DFLAG_ZER)		//property is limited to range (min, max, zero)
#define DFLAG_FPREC							0x00010000		//property is formated with prec property
#define DFLAG_NOCONVUNIT				0x00020000		//do not convert property units
//#define DFLAG_FPRECMASK				0x00F00000		//prec property mask
//
#define DFLAG_GRAYABLE					0x01000000		//property is able to enable/disable other properties by DFLAG_DISABLED flag
#define DFLAG_DISABLED					0x02000000		//property is disabled
#define DFLAG_IGNORE						0x04000000		//property read/write can be ignored
//
#define DFLAG_PRECSHIFT					20						//precition shift
#define DFLAG_PRECMASK					(0x0F<<DFLAG_PRECSHIFT)		//precition mask (0x00F00000)
#define DFLAG_SETPREC(prec)			((prec & 0x0F)<<DFLAG_PRECSHIFT)
#define DFLAG_GETPREC(flag)			((flag>>DFLAG_PRECSHIFT) & 0xF)

//group flags
#define DGROUP_LAST 0x80000000

//gui flags
#define DGFLAG_EMPH	0x00000001


//errors
#define D_ERROR_DONTNOTIFY 0x8FFFFEFF
//property errors
#define D_PERROR_NOTAVAILABLE 1
#define D_PERROR_BADRESPOND 2

//modes
#define D_PRMODE_PAUSE 0
#define D_PRMODE_RESUME 1

//texts
#define D_STRQUESTION TEXT("n/a")
#define D_STR_PROPENABLE TEXT("PROPENABLE")


//device filters
#define D_DEVFILTER_ECOM_TEST NULL		//ECOM test devices
#define D_DEVFILTER_ECOM_START NULL		//ECOM start/mark devices
#define D_DEVFILTER_ECOM_CHROM NULL		//ECOM standard chromatographic devices
#define D_DEVFILTER_ECOM_NONCHROM NULL		//ECOM non-chromatographic devices
#define D_DEVFILTER_ECOM_SPECIAL TEXT("ECOM_INTERNAL")		//ECOM internal devices
#define D_DEVFILTER_ANALTECH TEXT("ECOM_INTERNAL")		//ANALTECH devices only
#define D_DEVFILTER_JAISCAN TEXT("ECOM_INTERNAL")		//JAI Scan devices only
#define D_DEVFILTER_OTHER_CHROM NULL		//other chromatografic devices
#define D_DEVFILTER_OTHER_NONCHROM NULL		//other non-chromatographic devices
#define D_DEVFILTER_OTHER_SPECIAL TEXT("ECOM_INTERNAL")		//other special devices
#define D_DEVFILTER_DEVEL TEXT("ECOM_DEVEL")		//devices under development

//spec. make absorbance for voltage signal
#define D_USEVOLT_ABS

#ifdef D_USEVOLT_ABS
	#define D_SetABSProperty(prop, name, flags, type) D_SetProperty(prop, name, DC_("absorbance"), TEXT("mAU"), flags, type)
	#define D_SetABSPropertySuffix(prop, name, suffix, flags, type) D_SetPropertySuffix(prop, name, suffix, DC_("absorbance"), TEXT("mAU"), flags, type)
#else
	#define D_SetABSProperty(prop, name, flags, type) D_SetProperty(prop, name, DC_("voltage"), TEXT("mV"), flags, type)
	#define D_SetABSPropertySuffix(prop, name, suffix, flags, type) D_SetPropertySuffix(prop, name, suffix, DC_("voltage"), TEXT("mV"), flags, type)
#endif

#define D_SetVOLTProperty(prop, name, flags, type) D_SetProperty(prop, name, DC_("voltage"), TEXT("mV"), flags, type)
#define D_SetVOLTPropertySuffix(prop, name, suffix, flags, type) D_SetPropertySuffix(prop, name, suffix, DC_("voltage"), TEXT("mV"), flags, type)


//redefinitions
#ifdef ANALCHROM
	#include "app_analchrom/analchrom_devs.h"
#endif
#ifdef JAISCAN
	#include "app_jaiscan/jaiscan_devs.h"
#endif

//start/mask masks
#define D_SMMASK__ACTION		0xFFFF
#define D_SMMASK_ACT_MARK 	0x0001
#define D_SMMASK_ACT_START 	0x0002
#define D_SMMASK_ACT_STOP 	0x0004
#define D_SMMASK_ACT_ZERO 	0x0008
#define D_SMMASK_ACT_FCW	 	0x0010
#define D_SMMASK_ACT_SPEC 	0x0080

//indexes
#define D_PIDX_HOLDCOM 			0x7FFF
#define D_HOLDCOM_TIME	1000.0	//1s

//others
#define D_SP0 8							//space 0
#define D_SP1 10						//space 1
#define D_SP2 2							//space 2
#define D_STW 120						//st. control width
#define D_CTW 120						//control width
#define D_STH 20						//st. control height
#define D_CTH D_STH					//control height
#define D_CBH 120						//combobox control height
#define D_CBH2 240						//combobox control height
#define D_OFH (D_STH+2)			//horizontal offset
#define D_STT (33+3)				//st. control top
#define D_CTT (33)					//control top

#define D_GPL D_SP0					//group left
#define D_GPLL (D_SP0+3*D_SP1+D_STW+D_CTW+D_SP2)				//group left next
#define D_GPW (3*D_SP1+D_STW+D_CTW)						//group width
#define D_GPWW (2*D_GPW+D_SP2)					//group width both
#define D_GPT (D_SP1)				//group top
#define D_STL (D_SP0+D_SP1)				//st. control left
#define D_STLL (D_STL+D_GPW+D_SP2)				//st. control left next
#define D_CTL (D_SP0+2*D_SP1+D_STW)				//control left
#define D_CTLL (D_CTL+D_GPW+D_SP2)				//control left next
#define D_ARW (D_STW+D_SP1+D_CTW)					//area inside group

#define D_CB_DEFHEIGHT 120
#define D_DEFAULT_CTRLTIME 500		//control time 500 ms

//dialog constants
#define D_PA_H 20
#define D_PA_HOF (D_PA_H+2)
#define D_PA_HCB D_CB_DEFHEIGHT
#define D_PA_HBT (D_PA_H+1)
#define D_PA_HEB (D_PA_H+1)
#define D_PA_HCBI (D_PA_H-5)
#define D_PA_W1 122
#define D_PA_W2 108
#define D_PA_WB 75
#define D_PA_WBS 30
#define D_PA_S1 10
#define D_PA_S2 8
#define D_PA_S3 4
#define D_PA_S4 4
#define D_PA_TOF (26)
#define D_PA_GW1IN (D_PA_W1+D_PA_S3+D_PA_W2)
#define D_PA_GW1 (D_PA_S2+D_PA_GW1IN+D_PA_S2)
#define D_PA_GTOF(i) (D_PA_S1+(i)*D_PA_HOF)
#define D_PA_GLOF(i) (D_PA_S1+(i)*(D_PA_GW1+D_PA_S4))

#define D_PA_ROWGP(row) (D_PA_GTOF(row))
#define D_PA_ROWST(row) (D_PA_GTOF(row)+D_PA_TOF+3)
#define D_PA_ROWVAL(row) (D_PA_GTOF(row)+D_PA_TOF)
#define D_PA_COLGP(col) (D_PA_GLOF(col))
#define D_PA_COLST(col) (D_PA_GLOF(col)+D_PA_S2)
#define D_PA_COLVAL(col) (D_PA_GLOF(col)+D_PA_S2+D_PA_W1+D_PA_S3)
#define D_PA_GH(n) (((n)+2)*D_PA_HOF-D_PA_S3)
#define D_PA_GW(n) ((n)*(D_PA_GW1+D_PA_S4)-D_PA_S4)
#define D_PA_GWIN(n) (D_PA_GW((n))-2*D_PA_S2)
#define D_PA_GHIN(n) ((n)*D_PA_HOF)

#define D_PA_W2S 18
#define D_PA_W2W ((D_PA_W2-D_PA_W2S)/2)


//old
#define D_PA_CTOF1(i) (D_PA_GTOF(i)+D_PA_TOF+3)
#define D_PA_CTOF2(i) (D_PA_GTOF(i)+D_PA_TOF)
#define D_PA_GLOF1 D_PA_S1
#define D_PA_CLOF1 (D_PA_GLOF1+D_PA_S2)
#define D_PA_CLOF2 (D_PA_CLOF1+D_PA_W1+D_PA_S3)
#define D_PA_GLOF2 (D_PA_CLOF2+D_PA_W2+D_PA_S2+D_PA_S4)
#define D_PA_CLOF3 (D_PA_GLOF2+D_PA_S2)
#define D_PA_CLOF4 (D_PA_CLOF3+D_PA_W1+D_PA_S3)
#define D_PA_GLOF3 (D_PA_CLOF4+D_PA_W2+D_PA_S2+D_PA_S4)
#define D_PA_CLOF5 (D_PA_GLOF3+D_PA_S2)
#define D_PA_CLOF6 (D_PA_CLOF5+D_PA_W1+D_PA_S3)
#define D_PA_WW(n) (D_PA_GLOF1+D_PA_GW((n))+D_PA_GLOF1)
#define D_PA_BOF D_PA_WBS
#define D_PA_2BOF (2*D_PA_WBS)

//
#define D_PA_GW2IN (D_PA_GW2-2*D_PA_S2)
#define D_PA_SKIPO(i,n,size) ((i)*((size)/(n)))
#define D_PA_SKIPW(n,size) ((size)/(n)-1)
#define D_PA_SKIPO1(i,n) D_PA_SKIPO(i,n,D_PA_GW1IN)
#define D_PA_SKIPW1(n) D_PA_SKIPW(n,D_PA_GW1IN)
#define D_PA_SKIPO2(i,n) D_PA_SKIPO(i,n,D_PA_GW2IN)
#define D_PA_SKIPW2(n) D_PA_SKIPW(n,D_PA_GW2IN)

//define param constants
#define D_OL_WM_UPDATE (WM_USER+100)


//device online constants
#define D_OL_H 20		// 20?
#define D_OL_TOF 25
#define D_OL_LOF 18
#define D_OL_HOF (D_OL_H+2)
#define D_OL_HEB (D_OL_H+1)
#define D_OL_HBT (D_OL_H+1)
#define D_OL_SOF 1
#define D_OL_STW 120
#define D_OL_W 88
#define D_OL_SBW 30
#define D_OL_UDW 15
#define D_OL_MOF 30
#define D_OL_WLOF (D_OL_LOF+D_OL_STW+D_OL_W+2*D_OL_SBW+D_OL_MOF)
#define D_OL_WWLOF (D_OL_WLOF+D_OL_STW+D_OL_SOF+D_OL_W+D_OL_SBW)
#define D_OL_XLOF (D_OL_LOF+D_OL_STW+D_OL_SOF+D_OL_W+D_OL_SBW)
#define D_OL_XXLOF (D_OL_XLOF+D_OL_STW+D_OL_SOF+D_OL_W+D_OL_SBW)

//gui control (online) flags
#define D_OL_IDVAL 0
#define D_OL_IDST 100
#define D_OL_IDON 200
#define D_OL_IDOFF 300
#define D_OL_IDUD D_OL_IDON
#define D_OL_IDGB 1000
#define D_OL_IDOCB (D_OL_IDGB-2)
#define D_OL_IDOBT (D_OL_IDGB-1)
#define D_OL_IDUSER 2000

#define D_GETOLID_VAL(pidx) ((pidx)+D_OL_IDVAL)
#define D_GETOLID_ST(pidx) ((pidx)+D_OL_IDST)
#define D_GETOLID_ON(pidx) ((pidx)+D_OL_IDON)
#define D_GETOLID_OFF(pidx) ((pidx)+D_OL_IDOFF)
#define D_GETOLID_UD(pidx) ((pidx)+D_OL_IDUD)
#define D_GETOLID_VAL2 D_GETOLID_ON
#define D_GETOLH_VAL(hwnd, pidx) GetDlgItem(hwnd, D_GETOLID_VAL(pidx))
#define D_GETOLH_VAL2(hwnd, pidx) GetDlgItem(hwnd, D_GETOLID_VAL2(pidx))
#define D_OL_PIDUSER 60

#define D_OL_POS_C1 	0x000000
#define D_OL_POS_C2 	0x010000
#define D_OL_POS_C12 	0x020000
#define D_OL_XHEIGHT_MASK 	0xF00000	//height mask
#define D_OL_XHEIGHT_2 	0x100000	//2x
#define D_OL_XHEIGHT_3 	0x200000	//3x
#define D_OL_XHEIGHT_4 	0x300000	//4x

/* macros */
//#define D_FABS(x) ((x) < 0.0 ? -(x) : (x))

#define D_MAX_EXTIME_DEV 1500.0	//1.5s


#ifdef __cplusplus
extern "C" {
#endif

/* type definitions */
typedef enum {
	DT_BOOL = 0,
	DT_INT = 1,
	DT_UINT = 2,
	DT_DOUBLE = 3,
	DT_INT64 = 4,
	DT_UINT64 = 5,
	DT_HANDLE = 6,
	DT_MEMORY = 7,		//memory ? bytes (string)
	DT_STRING = 8,		//string
} DTYPE;

typedef union {
		BOOL b;									//DT_BOOL
		int i;									//DT_INT
		unsigned u;							//DT_UINT
		double d;								//DT_DOUBLE
//
		__int64 i64;						//DT_INT64
		unsigned __int64 u64;		//DT_UINT64
//
		HANDLE h;								//DT_HANDLE
		struct {
			unsigned size;
			unsigned char *buf;
		} m;										//DT_STRING or DT_MEMORY
} D_VAL;

/* type definitions */
typedef enum {
	DGUIT_NONE = 0,				//none
	DGUIT_SWITCH = 1,			//static, running/stoped, on, off
	DGUIT_READ = 2,				//static, readonly EB (left)
	DGUIT_VAL = 3,				//static, subclass. EB
	DGUIT_SPINVAL = 4,		//static, subclass. EB, updown
	DGUIT_COMBOBOX = 5,		//static, CB
	DGUIT_BUTTON = 6,			//button
	DGUIT_RANGE = 7,			//2 values in one
	//DGUIT_RADIO = 8,			//radio buttons (similar to combobox, ids?)
	DGUIT_RADIOSW = 9,		//radio switch buttons
	DGUIT_CHECKBITS = 10,	//checkbox buttons for bits (names from guidata, LSb)
	DGUIT_HEX = 12,				//value in hex format (little indian)
	DGUIT_BIGHEX = 13,		//value in hex format (big indian)
	//DGUIT_BIN = 14,				//value in binary format (little indian)
	DGUIT_SMACTION = 21,	//start+mark action (2x CB)
	DGUIT_SHOWMEM = 30,		//memory to list of values
	DGUIT_TABLE = 31,			//memory to table
	DGUIT_SPECIAL = 255		//special
} DGUITYPE;

typedef enum {
	DSPEC_NONE = 0,
	DSPEC_SYNC = 1,						//sychronize (LPARAM time)
	DSPEC_LOADCHECK = 2,			//check device after load (LPARAM dversion; v2.5 = 205)
	DSPEC_SHOWSPECTRUM = 9,		//show spectrum (LPARAM 0)
	DSPEC_LAMPCTRL = 11,			//lamp control (LPARAM lamp)
	DSPEC_IPAUSE = 12,				//pause before deinit
	DSPEC_LISTSIGNALS = 21,
	DSPEC_GETSIGNAL = 22,
} DSPECIAL;
#define DSPECIAL_MASK_ISOPER	0x80000000
#define DSPECIAL_MASK_OPER 		0x7FFFFFFF
#define DSPEC_ASK(s) ((s) | DSPECIAL_MASK_ISOPER)

//data queue item content structure
typedef struct {
	BOOL preserve;
	//int tmode;
	DWORD count;
	double *dtime;
	double *value;
} D_DATAQUEUECONTENT;

//data queue item structure
typedef struct {
	D_DATAQUEUECONTENT content;
	void *next;
} D_DATAQUEUEITEM;

//data queue structure
typedef struct {
	DWORD size;
	D_DATAQUEUEITEM *front;
	D_DATAQUEUEITEM *back;
	CRITICAL_SECTION cs;
} D_DATAQUEUE;

typedef HANDLE D_DHQUEUE;		//data queue handle

//device handle
typedef HANDLE HDEVICE;
typedef struct {
	DWORD dcount;
	HDEVICE *hd;
	LPARAM param;		//param
} HDEVICES;

typedef struct {
	const TCHAR *name;
	const TCHAR *alias;		//shown name
	const TCHAR *ver;
	const TCHAR *spec;
	const TCHAR *desc;
	const TCHAR *prod;
	const TCHAR *web;		//website
	DWORD model;
	DWORD flags;
	const TCHAR *filter;
} DEVICEINFO;

typedef BOOL (*DEVINFO)(HDEVICE, DEVICEINFO *);
typedef DWORD (*DEVPARAMS)(HDEVICE, void *);
typedef struct {
	DEVINFO info;
	DEVPARAMS params;
} DEVPOINTER;
typedef BOOL CALLBACK (*DEVICEDLGPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct {
	struct {
		BOOL disable;
		BYTE nmask;
	} baud;
	struct {
		BOOL disable;
		BYTE nmask;
	} databits;
	struct {
		BOOL disable;
		BYTE nmask;
	} parity;
	struct {
		BOOL disable;
		BYTE nmask;
	} stopbits;
	struct {
		BOOL disable;
		BYTE nmask;
	} handshake;
} COMMASK;

typedef struct {
	BOOL run;
	WORD perc;
	const TCHAR *text;
} D_STAT;

//scale
typedef struct {
	double my;		//myltiply y
	double oy;		//aditive y offset
} D_DEV_SCALE;

//signal
typedef struct {
	TCHAR name[D_MAX_SIGNALNAME_LEN];
	TCHAR rename[D_MAX_SIGNALNAME_LEN];
	WORD pidx;
	int disabled;
	int hidden;
	D_DEV_SCALE sc;
} D_DEV_SIGNAL;

typedef struct {
	DWORD sid;
	D_DEV_SIGNAL signal;
	//unit?
} D_DEV_SIGNALINFO;


//universal function pointers
typedef struct {
	DEVINFO pGetDeviceInfo;
	DWORD (*pSearch)(HDEVICE hD, HLIST hlist);		//returns number of found devices
	DWORD (*pOpen)(HDEVICE hD);
	DWORD (*pClose)(HDEVICE hD);
	BOOL (*pTestID)(HDEVICE hD, TCHAR *name, TCHAR *type, TCHAR *sw, TCHAR *sn);
	BOOL (*pValidate)(HDEVICE hD, DWORD pidx);
	DWORD (*pGetStatus)(HDEVICE hD, DWORD *status);
	DWORD (*pInit)(HDEVICE hD, D_STAT *state);		//init after setting control
	DWORD (*pPostInit)(HDEVICE hD, D_STAT *state);		//set state before loosing control
	DWORD (*pPrepare)(HDEVICE hD, D_STAT *state);		//set state before loosing control
	DWORD (*pSetOper)(HDEVICE hD, WORD oidx);
	DWORD (*pReadProp)(HDEVICE hD, WORD pidx);
	DWORD (*pWriteProp)(HDEVICE hD, WORD pidx, D_VAL val);
	DWORD (*pPreSetProp)(HDEVICE hD, WORD pidx, D_VAL val);
	DWORD (*pCriticalStop)(HDEVICE hD, LPARAM csmode);
	DWORD (*pPauseResume)(HDEVICE hD, LPARAM prmode);		//0=pause, 1=resume
	BOOL (*pSpecial)(HDEVICE hD, DSPECIAL specidx, LPARAM lparam);	//special function
	DEVICEDLGPROC pSetupCommDlgProc;
	DEVICEDLGPROC pSetupParamDlgProc;
	DEVICEDLGPROC pSetupInitDlgProc;
	DEVICEDLGPROC pOnLineDlgProc;
} U_FCE;

//detector function pointers
typedef struct {
	DWORD (*pAutozero)(HDEVICE);
	DWORD (*pGetSignal)(HDEVICE hD, int how, WORD idx, double *value, double *time, D_DHQUEUE *dhq);
	DWORD (*pGetWaveLengthRanges)(HDEVICE, int, double *);
	DWORD (*pGetWaveLength)(HDEVICE, int, double *, DWORD *);
	DWORD (*pSetWaveLength)(HDEVICE, int, double);
} DET_FCE;

//pump function pointers
typedef struct {
	DWORD (*pGetFlowRanges)(HDEVICE, double *, DWORD *);
	DWORD (*pGetPressureRanges)(HDEVICE, double *, DWORD *);
	DWORD (*pSetPressureRanges)(HDEVICE, double, double);
	DWORD (*pGetFlow)(HDEVICE, int, double *);
	DWORD (*pSetFlow)(HDEVICE, int, double);
} PUMP_FCE;

//fraction collector function pointers
typedef struct {
	unsigned tube_from;		//the first tube available
	unsigned tube_to;		//the last tube available
	unsigned tube_step;
	unsigned cw_valve;		//collect/waste valve available
} D_FC_INFO;

typedef struct {
	double flow;		//set actual flow to FC
	//
	double res_dbl[4];
	unsigned int res_u32[4];
} D_FC_PARS;
//
typedef struct {
	DWORD (*pGetFCInfo)(HDEVICE, D_FC_INFO *info);
	DWORD (*pStart)(HDEVICE hD, LPARAM vialpos);		//vialpos==0 -> drain
	DWORD (*pCollectWaste)(HDEVICE hD, LPARAM c_w);
	DWORD (*pNext)(HDEVICE hD);
	DWORD (*pStop)(HDEVICE hD);
	DWORD (*pSetPars)(HDEVICE hD, D_FC_PARS *pars);
} FC_FCE;
//
typedef enum {
	D_FCFCE_NONE = 0,
	D_FCFCE_INFO = 1,
	D_FCFCE_START = 2,
	D_FCFCE_COLLECT = 3,
	D_FCFCE_WASTE = 4,
	D_FCFCE_NEXT = 5,
	D_FCFCE_STOP = 6,
	D_FCFCE_SETPARS = 7,
} D_FCFCE;

//autosampler function pointers
typedef struct {
	unsigned int vial_from;		//the first vial available
	unsigned int vial_to;			//the last vial available
	unsigned int max_waitrdy;	//max. wait time in ms for AS ready
	int param;
} D_AS_INFO;
typedef enum {
	D_AS_STAT_READY = 0,
	D_AS_STAT_NOTREADY = 1,
	D_AS_STAT_ERROR = 2,
} D_AS_STAT;
#define D_AS_ERROR_NONE 0x00
#define D_AS_ERROR_BADPARAM 0x01
#define D_AS_ERROR_NOVIAL 0x02
#define D_AS_ERROR_INJECT 0x03
#define D_AS_ERROR_COMM 0x04
#define D_AS_ERROR_UNKNOWN 0x05
typedef struct {
	D_AS_STAT stat;
	unsigned int error;
} D_AS_STATUS;
//
typedef struct {
	DWORD (*pGetASInfo)(HDEVICE, D_AS_INFO *info);
	DWORD (*pGetASStatus)(HDEVICE hD, D_AS_STATUS *status);
	DWORD (*pMakeInjection)(HDEVICE hD, LPARAM vial);
	DWORD (*pStop)(HDEVICE hD);
} AS_FCE;
//
typedef enum {
	D_ASFCE_NONE = 0,
	D_ASFCE_INFO = 1,
	D_ASFCE_STATUS = 2,
	D_ASFCE_INJECT = 3,
	D_ASFCE_STOP = 4,
} D_ASFCE;

typedef struct {
	BOOL (*pSetSignalCount)(HDEVICE hD, DWORD count);
	BOOL (*pSetSignalInfo)(HDEVICE hD, DWORD sid, D_DEV_SIGNAL *pinfo);
	BOOL (*pSelectSignal)(HDEVICE hD, DWORD sid);
	BOOL (*pNotifySignalData)(HDEVICE hD, DWORD sid, DWORD count, double *dtime, double *value);	//should be fast as possible!
} SYS_FCE;


//...

typedef DWORD (*DEVFCE)(HDEVICE hD);
typedef DWORD (*DEVPARFCE)(HDEVICE hD, LPARAM param);
typedef DWORD (*DEVRFCE)(HDEVICE hD, WORD idx);
typedef DWORD (*DEVWFCE)(HDEVICE hD, WORD idx, D_VAL val);
typedef DWORD (*DEVOFCE)(HDEVICE hD, WORD idx);
typedef DWORD (*DEVOFCEPAR)(HDEVICE hD, WORD idx, LPARAM par);

typedef enum {
	DT_FCE_I = 0,
	DT_FCE_IPAR = 1,
	DT_FCE_U = 2,
	DT_FCE_E = 3,
	DT_FCE_R = 4,
	DT_FCE_W = 5,
	DT_FCE_P = 6,
	DT_FCE_O = 7,
} DTFCE;

//property structure
typedef struct {
	TCHAR label[D_MAX_PROPNAME_LEN];
	TCHAR suffix[D_MAX_PROPSUFFIX_LEN];
	struct {
		TCHAR q[D_MAX_PROPQUAN_LEN];
		TCHAR u[D_MAX_PROPUNIT_LEN];
	} unit;
	DWORD flags;			//flags: R, W, visible, enable/disable, store
	DTYPE type;				//int, unsigned, bool, double, memory, ..., hex, octal, ...
	DGUITYPE guitype;		//button, edit box, readonly EB, value EB, combobox
	TCHAR *guidata;
	DWORD guiflags;		//where to place
	DWORD group;				//group id
	//
	double time;
	D_VAL val;
	D_VAL min;
	D_VAL max;
	unsigned int prec;		//precision (low byte), format
	//
	D_DHQUEUE dhq;		//data queue
	D_VAL vbackup;
	DWORD lerr;				//last error
	DWORD users;			//user data
	//internal
	CHUI_CONVUNIT _uconv;
} D_DEV_PROP;

//oper. property structure
typedef struct {
	TCHAR label[D_MAX_PROPNAME_LEN];
	TCHAR suffix[D_MAX_PROPSUFFIX_LEN];
	DWORD flags;
} D_DEV_OPERPROP;

//status
typedef enum {
	D_STATUS_IDLE = 0,			//deinited
	D_STATUS_NOTREADY = 1,	//inited (not ready)
	D_STATUS_READY = 2,			//deinited
} D_STATUS;

//device structure (for detector, pump, ...)
typedef struct {
	//status
	D_STATUS status;
	//communication parameters
	COMH com;
	//model
	DWORD model;
	//device label
	TCHAR label[D_MAX_DEVNAME_LEN];
	//identification
	TCHAR id[D_MAX_ID_LEN];		//device indentificaton
	TCHAR cid[D_MAX_CID_LEN];		//communication identification
	//color
	COLORREF color;
	//timer delay
	DWORD tmindelay;		//min. delay
	DWORD tdelay;		//measure time interval	[ms]
	DWORD tdelay2;		//control time interval [ms]
	DWORD utimer1;
	DWORD utimer2;
	//comm. queue thread
	HANDLE hqueue;

	//device properties
	WORD n_p;
	D_DEV_PROP *p;
	//device operation properties
	WORD n_op;
	D_DEV_OPERPROP *op;

	//functions pointers
	U_FCE ufce;		//standard device functions
	DET_FCE detfce;		//detector functions
	//PUMP_FCE pumpfce;		//pump functions
	FC_FCE fcfce;		//fraction collector functions
	AS_FCE asfce;		//autosampler functions
	SYS_FCE sysfce;	//system-device functions only

	//signals
	WORD n_s;
	D_DEV_SIGNAL *s;
	int dis_meas_int;		//disable measure interval
	int stop_measure;
	int multi_measure;

	//data
	void *data;		//universal data pointer (memory would be freed during destroy)

	//reserved (intenal usage)
	HWND _hwnd;
	unsigned _tid;		//timer ID
	DWORD counter;
	DWORD _t0;
	BOOL _opened;
} D_DEVICE;

//
typedef struct {
	BOOL (*pGetDeviceInfo)(DEVICEINFO *);
} D_DEVICEITEM;

//
typedef struct {
	D_DEVICE *pD;
	TCHAR ttext[D_MAX_TEXT_LEN];
	HACCEL ha;
	HFONT hf;
	DWORD lparam;
} D_PROCSTOCK;

//filtering
typedef struct {
	double t0;
	unsigned prev_value;
	unsigned filt_value;
	int is_first;
} D_DTIMEFILT;




//-------------------------------

/* function declarations */
const TCHAR *D_GetAppName(void);
const TCHAR *D_GetAppVer(void);

//
BOOL D_InitGui(HWND hwnd);
//
const TCHAR *D_ModelText(DWORD);
const TCHAR *D_ModelAbbrText(DWORD model);
COLORREF D_ModelColor(DWORD);
BOOL D_IsDemoDevice(HDEVICE);
DWORD D_CountDevices(const DEVPOINTER *devs);
int D_ListCompareByName(const void *, const void *);
DWORD D_ListDevices(DEVINFO **, const TCHAR *);
DWORD D_ClearDeviceList(DEVINFO **);
DWORD D_ListDevicesInfo(DEVICEINFO **pinfo, const TCHAR *filter);
DWORD D_ClearDevicesInfoList(DEVICEINFO **pinfo);
DEVPARAMS D_SearchDeviceByName(const TCHAR *, DEVICEINFO *);
//
BOOL D_IsInternal(void);
void D_StoreInternal(BOOL state);
//
HDEVICE D_CreateDeviceByParams(DEVPARAMS params);
HDEVICE D_CreateDeviceByName(const TCHAR *devname);
BOOL D_SetDefaultLabel(HDEVICE hD);
BOOL D_SetDeviceLabel(HDEVICE hD, const TCHAR *label);
DWORD D_DefInitDevice(D_DEVICE *pD);
DWORD D_OpenDevice(HDEVICE hD);
DWORD D_CloseDevice(HDEVICE hD);
BOOL D_IsOpened(HDEVICE hD);
BOOL D_ClosingDevice(HDEVICE hD);
DWORD D_DestroyDevice(HDEVICE hD);
//
DWORD D_CreateSignals(HDEVICE hD);
DWORD D_RenameDevicesSignals(HDEVICES *hDs);
DWORD D_DestroySignals(HDEVICE hD);
BOOL D_ValidateSignals(HDEVICE hD);
unsigned D_CountEnabledSignals(HDEVICE hD, unsigned *total);
unsigned D_CountAllEnabledSignals(HDEVICES *pDs, unsigned *total);
DWORD D_ChooseOneChannelForDevice(HDEVICES *pDs, int param);
DWORD D_GetSignalID(WORD d_idx, WORD s_idx);
DWORD D_GetSignalIDByDevice(HDEVICES *pDs, HDEVICE hD, WORD s_idx);
//
DWORD D_CountDevicesByModel(HDEVICES *pDs, DWORD model);
DWORD D_TestDevice(HDEVICE);
//
BOOL D_GetCommIDString(HDEVICE hD, TCHAR *str, int len, int format);
BOOL D_MakeCommIDString(HDEVICE hD);
TCHAR *D_RetCommIDString(HDEVICE hD, int format);
//
BOOL D_TestCommErrors(HDEVICES *hDs, DWORD max_errs, DWORD *didx);
BOOL D_ClearCommErrors(HDEVICES *);
//
DWORD D_AddNewDevice(HDEVICE, HDEVICES *);
DWORD D_RemoveDevice(DWORD, HDEVICES *);
DWORD D_RemoveDeviceByHandle(HDEVICE, HDEVICES *);
DWORD D_RemoveAllDevices(HDEVICES *);
DWORD D_CopyDeviceProperty(D_DEV_PROP *psrc, D_DEV_PROP *pdest);
DWORD D_CopyDeviceValues(HDEVICE, HDEVICE);
DWORD D_CopyDevices(HDEVICES *, HDEVICES *);
BOOL D_ArePropertiesModified(D_DEV_PROP *p1, D_DEV_PROP *p2);
BOOL D_AreDevicesModified(HDEVICES *, HDEVICES *);
//
HLIST D_CreateComList(unsigned int max_port);
unsigned int D_SearchDevicePortsTh(HDEVICE hD, HLIST comlist, HLIST hlist, WORD *percent, unsigned int *tfound);
DWORD D_AutodetectDevices(HDEVICES *hDs, BOOL *run, WORD *percent, TCHAR **devname, unsigned *tfound, TCHAR *sel, TCHAR *filter);
DWORD D_SearchDevicePortTh(HDEVICE hD, HLIST hlist, BOOL *run, WORD *percent, unsigned int *tfound);
//
BYTE D_FindNextComport(HDEVICES *, HDEVICE *);
BOOL D_CompareCommSettings(COMH *pC1, COMH*pC2);
BOOL D_IncrementCommSettings(COMH *pC);
BOOL D_CorrectCommSettings(HDEVICES *hDs, HDEVICE *hD);
//
BOOL D_ValidateProps(HDEVICE hD, DWORD pidx);
BOOL D_ValidateDevices(HDEVICES *);
DWORD D_GetDeviceIndex(HDEVICE, HDEVICES *);
HDEVICE D_IsModelValid(DWORD idx, DWORD model_flags, HDEVICES *pDs);
//
DWORD D_StoreBackupValue(D_DEV_PROP *prop);
DWORD D_RestoreBackupValue(D_DEV_PROP *prop);
DWORD D_SetBackupValue(D_DEV_PROP *prop, D_VAL *val);
DWORD D_GetBackupValue(D_DEV_PROP *prop, D_VAL *val);
BOOL D_IsBackupValueModify(D_DEV_PROP *prop, D_VAL *val);
DWORD D_SetDevicesBackupValues(HDEVICES *s, HDEVICES *d);
DWORD D_GetDevicesBackupValues(HDEVICES *s, HDEVICES *d);
DWORD D_AreDevicesBackupValuesModified(HDEVICES *s, HDEVICES *d);
//
DWORD D_InitDevice(HDEVICE hD, HWND hwnd, int prepare, D_STAT *dstat);
DWORD D_PostInitDevice(HDEVICE hD, D_STAT *dstat);
//
BOOL D_MeasureDevice(HDEVICE hD, DWORD idx, HWND hwnd, UINT postmsg);
void CALLBACK D_MeasureTimerProc(UINT tid, UINT msg, DWORD hD, DWORD dw1, DWORD dw2);
DWORD D_RunMeasureTimer(HDEVICE hD);
DWORD D_StopMeasureTimer(HDEVICE hD);
//
DWORD D_PresetProp(HDEVICE hD, WORD pidx, D_VAL val);
//
DWORD D_PrepareDevice(HDEVICE hD, int mode, D_STAT *dstat);
DWORD D_SetDevicesBackupValues(HDEVICES *s, HDEVICES *d);
//
BOOL D_MakeFunction(HDEVICE hD, DEVFCE fce, HWND hwnd, UINT post_msg, UINT post_idx, const TCHAR *logname);
BOOL D_MakeFunctionParam(HDEVICE hD, DEVPARFCE fce, LPARAM param, HWND hwnd, UINT post_msg, UINT post_idx, const TCHAR *logname);
BOOL D_ReadFunction(HDEVICE hD, DEVRFCE rfce, WORD idx, HWND hwnd, UINT post_msg);
BOOL D_ReadFunctionPrior(HDEVICE hD, DEVRFCE rfce, WORD idx, HWND hwnd, UINT post_msg, int priority);
BOOL D_WriteFunction(HDEVICE hD, DEVWFCE wfce, WORD idx, D_VAL val, HWND hwnd, UINT post_msg);
BOOL D_WriteFunctionPrior(HDEVICE hD, DEVWFCE wfce, WORD idx, D_VAL val, HWND hwnd, UINT post_msg, int priority);
BOOL D_WriteFunctionTopPrior(HDEVICE hD, DEVWFCE wfce, WORD idx, D_VAL val, HWND hwnd, UINT post_msg, int priority);
BOOL D_WriteFunctionNoLog(HDEVICE hD, DEVWFCE wfce, WORD idx, D_VAL val, HWND hwnd, UINT post_msg);
BOOL D_MakeOperFunction(HDEVICE hD, DEVOFCE ofce, WORD idx, HWND hwnd, UINT post_msg);
BOOL D_MakeOperFunctionPrior(HDEVICE hD, DEVOFCE ofce, WORD idx, HWND hwnd, UINT post_msg, int priority);
void D_LogOperation(HDEVICE, DTFCE, DWORD, D_VAL *);
//
DWORD D_SetProperty(D_DEV_PROP *prop, const TCHAR *label,
										const TCHAR *quan, const TCHAR *unit,	DWORD flags, DTYPE type);
DWORD D_SetPropertySuffix(D_DEV_PROP *prop, const TCHAR *label, const TCHAR *suffix,
													const TCHAR *quan, const TCHAR *unit,	DWORD flags, DTYPE type);
DWORD D_SetOperProperty(D_DEV_OPERPROP *oprop, const TCHAR *label, DWORD flags);
DWORD D_SetOperPropertySuffix(D_DEV_OPERPROP *oprop, const TCHAR *label, const TCHAR *suffix, DWORD flags);
BOOL D_SetState(WORD, const TCHAR *, D_STAT *);
BOOL D_SetStatePerc(WORD perc, D_STAT *stat);
BOOL D_IsStateRun(D_STAT *stat);
BOOL D_ComparePropVal(D_DEV_PROP *, D_VAL *);
BOOL D_ValidatePropRange(D_DEV_PROP *prop);

/* interface functions */
//--- universal ---
BOOL D_GetDeviceInfoProc(DEVINFO infoproc, DEVICEINFO *dinfo);
BOOL D_GetDeviceInfo(HDEVICE, DEVICEINFO *);
const TCHAR *D_GetDeviceName(HDEVICE);
const TCHAR *D_GetDeviceAliasName(HDEVICE hD);
BOOL D_TestID(HDEVICE, TCHAR *, TCHAR *, TCHAR *, TCHAR *);
//
DWORD D_SetupReadAllParameters(HDEVICE hD, unsigned int *err_cnt, HSTRING hs);
DWORD D_SetupWriteAllParameters(HDEVICE hD, unsigned int *err_cnt, HSTRING hs);
//
DEVICEDLGPROC D_SetupCommDlgProc(HDEVICE hD);
DEVICEDLGPROC D_SetupParamDlgProc(HDEVICE hD);
DEVICEDLGPROC D_SetupInitDlgProc(HDEVICE hD);
DEVICEDLGPROC D_OnLineDlgProc(HDEVICE hD);
//
BOOL D_SetDefaultUnits(CHUI_UNITS *punits);
BOOL D_SetUnitConversion(HDEVICE hD, CHUI_UNITS *punits);
BOOL D_SetPropUnitConversion(D_DEV_PROP *prop, CHUI_UNITS *punits);
BOOL D_ConvertValueToDemandUnit(D_DEV_PROP *prop, double *dval);
BOOL D_ConvertValueToOriginUnit(D_DEV_PROP *prop, double *dval);
BOOL D_ConvertUnitDemandPrecision(D_DEV_PROP *prop, int *precision);
TCHAR *D_GetUnitLabel(D_DEV_PROP *prop);
//
BOOL D_MakePropLabel(D_DEV_PROP *prop, TCHAR *str, unsigned format);
BOOL D_MakeOperPropLabel(D_DEV_OPERPROP *oprop, TCHAR *str, unsigned format);
BOOL D_TestUserData(HWND, LONG);
BOOL D_RestDevDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam, D_DEVICE *pD);
BOOL D_RestSetupDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam, D_DEVICE *pD);

//--- useful ---
BOOL D_SendExErrorMsg(HDEVICE hD, DWORD err);
BOOL D_SendExStartMsg(HDEVICE hD, double t);
BOOL D_SendExStopMsg(HDEVICE hD, double t);
BOOL D_SendExPauseMsg(HDEVICE hD, double t);
BOOL D_SendExResumeMsg(HDEVICE hD, double t);
BOOL D_SendExMarkMsg(HDEVICE hD, double t, TCHAR *descr);
BOOL D_SendExZeroMsg(HDEVICE hD);
BOOL D_SendExFracCWMsg(HDEVICE hD);
BOOL D_SendDelayedStopMsg(HDEVICE hD, DWORD msdelay);
double D_GetExTime(LPARAM lparam);
void D_TestExTime(double *t);

//
WORD D_GetDevPropertiesByFlags(HDEVICE hD, DWORD flags);
WORD D_GetDevPropertyIndexByFlags(HDEVICE hD, DWORD flags, WORD offset);
const TCHAR *D_GetDevPropertyTextVar(HDEVICE hD, WORD pid, int type);

//
TCHAR *D_GetDevLabel(HDEVICE hD);
TCHAR *D_GetDevName(HDEVICE hD);
TCHAR *D_GetDevType(HDEVICE hD);
TCHAR *D_GetDevFirmware(HDEVICE hD);
TCHAR *D_GetDevSerialNumber(HDEVICE hD);

//
DWORD D_RunOperFunc(HDEVICES *hDs, WORD op_index);

//
void D_ShowFoundDeviceID(HWND hwnd, HDEVICE hD, int found);

//
DWORD D_CriticalStop(HDEVICES *hDs);
DWORD D_PauseDevices(HDEVICES *hDs);
DWORD D_ResumeDevices(HDEVICES *hDs);

//--- device dependent ---
//detector
BOOL D_IsAutozero(HDEVICES *);
DWORD D_RunAutozeros(HDEVICES *);
DWORD D_GetSignal(HDEVICE hD, int how, DWORD idx, double *abs, double *time, D_DHQUEUE *dhq);

//starter & marker
DWORD D_SendSMSignals(HDEVICE, HWND, UINT, UINT);
DWORD D_StopSMSignals(HDEVICE);
//pump

//FC
DWORD D_ProcFunctionFC(HDEVICE hD, D_FCFCE fcfce, unsigned int val);

//AS
DWORD D_ProcFunctionAS(HDEVICE hD, D_ASFCE asfce, unsigned int val);

//SPECIAL
BOOL D_IsSpectrum(HDEVICES *hDs);
DWORD D_ShowSpectrums(HDEVICES *hDs);
DWORD D_CheckLoadedDevice(HDEVICE hD, unsigned int dver);
BOOL D_IsLampControl(HDEVICES *hDs);
DWORD D_SetLamp(HDEVICES *hDs, BOOL lamp_state);
BOOL D_IsIPause(HDEVICES *hDs);



//------------------
BOOL D_CommTQueueProc(UINT msg, DPARAM dparam, MPARAM mparam, WPARAM wparam, LPARAM lparam, int run);
#define MQM_READVOLTAGE (MQM_USER+0)
#define MQM_MAKEFCE (MQM_USER+6)
#define MQM_MAKEPARFCE (MQM_USER+7)
#define MQM_READFCE (MQM_USER+8)
#define MQM_WRITEFCE (MQM_USER+9)
#define MQM_OPERFCE (MQM_USER+10)
#define MQM_OPERFCEPAR (MQM_USER+11)

//--- data queue ---
D_DHQUEUE D_CreateDataQueue(DWORD lparam);
DWORD D_DestroyDataQueue(D_DHQUEUE hQ);
DWORD D_AllocateDataQueueContent(DWORD size, D_DATAQUEUECONTENT *dqc);
DWORD D_DiscardDataQueueContent(D_DATAQUEUECONTENT *dqc);
BOOL D_PushDataQueueItem(D_DHQUEUE hQ, D_DATAQUEUECONTENT *dqc);
BOOL D_PopDataQueueItem(D_DHQUEUE hQ, D_DATAQUEUECONTENT *dqc);
BOOL D_GetDataQueueItem(D_DHQUEUE hQ, D_DATAQUEUECONTENT *dqc);
BOOL D_RemoveDataQueueItem(D_DHQUEUE hQ);
DWORD D_GetDataQueueContentTotalCount(D_DHQUEUE hQ);
BOOL D_ClearDataQueue(D_DHQUEUE hQ);

//---
DWORD D_LoadRawDevices(const char *src_charset, XML_TAGLIST *, HDEVICES *);
DWORD D_SaveRawDevices(const char *trg_charset, XML_TAGLIST *, HDEVICES *);

//---
BOOL D_PrepareOperCombobox(HWND hctl, D_DEVICE *pD, WORD def_idx);

BOOL D_PrepareCombobox(HWND hctl, D_DEV_PROP *prop);
//
BOOL D_GuiPrintShowMem(TCHAR *text, unsigned int maxlen, TCHAR *format, unsigned char *buf, unsigned int size);
BOOL D_GuiRTFPrintShowMem(HSTRING hs, TCHAR *format, unsigned char *buf, unsigned int size);
size_t D_GuiDataLen(const TCHAR *guidata);
const TCHAR *D_GetGuiDataValue(D_DEV_PROP *prop, D_VAL val);
const TCHAR *D_GetGuiData(D_DEV_PROP *prop);
//
BOOL D_PrepareValidNumSubclassing(HWND hctl, D_DEV_PROP *prop);
BOOL D_ModifyValidNumSubclassing(HWND hctl, D_DEV_PROP *prop, D_VAL *min, D_VAL *max, unsigned short *prec);
#define D_ModifyParamCtrlSB(hdlg, pD, pidx, min, max, prec) D_ModifyValidNumSubclassing(D_GETOLH_VAL(hdlg, (pidx)), &(pD)->p[pidx], min, max, prec);
BOOL D_PrepareRangeValidNumSubclassing(HWND hctl, int level, D_DEV_PROP *prop);
BOOL D_ModifyRangeValidNumSubclassing(HWND hctl, int level, D_DEV_PROP *prop, D_VAL *min, D_VAL *max);
BOOL D_ModifyRangeControls(HWND hdlg, D_DEVICE *pD, int pidx, D_VAL *min, D_VAL *max);

//BOOL D_PrepareStartMarkCB(HWND hctl, int mode);

//
typedef enum {
	D_CTRLSTYLE_PROP = 0,			//by property
	D_CTRLSTYLE_ST_EB = 1,		//static+editbox
	D_CTRLSTYLE_ST_CB = 2,		//static+combobox
	D_CTRLSTYLE_BT = 3,				//checkbox
	D_CTRLSTYLE_RNG = 4,			//range (uint16,uint16)
	D_CTRLSTYLE_ST_RS = 5,		//static+radio-switch
	D_CTRLSTYLE_ST_EBL = 7,		//static+editbox (left)
	D_CTRLSTYLE_PBT = 8,			//push button
	D_CTRLSTYLE_TAB = 11,			//table
	D_CTRLSTYLE_SMACT = 21,		//sm-action (2x CB)
} D_CTRLSTYLE;

#define D_GUICTRL_RC_MASK 	0xFF000000
#define D_GUICTRL_RC_1			0x00000000
#define D_GUICTRL_RC_2			0x02000000
#define D_GUICTRL_RC_3			0x03000000
#define D_GUICTRL_RC_4			0x04000000
#define D_GUICTRL_RC_5			0x05000000
#define D_GUICTRL_RC_N(x)		(((x) << 24) & D_GUICTRL_RC_MASK)

BOOL D_CreateParamGroup(HWND hwnd, TCHAR *title, int col, int row, int col_size, int row_size);
BOOL D_CreateParamCtrlParam(HWND hwnd, WORD pidx, D_DEVICE *pD, D_CTRLSTYLE style, TCHAR *title, int col, int row);
#define D_CreateParamCtrl(hwnd, pidx, pD, style, col, row) D_CreateParamCtrlParam(hwnd, pidx, pD, style, NULL, col, row)
BOOL D_DestroyParamCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD);
BOOL D_SetParamCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD);
BOOL D_GetParamCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD);
BOOL D_SetAllParamCtrls(HWND hwnd, D_DEVICE *pD);
BOOL D_GetAllParamCtrls(HWND hwnd, D_DEVICE *pD);
BOOL D_DestroyAllParamCtrls(HWND hwnd, D_DEVICE *pD);
//
BOOL D_CheckValidateParamRangeCtrl(HWND hwnd, int level, WORD pidx, D_DEVICE *pD);
//
BOOL D_CreateParamCheckBox(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int col, int row);
BOOL D_CreateParamCheckBoxBits(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id, int bt_per_row, int col, int row);
BOOL D_SetParamCheckBoxBits(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id);
BOOL D_GetParamCheckBoxBits(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id, unsigned int *uval);
//
BOOL D_CreateParamRadioBox(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int col, int row);
BOOL D_CreateParamRadioBoxes(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id, int bt_per_row, int col, int row);
BOOL D_SetParamRadioBoxes(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id);
BOOL D_GetParamRadioBoxes(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id, unsigned int *uval);
//
BOOL D_CreateParamTable(HWND hwnd, int id, TCHAR *title, TCHAR *format, int cols, int show_rows, int col, int row);
BOOL D_SetParamTable(HWND hwnd, WORD pidx, D_DEVICE *pD);
BOOL D_GetParamTable(HWND hwnd, WORD pidx, D_DEVICE *pD);
BOOL D_ModifyParamTableMinMax(HWND hwnd, WORD pidx, int cidx, double min_val, double max_val);
BOOL D_DestroyParamTable(HWND hwnd, WORD pidx, D_DEVICE *pD);
//
TCHAR *D_GetBoolString(TCHAR *prop_guidata, int on_button, BOOL state);
//
BOOL D_CreateOnLineGroup(HWND hwnd, TCHAR *title, int pos, int row_size);
BOOL D_CreateOnLineOperCB(HWND hwnd, D_DEVICE *pD, int pos);
BOOL D_CreateOnLineCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD, int pos);
BOOL D_DestroyOnLineCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD);
BOOL D_CreateOnLineReadEB(HWND hwnd, WORD pidx, TCHAR *title, int right, int pos);
BOOL D_DestroyOnLineReadEB(HWND hwnd, WORD pidx);
BOOL D_CreateOnLineRadioBT(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int pos);
BOOL D_CreateOnLineButtonPar(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int pos, int style);
#define D_CreateOnLineButton(hwnd, id, title, index, bt_per_row, pos) D_CreateOnLineButtonPar(hwnd, id, title, index, bt_per_row, pos, 0)
BOOL D_CreateOnLineCheckbox(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int pos);
BOOL D_SetOnLineControl(HWND hwnd, WORD pidx, D_DEVICE *pD, LPARAM lparam);
BOOL D_TestOnLineEBControl(HWND hctl, D_DEV_PROP *prop, D_VAL *val);
BOOL D_NotifyOnlineUDControl(HWND hwnd, WORD pidx, D_DEVICE *pD, int dir, double step);
//
BOOL D_GuiPrintTable(TCHAR *text, unsigned int maxlen, TCHAR *format, unsigned char *buf, unsigned int size, int rtf);


//filtering
//unsigned D_UFilterByUTime(unsigned time, unsigned value, D_UTIMEFILT *filt, unsigned filttime);
unsigned D_UFilterByDTime(double time, unsigned value, D_DTIMEFILT *filt, double filttime);

//network search structure

typedef enum {
	D_SEARCHSTATUS_OFF = 0,
	D_SEARCHSTATUS_ONLINE = 1,
	D_SEARCHSTATUS_USED = 2,
	D_SEARCHSTATUS_UNREACH = 3
} D_SEARCHSTATUS;

typedef struct {
	COM_MODE mode;
	int portno;											//interface (RS232 = COM)
	TCHAR ip[COM_MAX_IP_LEN];				//interface (ETH = IP)
	TCHAR uid[COM_MAX_USERID_LEN];		//interface (User ID)
	//
	//TCHAR cid[D_MAX_CID_LEN];
	TCHAR spec_id[D_MAX_SID_LEN];		//specification
	TCHAR id[D_MAX_ID_LEN];					//device identification (<name>\0<type>\0<fw>\0<serial number>\0\0)
	TCHAR name[D_MAX_ID_LEN];				//origin name
	unsigned int status;						//status
} D_DEV_SEARCH;

BOOL D_ListIProcDevSearchParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam);

//comparing
int D_GetSWIntNumber(HDEVICE hD);

//SP table
//trigger
#define D_CFLAG_TRIGNONE		0x000000
#define D_CFLAG_TRIGDOWN		0x000001
#define D_CFLAG_TRIGUP			0x000002
#define D_CFLAG_TRIGBOTH		0x000003
//actions
#define D_CFLAG_ACTNONE			0x000000
#define D_CFLAG_ACTMARK			0x000010
#define D_CFLAG_ACTSTART		0x000020
#define D_CFLAG_ACTSTOP			0x000030
#define D_CFLAG_ACTZERO			0x000040
#define D_CFLAG_ACTFCW			0x000050
#define D_CFLAG_ACTSPEC			0x000060
//input
#define D_CFLAG_INPNONE			0x000000
#define D_CFLAG_INP1				0x010000
#define D_CFLAG_INP2				0x020000
#define D_CFLAG_INP3				0x030000
#define D_CFLAG_INP4				0x040000
#define D_CFLAG_INP5				0x050000
#define D_CFLAG_INP6				0x060000
#define D_CFLAG_INP7				0x070000
#define D_CFLAG_INP8				0x080000
#define D_CFLAG_INP9				0x090000
#define D_CFLAG_INP10				0x0A0000
#define D_CFLAG_INP11				0x0B0000
#define D_CFLAG_INP12				0x0C0000
#define D_CFLAG_INP13				0x0D0000
#define D_CFLAG_INP14				0x0E0000
#define D_CFLAG_INP15				0x0F0000
//
#define D_CFLAG_TMASK				0x00000F
#define D_CFLAG_AMASK				0x0000F0
#define D_CFLAG_FMASK				0x00FF00
#define D_CFLAG_IMASK				0x0F0000
#define D_CFLAG_TGET(x) 		(((x) & D_CFLAG_TMASK) >> 0)
#define D_CFLAG_TSET(x) 		(((x) << 0) & D_CFLAG_TMASK)
#define D_CFLAG_AGET(x) 		(((x) & D_CFLAG_AMASK) >> 4)
#define D_CFLAG_ASET(x) 		(((x) << 4) & D_CFLAG_AMASK)
#define D_CFLAG_FGET(x) 		(((x) & D_CFLAG_FMASK) >> 8)
#define D_CFLAG_FSET(x) 		(((x) << 8) & D_CFLAG_FMASK)
#define D_CFLAG_IGET(x) 		(((x) & D_CFLAG_IMASK) >> 16)
#define D_CFLAG_ISET(x) 		(((x) << 16) & D_CFLAG_IMASK)
//
#define D_CFLAG_FMAX 				D_CFLAG_FSET(0xFFFF)
#define D_CFLAG_FDEF 				D_CFLAG_FSET(40)
#define D_CFLAG_TDEF				D_CFLAG_TRIGDOWN


HWND D_CreateSPTable(HWND hparent, int id, int inputs, const TCHAR **names, int col, int row);
BOOL D_WriteSPTable(HWND hga, D_DEV_PROP *prop, int inputs);
BOOL D_ReadSPTable(HWND hga, D_DEV_PROP *prop, int inputs);
//
HWND D_CreateSPTable2(HWND hparent, int id, unsigned int prop_rows, unsigned int inputs, const TCHAR **names, int col, int row);
BOOL D_DestroySPTable2(HWND hta);
BOOL D_WriteSPTable2(HWND hta, D_DEV_PROP *prop, unsigned int prop_rows, unsigned int inputs);
BOOL D_ReadSPTable2(HWND hta, D_DEV_PROP *prop, unsigned int prop_rows, unsigned int inputs);


typedef unsigned int D_SMVAL;
D_SMVAL D_TestInputSMEvents(unsigned int prev_val, unsigned int val, D_DEV_PROP *cprop, unsigned int inputs, D_SMVAL *smdir);
D_SMVAL D_TestInputSMEvents2(unsigned int prev_val, unsigned int val, D_DEV_PROP *cprop, unsigned int prop_rows, unsigned int inputs, D_SMVAL *smdir);
BOOL D_SendSMEvents(HDEVICE hD, D_SMVAL, double smtime);

//
DWORD D_GenerateCBIntData2(TCHAR *str, int count, const int *values, double mul);
DWORD D_GenerateCBDblData2(TCHAR *str, int count, const double *values, double mul);

//temporary dir
BOOL D_TempDirSet(const TCHAR *path);
TCHAR *D_TempDirGet();
TCHAR *D_TempDirAllocFilename(HDEVICE hD, const TCHAR *name);

/* error management */
typedef enum {
	D_SEVERITY_NONE = 0,				//nothing
	D_SEVERITY_LOGONLY = 1,			//log, no messagebox
	D_SEVERITY_WARNING = 2,			//log, warning messagebox
	D_SEVERITY_ERROR = 3,				//log, error messagebox
	D_SEVERITY_ERRORSTOP = 4,		//log, error messagebox + process stop function + stop/abort analysis or pause
} D_SEVERITY;

typedef struct {
	HDEVICE hD;
	SYSTEMTIME st;
	D_SEVERITY severity;
	int error;
	TCHAR *descr;
} D_DEVERROR;

DWORD D_EvokeError(HDEVICE hD, D_SEVERITY severity, int error, TCHAR *descr);

//
DWORD D_WritePropValue(HDEVICE hD, WORD pidx, D_VAL val);

//
DWORD D_SYSD_ListSignalsTo(HDEVICES *pDs);
BOOL D_SYSD_IsSignalSelected(HDEVICE hD, DWORD sid);
HDEVICE *D_SYSD_GenerateDevices(HDEVICES *pDs, DWORD sid);
BOOL D_SYSD_NotifyDataTo(HDEVICE *hDs, DWORD sid, DWORD count, double *dtime, double *value);

double D_fabs(double x);

#ifdef __cplusplus
}
#endif

#endif	/* end of _CHDEVICE_H_ */
