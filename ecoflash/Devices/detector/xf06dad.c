/*
 * xf06dad.c
 *
 * FLASH 06 DAD device - source file
 *
 * Author: Filip Kinovic
 * Version: 5.5
 * Date: 08.03.2017
*/

#define DEVICE_ALONE

#include <windows.h>
#include <math.h>
#include <richedit.h>
#include "xf06dad.h"
#include "../../useful.h"
#include "../../diag.h"
#ifndef DEVICE_ALONE
	#include "../../dlg.h"
	#include "../../res.h"
	#include "../../chsetup.h"
	#include "../../mymath.h"
#endif
#include "../../commfce.h"
#include "b2_protocol.h"

#ifdef DEBUG
	//#define F06DAD_SIMUL
#endif

#define F06DAD_USE_RS232_DEFAULT

//#define F06DAD_NOWLRANGELIMIT

/* variables */
extern HINSTANCE g_hInst;
#ifndef DEVICE_ALONE
	extern CH_SETUP g_chSetup;
#endif
extern double g_scaleX;
extern double g_scaleY;
#define GSX(arg) ((int)((double)(arg)*g_scaleX))		//scale by gscaleX
#define GSY(arg) ((int)((double)(arg)*g_scaleY))		//scale by gscaleY
#ifndef DEVICE_ALONE
	static DWORD g_ext = 0;		//txt
#endif

//#define XF06DAD_ENBAUDS
#define XF06DAD_ENABLE_C640
#ifdef DEBUG
	//#define XF06DAD_SIMULMODEL_CHAR 'H'
#endif


/* constants */
static const TCHAR *f06dadname = TEXT("FLASH06DAD");
static const TCHAR *f06dadalias = TEXT("FLASH06DAD");
static const TCHAR *f06dadver = TEXT("5.5");
static const TCHAR *f06dadspec = DC_("FLASH 06 DAD Detector");
static const TCHAR *f06daddesc = DC_(
	"FLASH06DAD/FLASH10DAD/FLASH12DAD is a UV diode array detector.\r\n"
	"\r\n"
	"These models are available:\r\n"
	"Model A\t 4ch\t 200--400 nm\t 38400bd\r\n"
	"Model B\t 1ch\t 200--400 nm\t 38400bd\r\n"
	"Model C\t 4ch\t 200--600 nm\t 38400bd\r\n"
	"Model D\t 2ch\t 200--400 nm\t 19200bd\r\n"
	"Model E\t 4ch\t 200--800 nm\t 38400bd\r\n"
	"Model F\t 4ch\t 200--800 nm\t 19200bd\r\n"
	"Model G\t 4ch\t 200--800 nm\t 19200bd\r\n"
	"Model H\t 4ch\t 200--800 nm\t 57600bd\r\n"
	"Model I\t 4ch\t 200--400 nm\t 57600bd\r\n"
	"\r\n"
	"This unit is used in liquid chromatography in flash and preparative applications. "
	"By using analytical cell can be detector used also for analytical purpose.\r\n"
	"\r\n"
	"The unit has several designes: build in unit with 24 V power supply and RS232 controlling; "
	"classical case with 90-264 V powering; case with keypad, display, analog outputs and 90-264 V powering; "
	"external cell conection by fibers with SMA connetors.");
static const TCHAR *f06dadprod = TEXT("ECOM s.r.o.");


//--- constants ---
#define _XF06DAD_WAITFORLAMP 15000		//max. 15s
#define _XF06DAD_MIN_TDELAY 25		//25 ms
#define _XF06DAD_DEF_TDELAY 100		//100 ms
#define _XF06DAD_WAITMEAS 1500		//1.5s

#define _XF06DAD_WLMIN 190
#define _XF06DAD_A_WLMAX 410		//A,B,D
#define _XF06DAD_C_WLMAX 630		//C
#define _XF06DAD_E_WLMAX 850		//E,F,G,H
#define _XF06DAD_WLDEFMIN 250
#define _XF06DAD_WLDEFMAX 350
#define _XF06DAD_HALFMIN 0
#define _XF06DAD_HALFMAX 15

#define _XF06DAD_STIMEA 400		//190--410, 38400bd, (13+[410-190+1]*6+14)*0.2604=352ms
#define _XF06DAD_STIMEB 400		//190--410, 38400bd, (13+[410-190+1]*6+14)*0.2604=352ms
#define _XF06DAD_STIMEC 750		//190--630, 38400bd, (13+[630-190+1]*6+14)*0.2604=696ms
#define _XF06DAD_STIMED 750		//190--410, 19200bd, (13+[410-190+1]*6+14)*0.5208=705ms
#define _XF06DAD_STIMEE 1200		//190--850, 38400bd, (13+[850-190+1]*6+14)*0.2604=1040ms (1080ms)
#define _XF06DAD_STIMEF 2400		//190--850, 19200bd, (13+[850-190+1]*6+14)*0.5208=2080ms (2160ms)
#define _XF06DAD_STIMEG 1600		//190--850, 19200bd, (13+[850-190+1]*4+14)*0.5208=1392ms (1400ms)
#define _XF06DAD_STIMEH 1200		//190--850, 57600bd, (13+[850-190+1]*6+14)*0.1736=694ms (800ms)
#define _XF06DAD_STIMEI 300		//190--410, 57600bd, (13+[410-190+1]*6+14)*0.1736=235ms

#define _XF06DAD_DTIMEA 		//0--255, 38400bd, (13+[256]*4+14)*0.2604=274ms
#define _XF06DAD_DTIMED 		//0--255, 19200bd, (13+[256]*4+14)*0.5208=548ms
#define _XF06DAD_DTIMEH 		//0--255, 57600bd, (13+[256]*4+14)*0.1736=183ms

#define _XF06DAD_IDXMIN 0x000
#define _XF06DAD_IDXMAX 0x0FF

#define _XF06DAD_TCMAX_V800 3
#define _XF06DAD_TCMAX_V855 6		//max. TC index for FW 8.55 and higher
#define _XF06DAD_TCMAX_V856 8		//max. TC index for FW 1.56 or 8.56 and higher
#define _XF06DAD_TCCOUNT (_XF06DAD_TCMAX_V856+1)

#define _XF06DAD_COMBUFF_MAX_SIZE 8192
#define _XF06DAD_SCAN_MAX_SIZE 1000		//wlmax-wlmin
#define _XF06DAD_SCAN_MIN_LENGTH 35

/* groups */
#define _XF06DAD_WLS_GROUP 1		//wavelengths
#define _XF06DAD_ABS_GROUP 2		//absorbances
#define _XF06DAD_INT_GROUP 3		//intensities
#define _XF06DAD_AOFS_GROUP 4		//analog offsets

/* strings */
#define _XF06DAD_STR_LAMPS_NCHANGE 	DC_("Don't change")
#define _XF06DAD_STR_LAMPS_OFF 			DC_("Off")
#define _XF06DAD_STR_LAMPS_ON 			DC_("On")
#define _XF06DAD_STR_ERRS						DC_("Decode errors")
#define _XF06DAD_STR_SCAN						DC_("Scan...")
#define _XF06DAD_STR_REPORT					DC_("Device report")

//constant labels
static const TCHAR *c_suffix[_XF06DAD_MAXCHANNELS+1] = {
	TEXT(" A"), TEXT(" B"), TEXT(" C"), TEXT(" D"), TEXT(" W")};

//TC/CB (Problem with combatibility of old versions!!!)
#define F06DAD_TC_0_5		0
#define F06DAD_TC_0_75 	1
#define F06DAD_TC_1 		2
#define F06DAD_TC_2 		3
#define F06DAD_TC_4 		4
#define F06DAD_TC_8 		5
#define F06DAD_TC_16 		6
#define F06DAD_TC_0_2 	7
#define F06DAD_TC_0_1 	8

#define F06DAD_TCCB_0_1 	0
#define F06DAD_TCCB_0_2 	1
#define F06DAD_TCCB_0_5		2
#define F06DAD_TCCB_0_75 	3
#define F06DAD_TCCB_1 		4
#define F06DAD_TCCB_2 		5
#define F06DAD_TCCB_4 		6
#define F06DAD_TCCB_8 		7
#define F06DAD_TCCB_16 		8

//const int c_cb2tc[_XF06DAD_TCCOUNT] = {8, 7, 0, 1, 2, 3, 4, 5, 6};
//const int c_tc2cb[_XF06DAD_TCCOUNT] = {2, 3, 4, 5, 6, 7, 8, 1, 0};

#ifdef F06DAD_SIMUL
	extern BOOL COM_log_enable;
	#define F06DAD_SendAndRcptStr F06DAD_SimulSendAndRcptStr
#else
	#define F06DAD_SendAndRcptStr COM_SendAndRcptStr2
	#define F06DAD_SendStr COM_SendStr
#endif


/* function definitions */

//*** function creates F06DAD device parameters
DWORD F06DAD_CreateDevParams(HDEVICE hD, void *param)
{
	D_DEVICE *pD;
	int i;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//set some comm. parameters
#ifdef F06DAD_USE_RS232_DEFAULT
	pD->com.mode = COM_MODE_RS232;
#else
	pD->com.mode = COM_MODE_ETHERNET;
#endif
	pD->com.com_flags.en_rs232 = 1;
	pD->com.com_flags.en_eth = 1;

	//
	pD->com.timeout = 300;
	pD->com.com_flags.en_timeout = 1;
	pD->com.terminal_char = RS232_TERMINAL_LF;
	pD->com.repeat = 0;
	pD->com.errs = 0;
	//RS232
	pD->com.ser_pars.baudrate = XF06DAD_MODELA_BAUD;
	pD->com.ser_pars.databits = RS232_DATABITS_8;
	pD->com.ser_pars.parity = RS232_PARITY_NONE;
	pD->com.ser_pars.stopbits = RS232_STOPBITS_1;
	pD->com.ser_pars.handshake = RS232_HANDSHAKE_NONE;
	pD->com.ser_pars.insize = XF06DAD_MAX_RCPTLEN;
	pD->com.ser_pars.outsize = XF06DAD_MAX_SENDLEN;
	//
	RS232_SetFlags(&pD->com.ser_pars, &pD->com.ser_flags);
#ifdef XF06DAD_ENBAUDS
	pD->com.ser_flags.en_baudrate |= RS232_M_BAUDRATE_19200 |
																	 RS232_M_BAUDRATE_38400 |
																	 RS232_M_BAUDRATE_57600 |
																	 RS232_M_BAUDRATE_115200;
#else
	pD->com.ser_flags.en_baudrate |= RS232_M_BAUDRATE_19200 |
																	 RS232_M_BAUDRATE_38400 |
																	 RS232_M_BAUDRATE_57600;
#endif

	//ETH
	__strcpy(pD->com.eth_pars.ip, XF06DAD_DEF_HOST);
	pD->com.eth_pars.port = XF06DAD_DEF_PORT;
	pD->com.eth_pars.cn_timeout = XF06DAD_CONNECT_TIMEOUT;
	//
	pD->com.eth_flags.en_port = 1;

	//model
	pD->model = D_MODEL_DETECTOR;		//detector
	//color
	pD->color = 0x000000FFL;		//red
	//delay
	pD->tmindelay = _XF06DAD_MIN_TDELAY;		//store min. tdelay
	pD->tdelay = _XF06DAD_DEF_TDELAY;		//measure time [ms]
	pD->tdelay2 = D_DEFAULT_CTRLTIME;		//control time [ms]

	//--- properties ---
	if (_XF06DAD_NP) {
		if ((pD->p = (D_DEV_PROP *)malloc(_XF06DAD_NP*sizeof(D_DEV_PROP))) == NULL)
			return(ERROR_OUTOFMEMORY);
		memset(pD->p, 0, _XF06DAD_NP*sizeof(D_DEV_PROP));
		//set properties
		D_SetProperty(&pD->p[_XF06DAD_MODEL], DC_("Model"), NULL, NULL, DFLAG_CONF, DT_UINT);
		pD->p[_XF06DAD_MODEL].val.u = XF06DAD_MODELE;		//type E (800nm)
		pD->p[_XF06DAD_MODEL].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_MODEL].guidata = TEXT("Flash06(10)DAD400 4ch (Model A)\0"
																				 "Flash06(10)DAD400 1ch (Model B)\0"
																				 "Flash06(10)DAD600 4ch (Model C)\0"
																				 "Flash06(10)DAD400 2ch (Model D)\0"
																				 "Flash06(10)DAD800 4ch (Model E)\0"
																				 "Flash06(10)DAD800 2ch (Model F)\0"
																				 "Flash06(10)DAD800 2ch (Model G)\0"
																				 "Flash12DAD800 4ch (Model H)\0"
																				 "Flash12DAD400 4ch (Model I)\0");
		D_SetProperty(&pD->p[_XF06DAD_LAMP], DC_("Lamp"), NULL, NULL, DFLAG_CTRL|DFLAG_RECV, DT_BOOL);
		pD->p[_XF06DAD_LAMP].guiflags = DGFLAG_EMPH;
    pD->p[_XF06DAD_LAMP].guitype = DGUIT_SWITCH;
		for (i=0; i<_XF06DAD_MAXCHANNELS; i++) {
			D_SetPropertySuffix(&pD->p[_XF06DAD_WLA+i], DC_("Wavelength"), c_suffix[i], DC_("length"), TEXT("nm"), DFLAG_CONF|DFLAG_CTRL|DFLAG_SEND|DFLAG_MINMAX, DT_DOUBLE);
			pD->p[_XF06DAD_WLA+i].val.d = 254.0;
			pD->p[_XF06DAD_WLA+i].min.d = _XF06DAD_WLMIN;
			pD->p[_XF06DAD_WLA+i].max.d = _XF06DAD_A_WLMAX;
			pD->p[_XF06DAD_WLA+i].group = _XF06DAD_WLS_GROUP;
			//pD->p[_XF06DAD_WLA+i].guiflags = DGFLAG_EMPH;
			if (i == 0)
				pD->p[_XF06DAD_WLA+i].flags |= DFLAG_RECV;
		}
		D_SetProperty(&pD->p[_XF06DAD_HALF], DC_("Half width"), NULL, NULL, DFLAG_CONF|DFLAG_SEND|DFLAG_RECV|DFLAG_READONLY, DT_UINT);
		pD->p[_XF06DAD_HALF].val.u = 4;
		D_SetProperty(&pD->p[_XF06DAD_FILTER], DC_("Time constant"), NULL, NULL, DFLAG_CONF|DFLAG_SEND|DFLAG_RECV|DFLAG_MAX, DT_UINT);
		pD->p[_XF06DAD_FILTER].val.u = 2;		//1.0s
		pD->p[_XF06DAD_FILTER].max.u = _XF06DAD_TCMAX_V856;
		pD->p[_XF06DAD_FILTER].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_FILTER].guidata = TEXT("0.5 s\0" "0.75 s\0" "1 s\0" "2 s\0" "4 s (v.55)\0" "8 s (v.55)\0" "16 s (v.55)\0" "0.2 s (v.56)\0" "0.1 s (v.56)\0");
		D_SetProperty(&pD->p[_XF06DAD_STAT], DC_("Status"), NULL, NULL, DFLAG_SAVE|DFLAG_RECV, DT_UINT);
		pD->p[_XF06DAD_STAT].guiflags = DGFLAG_EMPH;
    pD->p[_XF06DAD_STAT].guitype = DGUIT_READ;
		D_SetProperty(&pD->p[_XF06DAD_ERROR], DC_("Error"), NULL, NULL, DFLAG_SAVE, DT_UINT);
		pD->p[_XF06DAD_ERROR].guitype = DGUIT_HEX;
		D_SetProperty(&pD->p[_XF06DAD_LIFE], DC_("Life time"), DC_("time"), TEXT("h"), DFLAG_STAT|DFLAG_RECV|DFLAG_SETPREC(2)|DFLAG_NOCONVUNIT, DT_DOUBLE);
		D_SetProperty(&pD->p[_XF06DAD_CLAM], DC_("No. of lamps"), NULL, NULL, DFLAG_STAT, DT_UINT);
		for (i=0; i<_XF06DAD_MAXCHANNELS; i++) {
			D_SetABSPropertySuffix(&pD->p[_XF06DAD_ABSA+i], DC_("Absorbance"), c_suffix[i], DFLAG_MEASPROC|DFLAG_SETPREC(2), DT_DOUBLE);
			pD->p[_XF06DAD_ABSA+i].group = _XF06DAD_ABS_GROUP;
			if (i == 0)
				pD->p[_XF06DAD_ABSA+i].flags |= DFLAG_RECV|DFLAG_IGNORE;

		}
		for (i=0; i<_XF06DAD_MAXCHANNELS; i++) {
			D_SetPropertySuffix(&pD->p[_XF06DAD_INTA+i], DC_("Intensity"), c_suffix[i], DC_("relative"), TEXT("%"), DFLAG_SAVE|DFLAG_SETPREC(2), DT_DOUBLE);
			pD->p[_XF06DAD_INTA+i].group = _XF06DAD_INT_GROUP;
			if (i == 0)
				pD->p[_XF06DAD_ABSA+i].flags |= DFLAG_RECV|DFLAG_IGNORE;
		}
		D_SetABSPropertySuffix(&pD->p[_XF06DAD_ABSW], DC_("Absorbance"), c_suffix[_XF06DAD_MAXCHANNELS], DFLAG_MEASPROC|DFLAG_SETPREC(2)|DFLAG_NOSIGNAL, DT_DOUBLE);
		pD->p[_XF06DAD_ABSW].group = _XF06DAD_ABS_GROUP;
		//
		D_SetProperty(&pD->p[_XF06DAD_DFCE], DC_("D-function"), NULL, NULL, DFLAG_CONF|DFLAG_CTRL|DFLAG_SEND|DFLAG_RECV|DFLAG_IGNORE, DT_UINT);
		pD->p[_XF06DAD_DFCE].val.u = 0;		//none
		pD->p[_XF06DAD_DFCE].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_DFCE].guidata = TEXT("----\0" "A+B\0" "A" CSTR_MINUS "B\0" "(A+B)/2\0" "(A" CSTR_MINUS "B)/2\0" "max(A,B)\0" "A/B\0" CSTR_MINUS "A\0" "|A|\0");
		D_SetProperty(&pD->p[_XF06DAD_DTHR], DC_("D-threshold"), DC_("voltage"), TEXT("mV"), DFLAG_CONF|DFLAG_SEND|DFLAG_RECV|DFLAG_MINMAX|DFLAG_SETPREC(2)|DFLAG_IGNORE, DT_DOUBLE);
		pD->p[_XF06DAD_DTHR].val.d = _XF06DAD_DTHR_DEF;
		pD->p[_XF06DAD_DTHR].min.d = _XF06DAD_DTHR_MIN;
		pD->p[_XF06DAD_DTHR].max.d = _XF06DAD_DTHR_MAX;
		D_SetProperty(&pD->p[_XF06DAD_INJZERO], DC_("Autozero at injection"), NULL, NULL, DFLAG_CONF, DT_UINT);
		pD->p[_XF06DAD_INJZERO].val.u = 0;
    pD->p[_XF06DAD_INJZERO].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_INJZERO].guidata = DC_("Disabled") TEXT("\0") DC_("Enabled") TEXT("\0");
		D_SetProperty(&pD->p[_XF06DAD_WLSCANRANGE], DC_("Scan range"), DC_("length"), TEXT("nm"), DFLAG_CONF|DFLAG_SEND|DFLAG_RECV|DFLAG_MINMAX, DT_UINT);
		pD->p[_XF06DAD_WLSCANRANGE].val.u = (_XF06DAD_WLDEFMIN | (_XF06DAD_WLDEFMAX << 16));
		pD->p[_XF06DAD_WLSCANRANGE].min.u = (_XF06DAD_WLMIN | (_XF06DAD_WLMIN << 16));
		pD->p[_XF06DAD_WLSCANRANGE].max.u = (_XF06DAD_E_WLMAX | (_XF06DAD_E_WLMAX << 16));
		pD->p[_XF06DAD_WLSCANRANGE].guitype = DGUIT_RANGE;
		D_SetProperty(&pD->p[_XF06DAD_TEXTBY], DC_("Show text by"), NULL, NULL, DFLAG_CONF, DT_UINT);
		pD->p[_XF06DAD_TEXTBY].val.u = 2;		//by value
		pD->p[_XF06DAD_TEXTBY].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_TEXTBY].guidata = DC_("None") TEXT("\0") DC_("Wavelength") TEXT("\0") DC_("Value") TEXT("\0") DC_("Both") TEXT("\0");
		D_SetProperty(&pD->p[_XF06DAD_GRIDBY], DC_("Show grids by"), NULL, NULL, DFLAG_CONF, DT_UINT);
		pD->p[_XF06DAD_GRIDBY].val.u = 0;		//none
		pD->p[_XF06DAD_GRIDBY].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_GRIDBY].guidata = DC_("None") TEXT("\0") TEXT("X\0") TEXT("Y\0") DC_("Both") TEXT("\0");
		D_SetProperty(&pD->p[_XF06DAD_TEMP_WLRANGE], DC_("Available range"), DC_("length"), TEXT("nm"), DFLAG_SAVE, DT_UINT);
		pD->p[_XF06DAD_TEMP_WLRANGE].val.u = (_XF06DAD_WLMIN | (_XF06DAD_A_WLMAX << 16));
		pD->p[_XF06DAD_TEMP_WLRANGE].guitype = DGUIT_RANGE;
		//
		D_SetProperty(&pD->p[_XF06DAD_ALAMP], DC_("Lamp at power-on"), NULL, NULL, DFLAG_CONF|DFLAG_SEND|DFLAG_RECV|DFLAG_IGNORE, DT_UINT);
		pD->p[_XF06DAD_ALAMP].val.u = _XF06DAD_DEF_ALAMP;		//off
		pD->p[_XF06DAD_ALAMP].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_ALAMP].guidata = DC_("Off") TEXT("\0") DC_("On") TEXT("\0");
		D_SetProperty(&pD->p[_XF06DAD_ARATIO], DC_("Analog output ratio"), NULL, NULL, DFLAG_CONF|DFLAG_SEND|DFLAG_RECV|DFLAG_IGNORE, DT_UINT);
		pD->p[_XF06DAD_ARATIO].val.u = _XF06DAD_DEF_ARATIO;		//none
		pD->p[_XF06DAD_ARATIO].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_ARATIO].guidata = TEXT("0.05 AU/V\0" "0.1 AU/V\0" "0.25 AU/V\0" "0.5 AU/V\0" "1 AU/V\0" "2 AU/V\0" "3 AU/V\0" "5 AU/V\0");
		for (i=0; i<_XF06DAD_MAXCHANNELS; i++) {
			D_SetPropertySuffix(&pD->p[_XF06DAD_AOFSA+i], DC_("Analog out. offset"), c_suffix[i], DC_("voltage"), TEXT("mV"), DFLAG_CONF|DFLAG_SEND|DFLAG_SETPREC(_XF06DAD_PREC_AOFST)|DFLAG_IGNORE, DT_DOUBLE);
			pD->p[_XF06DAD_AOFSA+i].val.d = _XF06DAD_DEF_AOFST;
			pD->p[_XF06DAD_AOFSA+i].min.d = _XF06DAD_MIN_AOFST;
			pD->p[_XF06DAD_AOFSA+i].max.d = _XF06DAD_MAX_AOFST;
			pD->p[_XF06DAD_AOFSA+i].group = _XF06DAD_AOFS_GROUP;
			if (i == 0)
				pD->p[_XF06DAD_AOFSA+i].flags |= DFLAG_RECV;
		}
		D_SetProperty(&pD->p[_XF06DAD_BRIGHT], DC_("Brightness"), NULL, NULL, DFLAG_CONF|DFLAG_SEND|DFLAG_RECV|DFLAG_IGNORE, DT_UINT);
		pD->p[_XF06DAD_BRIGHT].val.u = _XF06DAD_DEF_BRIGHT;		//none
		pD->p[_XF06DAD_BRIGHT].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_BRIGHT].guidata = TEXT("1 (12.5%)\0" "2 (25%)\0" "3 (37.5%)\0" "4 (50%)\0" "5 (62.5%)\0" "6 (75%)\0" "7 (87.5%)\0" "8 (100%)\0");
		D_SetProperty(&pD->p[_XF06DAD_TEMP_DISPLAY], TEXT("Display"), NULL, NULL, DFLAG_SAVE, DT_UINT);
		//
		//D_SetProperty(&pD->p[_XF06DAD_TEMP_CONF], TEXT("t_config"), NULL, NULL, DFLAG_CONF, DT_UINT);
		//pD->p[_XF06DAD_TEMP_CONF].val.u = 0x0002;
		D_SetProperty(&pD->p[_XF06DAD_TEMP_EABS], TEXT("t_eabs"), NULL, NULL, 0, DT_UINT);
		D_SetProperty(&pD->p[_XF06DAD_TEMP_EINT], TEXT("t_eint"), NULL, NULL, 0, DT_UINT);
		D_SetProperty(&pD->p[_XF06DAD_TEMP_EABSW], TEXT("t_eabsw"), NULL, NULL, 0, DT_UINT);
		D_SetProperty(&pD->p[_XF06DAD_TEMP_COMBUFF], TEXT("t_buffer"), NULL, NULL, DFLAG_MEM, DT_MEMORY);
		pD->p[_XF06DAD_TEMP_COMBUFF].val.m.size = _XF06DAD_COMBUFF_MAX_SIZE;		//size for buffer
		pD->p[_XF06DAD_TEMP_COMBUFF].val.m.buf = malloc(pD->p[_XF06DAD_TEMP_COMBUFF].val.m.size);		//allocation
		D_SetProperty(&pD->p[_XF06DAD_TEMP_SCANDATA], TEXT("t_scandata"), NULL, NULL, DFLAG_MEM, DT_MEMORY);
		pD->p[_XF06DAD_TEMP_SCANDATA].val.m.size = 1+_XF06DAD_SCAN_MAX_SIZE*(2*sizeof(double)+2);		//size for buffer
		pD->p[_XF06DAD_TEMP_SCANDATA].val.m.buf = malloc(pD->p[_XF06DAD_TEMP_SCANDATA].val.m.size);		//allocation
		D_SetProperty(&pD->p[_XF06DAD_TEMP_SCANSIZE], TEXT("t_scansize"), NULL, NULL, 0, DT_UINT);
		D_SetProperty(&pD->p[_XF06DAD_TEMP_FIRSTSIG], TEXT("t_firstsignal"), NULL, NULL, 0, DT_UINT);
		D_SetProperty(&pD->p[_XF06DAD_TEMP_CRR], TEXT("t_report"), NULL, NULL, DFLAG_MEM, DT_HANDLE);
		D_SetProperty(&pD->p[_XF06DAD_TEMP_MODEL], DC_("Type model"), NULL, NULL, DFLAG_SAVE, DT_UINT);
		pD->p[_XF06DAD_TEMP_MODEL].val.u = XF06DAD_TMODEL_F06;
		pD->p[_XF06DAD_TEMP_MODEL].guitype = DGUIT_COMBOBOX;
		pD->p[_XF06DAD_TEMP_MODEL].guidata = TEXT("Flash06DAD\0"
																							"Flash10DAD\0"
																							"C640\0"
																							"Flash12DAD\0");
		D_SetProperty(&pD->p[_XF06DAD_INIT_LAMP], TEXT("i_lamp"), NULL, NULL, DFLAG_CONF, DT_BOOL);
		pD->p[_XF06DAD_INIT_LAMP].val.b = TRUE;
		pD->p[_XF06DAD_INIT_LAMP].guitype = DGUIT_BUTTON;
		D_SetProperty(&pD->p[_XF06DAD_INIT_AZERO], TEXT("i_azero"), NULL, NULL, DFLAG_CONF, DT_BOOL);
		pD->p[_XF06DAD_INIT_AZERO].val.b = FALSE;
		pD->p[_XF06DAD_INIT_AZERO].guitype = DGUIT_BUTTON;
		D_SetProperty(&pD->p[_XF06DAD_INIT_AZERODELAY], TEXT("i_azdelay"), NULL, NULL, DFLAG_CONF, DT_UINT);
		pD->p[_XF06DAD_INIT_AZERODELAY].val.u = 0;
		D_SetProperty(&pD->p[_XF06DAD_DEIN_LAMP], TEXT("d_lamp"), NULL, NULL, DFLAG_CONF, DT_BOOL);
		pD->p[_XF06DAD_DEIN_LAMP].val.b = FALSE;
		pD->p[_XF06DAD_DEIN_LAMP].guitype = DGUIT_BUTTON;
		D_SetProperty(&pD->p[_XF06DAD_TEMP_SPARS], TEXT("t_spars"), NULL, NULL, DFLAG_MEM, DT_MEMORY);
		pD->p[_XF06DAD_TEMP_SPARS].val.m.size = sizeof(F06DAD_SPARS);
		pD->p[_XF06DAD_TEMP_SPARS].val.m.buf = malloc(pD->p[_XF06DAD_TEMP_SPARS].val.m.size);		//allocation
		memset(pD->p[_XF06DAD_TEMP_SPARS].val.m.buf, 0, pD->p[_XF06DAD_TEMP_SPARS].val.m.size);
		D_SetProperty(&pD->p[_XF06DAD_TEMP_ONLINE], TEXT("t_online"), NULL, NULL, 0, DT_HANDLE);
		pD->p[_XF06DAD_TEMP_ONLINE].val.h = NULL;
	}
	pD->n_p = _XF06DAD_NP;

	//--- oper. properties ---
	if (_XF06DAD_NOP) {
		if ((pD->op = (D_DEV_OPERPROP *)malloc(_XF06DAD_NOP*sizeof(D_DEV_OPERPROP))) == NULL)
			return(ERROR_OUTOFMEMORY);
		memset(pD->op, 0, _XF06DAD_NOP*sizeof(D_DEV_OPERPROP));
		//set oper. properties
		D_SetOperProperty(&pD->op[_XF06DAD_AUTOZERO], DC_("Autozero"), 0);
		D_SetOperProperty(&pD->op[_XF06DAD_BEEP], DC_("Make a beep"), 0);
	}
	pD->n_op = _XF06DAD_NOP;

	//--- pointer proc. function ---
	//universal functions
	pD->ufce.pGetDeviceInfo = F06DAD_GetDeviceInfo;		//done
	pD->ufce.pSearch = F06DAD_Search;
	pD->ufce.pOpen = F06DAD_OpenDevice;
	pD->ufce.pClose = F06DAD_CloseDevice;
	pD->ufce.pTestID = F06DAD_TestID;		//done
	pD->ufce.pValidate = F06DAD_Validate;
	pD->ufce.pGetStatus = NULL;
	pD->ufce.pSetOper = F06DAD_MakeOper;
	pD->ufce.pInit = F06DAD_InitDetector;
	pD->ufce.pPostInit = F06DAD_PostInitDetector;
	pD->ufce.pReadProp = F06DAD_ReadProp;
	pD->ufce.pWriteProp = F06DAD_WriteProp;
	pD->ufce.pPreSetProp = F06DAD_PreSetProp;
	pD->ufce.pSpecial = F06DAD_Special;
	pD->ufce.pSetupParamDlgProc = F06DAD_SetupParamDlgProc;
	pD->ufce.pSetupInitDlgProc = F06DAD_SetupInitDlgProc;
	pD->ufce.pOnLineDlgProc = F06DAD_OnLineDlgProc;
	//detector functions
	pD->detfce.pAutozero = F06DAD_RunAutozero;
	pD->detfce.pGetSignal = F06DAD_GetAbs;		//done

	//!!!!!!!!!!!!! (see ALMEMO)
	pD->multi_measure = 1;

	return(NO_ERROR);
}

//--------------------- Universal Interface Functions -------------------------
//*** function gets device info (text pointers must not be freed)
BOOL F06DAD_GetDeviceInfo(HDEVICE hD, DEVICEINFO *dinfo)
{
	if (dinfo) {
		dinfo->name = f06dadname;
		dinfo->alias = f06dadalias;
		dinfo->ver = f06dadver;
		dinfo->spec = f06dadspec;
		dinfo->desc = f06daddesc;
		dinfo->prod = f06dadprod;
		dinfo->model = D_MODEL_DETECTOR;
		dinfo->flags = D_FLAG_AUTOZERO;		//autozero
#ifdef JAISCAN
		dinfo->filter = D_DEVFILTER_JAISCAN;
#else
		dinfo->filter = D_DEVFILTER_ECOM_CHROM;
#endif

		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------
//*** function opens device
DWORD F06DAD_OpenDevice(HDEVICE hD)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//logging
	pD->com.log_format = COM_LOGFORMAT_TEXT;

	//buffers
	pD->com.alloc = TRUE;
	pD->com.sendbufsize = XF06DAD_MAX_SENDLEN;
	pD->com.rcptbufsize = XF06DAD_MAX_RCPTLEN;

	//parameters
	pD->com.hProt = NULL;
	pD->com.SimulProc = NULL;

#ifdef F06DAD_SIMUL
	ret = COM_CreateBuffers(&pD->com);
#else
	ret = COM_OpenAndConf(&pD->com);
#endif

	return(ret);
}

//*** function closes device
DWORD F06DAD_CloseDevice(HDEVICE hD)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

#ifdef F06DAD_SIMUL
	COM_DestroyBuffers(&pD->com);
#else
	COM_Close(&pD->com);
#endif

	return(NO_ERROR);
}

//---------------------------------------------------------------
//*** function processes test function
DWORD F06DAD_Search(HDEVICE hD, HLIST hlist)
{
#ifdef F06DAD_SIMUL
	D_DEV_SEARCH dsearch;

	//clear structure
	memset(&dsearch, 0, sizeof(D_DEV_SEARCH));
	//set ETH mode
	dsearch.mode = COM_MODE_ETHERNET;
	//IP
	lstrcpy(dsearch.ip, _XF06DAD_DEF_HOST);
	//SPEC (MAC)
	lstrcpy(dsearch.spec_id, TEXT("SIMUL"));
	//ID
	if (hD) {
		F06DAD_TestID(hD, NULL, NULL, NULL, NULL);

		memcpy(dsearch.id, ((D_DEVICE *)hD)->id, sizeof(dsearch.id));
	}
	//add to list
	LIST_AppendItem(hlist, (LPARAM)&dsearch);

	return(LIST_GetSize(hlist));
#else

	return(B2PROT_SearchERVIN7(hD, hlist, NULL, 0));
#endif
}

//*** function tests and returns device ID
BOOL F06DAD_TestID(HDEVICE hD, TCHAR *name, TCHAR *type, TCHAR *sw, TCHAR *sn)
{
	D_DEVICE *pD;
	TCHAR *pid, *ppid;
	char *ptext;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//--- read ID ---
	//manage command

	//find device with baudrate
	while (1) {
		//--- 38400 ---
		pD->com.ser_pars.baudrate = 38400;
		F06DAD_CloseDevice(hD);
		ret = F06DAD_OpenDevice(hD);
		if (ret != NO_ERROR)
			break;
		F06DAD_SubscribeMode(hD, _XF06DAD_SUBSMODE_SABS, FALSE);	//stop scan subscription
		strcpy(pD->com.sendbuf, "#DTr\n");
		ret = F06DAD_SendAndRcptStr(&pD->com);
		if (ret == NO_ERROR && strncmp(pD->com.rcptbuf, "DTr", 3) == 0)
			break;

		//--- 19200 ---
		pD->com.ser_pars.baudrate = 19200;
		F06DAD_CloseDevice(hD);
		ret = F06DAD_OpenDevice(hD);
		if (ret != NO_ERROR)
			break;
		F06DAD_SubscribeMode(hD, _XF06DAD_SUBSMODE_SABS, FALSE);	//stop scan subscription
		strcpy(pD->com.sendbuf, "#DTr\n");
		ret = F06DAD_SendAndRcptStr(&pD->com);
		if (ret == NO_ERROR && strncmp(pD->com.rcptbuf, "DTr", 3) == 0)
			break;

		//--- 57600 ---
		pD->com.ser_pars.baudrate = 57600;
		F06DAD_CloseDevice(hD);
		ret = F06DAD_OpenDevice(hD);
		if (ret != NO_ERROR)
			break;
		F06DAD_SubscribeMode(hD, _XF06DAD_SUBSMODE_SABS, FALSE);	//stop scan subscription
		strcpy(pD->com.sendbuf, "#DTr\n");
		ret = F06DAD_SendAndRcptStr(&pD->com);
		if (ret == NO_ERROR && strncmp(pD->com.rcptbuf, "DTr", 3) == 0)
			break;
#ifdef XF06DAD_ENBAUDS
		//--- 115200 ---
		pD->com.ser_pars.baudrate = 115200;
		F06DAD_CloseDevice(hD);
		ret = F06DAD_OpenDevice(hD);
		if (ret != NO_ERROR)
			break;
		F06DAD_SubscribeMode(hD, _XF06DAD_SUBSMODE_SABS, FALSE);	//stop scan subscription
		strcpy(pD->com.sendbuf, "#DTr\n");
		ret = F06DAD_SendAndRcptStr(&pD->com);
		if (ret == NO_ERROR && strncmp(pD->com.rcptbuf, "DTr", 3) == 0)
			break;
#endif
		break;
	}

	//send and read
	if (ret == NO_ERROR) {
//		F06DAD_SendAndRcptStr(&pD->com);		//once more

		//decode response
		if (strcmp(pD->com.rcptbuf, "DTrFLASH06DAD") == 0) {
			pD->p[_XF06DAD_TEMP_MODEL].val.u = XF06DAD_TMODEL_F06;
			ret = 1;
		}
		else if (strcmp(pD->com.rcptbuf, "DTrFLASH10DAD") == 0) {
			pD->p[_XF06DAD_TEMP_MODEL].val.u = XF06DAD_TMODEL_F10;
			ret = 1;
		}
		else if (strcmp(pD->com.rcptbuf, "DTrC-640") == 0) {
			pD->p[_XF06DAD_TEMP_MODEL].val.u = XF06DAD_TMODEL_C640;
			ret = 1;
		}

		if (ret) {
			//response OK
			pid = pD->id;

			//decode name
			w2strcpy(pid, pD->com.rcptbuf+3);

			//test for display models
			ret = 0;
			strcpy(pD->com.sendbuf, "#XNr\n");
			if (F06DAD_SendAndRcptStr(&pD->com) == NO_ERROR) {
				pD->p[_XF06DAD_TEMP_DISPLAY].val.u = 1;
				if (strcmp(pD->com.rcptbuf, "XNrFLASH10DAD") == 0) {
					pD->p[_XF06DAD_TEMP_MODEL].val.u = XF06DAD_TMODEL_F10;
					ret = 1;
				}
				else if (strcmp(pD->com.rcptbuf, "XNrC-640") == 0) {
					pD->p[_XF06DAD_TEMP_MODEL].val.u = XF06DAD_TMODEL_C640;
					ret = 1;
				}
			}
			if (ret) {
				w2strcpy(pid, pD->com.rcptbuf+3);	//display name
			}

			//get model
			ptext = NULL;
			strcpy(pD->com.sendbuf, "#MDr\n");
			if (F06DAD_SendAndRcptStr(&pD->com) == NO_ERROR &&
					strncmp(pD->com.rcptbuf, "MDr", 3) == 0 &&
					strncmp(pD->com.rcptbuf+3, "ERR", 3) != 0) {
				ptext = pD->com.rcptbuf+3;
#ifdef XF06DAD_SIMULMODEL_CHAR
					*ptext = XF06DAD_SIMULMODEL_CHAR;
#endif

				//correct t-model
				if (F06DAD_IsModelSubScan(F06DAD_GetModelByChar(*ptext))) {
					pD->p[_XF06DAD_TEMP_MODEL].val.u = XF06DAD_TMODEL_F12;
					__strcpy(pid, TEXT("FLASH12DAD"));	//F12 name
				}
				else if (pD->p[_XF06DAD_TEMP_MODEL].val.u == XF06DAD_TMODEL_F10) {
					__strcpy(pid, TEXT("FLASH10DAD"));	//F10 name
				}

				//append model
				ppid = pid + lstrlen(pid);
				*(ppid++) = '-';
				w2strcpy(ppid, ptext);
			}

			if (name)
				lstrcpy(name, pid);		//get name
			else {
				//set default pump type by response
				if (pD->n_p && pD->p) {
					if (ptext != NULL)
						pD->p[_XF06DAD_MODEL].val.u = F06DAD_GetModelByChar(*ptext);
					else
						pD->p[_XF06DAD_MODEL].val.u = XF06DAD_MODELA;
				}
			}
			pid += lstrlen(pid)+1;

/*
			//detector FW when display
			if (pD->p[_XF06DAD_TEMP_MODEL].val.u == XF06DAD_TMODEL_F10) {
				//type (HW)
				strcpy(pD->com.sendbuf, "#XDr\n");
				//send and read
				if (F06DAD_SendAndRcptStr(&pD->com) == NO_ERROR &&
						strncmp(pD->com.rcptbuf, "XDr", 3) == 0 &&
						strncmp(pD->com.rcptbuf+3, "ERR", 3) != 0) {

				}
			}
*/
			//type (HW)
			strcpy(pD->com.sendbuf, "#HWr\n");
			//send and read
			if (F06DAD_SendAndRcptStr(&pD->com) == NO_ERROR &&
					strncmp(pD->com.rcptbuf, "HWr", 3) == 0 &&
					strncmp(pD->com.rcptbuf+3, "ERR", 3) != 0) {
				w2strcpy(pid, pD->com.rcptbuf+3);
			}
			else
				lstrcpy(pid, D_STRQUESTION);
			if (type)
				lstrcpy(type, pid);
			pid += lstrlen(pid)+1;

			//software version
			strcpy(pD->com.sendbuf, "#SWr\n");
			//send and read
			if (F06DAD_SendAndRcptStr(&pD->com) == NO_ERROR &&
					strncmp(pD->com.rcptbuf, "SWr", 3) == 0 &&
					strncmp(pD->com.rcptbuf+3, "ERR", 3) != 0) {
				w2strcpy(pid, pD->com.rcptbuf+3);
			}
			else
				lstrcpy(pid, D_STRQUESTION);
			if (sw)
				lstrcpy(sw, pid);
			pid += lstrlen(pid)+1;

			//decode serial number
			strcpy(pD->com.sendbuf, "#SNr\n");
			//send and read
			if (F06DAD_SendAndRcptStr(&pD->com) == NO_ERROR &&
					strncmp(pD->com.rcptbuf, "SNr", 3) == 0 &&
					strncmp(pD->com.rcptbuf+3, "ERR", 3) != 0) {
				w2strcpy(pid, pD->com.rcptbuf+3);
			}
			else
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

//*** function validates device prop. values
BOOL F06DAD_Validate(HDEVICE hD, DWORD pidx)
{
	D_DEVICE *pD;
	BOOL valid = TRUE;
	int channels, model, p;
	double min, max;

	if (hD) {
		pD = (D_DEVICE *)hD;

		if (pD->n_p && pD->p) {

			if (pidx == (DWORD)-1) {
				//model
				model = pD->p[_XF06DAD_MODEL].val.u;

				//abs channels + wavelengths
				channels = F06DAD_GetChannelsByModel(model);
				if (channels == 1) {
					pD->p[_XF06DAD_ABSB].flags &= ~DFLAG_MEAS;
					pD->p[_XF06DAD_ABSC].flags &= ~DFLAG_MEAS;
					pD->p[_XF06DAD_ABSD].flags &= ~DFLAG_MEAS;
					//
					pD->p[_XF06DAD_WLB].flags &= ~(DFLAG_CTRL|DFLAG_SEND);
					pD->p[_XF06DAD_WLC].flags &= ~(DFLAG_CTRL|DFLAG_SEND);
					pD->p[_XF06DAD_WLD].flags &= ~(DFLAG_CTRL|DFLAG_SEND);
				}
				else if (channels == 2) {
					pD->p[_XF06DAD_ABSB].flags |= DFLAG_MEAS;
					pD->p[_XF06DAD_ABSC].flags &= ~DFLAG_MEAS;
					pD->p[_XF06DAD_ABSD].flags &= ~DFLAG_MEAS;
					//
					pD->p[_XF06DAD_WLB].flags |= (DFLAG_CTRL|DFLAG_SEND);
					pD->p[_XF06DAD_WLC].flags &= ~(DFLAG_CTRL|DFLAG_SEND);
					pD->p[_XF06DAD_WLD].flags &= ~(DFLAG_CTRL|DFLAG_SEND);
				}
				else {
					pD->p[_XF06DAD_ABSB].flags |= DFLAG_MEAS;
					pD->p[_XF06DAD_ABSC].flags |= DFLAG_MEAS;
					pD->p[_XF06DAD_ABSD].flags |= DFLAG_MEAS;
					//
					pD->p[_XF06DAD_WLB].flags |= (DFLAG_CTRL|DFLAG_SEND);
					pD->p[_XF06DAD_WLC].flags |= (DFLAG_CTRL|DFLAG_SEND);
					pD->p[_XF06DAD_WLD].flags |= (DFLAG_CTRL|DFLAG_SEND);
				}
				//w-channels
				if (F06DAD_GetWChannelsByModel(model)) {
					pD->p[_XF06DAD_ABSW].flags |= DFLAG_MEAS;
				}
				else {
					pD->p[_XF06DAD_ABSW].flags &= ~DFLAG_MEAS;
				}

				//baudrate
				pD->com.ser_pars.baudrate = F06DAD_GetBaudrateByModel(model);

				//wavelength range
				min = _XF06DAD_WLMIN;
				max = F06DAD_GetWavelengthByModel(model);
				for (p=_XF06DAD_WLA; p<(_XF06DAD_WLA+_XF06DAD_MAXCHANNELS); p++) {
					pD->p[p].min.d = min;
					pD->p[p].max.d = max;
				}

			}
		}
	}
	return(valid);
}

//*** function gets device status (initiate, ready, comm. error, error, ...)
DWORD F06DAD_GetStatus(HDEVICE hD, DWORD *status)
{

	return(NO_ERROR);
}

//----------------------- Device Interface Functions -------------------------
//*** function reads property (to memory) by index
DWORD F06DAD_ReadProp(HDEVICE hD, WORD pidx)
{
	D_DEVICE *pD;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->n_p && pD->p) {
			switch (pidx) {
				case _XF06DAD_LAMP: return(F06DAD_GetLamp(hD, 1, 0, NULL, NULL));
				case _XF06DAD_WLA: return(F06DAD_GetWavelength(hD, 1, 0, NULL, NULL));
				case _XF06DAD_WLB: return(F06DAD_GetWavelength(hD, 1, 1, NULL, NULL));
				case _XF06DAD_WLC: return(F06DAD_GetWavelength(hD, 1, 2, NULL, NULL));
				case _XF06DAD_WLD: return(F06DAD_GetWavelength(hD, 1, 3, NULL, NULL));
				case _XF06DAD_HALF: return(F06DAD_GetHalfWidth(hD, 1, NULL, NULL));
				case _XF06DAD_FILTER: return(F06DAD_GetFilter(hD, 1, NULL, NULL));
				case D_PIDX_HOLDCOM:
				case _XF06DAD_STAT: return(F06DAD_GetState(hD, 1, NULL, NULL));
				case _XF06DAD_LIFE: return(F06DAD_GetLampLife(hD, 1, 0, NULL, NULL));
				case _XF06DAD_ABSA: return(F06DAD_GetAbs(hD, 1, 0, NULL, NULL, NULL));
				case _XF06DAD_ABSB: return(F06DAD_GetAbs(hD, 1, 1, NULL, NULL, NULL));
				case _XF06DAD_ABSC: return(F06DAD_GetAbs(hD, 1, 2, NULL, NULL, NULL));
				case _XF06DAD_ABSD: return(F06DAD_GetAbs(hD, 1, 3, NULL, NULL, NULL));
				case _XF06DAD_INTA: return(F06DAD_GetIntensity(hD, 1, 0, NULL, NULL));
				case _XF06DAD_INTB: return(F06DAD_GetIntensity(hD, 1, 1, NULL, NULL));
				case _XF06DAD_INTC: return(F06DAD_GetIntensity(hD, 1, 2, NULL, NULL));
				case _XF06DAD_INTD: return(F06DAD_GetIntensity(hD, 1, 3, NULL, NULL));
				case _XF06DAD_ABSW: return(F06DAD_GetAbs(hD, 1, 4, NULL, NULL, NULL));
				case _XF06DAD_DFCE: return(F06DAD_GetDFunction(hD, NULL, NULL));
				case _XF06DAD_DTHR: return(F06DAD_GetDThreshold(hD, NULL, NULL));
				case _XF06DAD_ALAMP: return(F06DAD_GetAutoLamp(hD, NULL, NULL));
				case _XF06DAD_ARATIO: return(F06DAD_GetAnalogRatio(hD, NULL, NULL));
				case _XF06DAD_AOFSA: return(F06DAD_GetAnalogOffset(hD, 1, 0, NULL, NULL));
				case _XF06DAD_AOFSB: return(F06DAD_GetAnalogOffset(hD, 1, 1, NULL, NULL));
				case _XF06DAD_AOFSC: return(F06DAD_GetAnalogOffset(hD, 1, 2, NULL, NULL));
				case _XF06DAD_AOFSD: return(F06DAD_GetAnalogOffset(hD, 1, 3, NULL, NULL));
				case _XF06DAD_BRIGHT: return(F06DAD_GetBrightness(hD, NULL, NULL));
				case _XF06DAD_WLSCANRANGE: return(F06DAD_GetWavelengthRange(hD, 1, 1, NULL, NULL));
				case _XF06DAD_TEMP_WLRANGE: return(F06DAD_GetWavelengthRange(hD, 1, 0, NULL, NULL));
				case _XF06DAD_TEMP_SCANDATA: return(F06DAD_GetScan(hD, 1, NULL, NULL, NULL, NULL));
				case _XF06DAD_TEMP_CRR: return(F06DAD_GetReport(hD));
			}
		}
	}
	return D_PERROR_NOTAVAILABLE;
}

//*** function writes property
DWORD F06DAD_WriteProp(HDEVICE hD, WORD pidx, D_VAL val)
{
	D_DEVICE *pD;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->n_p && pD->p) {
			switch (pidx) {
				case _XF06DAD_LAMP: return(F06DAD_SetLamp(hD, 0, val.b));
				case _XF06DAD_WLA: return(F06DAD_SetWavelength(hD, 0, val.d));
				case _XF06DAD_WLB: return(F06DAD_SetWavelength(hD, 1, val.d));
				case _XF06DAD_WLC: return(F06DAD_SetWavelength(hD, 2, val.d));
				case _XF06DAD_WLD: return(F06DAD_SetWavelength(hD, 3, val.d));
				case _XF06DAD_HALF: return(F06DAD_SetHalfWidth(hD, val.u));
				case _XF06DAD_FILTER: return(F06DAD_SetFilter(hD, val.u));
				case _XF06DAD_WLSCANRANGE: return(F06DAD_SetWavelengthScanRange(hD, val.u));
				case _XF06DAD_DFCE: return(F06DAD_SetDFunction(hD, val.u));
				case _XF06DAD_DTHR: return(F06DAD_SetDThreshold(hD, val.d));
				case _XF06DAD_ALAMP: return(F06DAD_SetAutoLamp(hD, val.u));
				case _XF06DAD_ARATIO: return(F06DAD_SetAnalogRatio(hD, val.u));
				case _XF06DAD_AOFSA: return(F06DAD_SetAnalogOffset(hD, 0, val.d));
				case _XF06DAD_AOFSB: return(F06DAD_SetAnalogOffset(hD, 1, val.d));
				case _XF06DAD_AOFSC: return(F06DAD_SetAnalogOffset(hD, 2, val.d));
				case _XF06DAD_AOFSD: return(F06DAD_SetAnalogOffset(hD, 3, val.d));
				case _XF06DAD_BRIGHT: return(F06DAD_SetBrightness(hD, val.u));
			}
		}
	}
	return NO_ERROR;
}

//*** function pre-sets property
DWORD F06DAD_PreSetProp(HDEVICE hD, WORD pidx, D_VAL val)
{
	D_DEVICE *pD;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->n_p && pD->p) {
			switch (pidx) {
				case _XF06DAD_WLA: return(F06DAD_PreSetWavelength(hD, 0, val.d));
				case _XF06DAD_WLB: return(F06DAD_PreSetWavelength(hD, 1, val.d));
				case _XF06DAD_WLC: return(F06DAD_PreSetWavelength(hD, 2, val.d));
				case _XF06DAD_WLD: return(F06DAD_PreSetWavelength(hD, 3, val.d));
				case _XF06DAD_AOFSA: return(F06DAD_PreSetAnalogOffset(hD, 0, val.d));
				case _XF06DAD_AOFSB: return(F06DAD_PreSetAnalogOffset(hD, 1, val.d));
				case _XF06DAD_AOFSC: return(F06DAD_PreSetAnalogOffset(hD, 2, val.d));
				case _XF06DAD_AOFSD: return(F06DAD_PreSetAnalogOffset(hD, 3, val.d));
			}
		}
	}
	return NO_ERROR;
}

//*** function initiates detector (make it ready to work properly, switch lamp on, ...)
DWORD F06DAD_InitDetector(HDEVICE hD, D_STAT *stat)
{
	D_DEVICE *pD;
	DWORD ret = NO_ERROR;
	WORD i;
	double t0;
	unsigned model;
	F06DAD_SPARS *spars;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;
	if (pD->n_p == 0 || pD->p == NULL)
		return(ERROR_INVALID_HANDLE);		//bad device handle (bad initiation)

	while (1) {

		// 1) identify device & test model
		D_SetState(0, D_("Identify device"), stat);
		model = pD->p[_XF06DAD_MODEL].val.u;		//store model
		if (!F06DAD_TestID(hD, NULL, NULL, NULL, NULL)) {
			pD->p[_XF06DAD_MODEL].val.u = model;		//restore model
			return(ERROR_FILE_INVALID);		//device not found
		}
		if (pD->p[_XF06DAD_MODEL].val.u != model) {
			pD->p[_XF06DAD_MODEL].val.u = model;		//restore model
			return(ERROR_INVALID_PARAMETER);		//invalid model
		}
		pD->p[_XF06DAD_MODEL].val.u = model;		//restore model

		// 2) stop subsriptions
		D_SetState(10, D_("Unsubscribe data"), stat);
		F06DAD_SubscribeMode(hD, _XF06DAD_SUBSMODE_ABS, FALSE);		//unsubsribe ABS
		F06DAD_SubscribeMode(hD, _XF06DAD_SUBSMODE_INT, FALSE);		//unsubsribe IT

		//get first enable signal!!!
		pD->p[_XF06DAD_TEMP_FIRSTSIG].val.u = 0;
		if (pD->n_s && pD->s) {
			for (i=0; i<pD->n_s; i++) {
				if (pD->s[i].disabled == 0)		//enabled?
					break;
			}
			if (i < pD->n_s)
				pD->p[_XF06DAD_TEMP_FIRSTSIG].val.u = i;
		}

		//create list handle for report
		pD->p[_XF06DAD_TEMP_CRR].val.h = LIST_Create(F06DAD_ReportItemProcParam);

		// 3) lamp
		ret = F06DAD_GetLamp(hD, 1, 0, NULL, NULL);
		if (ret != NO_ERROR)
			break;

		if (pD->p[_XF06DAD_INIT_LAMP].val.b && pD->p[_XF06DAD_LAMP].val.b == FALSE) {
			D_SetState(20, D_("Setting lamp on"), stat);
			//lamp on
			ret = F06DAD_SetLamp(hD, 0, TRUE);
			if (ret != NO_ERROR)
				return(ret);
			//wait until lamp on
			t0 = timer_GetTime();
			while (1) {
				ret = F06DAD_GetLamp(hD, 1, 0, NULL, NULL);
				if (ret != NO_ERROR || pD->p[_XF06DAD_LAMP].val.b || (timer_GetTime()-t0) > _XF06DAD_WAITFORLAMP)
					break;
				//test abort
				if (stat && stat->run == FALSE)
					return(ERROR_OPERATION_ABORTED);
				//delay
				Sleep(250);		//wait x seconds
				D_SetState(20+((timer_GetTime()-t0)*30)/_XF06DAD_WAITFORLAMP, D_("Wait for lamp"), stat);
			}
			//wait for measure mode (min. 1200ms)
			t0 = timer_GetTime();
			while ((timer_GetTime()-t0) < _XF06DAD_WAITMEAS) {
				//test abort
				if (stat && stat->run == FALSE)
					return(ERROR_OPERATION_ABORTED);
				//delay
				Sleep(250);		//wait x seconds
				D_SetState(50+((timer_GetTime()-t0)*20)/_XF06DAD_WAITMEAS, D_("Wait for lamp"), stat);
			}
		}

		// 4) setting parameters
		D_SetState(70, D_("Setting parameters"), stat);

		//init set buffered value
		spars = (F06DAD_SPARS *)pD->p[_XF06DAD_TEMP_SPARS].val.m.buf;
		if (spars) {
			for (i=0; i<_XF06DAD_MAXCHANNELS; i++) {
				spars->wl[i] = pD->p[_XF06DAD_WLA+i].vbackup.d;
			}
		}
		//F06DAD_WriteProp(hD, _XF06DAD_HALF, pD->p[_XF06DAD_HALF].val);		//set half width
		//F06DAD_WriteProp(hD, _XF06DAD_FILTER, pD->p[_XF06DAD_FILTER].val);		//set filter
		F06DAD_ReadProp(hD, _XF06DAD_TEMP_WLRANGE);		//read scan range
		//F06DAD_WriteProp(hD, _XF06DAD_WLSCANRANGE, pD->p[_XF06DAD_WLSCANRANGE].val);		//set scan range

		F06DAD_ReadProp(hD, _XF06DAD_TEMP_CRR);		//read report

		// 6) delayed autozero
		if (pD->p[_XF06DAD_INIT_LAMP].val.b && pD->p[_XF06DAD_INIT_AZERO].val.b && pD->p[_XF06DAD_LAMP].val.b) {
			D_SetState(80, D_("Autozero"), stat);
			//delay
			t0 = timer_GetTime();
			while ((timer_GetTime()-t0) < pD->p[_XF06DAD_INIT_AZERODELAY].val.u*1000) {
				//test abort
				if (stat && stat->run == FALSE)
					return(ERROR_OPERATION_ABORTED);
				Sleep(250);		//wait x seconds

				if (pD->p[_XF06DAD_INIT_AZERODELAY].val.u)
					D_SetState(60+((timer_GetTime()-t0)*40)/(pD->p[_XF06DAD_INIT_AZERODELAY].val.u*1000), D_("Autozero"), stat);
			}

			//set autozero
			ret = F06DAD_RunAutozero(hD);
			if (ret != NO_ERROR)
				return(ret);
			//??? wait on autozero done

		}

		//update status
		F06DAD_ReadProp(hD, _XF06DAD_STAT);

		// 8) done
		if (ret == NO_ERROR)
			D_SetState(100, D_("Done"), stat);

		break;
	}

	return(NO_ERROR);
}

//*** function initiates detector when closing communication
DWORD F06DAD_PostInitDetector(HDEVICE hD, D_STAT *stat)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;
	if (pD->n_p == 0 || pD->p == NULL)
		return(ERROR_INVALID_HANDLE);		//bad device handle (bad initiation)

	while (1) {
		D_SetState(0, D_("Deinitiation"), stat);

		// 1) setting lamp off
		if (pD->p[_XF06DAD_DEIN_LAMP].val.b && pD->p[_XF06DAD_LAMP].val.b == TRUE) {
			D_SetState(0, D_("Setting lamp off"), stat);
			//lamp off
			F06DAD_SetLamp(hD, 0, FALSE);
		}

		//discard list handle
		LIST_Discard(pD->p[_XF06DAD_TEMP_CRR].val.h);
		pD->p[_XF06DAD_TEMP_CRR].val.h = NULL;	//!!!

		D_SetState(100, D_("Done"), stat);
		break;
	}

	return(NO_ERROR);
}

//-------------------------------------------------------------------
//*** function text reception (ignore size==1)
BOOL F06DAD_TestABC(const char *str, unsigned n, unsigned offset, char beginchar)
{
	unsigned i;

	if (str && n > 1) {
		for (i=0; i<n; i++) {
			if (*(str+i*offset) != (beginchar+i))
				return(FALSE);
		}
	}
	return(TRUE);
}

//*** function text reception
BOOL F06DAD_TestXYZ(const char *str, unsigned n, unsigned offset, char beginchar)
{
	unsigned i;

	if (str && n) {
		for (i=0; i<n; i++) {
			if (*(str+i*offset) != (beginchar+i))
				return(FALSE);
		}
	}
	return(TRUE);
}

//*** function returns model by model-character
unsigned F06DAD_GetModelByChar(int c)
{
	switch (c) {
		default:
		case XF06DAD_MODELA_CHAR: return(XF06DAD_MODELA);
		case XF06DAD_MODELB_CHAR: return(XF06DAD_MODELB);
		case XF06DAD_MODELC_CHAR: return(XF06DAD_MODELC);
		case XF06DAD_MODELD_CHAR: return(XF06DAD_MODELD);
		case XF06DAD_MODELE_CHAR: return(XF06DAD_MODELE);
		case XF06DAD_MODELF_CHAR: return(XF06DAD_MODELF);
		case XF06DAD_MODELG_CHAR: return(XF06DAD_MODELG);
		case XF06DAD_MODELH_CHAR: return(XF06DAD_MODELH);
		case XF06DAD_MODELI_CHAR: return(XF06DAD_MODELI);
	}
}

//*** function gets dev. model
unsigned int F06DAD_GetModel(HDEVICE hD)
{
	D_DEVICE *pD;
	unsigned int model = XF06DAD_MODELA;

	if (hD) {
		pD = (D_DEVICE *)hD;
		model = pD->p[_XF06DAD_MODEL].val.u;
	}
	return(model);
}

//*** function returns wavelength by model
unsigned F06DAD_GetWavelengthByModel(int model)
{
	switch (model) {
		default:
		case XF06DAD_MODELA: return(_XF06DAD_A_WLMAX);
		case XF06DAD_MODELB: return(_XF06DAD_A_WLMAX);
		case XF06DAD_MODELC: return(_XF06DAD_C_WLMAX);
		case XF06DAD_MODELD: return(_XF06DAD_A_WLMAX);
		case XF06DAD_MODELE: return(_XF06DAD_E_WLMAX);
		case XF06DAD_MODELF: return(_XF06DAD_E_WLMAX);
		case XF06DAD_MODELG: return(_XF06DAD_E_WLMAX);
		case XF06DAD_MODELH: return(_XF06DAD_E_WLMAX);
		case XF06DAD_MODELI: return(_XF06DAD_A_WLMAX);
	}
}

//*** function returns number of channel by model
unsigned F06DAD_GetChannelsByModel(int model)
{
	switch (model) {
		default:
		case XF06DAD_MODELA: return(_XF06DAD_MAXCHANSA);
		case XF06DAD_MODELB: return(_XF06DAD_MAXCHANSB);
		case XF06DAD_MODELC: return(_XF06DAD_MAXCHANSA);
		case XF06DAD_MODELD: return(_XF06DAD_MAXCHANSD);
		case XF06DAD_MODELE: return(_XF06DAD_MAXCHANSA);
		case XF06DAD_MODELF: return(_XF06DAD_MAXCHANSD);
		case XF06DAD_MODELG: return(_XF06DAD_MAXCHANSD);
		case XF06DAD_MODELH: return(_XF06DAD_MAXCHANSA);
		case XF06DAD_MODELI: return(_XF06DAD_MAXCHANSA);
	}
}

//*** function returns number of channel by model
unsigned F06DAD_GetWChannelsByModel(int model)
{
	switch (model) {
		default:
		case XF06DAD_MODELA: return(_XF06DAD_MAXWCHANSA);
		case XF06DAD_MODELB: return(_XF06DAD_MAXWCHANSA);
		case XF06DAD_MODELC: return(_XF06DAD_MAXWCHANSA);
		case XF06DAD_MODELD: return(_XF06DAD_MAXWCHANSA);
		case XF06DAD_MODELE: return(_XF06DAD_MAXWCHANSA);
		case XF06DAD_MODELF: return(_XF06DAD_MAXWCHANSA);
		case XF06DAD_MODELG: return(_XF06DAD_MAXWCHANSG);
		case XF06DAD_MODELH: return(_XF06DAD_MAXWCHANSA);
		case XF06DAD_MODELI: return(_XF06DAD_MAXWCHANSA);
	}
}

//*** function returns baudrate by model
unsigned F06DAD_GetBaudrateByModel(int model)
{
	switch (model) {
		default:
		case XF06DAD_MODELA: return(XF06DAD_MODELA_BAUD);
		case XF06DAD_MODELB: return(XF06DAD_MODELA_BAUD);
		case XF06DAD_MODELC: return(XF06DAD_MODELA_BAUD);
		case XF06DAD_MODELD: return(XF06DAD_MODELD_BAUD);
		case XF06DAD_MODELE: return(XF06DAD_MODELA_BAUD);
		case XF06DAD_MODELF: return(XF06DAD_MODELD_BAUD);
		case XF06DAD_MODELG: return(XF06DAD_MODELD_BAUD);
		case XF06DAD_MODELH: return(XF06DAD_MODELH_BAUD);
		case XF06DAD_MODELI: return(XF06DAD_MODELH_BAUD);
	}
}

//*** function returns time interval for scanning by model
unsigned F06DAD_GetSTimingByModel(int model)
{
	switch (model) {
		default:
		case XF06DAD_MODELA: return(_XF06DAD_STIMEA);
		case XF06DAD_MODELB: return(_XF06DAD_STIMEB);
		case XF06DAD_MODELC: return(_XF06DAD_STIMEC);
		case XF06DAD_MODELD: return(_XF06DAD_STIMED);
		case XF06DAD_MODELE: return(_XF06DAD_STIMEE);
		case XF06DAD_MODELF: return(_XF06DAD_STIMEF);
		case XF06DAD_MODELG: return(_XF06DAD_STIMEG);
		case XF06DAD_MODELH: return(_XF06DAD_STIMEH);
		case XF06DAD_MODELI: return(_XF06DAD_STIMEI);
	}
}

//*** function returns if is model 800
int F06DAD_IsModel800(int model)
{
	switch (model) {
		//NO
		default:
		case XF06DAD_MODELA:
		case XF06DAD_MODELB:
		case XF06DAD_MODELC:
		case XF06DAD_MODELD:
		case XF06DAD_MODELI:
			return(0);
		//YES
		case XF06DAD_MODELE:
		case XF06DAD_MODELF:
		case XF06DAD_MODELG:
		case XF06DAD_MODELH:
			return(1);
	}
}

//*** function returns if model with subscription scan
int F06DAD_IsModelSubScan(int model)
{

	switch (model) {
		//NO
		default:
		case XF06DAD_MODELA:
		case XF06DAD_MODELB:
		case XF06DAD_MODELC:
		case XF06DAD_MODELD:
		case XF06DAD_MODELE:
		case XF06DAD_MODELF:
		case XF06DAD_MODELG:
			return(0);
		//YES
		case XF06DAD_MODELH:
		case XF06DAD_MODELI:
			return(1);
	}
}


//*** function returns firmware version (stored from init)
//note: 100=v1.00; 0=error
int F06DAD_GetFWVersion(HDEVICE hD)
{
	TCHAR *ptmp;

	if (hD && (ptmp = GetSubString(((D_DEVICE *)hD)->id, 2)) != NULL) {
		return(__atoi(ptmp));
	}
	return(0);
}

//*** test for diaply version
int F06DAD_IsDisplay(HDEVICE hD)
{
	D_DEVICE *pD;
	int disp = 0;

	if (hD) {
		pD = (D_DEVICE *)hD;
		disp = pD->p[_XF06DAD_TEMP_DISPLAY].val.u;
	}
	return(disp);
}

//-------------------------------------------------------------------
//*** function gets lamp status
DWORD F06DAD_GetLamp(HDEVICE hD, int how, WORD idx, BOOL *bval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch(how) {
		default:
		case 0:		//get lamp status from ram
			ret = NO_ERROR;
			break;

		case 1:		//read lamp status
			t0 = timer_GetTime();
			//manage command
			strcpy(pD->com.sendbuf, "#LPr\n");
			//send and read
			ret = F06DAD_SendAndRcptStr(&pD->com);
			//average abs. time between request and response
			pD->p[_XF06DAD_LAMP].time = (t0 + timer_GetTime())/2.0;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode response
			if (strstr(pD->com.rcptbuf, "LPrERR") != NULL)
				ret = 1;		//value not available
			else if (strcmp(pD->com.rcptbuf, "LPrT") == 0) {
				pD->p[_XF06DAD_LAMP].val.b = TRUE;		//lamp is on
			}
			else if (strcmp(pD->com.rcptbuf, "LPrF") == 0) {
				pD->p[_XF06DAD_LAMP].val.b = FALSE;		//lamp is off
			}
			else
				ret = 2;		//none or bad response

			pD->p[_XF06DAD_LAMP].lerr = ret;
			break;
	}

	//???
	if (bval)
		*bval = pD->p[_XF06DAD_LAMP].val.b;
	if (time)
		*time = pD->p[_XF06DAD_LAMP].time;

	return(ret);
}

//*** function sets lamp status
DWORD F06DAD_SetLamp(HDEVICE hD, DWORD idx, BOOL bval)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	strcpy(pD->com.sendbuf, bval ? "#LPwT\n" : "#LPwF\n");
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strstr(pD->com.rcptbuf, "LPwERR") != NULL)
		ret = 1;		//operation wasn't successful
	else if (strcmp(pD->com.rcptbuf, "LPwT") == 0) {
		pD->p[_XF06DAD_LAMP].val.b = TRUE;		//lamp is on
	}
	else if (strcmp(pD->com.rcptbuf, "LPwF") == 0) {
		pD->p[_XF06DAD_LAMP].val.b = FALSE;		//lamp is off
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function gets wavelength
DWORD F06DAD_GetWavelength(HDEVICE hD, int how, WORD idx, double *dval, double *time)
{
	D_DEVICE *pD;
	DWORD ret, lidx;
	unsigned i, channels;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch (idx) {
		default:
		case 0: lidx = _XF06DAD_WLA; break;
		case 1: lidx = _XF06DAD_WLB; break;
		case 2: lidx = _XF06DAD_WLC; break;
		case 3: lidx = _XF06DAD_WLD; break;
	}

	switch(how) {
		default:
		case 0:		//get lamp status from ram
			ret = NO_ERROR;
			break;

		case 1:		//read lamp status
			//get channels
			channels = F06DAD_GetChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);

			t0 = timer_GetTime();
			//manage command
			strcpy(pD->com.sendbuf, "#WLr\n");
			//send and read
			ret = F06DAD_SendAndRcptStr(&pD->com);
			//average abs. time between request and response
			pD->p[_XF06DAD_WLA].time = (t0 + timer_GetTime())/2.0;
			for (i=1; i<channels; i++)
				pD->p[_XF06DAD_WLA+i].time = pD->p[_XF06DAD_WLA].time;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode response
			if (strstr(pD->com.rcptbuf, "WLrERR") != NULL)
				ret = 1;		//value not available
			else if (strncmp(pD->com.rcptbuf, "WLr", 3) == 0 &&
							 F06DAD_TestABC(pD->com.rcptbuf+3, channels, 4, XF06DAD_ACHAR)) {		//test reception
				//read value from string ("ABrA+014200:0B...") + data error (2 bits each)
				if (channels > 1) {
					for (i=0; i<channels; i++)
						pD->p[_XF06DAD_WLA+i].val.d = (double)atoi(pD->com.rcptbuf+4+i*4);		//nm
				}
				else
					pD->p[_XF06DAD_WLA].val.d = (double)atoi(pD->com.rcptbuf+3);		//nm
			}
			else
				ret = 2;		//none or bad response

			for (i=0; i<channels; i++)
				pD->p[_XF06DAD_WLA+i].lerr = ret;
			break;
	}

	//???
	if (dval)
		*dval = pD->p[lidx].val.d;
	if (time)
		*time = pD->p[lidx].time;

	return(ret);
}

//*** function pre-sets wavelength
DWORD F06DAD_PreSetWavelength(HDEVICE hD, DWORD idx, double dval)
{
	F06DAD_SPARS *spars;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	spars = (F06DAD_SPARS *)((D_DEVICE *)hD)->p[_XF06DAD_TEMP_SPARS].val.m.buf;
	if (spars == NULL)
		return(ERROR_INVALID_HANDLE);

	if (idx < _XF06DAD_MAXCHANNELS) {
		spars->wl[idx] = dval;
	}
	return(NO_ERROR);
}

//*** function sets wavelength
DWORD F06DAD_SetWavelength(HDEVICE hD, DWORD idx, double dval)
{
	D_DEVICE *pD;
	DWORD ret;
	unsigned i, channels;
	F06DAD_SPARS *spars;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//pre-sets wavelengths
	ret = F06DAD_PreSetWavelength(hD, idx, dval);
	if (ret != NO_ERROR)
		return(ret);

	spars = (F06DAD_SPARS *)pD->p[_XF06DAD_TEMP_SPARS].val.m.buf;

	//channels
	channels = F06DAD_GetChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);

	//manage command
	if (channels > 1) {
		sprintf(pD->com.sendbuf, "#WLw");
		for (i=0; i<channels; i++) {
			sprintf(pD->com.sendbuf, "%s%c%03.0lf",
							pD->com.sendbuf,
							XF06DAD_ACHAR+i,
							spars->wl[i]);
		}
		strcat(pD->com.sendbuf, "\n");
	}
	else
		sprintf(pD->com.sendbuf, "#WLw%03.0lf\n", dval);
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "WLwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "WLw", 3) == 0 &&
					 F06DAD_TestABC(pD->com.rcptbuf+3, channels, 4, XF06DAD_ACHAR)) {		//test reception
		//read value from string ("ABrA+014200:0B...") + data error (2 bits each)
		if (channels > 1) {
			for (i=0; i<channels; i++)
				pD->p[_XF06DAD_WLA+i].val.d = (double)atoi(pD->com.rcptbuf+4+i*4);		//nm
		}
		else
			pD->p[_XF06DAD_WLA].val.d = (double)atoi(pD->com.rcptbuf+3);		//nm
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function gets half width
DWORD F06DAD_GetHalfWidth(HDEVICE hD, int how, unsigned *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch(how) {
		default:
		case 0:		//get lamp status from ram
			ret = NO_ERROR;
			break;

		case 1:		//read lamp status
			t0 = timer_GetTime();
			//manage command
			strcpy(pD->com.sendbuf, "#HFr\n");
			//send and read
			ret = F06DAD_SendAndRcptStr(&pD->com);
			//average abs. time between request and response
			pD->p[_XF06DAD_HALF].time = (t0 + timer_GetTime())/2.0;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode response
			if (strstr(pD->com.rcptbuf, "HFrERR") != NULL)
				ret = 1;		//value not available
			else if (strncmp(pD->com.rcptbuf, "HFr", 3) == 0) {
				pD->p[_XF06DAD_HALF].val.u = atoi(pD->com.rcptbuf+3);		//half width
			}
			else
				ret = 2;		//none or bad response

			pD->p[_XF06DAD_HALF].lerr = ret;
			break;
	}

	//???
	if (uval)
		*uval = pD->p[_XF06DAD_HALF].val.u;
	if (time)
		*time = pD->p[_XF06DAD_HALF].time;

	return(ret);
}

//*** function sets half width
DWORD F06DAD_SetHalfWidth(HDEVICE hD, unsigned uval)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (uval > 15)
		uval = 15;

	//manage command
	sprintf(pD->com.sendbuf, "#HFw%02d\n", uval);
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "HFwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "HFw", 3) == 0) {
		pD->p[_XF06DAD_HALF].val.u = atoi(pD->com.rcptbuf+3);		//half width
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function gets filter
DWORD F06DAD_GetFilter(HDEVICE hD, int how, unsigned *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch(how) {
		default:
		case 0:		//get lamp status from ram
			ret = NO_ERROR;
			break;

		case 1:		//read lamp status
			t0 = timer_GetTime();
			//manage command
			strcpy(pD->com.sendbuf, "#TCr\n");
			//send and read
			ret = F06DAD_SendAndRcptStr(&pD->com);
			//average abs. time between request and response
			pD->p[_XF06DAD_FILTER].time = (t0 + timer_GetTime())/2.0;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode response
			if (strstr(pD->com.rcptbuf, "TCrERR") != NULL)
				ret = 1;		//value not available
			else if (strncmp(pD->com.rcptbuf, "TCr", 3) == 0) {
				pD->p[_XF06DAD_FILTER].val.u = atoi(pD->com.rcptbuf+3);		//filter
			}
			else
				ret = 2;		//none or bad response

			pD->p[_XF06DAD_FILTER].lerr = ret;
			break;
	}

	//???
	if (uval)
		*uval = pD->p[_XF06DAD_FILTER].val.u;
	if (time)
		*time = pD->p[_XF06DAD_FILTER].time;

	return(ret);
}

//*** function sets filter (T90: 0.5s, 0.75s, 1s, 2s; T63: 4s, 8s, 0.2s, 0.1s)
DWORD F06DAD_SetFilter(HDEVICE hD, unsigned uval)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	sprintf(pD->com.sendbuf, "#TCw%01d\n", uval);
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "TCwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "TCw", 3) == 0) {
		pD->p[_XF06DAD_FILTER].val.u = atoi(pD->com.rcptbuf+3);		//filter
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function gets wl range (0-measure, 1-scan)
DWORD F06DAD_GetWavelengthRange(HDEVICE hD, int how, WORD idx, unsigned *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret, lidx;
	WORD w1, w2;
	double t0;
#ifndef F06DAD_NOWLRANGELIMIT
	WORD wlmax;
#endif

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (idx == 0)
		lidx = _XF06DAD_TEMP_WLRANGE;
	else
		lidx = _XF06DAD_WLSCANRANGE;

	switch(how) {
		default:
		case 0:		//get lamp status from ram
			ret = NO_ERROR;
			break;

		case 1:		//read lamp status
			t0 = timer_GetTime();
			//manage command
			strcpy(pD->com.sendbuf, idx == 0 ? "#WRr\n" : "#WSr\n");
			//send and read
			ret = F06DAD_SendAndRcptStr(&pD->com);
			//average abs. time between request and response
			pD->p[lidx].time = (t0 + timer_GetTime())/2.0;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode response
			if (strstr(pD->com.rcptbuf, idx == 0 ? "WRrERR" : "WSrERR") != NULL)
				ret = 1;		//value not available
			else if (strncmp(pD->com.rcptbuf, idx == 0 ? "WRrL" : "WSrX", 4) == 0 &&
							 *(pD->com.rcptbuf+7) == (idx == 0 ? 'U' : 'Y')) {
#ifndef F06DAD_NOWLRANGELIMIT
				wlmax = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
#endif
				//w1
				w1 = atoi(pD->com.rcptbuf+4);
#ifndef F06DAD_NOWLRANGELIMIT
				if (w1 < _XF06DAD_WLMIN)
					w1 = _XF06DAD_WLMIN;
				else if (w1 > wlmax)
					w1 = wlmax;
#endif
				//w2
				w2 = atoi(pD->com.rcptbuf+8);
#ifndef F06DAD_NOWLRANGELIMIT
				if (w2 < _XF06DAD_WLMIN)
					w2 = _XF06DAD_WLMIN;
				else if (w2 > wlmax)
					w2 = wlmax;
#endif
				//store w1 & w2
				pD->p[lidx].val.u = w1 | (w2 << 16);		//wl range
			}
			else
				ret = 2;		//none or bad response

			pD->p[lidx].lerr = ret;
			break;
	}

	//???
	if (uval)
		*uval = pD->p[lidx].val.u;
	if (time)
		*time = pD->p[lidx].time;

	return(ret);
}

//*** function sets wl range (scan)
DWORD F06DAD_SetWavelengthScanRange(HDEVICE hD, unsigned uval)
{
	D_DEVICE *pD;
	DWORD ret;
	WORD w1, w2;
#ifndef F06DAD_NOWLRANGELIMIT
	WORD wlmax;
#endif

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//test range
#ifndef F06DAD_NOWLRANGELIMIT
	wlmax = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
#endif
	w1 = uval & 0xFFFF;
#ifndef F06DAD_NOWLRANGELIMIT
	if (w1 < _XF06DAD_WLMIN)
		w1 = _XF06DAD_WLMIN;
	else if (w1 > wlmax)
		w1 = wlmax;
#endif
	w2 = uval >> 16;
#ifndef F06DAD_NOWLRANGELIMIT
	if (w2 < _XF06DAD_WLMIN)
		w2 = _XF06DAD_WLMIN;
	else if (w2 > wlmax)
		w2 = wlmax;
	if (w2 <= w1) {
		if (w2 < wlmax)
			w2 = w1 + 1;
		else
			w1 = w2 - 1;
	}
#endif

	//manage command
	sprintf(pD->com.sendbuf, "#WSwX%03dY%03d\n", w1, w2);
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "WSwERR", 6) == 0)
		ret = 1;		//value not available
	else if (strncmp(pD->com.rcptbuf, "WSwX", 4) == 0 &&
					 *(pD->com.rcptbuf+7) == 'Y') {
		pD->p[_XF06DAD_WLSCANRANGE].val.u = atoi(pD->com.rcptbuf+4)  | (atoi(pD->com.rcptbuf+7) << 16);		//wl range
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function gets absorbance
DWORD F06DAD_GetAbs(HDEVICE hD, int how, WORD idx, double *dval, double *time, D_DHQUEUE *dhq)
{
	D_DEVICE *pD;
	DWORD ret, lidx, i, tmp;
	unsigned channels, wchannels;
	char *pt;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch (idx) {
		default:
		case 0: lidx = _XF06DAD_ABSA; break;
		case 1: lidx = _XF06DAD_ABSB; break;
		case 2: lidx = _XF06DAD_ABSC; break;
		case 3: lidx = _XF06DAD_ABSD; break;
		case 4: lidx = _XF06DAD_ABSW; break;
	}

	switch(how) {
		default:
		case 0:		//get absorbance from ram
			ret = NO_ERROR;
			break;

		case 1:		//read absorbance
//			if (idx == pD->p[_XF06DAD_TEMP_FIRSTSIG].val.u) {
				//get channels
				channels = F06DAD_GetChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);
				wchannels = F06DAD_GetWChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);

				t0 = timer_GetTime();		//time of request
				//manage command
				strcpy(pD->com.sendbuf, "#ABr\n");
				//send and read
				ret = F06DAD_SendAndRcptStr(&pD->com);
				//average abs. time between request and response
				pD->p[_XF06DAD_ABSA].time = (double)((t0 + timer_GetTime())/2.0);
				for (i=1; i<channels; i++)
					pD->p[_XF06DAD_ABSA+i].time = pD->p[_XF06DAD_ABSA].time;
				for (i=0; i<wchannels; i++)
					pD->p[_XF06DAD_ABSW+i].time = pD->p[_XF06DAD_ABSA].time;

				DIAG_LogPrintf("F06DAD_GetAbs: %.0lf ms", pD->p[_XF06DAD_ABSA].time);

				if (ret != NO_ERROR)
					break;		//error (timeout)


				//decode response
				if (strncmp(pD->com.rcptbuf, "ABrERR", 6) == 0)
					ret = 1;		//value not available
				else if (strncmp(pD->com.rcptbuf, "ABr", 3) == 0 &&
								 F06DAD_TestABC(pD->com.rcptbuf+3, channels, 10, XF06DAD_ACHAR)) {		//test reception
					//read value from string ("ABrA+014200:0B...") + data error (2 bits each)
					tmp = 0;
					pt = pD->com.rcptbuf+4;
					if (channels > 1) {
						for (i=0; i<channels; i++) {
							pD->p[_XF06DAD_ABSA+i].val.d = (double)atoi(pt)*1.0e-2;		//mV
							tmp |= (atoi(pt+8) & 0x3) << 2*i;
							pt += 10;
						}
					}
					else {
						pD->p[_XF06DAD_ABSA].val.d = (double)atoi(pD->com.rcptbuf+3)*1.0e-2;		//mV
						tmp |= (atoi(pD->com.rcptbuf+11) & 0x3);
					}
					//store errs
					pD->p[_XF06DAD_TEMP_EABS].val.u = tmp;

					//w-channels
					tmp = 0;
					pt = pD->com.rcptbuf+3+channels*10;
					if (F06DAD_TestXYZ(pt, wchannels, 10, XF06DAD_WCHAR)) {
						pt++;
						for (i=0; i<wchannels; i++) {
							pD->p[_XF06DAD_ABSW+i].val.d = (double)atoi(pt)*1.0e-2;		//mV
							tmp |= (atoi(pt+8) & 0x3) << 2*i;
							pt += 10;
						}
					}
					pD->p[_XF06DAD_TEMP_EABSW].val.u = tmp;
				}
				else
					ret = 2;		//none or bad response

				//save error
				for (i=0; i<channels; i++)
					pD->p[_XF06DAD_ABSA+i].lerr = ret;
				for (i=0; i<wchannels; i++)
					pD->p[_XF06DAD_ABSW+i].lerr = ret;
//			}

			ret = pD->p[lidx].lerr;
			break;
	}

	//???
	if (dval)
		*dval = pD->p[lidx].val.d;
	if (time)
		*time = pD->p[lidx].time;

	return(ret);
}

//*** function gets intensity
DWORD F06DAD_GetIntensity(HDEVICE hD, int how, WORD idx, double *dval, double *time)
{
	D_DEVICE *pD;
	DWORD ret, lidx, i, tmp;
	unsigned channels;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch (idx) {
		default:
		case 0: lidx = _XF06DAD_INTA; break;
		case 1: lidx = _XF06DAD_INTB; break;
		case 2: lidx = _XF06DAD_INTC; break;
		case 3: lidx = _XF06DAD_INTD; break;
	}

	switch(how) {
		default:
		case 0:		//get absorbance from ram
			ret = NO_ERROR;
			break;

		case 1:		//read absorbance
			if (idx == 0) {
				//get channels
				channels = F06DAD_GetChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);

				t0 = timer_GetTime();		//time of request
				//manage command
				strcpy(pD->com.sendbuf, "#ITr\n");
				//send and read
				ret = F06DAD_SendAndRcptStr(&pD->com);
				//average abs. time between request and response
				pD->p[_XF06DAD_INTA].time = (t0 + timer_GetTime())/2.0;
				for (i=1; i< channels; i++)
					pD->p[_XF06DAD_INTA+i].time = pD->p[_XF06DAD_INTA].time;

				if (ret != NO_ERROR)
					break;		//error (timeout)

				//decode response
				if (strncmp(pD->com.rcptbuf, "ITrERR", 6) == 0)
					ret = 1;		//value not available
				else if (strncmp(pD->com.rcptbuf, "ITr", 3) == 0 &&
								 F06DAD_TestABC(pD->com.rcptbuf+3, channels, 8, XF06DAD_ACHAR)) {		//test reception
					//read value from string (ITrA+08500:0B...") + data error (2 bits each)
					tmp = 0;
					if (channels > 1) {
						for (i=0; i<channels; i++) {
							pD->p[_XF06DAD_INTA+i].val.d = (double)atoi(pD->com.rcptbuf+4+i*8)*1.0e-2;		//%
							tmp |= (atoi(pD->com.rcptbuf+10+i*8) & 0x3) << 2*i;
						}
					}
					else {
						pD->p[_XF06DAD_INTA].val.d = (double)atoi(pD->com.rcptbuf+3)*1.0e-2;		//%
						tmp |= (atoi(pD->com.rcptbuf+9) & 0x3);
					}
					pD->p[_XF06DAD_TEMP_EINT].val.u = tmp;
				}
				else
					ret = 2;		//none or bad response
			}

			//save error
			if (idx == 0) {
				for (i=0; i<channels; i++)
					pD->p[_XF06DAD_INTA+i].lerr = ret;
			}
			else
				ret = pD->p[lidx].lerr;
			break;
	}


	//???
	if (dval)
		*dval = pD->p[lidx].val.d;
	if (time)
		*time = pD->p[lidx].time;
	return(ret);
}

//*** function gets life-time of lamp
DWORD F06DAD_GetLampLife(HDEVICE hD, int how, WORD idx, double *dval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch(how) {
		default:
		case 0:		//get lamp status from ram
			ret = NO_ERROR;
			break;

		case 1:		//read lamp status
			t0 = timer_GetTime();
			//manage command
			strcpy(pD->com.sendbuf, "#LLr\n");
			//send and read
			ret = F06DAD_SendAndRcptStr(&pD->com);
			//average abs. time between request and response
			pD->p[_XF06DAD_LIFE].time = (t0 + timer_GetTime())/2.0;
			pD->p[_XF06DAD_CLAM].time = pD->p[_XF06DAD_LIFE].time;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode response
			if (strncmp(pD->com.rcptbuf, "LLrERR", 6) == 0)
				ret = 1;		//value not available
			else if (strncmp(pD->com.rcptbuf, "LLrL", 4) == 0 && *(pD->com.rcptbuf+6) == 'H') {
				pD->p[_XF06DAD_CLAM].val.u = atoi(pD->com.rcptbuf+4);		//number of lamps
				pD->p[_XF06DAD_LIFE].val.d = (double)atoi(pD->com.rcptbuf+7) * 1.0e-2;		//lamp life in hours
			}
			else
				ret = 2;		//none or bad response

			pD->p[_XF06DAD_LIFE].lerr = ret;
			pD->p[_XF06DAD_CLAM].lerr = ret;
			break;
	}

	//???
	if (dval)
		*dval = pD->p[_XF06DAD_LIFE].val.d;
	if (time)
		*time = pD->p[_XF06DAD_LIFE].time;

	return(ret);
}

//*** function gets actual state
DWORD F06DAD_GetState(HDEVICE hD, int how, unsigned *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;
	unsigned int e, prev_err;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch(how) {
		default:
		case 0:		//get lamp status from ram
			ret = NO_ERROR;
			break;

		case 1:		//read lamp status
			t0 = timer_GetTime();
			//manage command
			strcpy(pD->com.sendbuf, "#STr\n");
			//send and read
			ret = F06DAD_SendAndRcptStr(&pD->com);
			//average abs. time between request and response
			pD->p[_XF06DAD_STAT].time = (t0 + timer_GetTime())/2.0;
			pD->p[_XF06DAD_ERROR].time = pD->p[_XF06DAD_STAT].time;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode response
			if (strncmp(pD->com.rcptbuf, "STrERR", 6) == 0) {
				ret = 1;
			}
			else if (strncmp(pD->com.rcptbuf, "STr", 3) == 0) {
				prev_err = F06DAD_GetTopError(pD->p[_XF06DAD_ERROR].val.u);
				pD->p[_XF06DAD_STAT].val.u = HexStringNumNToIntA(pD->com.rcptbuf+3, 2);
				pD->p[_XF06DAD_ERROR].val.u = F06DAD_DecodeErrorA(pD->com.rcptbuf+5);

				//dev. error
				e = F06DAD_GetTopError(pD->p[_XF06DAD_ERROR].val.u);
				if (e && prev_err != e) {
					D_EvokeError(hD, D_SEVERITY_ERROR, e, (TCHAR*)F06DAD_GetErrorDescription(e));
				}
				prev_err = e;

			}
			else {
				ret = 2;
			}

			pD->p[_XF06DAD_STAT].lerr = ret;
			pD->p[_XF06DAD_ERROR].lerr = ret;
			break;
	}

	//???
	if (uval)
		*uval = pD->p[_XF06DAD_STAT].val.u;
	if (time)
		*time = pD->p[_XF06DAD_STAT].time;

	return(ret);
}

//*** function gets error
DWORD F06DAD_GetError(HDEVICE hD, int how, unsigned *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch(how) {
		default:
		case 0:		//get from ram
			ret = NO_ERROR;
			break;

		case 1:		//read state
			ret = F06DAD_GetState(hD, 1, NULL, NULL);
			break;
	}

	//???
	if (uval)
		*uval = pD->p[_XF06DAD_ERROR].val.u;
	if (time)
		*time = pD->p[_XF06DAD_ERROR].time;

	return(ret);
}

//---------------------------------------------
//*** function sets scan mode
BOOL F06DAD_SetScanMode(HDEVICE hD, int mode)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	*(char *)(pD->p[_XF06DAD_TEMP_SCANDATA].val.m.buf) = mode;
	return(TRUE);
}

//*** function gets actual scan mode
int F06DAD_GetDataScanMode(HDEVICE hD)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(0);
	pD = (D_DEVICE *)hD;
	return(*(char *)(pD->p[_XF06DAD_TEMP_SCANDATA].val.m.buf+1));
}

//*** function gets absorbance/intensity/transmition scan
DWORD F06DAD_GetScan(HDEVICE hD, int how, DWORD *nn, double **xx, double **yy, char **ee)
{
	D_DEVICE *pD;
	DWORD ret, cx, cy, cz, len, sum, i;
	char *ptext, *ptmp, *e;
	double *x, *y;
	int s_mode, sign;
	DWORD min, max;
	const char *c_text;
	double t0;
	unsigned int tout;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	ptext = (char *)pD->p[_XF06DAD_TEMP_COMBUFF].val.m.buf;
	s_mode = *(char *)(pD->p[_XF06DAD_TEMP_SCANDATA].val.m.buf);
	x = (double *)(pD->p[_XF06DAD_TEMP_SCANDATA].val.m.buf+2);
	y = x + _XF06DAD_SCAN_MAX_SIZE;
	e = (char *)(y + _XF06DAD_SCAN_MAX_SIZE);

	if (how) {
		//set scan mode
		switch (s_mode) {
			default:
				s_mode = _XF06DAD_SCANMODE_ABS;
			case _XF06DAD_SCANMODE_ABS: c_text = "SAr"; break;		//absorbance
			case _XF06DAD_SCANMODE_INT: c_text = "SIr"; break;		//intensity
			case _XF06DAD_SCANMODE_IDX: c_text = "SPr"; break;		//transmition
		}

		while (1) {
			t0 = timer_GetTime();
			//manage command
			*(char *)(pD->p[_XF06DAD_TEMP_SCANDATA].val.m.buf+1) = s_mode;
			sprintf(pD->com.sendbuf, "#%s\n", c_text);
			//send and read
			tout = pD->com.timeout;		//store previous timeout
			pD->com.timeout = 3000;		//longer timeout
			ret = F06DAD_SendAndRcptStr(&pD->com);
			pD->com.timeout = tout;		//restore previous timout
			ptext = pD->com.rcptbuf;
			len = strlen(ptext);
			//average time
			pD->p[_XF06DAD_TEMP_SCANDATA].time = (timer_GetTime()+t0)/2.0;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode response
			if (strncmp(ptext+3, "ERR", 3) == 0) {
				ret = 1;
				break;
			}
			else if (strncmp(ptext, c_text, 3) == 0 &&
							 len >= _XF06DAD_SCAN_MIN_LENGTH &&
							 *(ptext+3) == 'X' &&
							 *(ptext+7) == 'Y' &&
							 *(ptext+11) == 'Z' &&
							 *(ptext+13) == ':') {
				if (s_mode == _XF06DAD_SCANMODE_IDX) {
					min = _XF06DAD_IDXMIN;
					max = _XF06DAD_IDXMAX;
				}
				else {
					min = _XF06DAD_WLMIN;
					max = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
				}
				cx = HexStringNumNToIntA(ptext+4, 3);
				cy = HexStringNumNToIntA(ptext+8, 3);
				cz = HexStringNumNToIntA(ptext+12, 1);
				ptext += 14;
				if (cy >= cx &&
#ifndef F06DAD_NOWLRANGELIMIT
						cx >= min && cy <= max &&
#endif
						len >= (29+(cy-cx+1)*cz) && *(ptext+(cy-cx+1)*cz) == ':') {
					sum = 0;
					ptmp = ptext;
					len = (cy-cx+1)*cz;		//number of characters for data
					while ((ptmp - ptext) < len)
						sum += *(ptmp++);
					ptmp++;		//skip ':'
					sum = (sum & 0xFFFFFF) - HexStringNumNToIntA(ptmp, 6);
					if (sum == 0) {
						//OK - read data
						len = cy-cx+1;
						for (i=0; i<len; i++) {
							x[i] = (double)(cx+i);
							sum = HexStringNumNToIntA(ptext, cz);
							switch (s_mode) {
								case _XF06DAD_SCANMODE_ABS:
									//absorbance
									if (cz <= 4) {		//model G
										e[i] = (sum >> 14) & 0x3;		//get error
										sign = (sum & 0x2000) ? 1 : 0;		//get sign '-' : '+'
										y[i] = (double)(sum & 0x1FFF);
									}
									else {
										e[i] = (sum >> 22) & 0x3;		//get error
										sign = (sum & 0x200000) ? 1 : 0;		//get sign '-' : '+'
										y[i] = (double)(sum & 0x1FFFFF) * 1.0e-2;
									}
									if (sign)
										y[i] *= -1.0;		//negative
									break;
								case _XF06DAD_SCANMODE_INT:
									//intensity
									e[i] = (sum >> 15) & 0x1;		//get error
									y[i] = (double)(sum & 0x7FFF) * 1.0e-2;
									break;
								case _XF06DAD_SCANMODE_IDX:
									//trasmission
									y[i] = (double)sum;
									e[i] = 0;		//get error
									break;
							}
							ptext += cz;
						}
						pD->p[_XF06DAD_TEMP_SCANSIZE].val.u = len;
						ret = 0;
						break;		//OK
					}
				}
			}
			ret = 2;		//error
			break;
		}
		pD->p[_XF06DAD_TEMP_SCANDATA].lerr = ret;
	}
	else
		ret = NO_ERROR;

	if (nn)
		*nn = pD->p[_XF06DAD_TEMP_SCANSIZE].val.u;
	if (xx)
		*xx = x;
	if (yy)
		*yy = y;
	if (ee)
		*ee = e;

	return(ret);
}

//------------------
/* start subscription scan */
DWORD F06DAD_StartSScan(HDEVICE hD)
{
	D_DEVICE *pD;
	F06DAD_SPARS *spars;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	spars = (F06DAD_SPARS *)pD->p[_XF06DAD_TEMP_SPARS].val.m.buf;
	if (spars == NULL)
		return(ERROR_INVALID_HANDLE);

	//reset
	spars->_count = 0;
	spars->_rest = 0;
	spars->_err = 0;

	//set smode
	*(char *)(pD->p[_XF06DAD_TEMP_SCANDATA].val.m.buf+1) = _XF06DAD_SCANMODE_SABS;

	//manage command
	strcpy(pD->com.sendbuf, "#SAs\n");
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "SAsERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "SAs", 3) == 0) {
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

/* stop subscription scan */
DWORD F06DAD_StopSScan(HDEVICE hD)
{
	return(F06DAD_SubscribeMode(hD, _XF06DAD_SUBSMODE_SABS, FALSE));
}

#define F06DAD_SSCAN_MINSIZE 8		//SAsERR4\n
#define F06DAD_SSCAN_MINHSIZE 32	//SAsX000S1N000Z4::00000009000000\n
//*** function gets subscription absorbance scan from buffer
DWORD F06DAD_GetSScan(HDEVICE hD, DWORD *nn, double **xx, double **yy, char **ee)
{
	D_DEVICE *pD;
	F06DAD_SPARS *spars;
	DWORD ret;
	unsigned int cx, cs, cn, cz, in, len, sum, j, k, n = 0;
	char *ptext, *pbuf, *ptmp, *e;
	double *x, *y;
	//int s_mode;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	ptext = (char *)pD->p[_XF06DAD_TEMP_COMBUFF].val.m.buf;
	spars = (F06DAD_SPARS *)pD->p[_XF06DAD_TEMP_SPARS].val.m.buf;
	if (ptext == NULL || spars == NULL)
		return(ERROR_INVALID_HANDLE);

	//s_mode = _XF06DAD_SCANMODE_SABS;
	x = (double *)(pD->p[_XF06DAD_TEMP_SCANDATA].val.m.buf+2);
	y = x + _XF06DAD_SCAN_MAX_SIZE;
	e = (char *)(y + _XF06DAD_SCAN_MAX_SIZE);

	//max len
	len = XF06DAD_MAX_RCPTLEN-spars->_rest;
	ptext = pD->com.rcptbuf;

	//read now
	ret = COM_Read(&pD->com, (char*)ptext+spars->_rest, len, &j, 0);
	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	len = j + spars->_rest;
	pbuf = ptext;
	//search through data
	while ((ptmp = memchr(pbuf, 'S', len - (pbuf-ptext)))) {
		pbuf = ptmp;
		j = len - (ptmp-ptext);

		//rest
		if (j < 3)
			break;		//not enought data
		//test for SAs
		if (memcmp(ptmp, "SAs", 3) != 0) {
			pbuf = ptmp+1;
			spars->_err++;
			continue;		//error
		}

		//header
		if (j < F06DAD_SSCAN_MINHSIZE)
			break;	//not enought data
		//check header format "SAsX000S3N000Z4:<data>:0000000900000000\n"
		//								 or "SAsX000S1N000Z4:<data>:00000009000000\n"
		if (*(ptmp+3) != 'X' || *(ptmp+7) != 'S' || *(ptmp+9) != 'N' || *(ptmp+13) != 'Z' || *(ptmp+15) != ':') {
			pbuf = ptmp+1;
			spars->_err++;
			continue;		//error
		}
		cx = HexStringNumNToIntA(ptmp+4, 3);
		cs = HexStringNumNToIntA(ptmp+8, 1);
		cn = HexStringNumNToIntA(ptmp+10, 3);
		cz = HexStringNumNToIntA(ptmp+14, 1);
		ptmp += 16;
		k = cn*cz;
		//test size
		if (j < (F06DAD_SSCAN_MINHSIZE+k))
			break;	//not enought data
		//test data
		if (*(ptmp+k+15) == '\n')
			in = 9;
		else if (*(ptmp+k+17) == '\n')
			in = 11;
		else
			in = 0;
		if (*(ptmp+k) != ':' || in == 0) {
			pbuf = ptmp+1;
			spars->_err++;
			continue;		//error
		}

		//check checksum
		sum = 0;
		for (j=0; j<k; j++)
			sum += *(ptmp+j);
		sum = (sum & 0xFFFFFF) - HexStringNumNToIntA(ptmp+k+1, 6);
		if (sum != 0) {
			pbuf = ptmp+1;
			spars->_err++;
			continue;		//error checksum
		}

		//OK - read data
		for (j=0; j<cn; j++) {
			//wl
			x[j] = (double)(cx);
			cx += cs;
			//abs
			sum = HexStringNumNToIntA(ptmp, cz);
			ptmp += cz;
			e[j] = (sum >> 14) & 0x3;		//get error
			if (sum & 0x2000)
				y[j] = -(double)(sum & 0x1FFF);
			else
				y[j] = (double)(sum & 0x1FFF);
		}
		n = cn;
		pD->p[_XF06DAD_TEMP_SCANSIZE].val.u = n;

		//OK - status
		ptmp += 7;

		spars->_count++;
		spars->_st = HexStringNumNToIntA(ptmp, 2);

		//next
		pbuf = ptmp+in;
	}

	//move data
	j = len - (pbuf-ptext);
	if (pbuf > ptext) {
		memmove(ptext, pbuf, j);
	}
	spars->_rest = j;

	//pointer data
	if (n) {
		if (nn)
			*nn = n;
		if (xx)
			*xx = x;
		if (yy)
			*yy = y;
		if (ee)
			*ee = e;
		return(0);
	}

	return(1);	//no data
}

/* get subscription scan params */
F06DAD_SPARS *F06DAD_GetSScanParams(HDEVICE hD)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(NULL);
	pD = (D_DEVICE *)hD;

	return((F06DAD_SPARS *)pD->p[_XF06DAD_TEMP_SPARS].val.m.buf);
}

/* make autozero during subscription scan */
DWORD F06DAD_MakeSScanAutozero(HDEVICE hD)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	strcpy(pD->com.sendbuf, "#ZRw\n");
	*pD->com.rcptbuf = '\0';
	//send and read
	return(F06DAD_SendStr(&pD->com));
}

//----------------
//*** function gets D-function
DWORD F06DAD_GetDFunction(HDEVICE hD, unsigned int *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	while (1) {
		t0 = timer_GetTime();
		//manage command
		strcpy(pD->com.sendbuf, "#ODr\n");
		//send and read
		ret = F06DAD_SendAndRcptStr(&pD->com);
		//average abs. time between request and response
		pD->p[_XF06DAD_DFCE].time = (t0 + timer_GetTime())/2.0;
		if (ret != NO_ERROR)
			break;		//error (timeout)

		//decode response
		if (strstr(pD->com.rcptbuf, "ODrERR") != NULL)
			ret = 1;		//value not available
		else if (strncmp(pD->com.rcptbuf, "ODr", 3) == 0) {
			pD->p[_XF06DAD_DFCE].val.u = atoi(pD->com.rcptbuf+3);
		}
		else
			ret = 2;		//none or bad response

		pD->p[_XF06DAD_DFCE].lerr = ret;
		break;
	}

	//???
	if (uval)
		*uval = pD->p[_XF06DAD_DFCE].val.u;
	if (time)
		*time = pD->p[_XF06DAD_DFCE].time;

	return(ret);
}

//*** function sets D-function
DWORD F06DAD_SetDFunction(HDEVICE hD, unsigned int uval)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	sprintf(pD->com.sendbuf, "#ODw%01d\n", uval);
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "ODwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "ODw", 3) == 0) {
		pD->p[_XF06DAD_DFCE].val.u = atoi(pD->com.rcptbuf+3);
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function gets D-threshold
DWORD F06DAD_GetDThreshold(HDEVICE hD, double *dval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	while (1) {
		t0 = timer_GetTime();
		//manage command
		strcpy(pD->com.sendbuf, "#OTr\n");
		//send and read
		ret = F06DAD_SendAndRcptStr(&pD->com);
		//average abs. time between request and response
		pD->p[_XF06DAD_DTHR].time = (t0 + timer_GetTime())/2.0;
		if (ret != NO_ERROR)
			break;		//error (timeout)

		//decode response
		if (strstr(pD->com.rcptbuf, "OTrERR") != NULL)
			ret = 1;		//value not available
		else if (strncmp(pD->com.rcptbuf, "OTr", 3) == 0) {
			pD->p[_XF06DAD_DTHR].val.d = (double)atoi(pD->com.rcptbuf+3) * 0.01;		//10uV -> mV
		}
		else
			ret = 2;		//none or bad response

		pD->p[_XF06DAD_DTHR].lerr = ret;
		break;
	}

	//???
	if (dval)
		*dval = pD->p[_XF06DAD_DTHR].val.d;
	if (time)
		*time = pD->p[_XF06DAD_DTHR].time;

	return(ret);
}

//*** function sets D-threshold
DWORD F06DAD_SetDThreshold(HDEVICE hD, double dval)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	sprintf(pD->com.sendbuf, "#OTw%05d\n", (unsigned int)(dval * 100.0 +0.5));		//mV -> 10uV
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "OTwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "OTw", 3) == 0) {
		pD->p[_XF06DAD_DTHR].val.d = (double)atoi(pD->com.rcptbuf+3) * 0.01;		//10uV -> mV
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//----------
//*** function gets auto-lamp
DWORD F06DAD_GetAutoLamp(HDEVICE hD, unsigned int *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	while (1) {
		t0 = timer_GetTime();
		//manage command
		strcpy(pD->com.sendbuf, "#ALr\n");
		//send and read
		ret = F06DAD_SendAndRcptStr(&pD->com);
		//average abs. time between request and response
		pD->p[_XF06DAD_ALAMP].time = (t0 + timer_GetTime())/2.0;
		if (ret != NO_ERROR)
			break;		//error (timeout)

		//decode response
		if (strstr(pD->com.rcptbuf, "ALrERR") != NULL)
			ret = 1;		//value not available
		else if (strncmp(pD->com.rcptbuf, "ALr", 3) == 0) {
			pD->p[_XF06DAD_ALAMP].val.u = atoi(pD->com.rcptbuf+3) ? 1 : 0;
		}
		else
			ret = 2;		//none or bad response

		pD->p[_XF06DAD_ALAMP].lerr = ret;
		break;
	}

	//???
	if (uval)
		*uval = pD->p[_XF06DAD_ALAMP].val.u;
	if (time)
		*time = pD->p[_XF06DAD_ALAMP].time;

	return(ret);
}

//*** function sets auto-lamp
DWORD F06DAD_SetAutoLamp(HDEVICE hD, unsigned int uval)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	sprintf(pD->com.sendbuf, "#ALw%d\n", uval ? 1 : 0);
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "ALwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "ALw", 3) == 0) {
		pD->p[_XF06DAD_ALAMP].val.u = atoi(pD->com.rcptbuf+3) ? 1 : 0;
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//----------

//*** function gets analog ratio
DWORD F06DAD_GetAnalogRatio(HDEVICE hD, unsigned int *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (!F06DAD_IsDisplay(hD)) {
		return(D_PERROR_NOTAVAILABLE);
	}

	while (1) {
		t0 = timer_GetTime();
		//manage command
		strcpy(pD->com.sendbuf, "#XGr\n");
		//send and read
		ret = F06DAD_SendAndRcptStr(&pD->com);
		//average abs. time between request and response
		pD->p[_XF06DAD_ARATIO].time = (t0 + timer_GetTime())/2.0;
		if (ret != NO_ERROR)
			break;		//error (timeout)

		//decode response
		if (strstr(pD->com.rcptbuf, "XGrERR") != NULL)
			ret = 1;		//value not available
		else if (strncmp(pD->com.rcptbuf, "XGr", 3) == 0) {
			pD->p[_XF06DAD_ARATIO].val.u = atoi(pD->com.rcptbuf+3);
		}
		else
			ret = 2;		//none or bad response

		pD->p[_XF06DAD_ARATIO].lerr = ret;
		break;
	}

	//???
	if (uval)
		*uval = pD->p[_XF06DAD_ARATIO].val.u;
	if (time)
		*time = pD->p[_XF06DAD_ARATIO].time;

	return(ret);
}

//*** function sets analog ratio
DWORD F06DAD_SetAnalogRatio(HDEVICE hD, unsigned int uval)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (!F06DAD_IsDisplay(hD)) {
		return(D_PERROR_NOTAVAILABLE);
	}

	//manage command
	sprintf(pD->com.sendbuf, "#XGw%d\n", uval);
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "XGwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "XGw", 3) == 0) {
		pD->p[_XF06DAD_ARATIO].val.u = atoi(pD->com.rcptbuf+3);
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function gets analog offset
DWORD F06DAD_GetAnalogOffset(HDEVICE hD, int how, WORD idx, double *dval, double *time)
{
	D_DEVICE *pD;
	DWORD ret, lidx;
	unsigned i;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (!F06DAD_IsDisplay(hD)) {
		return(D_PERROR_NOTAVAILABLE);
	}

	switch (idx) {
		default:
		case 0: lidx = _XF06DAD_AOFSA; break;
		case 1: lidx = _XF06DAD_AOFSB; break;
		case 2: lidx = _XF06DAD_AOFSC; break;
		case 3: lidx = _XF06DAD_AOFSD; break;
	}

	switch(how) {
		default:
		case 0:		//get lamp status from ram
			ret = NO_ERROR;
			break;

		case 1:		//read lamp status

			t0 = timer_GetTime();
			//manage command
			strcpy(pD->com.sendbuf, "#XOr\n");
			//send and read
			ret = F06DAD_SendAndRcptStr(&pD->com);
			//average abs. time between request and response
			pD->p[_XF06DAD_AOFSA].time = (t0 + timer_GetTime())/2.0;
			for (i=1; i<_XF06DAD_MAXCHANNELS; i++)
				pD->p[_XF06DAD_AOFSA+i].time = pD->p[_XF06DAD_AOFSA].time;

			if (ret != NO_ERROR)
				break;		//error (timeout)

			//decode response
			if (strstr(pD->com.rcptbuf, "XOrERR") != NULL)
				ret = 1;		//value not available
			else if (strncmp(pD->com.rcptbuf, "XOr", 3) == 0 &&
							 F06DAD_TestABC(pD->com.rcptbuf+3, _XF06DAD_MAXCHANNELS, 8, XF06DAD_ACHAR)) {		//test reception
				//read value from string ("XOrA+001200B+000000C+000000D+000000\n")
				for (i=0; i<_XF06DAD_MAXCHANNELS; i++) {
					pD->p[_XF06DAD_AOFSA+i].val.d = (double)atoi(pD->com.rcptbuf+4+i*8) * 0.01;		//-> mV
				}
			}
			else
				ret = 2;		//none or bad response

			for (i=0; i<_XF06DAD_MAXCHANNELS; i++)
				pD->p[_XF06DAD_AOFSA+i].lerr = ret;
			break;
	}

	//???
	if (dval)
		*dval = pD->p[lidx].val.d;
	if (time)
		*time = pD->p[lidx].time;

	return(ret);
}

//*** function pre-sets analog offset
DWORD F06DAD_PreSetAnalogOffset(HDEVICE hD, DWORD idx, double dval)
{
	F06DAD_SPARS *spars;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	spars = (F06DAD_SPARS *)((D_DEVICE *)hD)->p[_XF06DAD_TEMP_SPARS].val.m.buf;
	if (spars == NULL)
		return(ERROR_INVALID_HANDLE);

	if (idx < _XF06DAD_MAXCHANNELS) {
		spars->aofs[idx] = dval;
	}
	return(NO_ERROR);
}

//*** function sets analog offset
DWORD F06DAD_SetAnalogOffset(HDEVICE hD, DWORD idx, double dval)
{
	D_DEVICE *pD;
	DWORD ret;
	unsigned i;
	F06DAD_SPARS *spars;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (!F06DAD_IsDisplay(hD)) {
		return(D_PERROR_NOTAVAILABLE);
	}

	//pre-sets
	ret = F06DAD_PreSetAnalogOffset(hD, idx, dval);
	if (ret != NO_ERROR)
		return(ret);

	spars = (F06DAD_SPARS *)pD->p[_XF06DAD_TEMP_SPARS].val.m.buf;

	//manage command
	sprintf(pD->com.sendbuf, "#XOw");
	for (i=0; i<_XF06DAD_MAXCHANNELS; i++) {
		sprintf(pD->com.sendbuf, "%s%c%+07.0lf",
						pD->com.sendbuf,
						XF06DAD_ACHAR+i,
						spars->aofs[i] * 100.0);
	}
	strcat(pD->com.sendbuf, "\n");
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "WLwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "WLw", 3) == 0 &&
					 F06DAD_TestABC(pD->com.rcptbuf+3, _XF06DAD_MAXCHANNELS, 8, XF06DAD_ACHAR)) {		//test reception
		//read value from string ("XOrA+001200B+000000C+000000D+000000\n")
		for (i=0; i<_XF06DAD_MAXCHANNELS; i++) {
			pD->p[_XF06DAD_AOFSA+i].val.d = (double)atoi(pD->com.rcptbuf+4+i*8) * 0.01;		//-> mV
		}
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function gets brightness
DWORD F06DAD_GetBrightness(HDEVICE hD, unsigned int *uval, double *time)
{
	D_DEVICE *pD;
	DWORD ret;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (!F06DAD_IsDisplay(hD)) {
		return(D_PERROR_NOTAVAILABLE);
	}

	while (1) {
		t0 = timer_GetTime();
		//manage command
		strcpy(pD->com.sendbuf, "#XBr\n");
		//send and read
		ret = F06DAD_SendAndRcptStr(&pD->com);
		//average abs. time between request and response
		pD->p[_XF06DAD_BRIGHT].time = (t0 + timer_GetTime())/2.0;
		if (ret != NO_ERROR)
			break;		//error (timeout)

		//decode response
		if (strstr(pD->com.rcptbuf, "XBrERR") != NULL)
			ret = 1;		//value not available
		else if (strncmp(pD->com.rcptbuf, "XBr", 3) == 0) {
			pD->p[_XF06DAD_BRIGHT].val.u = atoi(pD->com.rcptbuf+3) - 1;
		}
		else
			ret = 2;		//none or bad response

		pD->p[_XF06DAD_BRIGHT].lerr = ret;
		break;
	}

	//???
	if (uval)
		*uval = pD->p[_XF06DAD_BRIGHT].val.u;
	if (time)
		*time = pD->p[_XF06DAD_BRIGHT].time;

	return(ret);
}

//*** function sets brightness
DWORD F06DAD_SetBrightness(HDEVICE hD, unsigned int uval)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (!F06DAD_IsDisplay(hD)) {
		return(D_PERROR_NOTAVAILABLE);
	}

	//manage command
	sprintf(pD->com.sendbuf, "#XBw%d\n", uval+1);
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "XBwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "XBw", 3) == 0) {
		pD->p[_XF06DAD_BRIGHT].val.u = atoi(pD->com.rcptbuf+3);
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//----------------------------------------------------------
//*** function sets oper property by index
DWORD F06DAD_MakeOper(HDEVICE hD, WORD oidx)
{
	D_DEVICE *pD;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->n_op && pD->op) {
			switch (oidx) {
				case _XF06DAD_AUTOZERO: return(F06DAD_RunAutozero(hD));
				case _XF06DAD_BEEP: return(F06DAD_MakeBeep(hD, 2));
			}
		}
	}
	return NO_ERROR;
}

//----------------------------------------------------------
//*** function runs autozero
DWORD F06DAD_RunAutozero(HDEVICE hD)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//manage command
	strcpy(pD->com.sendbuf, "#ZRw\n");
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);

	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "ZRwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "ZRw", 3) == 0) {
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function makes beep
DWORD F06DAD_MakeBeep(HDEVICE hD, unsigned type)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (type < 1)
		type = 1;
	else if (type > 3)
		type = 3;

	//manage command
	sprintf(pD->com.sendbuf, "#SGw%d\n", type);
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);
	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, "SGwERR", 6) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, "SGw", 3) == 0) {
	}
	else
		ret = 2;		//none or bad response

	return(ret);
}

//*** function sets/unsets abs. subscribe mode
DWORD F06DAD_SubscribeMode(HDEVICE hD, int mode, BOOL state)
{
	D_DEVICE *pD;
	DWORD ret;
	const char *c_str;
	unsigned int nr;
	double t0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	switch (mode) {
		default:
		case 0: c_str = "AB"; break;
		case 1: c_str = "IT"; break;
		case 2: c_str = "SA"; break;
	}

	//manage command
	sprintf(pD->com.sendbuf, "#%s%c\n", c_str, state ? 's' : 'u');
	//send and read
	ret = F06DAD_SendAndRcptStr(&pD->com);
	if (ret != NO_ERROR)
		return(ret);		//error (timeout)

	//decode response
	if (strncmp(pD->com.rcptbuf, pD->com.sendbuf, 3) == 0 && strncmp(pD->com.rcptbuf+3, "ERR", 3) == 0)
		ret = 1;		//operation wasn't successful
	else if (strncmp(pD->com.rcptbuf, c_str, 2) == 0) {
	}
	else
		ret = 2;		//none or bad response

	//stop subscription?
	if (!state) {
		//wait for no data
		t0 = timer_GetTime();
		do {
			//flush
			COM_Flush(&pD->com);
			//wait
			Sleep(100);
			//read
			nr = 0;
			COM_Read(&pD->com, pD->com.rcptbuf, 128, &nr, 0);
		} while (nr > 0 && (timer_GetTime()-t0) < 5000.0);
	}

	return(ret);
}


//--------------- Device Dialogs --------------------------------------------------
//*** function processes setup dialog - parameters subdialog
BOOL CALLBACK F06DAD_SetupParamDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static HDEVICE hD = NULL;
	D_DEVICE *pD;
	D_VAL min, max;
	unsigned int i, j, w, nw;
	DWORD wlmax;

	switch(msg) {
		case WM_INITDIALOG:
			if (lparam) {
				hD = (HDEVICE)lparam;
				pD = (D_DEVICE *)hD;

				//create & initiate parameters

				//group box
				i = 0;
				D_CreateParamGroup(hdlg, D_("Model parameters"), 0, i, 2, 1);
				//model
				D_CreateParamCtrl(hdlg, _XF06DAD_MODEL, pD, D_CTRLSTYLE_PROP, 0 | D_GUICTRL_RC_2, i++);

				i += 2;
				D_CreateParamGroup(hdlg, D_("Basic parameters"), 0, i, 2, 6);
				//wl
				for (w=0; w<_XF06DAD_MAXCHANNELS; w++) {
					D_CreateParamCtrl(hdlg, _XF06DAD_WLA+w, pD, D_CTRLSTYLE_PROP, 0, i+w);
				}
				//half width
				D_CreateParamCtrl(hdlg, _XF06DAD_HALF, pD, D_CTRLSTYLE_PROP, 1, i++);
				//filter
				D_CreateParamCtrl(hdlg, _XF06DAD_FILTER, pD, D_CTRLSTYLE_PROP, 1, i++);
				//D-fce
				D_CreateParamCtrl(hdlg, _XF06DAD_DFCE, pD, D_CTRLSTYLE_PROP, 1, i++);
				//D-threshold
				D_CreateParamCtrl(hdlg, _XF06DAD_DTHR, pD, D_CTRLSTYLE_PROP, 1, i++);
				//injzero
				D_CreateParamCtrl(hdlg, _XF06DAD_INJZERO, pD, D_CTRLSTYLE_PROP, 1, i++);
				//auto-lamp
				D_CreateParamCtrl(hdlg, _XF06DAD_ALAMP, pD, D_CTRLSTYLE_PROP, 1, i++);

				i += 2;
				j = i;
				D_CreateParamGroup(hdlg, D_("Scan parameters"), 0, i, 1, 6);
				//wl-range
				D_CreateParamCtrl(hdlg, _XF06DAD_WLSCANRANGE, pD, D_CTRLSTYLE_PROP, 0, i++);
				//text by
				D_CreateParamCtrl(hdlg, _XF06DAD_TEXTBY, pD, D_CTRLSTYLE_PROP, 0, i++);
				//grid by
				D_CreateParamCtrl(hdlg, _XF06DAD_GRIDBY, pD, D_CTRLSTYLE_PROP, 0, i++);

				i = j;
				D_CreateParamGroup(hdlg, D_("Optional parameters"), 1, i, 1, _XF06DAD_MAXCHANNELS+2);
				//ratio
				D_CreateParamCtrl(hdlg, _XF06DAD_ARATIO, pD, D_CTRLSTYLE_PROP, 1, i++);
				//offset
				for (w=0; w<_XF06DAD_MAXCHANNELS; w++) {
					D_CreateParamCtrl(hdlg, _XF06DAD_AOFSA+w, pD, D_CTRLSTYLE_PROP, 1, i++);
				}
				//brightness
				D_CreateParamCtrl(hdlg, _XF06DAD_BRIGHT, pD, D_CTRLSTYLE_PROP, 1, i++);

				//enable/disable
				SendMessage(hdlg, WMD_USER+0, 0, 0);	//by model
				SendMessage(hdlg, WMD_USER+1, 0, 0);	//by D-fce
			}
			break;

		case WMD_ENABLE:
			//disable model
			EnableWindow(D_GETOLH_VAL(hdlg, _XF06DAD_MODEL), wparam);
			return TRUE;

		case WMD_UPDATE:
			pD = (D_DEVICE *)hD;
  		if (pD == NULL || pD->p == NULL)
				return TRUE;
			D_SetParamCtrl(hdlg, _XF06DAD_MODEL, pD);
			D_SetParamCtrl(hdlg, _XF06DAD_DFCE, pD);
			SendMessage(hdlg, WMD_USER+0, 0, 0);		//by model
			SendMessage(hdlg, WMD_USER+1, 0, 0);		//by D-fce
			D_SetAllParamCtrls(hdlg, (D_DEVICE *)hD);
       return TRUE;

		case WMD_APPLY:
      break;

    case (WMD_USER+0):	//modify by model
      pD = (D_DEVICE *)hD;
  		if (pD == NULL || pD->p == NULL)
				return TRUE;
			//get model
			i = SendMessage(D_GETOLH_VAL(hdlg, _XF06DAD_MODEL), CB_GETCURSEL, 0, 0);
			if (i >= XF06DAD_MAXMODELS)
				i = XF06DAD_MODELE;
			//wl limits
			wlmax = F06DAD_GetWavelengthByModel(i);
			//enable/disable wavelengths
			nw = F06DAD_GetChannelsByModel(i);

      //wl
      for (w=0; w<_XF06DAD_MAXCHANNELS; w++) {
				min.d = (double)_XF06DAD_WLMIN;
				max.d = (double)wlmax;
				D_ModifyParamCtrlSB(hdlg, pD, _XF06DAD_WLA+w, &min, &max, NULL);
				EnableWindow(D_GETOLH_VAL(hdlg, _XF06DAD_WLA+w), nw > w ? TRUE : FALSE);
      }

      //scan-wl
			min.u = _XF06DAD_WLMIN | (_XF06DAD_WLMIN << 16);
			max.u = (wlmax & 0xFFFF) | (wlmax << 16);
			D_ModifyRangeControls(hdlg, pD, _XF06DAD_WLSCANRANGE, &min, &max);

      //analog offset
 			nw = F06DAD_GetChannelsByModel(i);
			for (w=0; w<_XF06DAD_MAXCHANNELS; w++) {
				EnableWindow(D_GETOLH_VAL(hdlg, _XF06DAD_AOFSA+w), nw > w ? TRUE : FALSE);
      }

      return TRUE;

    case (WMD_USER+1):	//modify by D-fce
      pD = (D_DEVICE *)hD;
  		if (pD == NULL || pD->p == NULL)
				return TRUE;
			//get model
			i = SendMessage(D_GETOLH_VAL(hdlg, _XF06DAD_DFCE), CB_GETCURSEL, 0, 0);
			EnableWindow(D_GETOLH_VAL(hdlg, _XF06DAD_DTHR), i == _XF06DAD_DFCE_IDX_ADIVB ? TRUE : FALSE);		//enable for A/B
      return TRUE;

		case WM_COMMAND:
			if (lparam) {
				if (HIWORD(wparam) == CBN_SELCHANGE) {
					switch(LOWORD(wparam)) {
						//by model
						case D_GETOLID_VAL(_XF06DAD_MODEL):
							SendMessage(hdlg, WMD_USER+0, 0, 0);
							break;
						//by D-fce
						case D_GETOLID_VAL(_XF06DAD_DFCE):
							SendMessage(hdlg, WMD_USER+1, 0, 0);
							break;
						default: break;
					}
				}
				else if (HIWORD(wparam) == EN_KILLFOCUS) {

				}
			}
			break;

/*
		case WM_COMMAND:
			if (lparam) {
				if (HIWORD(wparam) == EN_KILLFOCUS) {
					switch(LOWORD(wparam)) {
						case 13:
							//wl max limit
							wlmax = F06DAD_GetMaxWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
							//get low
							hctl = GetDlgItem(hdlg, 17);
							GetEditUval(hctl, &nval);
							//get high
							hctl = GetDlgItem(hdlg, 19);
							GetEditUval(hctl, &nval2);
							//correct
							if (nval2.u <= nval.u) {
								if (nval2.u < wlmax) {
									nval2.u = nval.u + 1;
									SetEditUval(hctl, nval2);		//set high
									GetEditUval(hctl, &nval2);
								}
								if (nval.u >= nval2.u) {
									nval.u = nval2.u - 1;
									SetEditUval(GetDlgItem(hdlg, 17), nval);		//set low
								}
							}
							return TRUE;
						case 17:
							//get high
							hctl = GetDlgItem(hdlg, 19);
							GetEditUval(hctl, &nval2);
							//get low
							hctl = GetDlgItem(hdlg, 17);
							GetEditUval(hctl, &nval);
							//correct
							if (nval.u >= nval2.u) {
								if (nval.u > _XF06DAD_WLMIN) {
									nval.u = nval2.u - 1;
									SetEditUval(hctl, nval);		//set low
									GetEditUval(hctl, &nval);
								}
								if (nval2.u <= nval.u) {
									nval2.u = nval.u + 1;
									SetEditUval(GetDlgItem(hdlg, 19), nval2);		//set high
								}
							}
							return TRUE;
						default:
							break;
					}
				}
			}
			break;
*/

		case WM_CLOSE:
			return TRUE;

		case WM_DESTROY:
			hD = NULL;
			pD = NULL;
			break;

	}
	return(D_RestSetupDlgProc(hdlg, msg, wparam, lparam, (D_DEVICE *)hD));
}
/*
{
	#define F06DAD_GSKIP 80
	static HDEVICE hD = NULL;
	static D_DEVICE *pD;
	static HWND hctl;
	U_NVAL nval, nval2;
	TCHAR ttext[128];
	unsigned i;
	DWORD wlmax;

	switch(msg) {
		case WM_INITDIALOG:
			if (lparam) {
				hD = (HDEVICE)lparam;
				pD = (D_DEVICE *)hD;

				//create & initiate parameters

				//group box
				i = 0;
				hctl = CreateWindow(CLASS_BUTTON, D_("Writable parameters"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
														8, 10+i*22, 272+F06DAD_GSKIP, 32+10*22, hdlg, (HMENU)100, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

				//model
				D_MakePropLabel(&pD->p[_XF06DAD_MODEL], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)2, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindow(CLASS_COMBOBOX, TEXT(""),
														WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
														150, 33+i*22, 120+F06DAD_GSKIP, 120, hdlg, (HMENU)3, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				D_PrepareCombobox(hctl, &pD->p[_XF06DAD_MODEL]);
				wlmax = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);

				//wavelength A
				i = 1;
				D_MakePropLabel(&pD->p[_XF06DAD_WLA], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)4, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
														150, 33+i*22, 120, 20, hdlg, (HMENU)5, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUDBL | 0 | U_NFLAGMIN | U_NFLAGMAX,
																	(double)_XF06DAD_WLMIN, (double)wlmax);
				nval.d = pD->p[_XF06DAD_WLA].val.d;
				SetEditUval(hctl, nval);

				//wavelength B
				i = 2;
				D_MakePropLabel(&pD->p[_XF06DAD_WLB], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)6, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
														150, 33+i*22, 120, 20, hdlg, (HMENU)7, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUDBL | 0 | U_NFLAGMIN | U_NFLAGMAX,
																	(double)_XF06DAD_WLMIN, (double)wlmax);
				nval.d = pD->p[_XF06DAD_WLB].val.d;
				SetEditUval(hctl, nval);

				//wavelength C
				i = 3;
				D_MakePropLabel(&pD->p[_XF06DAD_WLC], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)8, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
														150, 33+i*22, 120, 20, hdlg, (HMENU)9, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUDBL | 0 | U_NFLAGMIN | U_NFLAGMAX,
																	(double)_XF06DAD_WLMIN, (double)wlmax);
				nval.d = pD->p[_XF06DAD_WLC].val.d;
				SetEditUval(hctl, nval);

				//wavelength D
				i = 4;
				D_MakePropLabel(&pD->p[_XF06DAD_WLD], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)10, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
														150, 33+i*22, 120, 20, hdlg, (HMENU)11, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUDBL | 0 | U_NFLAGMIN | U_NFLAGMAX,
																	(double)_XF06DAD_WLMIN, (double)wlmax);
				nval.d = pD->p[_XF06DAD_WLD].val.d;
				SetEditUval(hctl, nval);

				//half width
				i = 5;
				D_MakePropLabel(&pD->p[_XF06DAD_HALF], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)12, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
														150, 33+i*22, 120, 20, hdlg, (HMENU)13, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUINT | 0 | U_NFLAGMIN | U_NFLAGMAX,
																	_XF06DAD_HALFMIN, _XF06DAD_HALFMAX);
				nval.u = pD->p[_XF06DAD_HALF].val.u;
				SetEditUval(hctl, nval);

				//filter
				i = 6;
				D_MakePropLabel(&pD->p[_XF06DAD_FILTER], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)14, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindow(CLASS_COMBOBOX, TEXT(""),
														WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
														150, 33+i*22, 120, 120, hdlg, (HMENU)15, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				D_PrepareCombobox(hctl, &pD->p[_XF06DAD_FILTER]);

				//D-fce
				i = 7;
				D_MakePropLabel(&pD->p[_XF06DAD_DFCE], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)30, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindow(CLASS_COMBOBOX, TEXT(""),
														WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
														150, 33+i*22, 120, 120, hdlg, (HMENU)31, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				D_PrepareCombobox(hctl, &pD->p[_XF06DAD_DFCE]);

				//D-threshold
				i = 8;
				D_MakePropLabel(&pD->p[_XF06DAD_DTHR], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)32, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
														150, 33+i*22, 120, 20, hdlg, (HMENU)33, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				D_PrepareValidNumSubclassing(hctl, &pD->p[_XF06DAD_DTHR]);

				//inj. autozero
				i = 9;
				D_MakePropLabel(&pD->p[_XF06DAD_INJZERO], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)34, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindow(CLASS_COMBOBOX, TEXT(""),
														WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
														150, 33+i*22, 120, 120, hdlg, (HMENU)35, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				D_PrepareCombobox(hctl, &pD->p[_XF06DAD_INJZERO]);

				//group box
				i = 12;
				hctl = CreateWindow(CLASS_BUTTON, D_("Scan parameters"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
														8, 10+i*22, 272+F06DAD_GSKIP, 32+3*22, hdlg, (HMENU)101, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

				//scan range
				i = 12;
				D_MakePropLabel(&pD->p[_XF06DAD_WLSCANRANGE], ttext, 0x3);
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)16, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//min
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
														150, 33+i*22, 50, 20, hdlg, (HMENU)17, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUINT | 0 | U_NFLAGMIN | U_NFLAGMAX,
																	_XF06DAD_WLMIN, wlmax);
				nval.u = (pD->p[_XF06DAD_WLSCANRANGE].val.u & 0xFFFF);
				SetEditUval(hctl, nval);
				hctl = CreateWindow(CLASS_STATIC, TEXT("---"), WS_CHILD | WS_VISIBLE | SS_CENTER,
														201, 33+i*22+3, 18, 20, hdlg, (HMENU)18, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//max
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
														220, 33+i*22, 50, 20, hdlg, (HMENU)19, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUINT | 0 | U_NFLAGMIN | U_NFLAGMAX,
																	_XF06DAD_WLMIN, wlmax);
				nval.u = (pD->p[_XF06DAD_WLSCANRANGE].val.u >> 16);
				SetEditUval(hctl, nval);

				//texts
				i = 13;
				__sprintf(ttext, TEXT("%s:"), D_("Show texts by"));
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)20, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindow(CLASS_COMBOBOX, TEXT(""),
														WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
														150, 33+i*22, 120, 120, hdlg, (HMENU)21, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("None"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Wavelength"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Value"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Both"));
				SendMessage(hctl, CB_SETCURSEL, pD->p[_XF06DAD_TEMP_CONF].val.u & 0x03, 0);
				//grids
				i = 14;
				__sprintf(ttext, TEXT("%s:"), D_("Show grids by"));
				hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE | SS_RIGHT,
														18, 33+i*22+3, 150-18-10, 20, hdlg, (HMENU)22, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindow(CLASS_COMBOBOX, TEXT(""),
														WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
														150, 33+i*22, 120, 120, hdlg, (HMENU)23, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("None"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)TEXT("X"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)TEXT("Y"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Both"));
				SendMessage(hctl, CB_SETCURSEL, (pD->p[_XF06DAD_TEMP_CONF].val.u >> 8) & 0x03, 0);

				//enable/disable WLs
				SendMessage(hdlg, WM_COMMAND, MAKELONG(3, CBN_SELCHANGE), (LPARAM)GetDlgItem(hdlg, 3));

				//enable/disable D-threshold
				SendMessage(hdlg, WM_COMMAND, MAKELONG(31, CBN_SELCHANGE), (LPARAM)GetDlgItem(hdlg, 31));

				ScaleChildWindowsByScreen(hdlg);		//scale window
			}
			break;


		case WMD_ENABLE:
			//disable model
			EnableWindow(GetDlgItem(hdlg, 3), wparam);
			return TRUE;

		case WMD_UPDATE:
			//update
			if (pD->p && pD->n_p) {
				hctl = GetDlgItem(hdlg, 3);
				SendMessage(hctl, CB_SETCURSEL, pD->p[_XF06DAD_MODEL].val.u, 0);
				SendMessage(hdlg, WM_COMMAND, MAKELONG(3, CBN_SELCHANGE), (LPARAM)hctl);
				nval.d = pD->p[_XF06DAD_WLA].val.d;
				SetEditUval(GetDlgItem(hdlg, 5), nval);
				nval.d = pD->p[_XF06DAD_WLB].val.d;
				SetEditUval(GetDlgItem(hdlg, 7), nval);
				nval.d = pD->p[_XF06DAD_WLC].val.d;
				SetEditUval(GetDlgItem(hdlg, 9), nval);
				nval.d = pD->p[_XF06DAD_WLD].val.d;
				SetEditUval(GetDlgItem(hdlg, 11), nval);
				nval.u = pD->p[_XF06DAD_HALF].val.u;
				SetEditUval(GetDlgItem(hdlg, 13), nval);
				SendMessage(GetDlgItem(hdlg, 15), CB_SETCURSEL, pD->p[_XF06DAD_FILTER].val.u, 0);
				nval.u = (pD->p[_XF06DAD_WLSCANRANGE].val.u & 0xFFFF);
				SetEditUval(GetDlgItem(hdlg, 17), nval);
				nval.u = (pD->p[_XF06DAD_WLSCANRANGE].val.u >> 16);
				SetEditUval(GetDlgItem(hdlg, 19), nval);
				SendMessage(GetDlgItem(hdlg, 21), CB_SETCURSEL, pD->p[_XF06DAD_TEMP_CONF].val.u & 0x03, 0);
				SendMessage(GetDlgItem(hdlg, 23), CB_SETCURSEL, (pD->p[_XF06DAD_TEMP_CONF].val.u >> 8) & 0x03, 0);
				SendMessage(GetDlgItem(hdlg, 31), CB_SETCURSEL, pD->p[_XF06DAD_DFCE].val.u, 0);
				nval.d = pD->p[_XF06DAD_DTHR].val.d;
				SetEditUval(GetDlgItem(hdlg, 33), nval);
				SendMessage(GetDlgItem(hdlg, 35), CB_SETCURSEL, pD->p[_XF06DAD_INJZERO].val.u, 0);

				SendMessage(hdlg, WM_COMMAND, MAKELONG(3, CBN_SELCHANGE), (LPARAM)GetDlgItem(hdlg, 3));
				SendMessage(hdlg, WM_COMMAND, MAKELONG(31, CBN_SELCHANGE), (LPARAM)GetDlgItem(hdlg, 31));
			}
			return TRUE;

		case WMD_APPLY:
			//apply
			if (pD->p && pD->n_p) {
				pD->p[_XF06DAD_MODEL].val.u = SendMessage(GetDlgItem(hdlg, 3), CB_GETCURSEL, 0, 0);
				GetEditUval(GetDlgItem(hdlg, 5), &nval);
				pD->p[_XF06DAD_WLA].val.d = nval.d;
				GetEditUval(GetDlgItem(hdlg, 7), &nval);
				pD->p[_XF06DAD_WLB].val.d = nval.d;
				GetEditUval(GetDlgItem(hdlg, 9), &nval);
				pD->p[_XF06DAD_WLC].val.d = nval.d;
				GetEditUval(GetDlgItem(hdlg, 11), &nval);
				pD->p[_XF06DAD_WLD].val.d = nval.d;
				GetEditUval(GetDlgItem(hdlg, 13), &nval);
				pD->p[_XF06DAD_HALF].val.u = nval.u;
				pD->p[_XF06DAD_FILTER].val.u = SendMessage(GetDlgItem(hdlg, 15), CB_GETCURSEL, 0, 0);
				GetEditUval(GetDlgItem(hdlg, 17), &nval);
				GetEditUval(GetDlgItem(hdlg, 19), &nval2);
				pD->p[_XF06DAD_WLSCANRANGE].val.u = nval.u | (nval2.u << 16);
				i = SendMessage(GetDlgItem(hdlg, 21), CB_GETCURSEL, 0, 0);
				pD->p[_XF06DAD_TEMP_CONF].val.u = (pD->p[_XF06DAD_TEMP_CONF].val.u & ~0x00FF) | i;
				i = SendMessage(GetDlgItem(hdlg, 23), CB_GETCURSEL, 0, 0);
				pD->p[_XF06DAD_TEMP_CONF].val.u = (pD->p[_XF06DAD_TEMP_CONF].val.u & ~0xFF00) | (i << 8);
				pD->p[_XF06DAD_DFCE].val.u = SendMessage(GetDlgItem(hdlg, 31), CB_GETCURSEL, 0, 0);
				GetEditUval(GetDlgItem(hdlg, 33), &nval);
				pD->p[_XF06DAD_DTHR].val.d = nval.d;
				pD->p[_XF06DAD_INJZERO].val.u = SendMessage(GetDlgItem(hdlg, 35), CB_GETCURSEL, 0, 0);
			}
			return TRUE;

		case WM_COMMAND:
			if (lparam) {
				if (HIWORD(wparam) == EN_KILLFOCUS) {
					switch(LOWORD(wparam)) {
						case 13:
							//wl max limit
							wlmax = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
							//get low
							hctl = GetDlgItem(hdlg, 17);
							GetEditUval(hctl, &nval);
							//get high
							hctl = GetDlgItem(hdlg, 19);
							GetEditUval(hctl, &nval2);
							//correct
							if (nval2.u <= nval.u) {
								if (nval2.u < wlmax) {
									nval2.u = nval.u + 1;
									SetEditUval(hctl, nval2);		//set high
									GetEditUval(hctl, &nval2);
								}
								if (nval.u >= nval2.u) {
									nval.u = nval2.u - 1;
									SetEditUval(GetDlgItem(hdlg, 17), nval);		//set low
								}
							}
							return TRUE;
						case 17:
							//get high
							hctl = GetDlgItem(hdlg, 19);
							GetEditUval(hctl, &nval2);
							//get low
							hctl = GetDlgItem(hdlg, 17);
							GetEditUval(hctl, &nval);
							//correct
							if (nval.u >= nval2.u) {
								if (nval.u > _XF06DAD_WLMIN) {
									nval.u = nval2.u - 1;
									SetEditUval(hctl, nval);		//set low
									GetEditUval(hctl, &nval);
								}
								if (nval2.u <= nval.u) {
									nval2.u = nval.u + 1;
									SetEditUval(GetDlgItem(hdlg, 19), nval2);		//set high
								}
							}
							return TRUE;
						default:
							break;
					}
				}
				else if (HIWORD(wparam) == CBN_SELCHANGE) {
					switch(LOWORD(wparam)) {
						case 3:
							//get index
							i = SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
							//wl limits
							wlmax = F06DAD_GetWavelengthByModel(i);
							//enable/disable wavelengths
							i = F06DAD_GetChannelsByModel(i);
							//WLA every time
							hctl = GetDlgItem(hdlg, 5);
							EnableWindow(hctl, i >= 1 ? TRUE : FALSE);
							UpdateValidNumStrSubclassing(hctl, U_NFLAGUDBL | 0 | U_NFLAGMIN | U_NFLAGMAX,
																					 (double)_XF06DAD_WLMIN, (double)wlmax);
							//WLB
							hctl = GetDlgItem(hdlg, 7);
							EnableWindow(hctl, i >= 2 ? TRUE : FALSE);
							UpdateValidNumStrSubclassing(hctl, U_NFLAGUDBL | 0 | U_NFLAGMIN | U_NFLAGMAX,
																					 (double)_XF06DAD_WLMIN, (double)wlmax);
							//WLC
							hctl = GetDlgItem(hdlg, 9);
							EnableWindow(hctl, i >= 3 ? TRUE : FALSE);
							UpdateValidNumStrSubclassing(hctl, U_NFLAGUDBL | 0 | U_NFLAGMIN | U_NFLAGMAX,
																					 (double)_XF06DAD_WLMIN, (double)wlmax);
							//WLD
							hctl = GetDlgItem(hdlg, 11);
							EnableWindow(hctl, i >= 4 ? TRUE : FALSE);
							UpdateValidNumStrSubclassing(hctl, U_NFLAGUDBL | 0 | U_NFLAGMIN | U_NFLAGMAX,
																					 (double)_XF06DAD_WLMIN, (double)wlmax);
							//min range
							hctl = GetDlgItem(hdlg, 17);
							UpdateValidNumStrSubclassing(hctl, U_NFLAGUINT | 0 | U_NFLAGMIN | U_NFLAGMAX,
																					 _XF06DAD_WLMIN, wlmax);		//scan range MIN
							//max range
							hctl = GetDlgItem(hdlg, 19);
							UpdateValidNumStrSubclassing(hctl, U_NFLAGUINT | 0 | U_NFLAGMIN | U_NFLAGMAX,
																					 _XF06DAD_WLMIN, wlmax);		//scan range MAX
							break;
						case 31:		//d-fce
							//get index
							i = SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
							//enable/disable d-threshold
							hctl = GetDlgItem(hdlg, 33);
							EnableWindow(hctl, i == _XF06DAD_DFCE_IDX_ADIVB ? TRUE : FALSE);		//enable for A/B
							break;
						default:
							break;
					}
				}
			}
			break;

		case WM_CLOSE:
			return TRUE;

		case WM_DESTROY:
			//clear memory
			ClearValidNumStrSubclassing(GetDlgItem(hdlg, 5));
			ClearValidNumStrSubclassing(GetDlgItem(hdlg, 7));
			ClearValidNumStrSubclassing(GetDlgItem(hdlg, 9));
			ClearValidNumStrSubclassing(GetDlgItem(hdlg, 11));
			ClearValidNumStrSubclassing(GetDlgItem(hdlg, 13));
			ClearValidNumStrSubclassing(GetDlgItem(hdlg, 17));
			ClearValidNumStrSubclassing(GetDlgItem(hdlg, 19));
			ClearValidNumStrSubclassing(GetDlgItem(hdlg, 33));

			hD = NULL;
			pD = NULL;
			break;

	}
	return FALSE;
}
*/


//*** function processes setup dialog - parameters subdialog
BOOL CALLBACK F06DAD_SetupInitDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static HDEVICE hD = NULL;
	D_DEVICE *pD;
	int i;

	switch(msg) {
		case WM_INITDIALOG:
			if (lparam) {
				hD = (HDEVICE)lparam;
				pD = (D_DEVICE *)hD;
				if (pD->n_p == 0 || pD->p == NULL)
					break;

				//create & initiate parameters

				//--- init ---
				i = 0;
				D_CreateParamGroup(hdlg, D_("Start-up parameters"), 0, i, 1, 3);
				//init. lamp
				D_CreateParamCtrlParam(hdlg, _XF06DAD_INIT_LAMP, pD, D_CTRLSTYLE_PROP, D_("Set lamp on"), 0, i++);
				//init. autozero
				D_CreateParamCtrlParam(hdlg, _XF06DAD_INIT_AZERO, pD, D_CTRLSTYLE_PROP, D_("Delayed autozero"), 0, i++);
				//autozero delay
				D_CreateParamCtrlParam(hdlg, _XF06DAD_INIT_AZERODELAY, pD, D_CTRLSTYLE_PROP, D_("Delay [s]"), 0, i++);


				//--- deinit ---
				i = 0;
				D_CreateParamGroup(hdlg, D_("Stop parameters"), 1, i, 1, 3);
				//deinit. lamps
				D_CreateParamCtrlParam(hdlg, _XF06DAD_DEIN_LAMP, pD, D_CTRLSTYLE_PROP, D_("Set lamp off"), 1, i++);

				//
				SendMessage(hdlg, WMD_USER+0, 0, 0);		//delay
			}
			break;

		case WMD_UPDATE:
			pD = (D_DEVICE *)hD;
  		if (pD == NULL || pD->p == NULL)
				return TRUE;
			D_SetAllParamCtrls(hdlg, (D_DEVICE *)hD);
			SendMessage(hdlg, WMD_USER+0, 0, 0);		//delay
			return TRUE;

    case (WMD_USER+0):	//delay enable/disable
      pD = (D_DEVICE *)hD;
  		if (pD == NULL || pD->p == NULL)
				return TRUE;
			//
			i = IsDlgButtonChecked(hdlg, D_GETOLID_VAL(_XF06DAD_INIT_LAMP));
			EnableWindow(D_GETOLH_VAL(hdlg, _XF06DAD_INIT_AZERO), i ? TRUE : FALSE);
			i &= IsDlgButtonChecked(hdlg, D_GETOLID_VAL(_XF06DAD_INIT_AZERO));
			EnableWindow(D_GETOLH_VAL(hdlg, _XF06DAD_INIT_AZERODELAY), i ? TRUE : FALSE);
      return TRUE;

		case WM_COMMAND:
			if (lparam) {
				switch(LOWORD(wparam)) {
					case D_GETOLID_VAL(_XF06DAD_INIT_LAMP):
						SendMessage(hdlg, WMD_USER+0, 0, 0);
						return TRUE;
					case D_GETOLID_VAL(_XF06DAD_INIT_AZERO):
						SendMessage(hdlg, WMD_USER+0, 0, 0);
						return TRUE;
					default:
						break;
				}
			}
			break;

		case WM_DESTROY:
			hD = NULL;
			pD = NULL;
			break;

	}
	return(D_RestSetupDlgProc(hdlg, msg, wparam, lparam, (D_DEVICE *)hD));
}

//*** function processes ready dialog - device on-line configuration
BOOL CALLBACK F06DAD_OnLineDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	struct ol_data_st {
		D_DEVICE *pD;
		TCHAR text[MAX_PATH];
		HWND hReportDlg;
		HWND hScanDlg;
	} *pdata = NULL;
	D_DEVICE *pD = NULL;
	TCHAR *ttext = NULL;
	const TCHAR *ptext;
	HWND hctl;
	int i;
	WORD p;
	unsigned channels, wchannels;
	const TCHAR *c_data_err[3] = {TEXT("OVF"), TEXT("UNF"), TEXT("BGN")};
	#define F06DAD_OLBT_SCAN (D_OL_IDUSER+101)
	#define F06DAD_OLBT_REPORT (D_OL_IDUSER+102)
	#define F06DAD_OLBT_ERRS (D_OL_IDUSER+103)


	if (msg == WM_INITDIALOG) {
		pD = (D_DEVICE *)lparam;
		//allocation
		pdata = malloc(sizeof(struct ol_data_st));
		SetWindowLong(hdlg, GWLP_USERDATA, (LONG)pdata);		//store handles !!!!!

		if (pdata) {
			pdata->pD = pD;
			pdata->hReportDlg = NULL;
			pdata->hScanDlg = NULL;
			ttext = pdata->text;
		}

		//store online handle
		pD->p[_XF06DAD_TEMP_ONLINE].val.h = hdlg;

		//create & initiate parameters
		channels = F06DAD_GetChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);
		wchannels = F06DAD_GetWChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);

		//-------------- adjust. controls ------------
		i = 0;
		D_CreateOnLineGroup(hdlg, D_("Adjustable parameters"), i, 6);
		//lamps
		D_CreateOnLineCtrl(hdlg, _XF06DAD_LAMP, pD, i++);
		//wl
		for (p=0; p<_XF06DAD_MAXCHANNELS; p++) {
			if (p < channels) {
				D_CreateOnLineCtrl(hdlg, _XF06DAD_WLA+p, pD, i++);
			}
		}
		//tc
		D_CreateOnLineCtrl(hdlg, _XF06DAD_FILTER, pD, i++);

		//-------------- status controls -------------
		i = 0;
		D_CreateOnLineGroup(hdlg, D_("Status parameters"), i | D_OL_POS_C2, 6);
		//status
		D_CreateOnLineCtrl(hdlg, _XF06DAD_STAT, pD, i++ | D_OL_POS_C2);
		//errors
		D_CreateOnLineCtrl(hdlg, _XF06DAD_ERROR, pD, i++ | D_OL_POS_C2);
		//lamp time
		D_CreateOnLineCtrl(hdlg, _XF06DAD_LIFE, pD, i++ | D_OL_POS_C2);
		//lamps
		D_CreateOnLineCtrl(hdlg, _XF06DAD_CLAM, pD, i++ | D_OL_POS_C2);
		//hf (readonly)
		D_CreateOnLineCtrl(hdlg, _XF06DAD_HALF, pD, i++ | D_OL_POS_C2);

		//-------------- measure controls -------------
		i = 8;
		D_CreateOnLineGroup(hdlg, D_("Measure parameters"), i | D_OL_POS_C12, channels+wchannels);
		for (p=0; p<channels; p++) {
			//abs
			D_CreateOnLineCtrl(hdlg, _XF06DAD_ABSA+p, pD, i | D_OL_POS_C1);
			//int
			D_CreateOnLineCtrl(hdlg, _XF06DAD_INTA+p, pD, i | D_OL_POS_C2);
			i++;
		}
		for (p=0; p<wchannels; p++) {
			//abs
			D_CreateOnLineCtrl(hdlg, _XF06DAD_ABSW+p, pD, i | D_OL_POS_C1);
			i++;
		}

		//-------------- oper. controls -------------
		i += 2;
		D_CreateOnLineOperCB(hdlg, pD, i);

		//-------------- advance buttons -------------
		D_CreateOnLineGroup(hdlg, TEXT("Advanced buttons"), i | D_OL_POS_C2, 2);
		//scan button
		D_CreateOnLineButton(hdlg, F06DAD_OLBT_SCAN, _XF06DAD_STR_SCAN, 0, 2, i | D_OL_POS_C2 | D_OL_XHEIGHT_2);
		//error button
		D_CreateOnLineButton(hdlg, F06DAD_OLBT_ERRS, _XF06DAD_STR_ERRS, 1, 2, i | D_OL_POS_C2);
		i++;
		//report button
		D_CreateOnLineButton(hdlg, F06DAD_OLBT_REPORT, _XF06DAD_STR_REPORT, 1, 2, i | D_OL_POS_C2);

		//get SN
		ptext = D_GetDevSerialNumber((HDEVICE)pD);

		//----------- spectrum dialog -------------
		if (pdata) {
			__sprintf(ttext, TEXT("%s (SN %s)"), D_("Scanning..."), ptext ? ptext : TEXT(""));
			pdata->hScanDlg = CreateSimpleDialogIndirect((HINSTANCE)GetWindowLong(hdlg, GWLP_HINSTANCE),
																						pD->_hwnd, WS_SYSMENU | DS_MODALFRAME | WS_CAPTION | WS_SIZEBOX | WS_MAXIMIZEBOX,
																						ttext,
																						0, 0, 400, 300,
																						F06DAD_ScanDlgProc, (LPARAM)pD);
		}
		//----------- report dialog -------------
		if (pdata) {
			__sprintf(ttext, TEXT("%s (SN %s)"), D_("Device report"), ptext ? ptext : TEXT(""));
			pdata->hReportDlg = CreateSimpleDialogIndirect((HINSTANCE)GetWindowLong(hdlg, GWLP_HINSTANCE),
																						pD->_hwnd, WS_SYSMENU | DS_MODALFRAME | WS_CAPTION | WS_SIZEBOX | WS_MAXIMIZEBOX,
																						ttext,
																						0, 0, 400, 300,
																						F06DAD_ReportDlgProc, (LPARAM)pD);
		}


		//disable some controls (WLx, ABSx, ITx)
		SendMessage(hdlg, WMD_ENABLE, 0, 0);
	}
	else {
		pdata = (struct ol_data_st *)GetWindowLong(hdlg, GWLP_USERDATA);		//get handles
		if (pdata) {
			pD = pdata->pD;
			ttext = pdata->text;
		}

		//process rest messages
		switch(msg) {
			case D_OL_WM_UPDATE:
				if (pD && pD->p) {

					switch (wparam) {
						case _XF06DAD_STAT:
							//status
							hctl = D_GETOLH_VAL(hdlg, wparam);
							D_TestUserData(hctl, (LONG)lparam);		//test & store response
							ptext = F06DAD_GetStatusDescription(pD->p[wparam].val.u);
							if (ptext && IsCtrlTextDiff(hctl, ptext))
								SetWindowText(hctl, ptext);
							//error
							hctl = D_GETOLH_VAL(hdlg, _XF06DAD_ERROR);
							D_TestUserData(hctl, (LONG)lparam);		//test & store response
							//test model
							F06DAD_PrintError(pD->p[_XF06DAD_ERROR].val.u,
																F06DAD_IsModel800(pD->p[_XF06DAD_MODEL].val.u) ? 4 : 3,
																ttext);
							if (IsCtrlTextDiff(hctl, ttext))
								SetWindowText(hctl, ttext);
							return TRUE;
						case _XF06DAD_ABSA:
							{
							double dblval;
							int prec;
							channels = F06DAD_GetChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);
							for (i=0; i<channels; i++) {
								hctl = D_GETOLH_VAL(hdlg, _XF06DAD_ABSA+i);
								D_TestUserData(hctl, (LONG)lparam);		//test & store response
								dblval = pD->p[_XF06DAD_ABSA+i].val.d;
								D_ConvertValueToDemandUnit(&pD->p[_XF06DAD_ABSA+i], &dblval);	//unit conversion
								prec = DFLAG_GETPREC(pD->p[_XF06DAD_ABSA+i].flags);
								D_ConvertUnitDemandPrecision(& pD->p[_XF06DAD_ABSA+i], &prec);
								p = (pD->p[_XF06DAD_TEMP_EABS].val.u >> (2*i)) & 0x3;
								__sprintf(ttext, TEXT("%.*lf%s%s"),
													prec,
													dblval,
													p == 0 ? TEXT("") : TEXT(" "),
													p == 0 ? TEXT("") : c_data_err[p-1]);
								if (IsCtrlTextDiff(hctl, ttext))
									SetWindowText(hctl, ttext);
							}
							wchannels = F06DAD_GetWChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);
							for (i=0; i<wchannels; i++) {
								hctl = D_GETOLH_VAL(hdlg, _XF06DAD_ABSW+i);
								D_TestUserData(hctl, (LONG)lparam);		//test & store response
								dblval = pD->p[_XF06DAD_ABSW+i].val.d;
								D_ConvertValueToDemandUnit(&pD->p[_XF06DAD_ABSW+i], &dblval);	//unit conversion
								prec = DFLAG_GETPREC(pD->p[_XF06DAD_ABSW+i].flags);
								D_ConvertUnitDemandPrecision(& pD->p[_XF06DAD_ABSW+i], &prec);
								p = (pD->p[_XF06DAD_TEMP_EABSW].val.u >> (2*i)) & 0x3;
								__sprintf(ttext, TEXT("%.*lf%s%s"),
													prec,
													dblval,
													p == 0 ? TEXT("") : TEXT(" "),
													p == 0 ? TEXT("") : c_data_err[p-1]);
								if (IsCtrlTextDiff(hctl, ttext))
									SetWindowText(hctl, ttext);
							}
							}
							return TRUE;
						case _XF06DAD_INTA:
							channels = F06DAD_GetChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);
							for (i=0; i<channels; i++) {
								hctl = D_GETOLH_VAL(hdlg, _XF06DAD_INTA+i);
								D_TestUserData(hctl, (LONG)lparam);		//test & store response
								p = (pD->p[_XF06DAD_TEMP_EINT].val.u >> (2*i)) & 0x3;
								__sprintf(ttext, TEXT("%.2lf%s%s"),
													pD->p[wparam+i].val.d,
													p == 0 ? TEXT("") : TEXT(" "),
													p == 0 ? TEXT("") : c_data_err[p-1]);
								if (IsCtrlTextDiff(hctl, ttext))
									SetWindowText(hctl, ttext);
							}
							return TRUE;
					}
				}
				break;

			case WM_COMMAND:
				if (lparam && pD) {
					//device properties
					if (pD->n_p && pD->p) {
						if (HIWORD(wparam) == BN_CLICKED) {
							switch(LOWORD(wparam)) {
								case F06DAD_OLBT_SCAN:
									if (pdata && pdata->hScanDlg)
										SetWindowPos(pdata->hScanDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
									return TRUE;
								case F06DAD_OLBT_REPORT:
									if (pdata && pdata->hReportDlg)
										SetWindowPos(pdata->hReportDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
									return TRUE;
								case F06DAD_OLBT_ERRS:
									ptext = D_GetDevSerialNumber((HDEVICE)pD);
									__sprintf(ttext, TEXT("%s (SN %s)"), D_("Errors"), ptext ? ptext : TEXT(""));
									F06DAD_ErrorsDecodeBox(hdlg, pD->p[_XF06DAD_ERROR].val.u, ttext, F06DAD_GetErrorDescription);
								default:
									break;
							}
						}
					}
				}
				break;

			case WM_TIMER:
				if (pD && pD->n_p && pD->p) {
					//abs
					if (pD->counter%2 == 0)
						SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_ABSA, pD->p[_XF06DAD_ABSA].lerr);		//abs. update everytime
					//other
					switch (pD->counter % 21) {
						case 0:
						case 7:
						case 14:
							i = _XF06DAD_STAT;
							break;
						case 2: i = _XF06DAD_LAMP; break;
						case 4: i = _XF06DAD_INTA; break;
						case 11: i = _XF06DAD_LIFE; break;
						case 6: i = _XF06DAD_WLA; break;
						case 13: i = _XF06DAD_HALF; break;
						case 20: i = _XF06DAD_FILTER; break;
						default: i = -1; break;
					}
					if (i != -1)
						D_ReadFunction((HDEVICE)pD, F06DAD_ReadProp, i, hdlg, D_OL_WM_UPDATE);
					pD->counter++;
					return TRUE;
				}
				break;

			case WMD_ENABLE:
				D_RestDevDlgProc(hdlg, msg, wparam, lparam, pD);
				return(TRUE);

			case WMD_START:
				if (pD && pD->p[_XF06DAD_INJZERO].val.u) {
					D_MakeOperFunctionPrior((HDEVICE)pD, F06DAD_MakeOper, _XF06DAD_AUTOZERO, NULL, 0, MQM_PRIORITY_HIGH);		//autozero
				}
				return TRUE;

			case WM_CLOSE:
				return TRUE;

			case WM_DESTROY:
				if (pD->n_p && pD->p) {
					//clear online handle
					pD->p[_XF06DAD_TEMP_ONLINE].val.h = NULL;
				}
				pD = NULL;

				//free memories
				if (pdata) {
					//destroy windows
					if (pdata->hScanDlg)
						DestroyWindow(pdata->hScanDlg);
					if (pdata->hReportDlg)
						DestroyWindow(pdata->hReportDlg);

					free((void *)pdata);
				}
				pdata = NULL;
				SetWindowLong(hdlg, GWLP_USERDATA, (LONG)pdata);

				break;
		}
	}
	return(D_RestDevDlgProc(hdlg, msg, wparam, lparam, pD));
}


//-------------------------------------------------------------

#define F06DAD_ERRORS 3
typedef struct {
	DWORD lhP;
	DWORD lhPE[F06DAD_ERRORS];
	DWORD lhT[_XF06DAD_MAXCHANNELS];
	DWORD lhTE;
	struct {
		unsigned n;
		double x[_XF06DAD_SCAN_MAX_SIZE];
		double y[_XF06DAD_SCAN_MAX_SIZE];
	} points[F06DAD_ERRORS];
	//
	BOOL run;
	//
	int mode;
	DWORD n;
	double *x;
	double *y;
	char *e;
	//
	//int mm_enable;
} XF06DAD_PSCAN;

static const MGCFONT c_gfont = {15, MGC_FONT_BOLD, 0, TEXT("Courier New")};

//*** graph custom draw function
int XF06DAD_ScanCustomDrawProc(HWND hwnd, HDC hdc, MGGCUSTOMDRAWPOS *cdpos, unsigned user1, unsigned user2)
{
#ifndef DEVICE_ALONE

	TCHAR ttext[32];
	XF06DAD_PSCAN *pscan;
	HPEN hpen, hpenorg;
	HFONT hfont, hfontorg;
	RECT rc;
	double min, max;

	if (cdpos && user1) {
		pscan = (XF06DAD_PSCAN *)user1;

		//test for data;
		if (pscan /*&& pscan->mm_enable*/ && pscan->n && pscan->y) {
			//get min & max
			MM_GetGlobalExtrems(pscan->n, pscan->y, NULL, NULL, &min, &max);

			//select pen
			hpen = CreatePen(MGC_LSTYLE_SOLID, 1, MGC_COLOR_DGRAY);
			hpenorg = (HPEN)SelectObject(hdc, hpen);
			SelectObject(hdc, GetStockObject(WHITE_BRUSH));
			//set font
			hfont = MGC_MakeScaledFont(hdc, (MGCFONT *)&c_gfont, NULL);
			hfontorg = SelectObject(hdc, hfont);

			//set rectangle
			rc.right = cdpos->area_rc.right-5;
			rc.left = cdpos->area_rc.left+5;
			rc.bottom = cdpos->area_rc.bottom-5;
			rc.top = rc.bottom-20;
			if (rc.top < cdpos->area_rc.top)
				rc.top = cdpos->area_rc.top;

			//rectange
			//Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

			//--- texts ---
			SetTextAlign(hdc, TA_BOTTOM|TA_RIGHT);		//text align
			SetBkMode(hdc, TRANSPARENT);		//background mode
			//SetBkColor(hdc, MGC_COLOR_WHITE);
			SetTextColor(hdc, MGC_COLOR_BLACK);
			//text
			__sprintf(ttext, TEXT("MIN: %.2lf, MAX: %.2lf"), min, max);
			TextOut(hdc, rc.right, rc.bottom, ttext, lstrlen(ttext));

			//delete font
			SelectObject(hdc, hfontorg);
			DeleteObject(hfont);
			//delete pen
			SelectObject(hdc, hpenorg);
			DeleteObject(hpen);
		}

		return(TRUE);
	}
#endif
	return(FALSE);
}

//*** function processes ready dialog - device on-line configuration
BOOL CALLBACK F06DAD_ScanDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	const COLORREF points_color[F06DAD_ERRORS] = {MGC_COLOR_DBLUE, MGC_COLOR_DGREEN, MGC_COLOR_DRED};
	D_PROCSTOCK *pPS = NULL;
	D_DEVICE *pD = NULL;
	XF06DAD_PSCAN *pScan = NULL;
	TCHAR *ttext = NULL;
	int i, j;
	RECT rc;
	HWND hctl;
	double dbl;
	U_NVAL nval, nval2;
	D_VAL dval;
	unsigned channels;
	DWORD wlmax;
	MGGPLOTPARAMS pparams;
	MGGTEXTPARAMS tparams;
	MGGAXISPARAMS aparams;
	MGGCUSTOMDRAWPARAMS cdparams;
	MGGINFO *pinfo;
#define F06DAD_P_GPH (3*D_OL_H+4)
#define F06DAD_P_STWM 30
#define F06DAD_P_EBW 60
#define F06DAD_P_BUTTONS 6
//
#define F06DAD_PGA_GRAPH 11
#define F06DAD_PGP_SCANRANGE 12
#define F06DAD_PST_MIN 13
#define F06DAD_PST_MAX 14
#define F06DAD_PEB_SCANRANGE1 15
#define F06DAD_PEB_SCANRANGE2 16
#define F06DAD_PGP_TEXTS 17
#define F06DAD_PCB_TEXTS 18
#define F06DAD_PGP_GRIDS 19
#define F06DAD_PCB_GRIDS 20
#define F06DAD_PGP_HALF 21
#define F06DAD_PEB_HALF 22
#define F06DAD_PST_WLRANGE 23
//
#define F06DAD_PCB_MODE 31
#define F06DAD_PBT_MAKESCAN 32
#define F06DAD_PBT_RUNSTOP 33
#define F06DAD_PBT_YFULL 34
#define F06DAD_PBT_YSCALE 35
#define F06DAD_PBT_EXPORT 36
#define F06DAD_PBT_CLOSE IDCANCEL
//
#define F06DAD_YFULL_ABSMIN -100.0
#define F06DAD_YFULL_ABSMAX 1200.0
#define F06DAD_YFULL_INTMIN -10.0
#define F06DAD_YFULL_INTMAX 120.0
#define F06DAD_XMIN_RANGE 0.1
#define F06DAD_YMIN_RANGE 0.01

	if (msg == WM_INITDIALOG) {
		//allocation
		pPS = (D_PROCSTOCK *)malloc(sizeof(D_PROCSTOCK));
		//zero memory
		memset(pPS, 0, sizeof(D_PROCSTOCK));
		SetWindowLong(hdlg, GWLP_USERDATA, (LONG)pPS);		//store handle !!!!!

		//init. stock
		pD = (D_DEVICE *)lparam;		//device handle
		if (pPS) {
			pPS->pD = pD;		//store device handle
			ttext = pPS->ttext;
			pScan = (XF06DAD_PSCAN *)malloc(sizeof(XF06DAD_PSCAN));
			memset(pScan, 0, sizeof(XF06DAD_PSCAN));
			pPS->lparam = (DWORD)pScan;

			//init memories
			if (pScan) {
				//set mode
				F06DAD_SetScanMode(pD, pScan->mode);

				GetClientRect(hdlg, &rc);
				rc.left++;
				rc.top++;
				rc.right--;
				rc.bottom--;

				//graph
				rc.top += 1+GSY(F06DAD_P_GPH);
				rc.bottom -= 1+GSY(D_OL_H);

				hctl = CreateWindowEx(0, MGC_GRAPHCLASSNAME,
															TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | MGG_GAS_NOTIFY,
															rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
															hdlg, (HMENU)F06DAD_PGA_GRAPH, NULL, NULL);
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_BORDER_VISIBLE, (LPARAM)FALSE);		//vis.
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_BORDER_BGCOLOR, (LPARAM)GetSysColor(COLOR_BTNFACE));		//bg.
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_BORDER_SPACE, (LPARAM)0);		//space
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_AREA_BGCOLOR, (LPARAM)MGC_COLOR_WHITE);		//area bg.
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_TITLE_VISIBLE, (LPARAM)FALSE);		//title off
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_LEGEND_VISIBLE, (LPARAM)FALSE);		//legend off
				__sprintf(ttext, TEXT("%s [%s]"), D_("Wavelength"), pD->p[_XF06DAD_WLA].unit.u);
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_XAXIS_LABEL, (LPARAM)ttext);		//x-axis label
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_XAXIS_LABEL_VISIBLE, (LPARAM)TRUE);		//x-axis label vis.
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_YAXIS_LABEL_VISIBLE, (LPARAM)TRUE);		//y-axis label vis.
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_XAXIS_TICKS_GRID,
										(LPARAM)((pD->p[_XF06DAD_GRIDBY].val.u & 0x1) ? TRUE : FALSE));		//x-axis grids
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_YAXIS_TICKS_GRID,
										(LPARAM)((pD->p[_XF06DAD_GRIDBY].val.u & 0x2) ? TRUE : FALSE));		//y-axis grids
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_ZOOM_ENABLE, (LPARAM)TRUE);		//enable zoom
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_ZOOM_POFFSET, MAKELONG(0, 80));		//fullscale poffsets
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_OTHER_MP_ENABLE, TRUE);		//show cursor position
				//SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_OTHER_MP_STYLE, (LPARAM)TEXT("x:%.1lf; y:%.1lf"));		//mp. display style
				//pScan->mm_enable = TRUE;
				cdparams.proc = XF06DAD_ScanCustomDrawProc;
				cdparams.user1 = (unsigned)pScan;
				cdparams.user2 = 0;
				SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_CUSTOMDRAW, (LPARAM)&cdparams);		//custom draw parameters

				//plot
				memset(&pparams, 0, sizeof(MGGPLOTPARAMS));
				pparams.mask = MGG_PLOTMASK_VIS | MGG_PLOTMASK_DATA | MGG_PLOTMASK_LINE | MGG_PLOTMASK_POINT;
				pparams.visible = TRUE;
				pparams.data.alloc = FALSE;
				pparams.data.n = 0;
				pparams.data.x = NULL;
				pparams.data.y = NULL;
				pparams.line.style = MGC_LSTYLE_SOLID;
				pparams.line.color = MGC_COLOR_DBLUE;
				pparams.line.width = 0.6;
				pparams.point.style = MGC_PSTYLE_NONE;
				pScan->lhP = SendMessage(hctl, MGG_GAM_INSERTPLOT, -1, (LPARAM)&pparams);		//add plot
				//error plots
				pparams.mask = MGG_PLOTMASK_VIS | MGG_PLOTMASK_DATA | MGG_PLOTMASK_LINE | MGG_PLOTMASK_POINT;
				pparams.visible = TRUE;
				pparams.data.alloc = FALSE;
				pparams.data.n = 0;
				pparams.data.x = NULL;
				pparams.data.y = NULL;
				pparams.line.style = MGC_LSTYLE_NONE;
				pparams.line.color = MGC_COLOR_DBLUE;
				pparams.line.width = 0.6;
				pparams.point.style = MGC_PSTYLE_FILLCIRCLE;
				pparams.point.size = 4.0;
				for (j=0; j<F06DAD_ERRORS; j++) {
					pparams.point.fcolor = points_color[j];
					pparams.point.bcolor = pparams.point.fcolor;
					pScan->lhPE[j] = SendMessage(hctl, MGG_GAM_INSERTPLOT, -1, (LPARAM)&pparams);		//add plot
				}
				//texts
				channels = F06DAD_GetChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);
				memset(&tparams, 0, sizeof(MGGTEXTPARAMS));
				tparams.mask = MGG_TEXTMASK_VIS |
											 MGG_TEXTMASK_ZMODE |
											 MGG_TEXTMASK_DATA |
											 MGG_TEXTMASK_COLOR |
											 MGG_TEXTMASK_CONLINE;
				tparams.visible = TRUE;
				tparams.zmode = MGG_TEXTZMODE_NORMAL;
				tparams.data.x = 0.0;
				tparams.data.y = 0.0;
				tparams.data.text = NULL;
				tparams.data.maxlen = 0;
				tparams.color = MGC_COLOR_DBLUE;
				tparams.conline.style = MGC_LSTYLE_SOLID;
				tparams.conline.color = tparams.color;
				tparams.conline.width = 0.4;
				for (i=0; i<channels; i++) {
					pScan->lhT[i] = SendMessage(hctl, MGG_GAM_INSERTTEXT, -1, (LPARAM)&tparams);		//add text
				}
				//background text (error)
				tparams.mask = MGG_TEXTMASK_VIS |
											 MGG_TEXTMASK_ZMODE |
											 MGG_TEXTMASK_DATA |
											 MGG_TEXTMASK_COLOR |
											 MGG_TEXTMASK_ALIGN |
											 MGG_TEXTMASK_FONT;
				tparams.visible = FALSE;
				tparams.zmode = MGG_TEXTZMODE_UNDERCENTER;
				tparams.data.x = 0.0;
				tparams.data.y = 0.0;
				tparams.data.text = D_("Error");
				tparams.data.maxlen = __strlen(tparams.data.text);
				tparams.color = MixTwoColors(MGC_COLOR_DRED, MGC_COLOR_WHITE, 0.5);
				tparams.align = MGC_ALIGN_MC;
				tparams.font.height = GSY(56);
				tparams.font.tflag = FW_BOLD;
				tparams.font.angle = 300;
				lstrcpyn(tparams.font.name, TEXT("Arial"), LF_FACESIZE);
				pScan->lhTE = SendMessage(hctl, MGG_GAM_INSERTTEXT, -1, (LPARAM)&tparams);		//add text

				//--- controls ---
				//scan range
				i = 0;
				rc.top -= 1+GSY(F06DAD_P_GPH);
				//scan range group box
				D_MakePropLabel(&pD->p[_XF06DAD_WLSCANRANGE], ttext, 0x1);
				hctl = CreateWindow(CLASS_BUTTON, ttext,
														WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
														rc.left, rc.top, GSX(2*(F06DAD_P_STWM+F06DAD_P_EBW)+3*D_OL_MOF), GSY(F06DAD_P_GPH),
														hdlg, (HMENU)F06DAD_PGP_SCANRANGE, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//min
				hctl = CreateWindow(CLASS_STATIC, D_("Min"),
														WS_CHILD | WS_VISIBLE,
														rc.left+GSX(D_OL_MOF), rc.top+GSY(19), GSX(F06DAD_P_STWM), GSY(D_OL_H),
														hdlg, (HMENU)F06DAD_PST_MIN, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//wl. min
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""),
															WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
															rc.left+GSX(D_OL_MOF+F06DAD_P_STWM), rc.top+GSY(17), GSX(F06DAD_P_EBW), GSY(D_OL_H),
															hdlg, (HMENU)F06DAD_PEB_SCANRANGE1, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUINT | 0 | U_NFLAGMIN | U_NFLAGMAX,
																	pD->p[_XF06DAD_TEMP_WLRANGE].val.u & 0xFFFF,
																	pD->p[_XF06DAD_TEMP_WLRANGE].val.u >> 16);
				//max
				hctl = CreateWindow(CLASS_STATIC, D_("Max"),
														WS_CHILD | WS_VISIBLE,
														rc.left+GSX(2*D_OL_MOF+F06DAD_P_STWM+F06DAD_P_EBW), rc.top+GSY(18), GSX(F06DAD_P_STWM), GSY(D_OL_H),
														hdlg, (HMENU)F06DAD_PST_MAX, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//wl. max
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""),
															WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
															rc.left+GSX(2*D_OL_MOF+2*F06DAD_P_STWM+F06DAD_P_EBW), rc.top+GSY(17), GSX(F06DAD_P_EBW), GSY(D_OL_H),
															hdlg, (HMENU)F06DAD_PEB_SCANRANGE2, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUINT | 0 | U_NFLAGMIN | U_NFLAGMAX,
																	pD->p[_XF06DAD_TEMP_WLRANGE].val.u & 0xFFFF,
																	pD->p[_XF06DAD_TEMP_WLRANGE].val.u >> 16);
				//available scan range
				hctl = CreateWindow(CLASS_STATIC, TEXT(""),
														WS_CHILD | WS_VISIBLE,
														rc.left+GSX(D_OL_MOF), rc.top+GSY(19+D_OL_H+2), GSX(2*(F06DAD_P_STWM+F06DAD_P_EBW)), GSY(D_OL_H),
														hdlg, (HMENU)F06DAD_PST_WLRANGE, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//texts
				hctl = CreateWindow(CLASS_BUTTON, D_("Show texts by"),
														WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
														rc.left+GSX(2*(F06DAD_P_STWM+F06DAD_P_EBW)+3*D_OL_MOF+D_OL_SOF), rc.top, GSX(2*D_OL_MOF+D_OL_STW), GSY(F06DAD_P_GPH),
														hdlg, (HMENU)F06DAD_PGP_TEXTS, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

				hctl = CreateWindow(CLASS_COMBOBOX, TEXT(""),
														WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
														rc.left+GSX(2*(F06DAD_P_STWM+F06DAD_P_EBW)+4*D_OL_MOF+D_OL_SOF), rc.top+GSY(17), GSX(D_OL_STW), GSY(D_CB_DEFHEIGHT),
														hdlg, (HMENU)F06DAD_PCB_TEXTS, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("None"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Wavelength"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Value"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Both"));
				SendMessage(hctl, CB_SETCURSEL, pD->p[_XF06DAD_TEXTBY].val.u & 0x03, 0);
				//grids
				hctl = CreateWindow(CLASS_BUTTON, D_("Show grids by"),
														WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
														rc.left+GSX(2*(F06DAD_P_STWM+F06DAD_P_EBW)+5*D_OL_MOF+2*D_OL_SOF+D_OL_STW), rc.top, GSX(2*D_OL_MOF+D_OL_STW), GSY(F06DAD_P_GPH),
														hdlg, (HMENU)F06DAD_PGP_GRIDS, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

				hctl = CreateWindow(CLASS_COMBOBOX, TEXT(""),
														WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
														rc.left+GSX(2*(F06DAD_P_STWM+F06DAD_P_EBW)+6*D_OL_MOF+2*D_OL_SOF+D_OL_STW), rc.top+GSY(17), GSX(D_OL_STW), GSY(D_CB_DEFHEIGHT),
														hdlg, (HMENU)F06DAD_PCB_GRIDS, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("None"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)TEXT("X"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)TEXT("Y"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Both"));
				SendMessage(hctl, CB_SETCURSEL, pD->p[_XF06DAD_GRIDBY].val.u & 0x03, 0);

				//half width
				D_MakePropLabel(&pD->p[_XF06DAD_HALF], ttext, 0x1);
				hctl = CreateWindow(CLASS_BUTTON, ttext,
														WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
														rc.left+GSX(2*(F06DAD_P_STWM+F06DAD_P_EBW)+7*D_OL_MOF+3*D_OL_SOF+2*D_OL_STW), rc.top, GSX(2*D_OL_MOF+F06DAD_P_EBW), GSY(F06DAD_P_GPH),
														hdlg, (HMENU)F06DAD_PGP_HALF, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, TEXT(""),
															WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT	,
															rc.left+GSX(2*(F06DAD_P_STWM+F06DAD_P_EBW)+8*D_OL_MOF+3*D_OL_SOF+2*D_OL_STW), rc.top+GSY(17), GSX(F06DAD_P_EBW), GSY(D_OL_H),
															hdlg, (HMENU)F06DAD_PEB_HALF, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SetValidNumStrSubclassing(hctl, U_NFLAGUINT | 0 | U_NFLAGMIN | U_NFLAGMAX, _XF06DAD_HALFMIN, _XF06DAD_HALFMAX);

				rc.bottom++;
				//mode (Abs./Int.)
				i = 0;
				hctl = CreateWindow(CLASS_COMBOBOX, TEXT(""),
														WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
														rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom, (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_CB_DEFHEIGHT),
														hdlg, (HMENU)F06DAD_PCB_MODE, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Absorbance"));
				SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Intensity"));
				SendMessage(hctl, CB_SETCURSEL, 0, 0);
				//make a scan
				i = 1;
				hctl = CreateWindow(CLASS_BUTTON, D_("Make a scan"),
														WS_CHILD | WS_VISIBLE | WS_TABSTOP,
														rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom, (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H),
														hdlg, (HMENU)F06DAD_PBT_MAKESCAN, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//run/stop
				i = 2;
				hctl = CreateWindow(CLASS_BUTTON, D_("Run scanning"),
														WS_CHILD | WS_VISIBLE | WS_TABSTOP,
														rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom, (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H),
														hdlg, (HMENU)F06DAD_PBT_RUNSTOP, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//yfull
				i = 3;
				hctl = CreateWindow(CLASS_BUTTON, D_("YFull"),
														WS_CHILD | WS_VISIBLE | WS_TABSTOP,
														rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom, (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H),
														hdlg, (HMENU)F06DAD_PBT_YFULL, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//yscale
				i = 4;
				hctl = CreateWindow(CLASS_BUTTON, D_("YScale"),
														WS_CHILD | WS_VISIBLE | WS_TABSTOP,
														rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom, (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H),
														hdlg, (HMENU)F06DAD_PBT_YSCALE, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
				//export
				i = 5;
				hctl = CreateWindow(CLASS_BUTTON, D_("Export"),
														WS_CHILD | WS_VISIBLE | WS_TABSTOP,
														rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom, (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H),
														hdlg, (HMENU)F06DAD_PBT_EXPORT, NULL, NULL);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
			}
		}

		if (pD && pD->n_p && pD->p) {
			//create & initiate parameters

		}

		//ScaleChildWindowsByScreen(hdlg);		//scale window

		if (pD && pD->n_p && pD->p) {

			//start timer
			pD->counter = 0;
		}

		//center window
		CenterParentWindow(hdlg);
		//hide
		ShowWindow(hdlg, SW_HIDE);
		//start timer
		SetTimer(hdlg, 1, pD && pD->p ? F06DAD_GetSTimingByModel(pD->p[_XF06DAD_MODEL].val.u) : 333, NULL);

		//init. controls
		//SendMessage(hdlg, WM_COMMAND, MAKELONG(F06DAD_PBT_MAKESCAN, BN_CLICKED), 0);
		SendMessage(hdlg, WM_COMMAND, MAKELONG(F06DAD_PBT_YFULL, BN_CLICKED), 0);
		SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_WLSCANRANGE, 0);
		SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_HALF, 0);
		SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_TEMP_WLRANGE, 0);

	}
	else {
		pPS = (D_PROCSTOCK *)GetWindowLong(hdlg, GWLP_USERDATA);		//get device handle
		if (pPS) {
			pD = pPS->pD;		//get device handle
			ttext = pPS->ttext;
			pPS->hf = NULL;
			pScan = (XF06DAD_PSCAN *)pPS->lparam;
		}

		//process rest messages
		switch(msg) {

			case D_OL_WM_UPDATE:
				if (pD && pD->n_p && pD->p) {
					switch (wparam) {
						case _XF06DAD_TEMP_SCANDATA:
							if (pScan) {
								hctl = GetDlgItem(hdlg, F06DAD_PGA_GRAPH);

								if (pScan->mode == F06DAD_GetDataScanMode(pD)) {
									//mode is correct
									F06DAD_GetScan(pD, 0, &pScan->n, &pScan->x, &pScan->y, &pScan->e);
								}
								else {
									pScan->n = 0;
									pScan->x = NULL;
									pScan->y = NULL;
									pScan->e = NULL;
								}

								//y-label
								__sprintf(ttext, TEXT("%s [%s]"),
													pScan->mode == 0 ? D_("Absorbance") : D_("Intensity"),
													pD->p[pScan->mode == 0 ? _XF06DAD_ABSA : _XF06DAD_INTA].unit.u);
								SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_YAXIS_LABEL, (LPARAM)ttext);

								//plot
								pparams.data.alloc = FALSE;
								pparams.data.n = pScan->n;
								pparams.data.x = pScan->x;
								pparams.data.y = pScan->y;
								SendMessage(hctl, MGG_GAM_SETPLOTDATA, pScan->lhP, (LPARAM)&pparams.data);

								//error plots
								for (j=0; j<F06DAD_ERRORS; j++)
									pScan->points[j].n = 0;
								for (i=0; i<pScan->n; i++) {
									for (j=0; j<F06DAD_ERRORS; j++) {
										if (pScan->e[i] == (j+1)) {
											pScan->points[j].x[pScan->points[j].n] = pScan->x[i];
											pScan->points[j].y[pScan->points[j].n] = pScan->y[i];
											pScan->points[j].n++;
										}
									}
								}
								for (j=0; j<F06DAD_ERRORS; j++) {
									pparams.data.alloc = FALSE;
									pparams.data.n = pScan->points[j].n;
									pparams.data.x = pScan->points[j].x;
									pparams.data.y = pScan->points[j].y;
									SendMessage(hctl, MGG_GAM_SETPLOTDATA, pScan->lhPE[j], (LPARAM)&pparams.data);
								}

								//wl max
								wlmax = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
								//indexes
								channels = F06DAD_GetChannelsByModel(pD->p[_XF06DAD_MODEL].val.u);
								for (i=0; i<channels; i++) {
									if (pScan->n) {
										j = (int)pD->p[_XF06DAD_WLA+i].val.d;
										if (j >= (int)pScan->x[0] && j <= (int)pScan->x[pScan->n-1])
											j = j - (int)pScan->x[0];
										else
											j = -1;
									}
									else
										j = -1;
									dbl = j < 0 ? pD->p[(pScan->mode == 0 ? _XF06DAD_ABSA : _XF06DAD_INTA)+i].val.d : pScan->y[j];
									switch ((pD->p[_XF06DAD_TEXTBY].val.u & 0x3)) {
										case 0: __strcpy(ttext, TEXT("")); break;
										case 1: __sprintf(ttext, TEXT("%.lf"), pD->p[_XF06DAD_WLA+i].val.d); break;
										default:
										case 2: __sprintf(ttext, TEXT("%.02lf"), dbl); break;
										case 3: __sprintf(ttext, TEXT("%.lf; %.02lf"), pD->p[_XF06DAD_WLA+i].val.d, dbl); break;
									}
									tparams.data.x = pD->p[_XF06DAD_WLA+i].val.d;
									tparams.data.y = dbl;
									tparams.data.text = ttext;
									tparams.data.maxlen = __strlen(tparams.data.text);
									SendMessage(hctl, MGG_GAM_SETTEXTDATA, pScan->lhT[i], (LPARAM)&tparams.data);
									if (*ttext) {
										//x, left-right align
										if (pD->p[_XF06DAD_WLA+i].val.d > 0.95*(double)wlmax) {
											tparams.offset.x = -15;
											tparams.align = MGC_ALIGN_BLR;
										}
										else {
											tparams.offset.x = 15;
											tparams.align = MGC_ALIGN_BLL;
										}
										//y, top-bottom align
										if (dbl > 0.8*(pScan->mode == 0 ? F06DAD_YFULL_ABSMAX : F06DAD_YFULL_INTMAX)) {
											tparams.offset.y = -15;
											tparams.align += 6;		//top
										}
										else
											tparams.offset.y = 15;

										tparams.mask = MGG_TEXTMASK_OFFSET | MGG_TEXTMASK_ALIGN;
										SendMessage(hctl, MGG_GAM_SETTEXTPARAMS, pScan->lhT[i], (LPARAM)&tparams);
									}
									tparams.mask = MGG_TEXTMASK_VIS;
									tparams.visible = *ttext ? TRUE : FALSE;
									SendMessage(hctl, MGG_GAM_SETTEXTPARAMS, pScan->lhT[i], (LPARAM)&tparams);
								}

								//error text
								tparams.mask = MGG_TEXTMASK_VIS;
								tparams.visible = pD->p[_XF06DAD_TEMP_SCANDATA].lerr == 0 ? FALSE : TRUE;
								SendMessage(hctl, MGG_GAM_SETTEXTPARAMS, pScan->lhTE, (LPARAM)&tparams);

								SendMessage(hctl, MGG_GAM_UPDATE, 0, 0);
							}

							break;
						case _XF06DAD_WLSCANRANGE:
							//w1
							nval.u = pD->p[wparam].val.u & 0xFFFF;
							hctl = GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE1);
							if (!SendMessage(hctl, EM_GETMODIFY, 0, 0)) {
								GetEditUval(hctl, &nval2);
								if (nval.u != nval2.u)
									SetEditUval(hctl, nval);
							}
							//w2
							nval.u = pD->p[wparam].val.u >> 16;
							hctl = GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE2);
							if (!SendMessage(hctl, EM_GETMODIFY, 0, 0)) {
								GetEditUval(hctl, &nval2);
								if (nval.u != nval2.u)
									SetEditUval(hctl, nval);
							}
							break;
						case _XF06DAD_HALF:
							hctl = GetDlgItem(hdlg, F06DAD_PEB_HALF);
							if (!SendMessage(hctl, EM_GETMODIFY, 0, 0)) {
								nval.u = pD->p[wparam].val.u;
								GetEditUval(hctl, &nval2);
								if (nval2.u != nval.u)
									SetEditUval(hctl, nval);
							}
							break;
						case _XF06DAD_TEMP_WLRANGE:
							hctl = GetDlgItem(hdlg, F06DAD_PST_WLRANGE);
							nval.u = pD->p[wparam].val.u;
							__sprintf(ttext, TEXT("%s [nm]: (%u - %u)"),
												D_("Available range"),
												nval.u & 0xFFFF, (nval.u >> 16) & 0xFFFF);
							if (IsCtrlTextDiff(hctl, ttext))
								SetWindowText(hctl, ttext);
							break;
					}
				}
				return TRUE;

			case WM_WINDOWPOSCHANGING:
				if (lparam) {
					if (((WINDOWPOS *)lparam)->cx < GSX(340))
						((WINDOWPOS *)lparam)->cx = GSX(340);
					if (((WINDOWPOS *)lparam)->cy < GSY(230))
						((WINDOWPOS *)lparam)->cy = GSY(230);
				}
				return TRUE;

			case WM_SIZE:
				if (pScan) {
					GetClientRect(hdlg, &rc);
					rc.left++;
					rc.top++;
					rc.right--;
					rc.bottom--;

					//graph
					rc.top += 1+GSY(F06DAD_P_GPH);
					rc.bottom -= 1+GSY(D_OL_H);
					MoveWindow(GetDlgItem(hdlg, F06DAD_PGA_GRAPH),
										 rc.left, rc.top,
										 rc.right-rc.left,
										 rc.bottom-rc.top, TRUE);

					rc.top -= 1+GSY(F06DAD_P_GPH);
					//

					rc.bottom++;
					//mode
					i = 0;
					MoveWindow(GetDlgItem(hdlg, F06DAD_PCB_MODE),
										 rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom,
										 (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_CB_DEFHEIGHT), FALSE);
					//make a scan
					i = 1;
					MoveWindow(GetDlgItem(hdlg, F06DAD_PBT_MAKESCAN),
										 rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom,
										 (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H), FALSE);
					//run/stop
					i = 2;
					MoveWindow(GetDlgItem(hdlg, F06DAD_PBT_RUNSTOP),
										 rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom,
										 (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H), FALSE);
					//yfull
					i = 3;
					MoveWindow(GetDlgItem(hdlg, F06DAD_PBT_YFULL),
										 rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom,
										 (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H), FALSE);
					//yscale
					i = 4;
					MoveWindow(GetDlgItem(hdlg, F06DAD_PBT_YSCALE),
										 rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom,
										 (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H), FALSE);
					//export
					i = 5;
					MoveWindow(GetDlgItem(hdlg, F06DAD_PBT_EXPORT),
										 rc.left+i*(rc.right-rc.left)/F06DAD_P_BUTTONS, rc.bottom,
										 (rc.right-rc.left)/F06DAD_P_BUTTONS, GSY(D_OL_H), FALSE);
				}
				RedrawWindow(hdlg, NULL, NULL, RDW_INVALIDATE);
				return TRUE;

			case WM_SETFOCUS:
				if (pScan) {
					SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_TEMP_SCANDATA, 0);
					SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_WLSCANRANGE, 0);
					SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_HALF, 0);
					RedrawWindow(hdlg, NULL, NULL, RDW_INVALIDATE);
				}
				return TRUE;

			case WM_COMMAND:
				if (pD && pScan) {
					//device properties
					if (pD->n_p && pD->p) {
						if (HIWORD(wparam) == BN_CLICKED) {		//BN_CLICKED = 0
							switch(LOWORD(wparam)) {
								case F06DAD_PBT_MAKESCAN:
									//set mode
									*(char *)pD->p[_XF06DAD_TEMP_SCANDATA].val.m.buf = (pScan->mode == 0 ? 0 : 1);
									D_ReadFunction((HDEVICE)pD, F06DAD_ReadProp, _XF06DAD_TEMP_SCANDATA, hdlg, D_OL_WM_UPDATE);
									return TRUE;
								case F06DAD_PBT_RUNSTOP:
									pScan->run = pScan->run ? FALSE : TRUE;
									SetWindowText((HWND)lparam, pScan->run ? D_("Stop scanning") : D_("Run scanning"));
									return TRUE;
								case F06DAD_PBT_YFULL:
									hctl = GetDlgItem(hdlg, F06DAD_PGA_GRAPH);
									//wl max
									wlmax = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
									//x-axis
									aparams.mask = MGG_AXISMASK_MIN | MGG_AXISMASK_MAX;
									aparams.min = (double)_XF06DAD_WLMIN;
									aparams.max = (double)wlmax;
									SendMessage(hctl, MGG_GAM_SETAXISPARAMS, MGG_AXIS_X, (LPARAM)&aparams);
									//y-axis
									aparams.mask = MGG_AXISMASK_MIN | MGG_AXISMASK_MAX;
									aparams.min = pScan->mode == 0 ? F06DAD_YFULL_ABSMIN : F06DAD_YFULL_INTMIN;
									aparams.max = pScan->mode == 0 ? F06DAD_YFULL_ABSMAX : F06DAD_YFULL_INTMAX;
									SendMessage(hctl, MGG_GAM_SETAXISPARAMS, MGG_AXIS_Y, (LPARAM)&aparams);

									SendMessage(hctl, MGG_GAM_UPDATE, 0, 0);
									return TRUE;
								case F06DAD_PBT_YSCALE:
									hctl = GetDlgItem(hdlg, F06DAD_PGA_GRAPH);
									//wl max
									wlmax = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
									//x-axis
									aparams.mask = MGG_AXISMASK_MIN | MGG_AXISMASK_MAX;
									aparams.min = (double)_XF06DAD_WLMIN;
									aparams.max = (double)wlmax;
									SendMessage(hctl, MGG_GAM_SETAXISPARAMS, MGG_AXIS_X, (LPARAM)&aparams);
									//y-axis
									SendMessage(hctl, MGG_GAM_FULLSCALE, MGG_AXIS_Y, MAKELONG(0, 80));

									SendMessage(hctl, MGG_GAM_UPDATE, 0, 0);
									return TRUE;
								case F06DAD_PBT_EXPORT:
								case 10001:		//from popup menu -> export
#ifndef DEVICE_ALONE
									//default name
									ttext = (pScan->mode == 0 ? TEXT("f06scan_abs") : TEXT("f06scan_int"));
									if (DLG_GetSaveFile(g_hInst, hdlg, NULL, g_chSetup.datadir, ttext,
																		  IDS_EXPORTEXTFILTER, &g_ext, &ttext)) {
										if (g_ext == 1) {
											//expot graph to EMF
											F06DAD_ExportToEMF(GetDlgItem(hdlg, F06DAD_PGA_GRAPH), ttext, 0, 0);
										}
										else {
											//export to TXT
											F06DAD_ExportScan(hdlg, ttext, pScan->n, pScan->x, pScan->y, pScan->e, pScan->mode == 0 ? 0 : 1);
											g_ext = 0;
										}
										free((void *)ttext);		//free memory
										ttext = NULL;
									}
#endif
									return TRUE;
								case 10002:
									//toggle min/max visibility
									//pScan->mm_enable = pScan->mm_enable ? FALSE : TRUE;
									return TRUE;
								default:
									break;
							}
						}
						else if (HIWORD(wparam) == CBN_SELCHANGE) {
							switch(LOWORD(wparam)) {
								case F06DAD_PCB_MODE:
									pScan->mode = (SendMessage(GetDlgItem(hdlg, LOWORD(wparam)), CB_GETCURSEL, 0, 0) == 1 ? 1 : 0);
									SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_TEMP_SCANDATA, 0);
									SendMessage(hdlg, WM_COMMAND, MAKELONG(F06DAD_PBT_YFULL, BN_CLICKED), 0);
									return TRUE;
								case F06DAD_PCB_TEXTS:
									j = SendMessage(GetDlgItem(hdlg, LOWORD(wparam)), CB_GETCURSEL, 0, 0);
									pD->p[_XF06DAD_TEXTBY].val.u = j;
									SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_TEMP_SCANDATA, 0);
									return TRUE;
								case F06DAD_PCB_GRIDS:
									j = SendMessage(GetDlgItem(hdlg, LOWORD(wparam)), CB_GETCURSEL, 0, 0);
									pD->p[_XF06DAD_GRIDBY].val.u = j;
									if (pScan) {
										hctl = GetDlgItem(hdlg, F06DAD_PGA_GRAPH);

										SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_XAXIS_TICKS_GRID,
																(LPARAM)((pD->p[_XF06DAD_GRIDBY].val.u & 0x1) ? TRUE : FALSE));
										SendMessage(hctl, MGG_GAM_SETPARAM, MGG_GPAR_YAXIS_TICKS_GRID,
																(LPARAM)((pD->p[_XF06DAD_GRIDBY].val.u & 0x2) ? TRUE : FALSE));
										SendMessage(hctl, MGG_GAM_UPDATE, 0, 0);
									}
									return TRUE;
								default:
									break;
							}
						}
						else if (HIWORD(wparam) == EN_KILLFOCUS) {
							switch(LOWORD(wparam)) {
								case F06DAD_PEB_SCANRANGE1:
									//wl max
									wlmax = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
									//get low
									hctl = GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE1);
									GetEditUval(hctl, &nval);
									//get high
									hctl = GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE2);
									GetEditUval(hctl, &nval2);
									//correct
									if (nval2.u <= nval.u) {
										if (nval2.u < wlmax) {
											nval2.u = nval.u + 1;
											SetEditUval(hctl, nval2);		//set high
											GetEditUval(hctl, &nval2);
										}
										if (nval.u >= nval2.u) {
											nval.u = nval2.u - 1;
											SetEditUval(GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE1), nval);		//set low
										}
									}
									dval.u = (nval.u & 0xFFFF) | (nval2.u << 16);
									if (dval.u != pD->p[_XF06DAD_WLSCANRANGE].val.u) {
										D_WriteFunction((HDEVICE)pD, F06DAD_WriteProp, _XF06DAD_WLSCANRANGE, dval, hdlg, WMD_RESPONSE);
										D_ReadFunction((HDEVICE)pD, F06DAD_ReadProp, _XF06DAD_WLSCANRANGE, hdlg, D_OL_WM_UPDATE);
									}
									return TRUE;
								case F06DAD_PEB_SCANRANGE2:
									//wl max
									wlmax = F06DAD_GetWavelengthByModel(pD->p[_XF06DAD_MODEL].val.u);
									//get high
									hctl = GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE2);
									GetEditUval(hctl, &nval2);
									//get low
									hctl = GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE1);
									GetEditUval(hctl, &nval);
									//correct
									if (nval.u >= nval2.u) {
										if (nval.u < wlmax) {
											nval.u = nval2.u - 1;
											SetEditUval(hctl, nval);		//set low
											GetEditUval(hctl, &nval);
										}
										if (nval2.u <= nval.u) {
											nval2.u = nval.u + 1;
											SetEditUval(GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE2), nval);		//set high
										}
									}
									dval.u = (nval.u & 0xFFFF) | (nval2.u << 16);
									if (dval.u != pD->p[_XF06DAD_WLSCANRANGE].val.u) {
										D_WriteFunction((HDEVICE)pD, F06DAD_WriteProp, _XF06DAD_WLSCANRANGE, dval, hdlg, WMD_RESPONSE);
										D_ReadFunction((HDEVICE)pD, F06DAD_ReadProp, _XF06DAD_WLSCANRANGE, hdlg, D_OL_WM_UPDATE);
									}
									return TRUE;
								case F06DAD_PEB_HALF:
									hctl = GetDlgItem(hdlg, LOWORD(wparam));
									GetEditUval(hctl, &nval);
									dval.u = nval.u;
									if (dval.u != pD->p[_XF06DAD_HALF].val.u) {
										D_WriteFunctionPrior((HDEVICE)pD, F06DAD_WriteProp, _XF06DAD_HALF, dval, hdlg, WMD_RESPONSE, MQM_PRIORITY_HIGHER);
										D_ReadFunctionPrior((HDEVICE)pD, F06DAD_ReadProp, _XF06DAD_HALF, hdlg, D_OL_WM_UPDATE, MQM_PRIORITY_HIGHER);
										D_ReadFunctionPrior((HDEVICE)pD, F06DAD_ReadProp, _XF06DAD_TEMP_WLRANGE, hdlg, D_OL_WM_UPDATE, MQM_PRIORITY_HIGHER);
									}
									return TRUE;

								default:
									break;
							}
						}
					}
				}
				break;

			case WM_NOTIFY:
				if (wparam == F06DAD_PGA_GRAPH) {
					hctl = GetDlgItem(hdlg, wparam);
					if (lparam) {
						pinfo = (MGGINFO *)lparam;
						switch (((NMHDR *)lparam)->code) {
							case MGG_GAN_RCLICK:
								ClientToScreen(hctl, &pinfo->pos);
								F06DAD_CreateScanExportPopup(hdlg, pinfo->pos.x, pinfo->pos.y, (void *)pScan);
								break;
						}
					}
				}
				break;

			case WM_CONTEXTMENU:
				break;

			case WM_TIMER:
				if (pScan) {
					if (IsWindowVisible(hdlg) && pScan->run) {
						if (pD && pD->n_p && pD->p) {
							SendMessage(hdlg, WM_COMMAND, MAKELONG(F06DAD_PBT_MAKESCAN, BN_CLICKED), 0);
							SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_WLSCANRANGE, 0);
							SendMessage(hdlg, D_OL_WM_UPDATE, _XF06DAD_HALF, 0);
						}
					}
				}
				break;

			case WM_CLOSE:
				//EndDialog(hdlg, 0);
				ShowWindow(hdlg, SW_HIDE);
				return TRUE;

			case WM_DESTROY:
				//stop timer
				KillTimer(hdlg, 1);

				if (pD->n_p && pD->p) {
					hctl = GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE1);
					ClearValidNumStrSubclassing(hctl);
					hctl = GetDlgItem(hdlg, F06DAD_PEB_SCANRANGE2);
					ClearValidNumStrSubclassing(hctl);
					hctl = GetDlgItem(hdlg, F06DAD_PEB_HALF);
					ClearValidNumStrSubclassing(hctl);
				}


				//destroy graph
				if (pScan) {
					free((void *)pScan);		//destroy memory
				}
				pScan = NULL;

				if (pPS) {
					if (pPS->hf)
						DeleteObject(pPS->hf);		//delete font
					free((void *)pPS);
				}
				pPS = NULL;
				SetWindowLong(hdlg, GWLP_USERDATA, (LONG)pPS);
				break;
		}
	}

	return FALSE;
}


//------------------------------------------------------------------
//*** function creates export popup menu
BOOL F06DAD_CreateScanExportPopup(HWND hwnd, int x, int y, void *param)
{
	HMENU hPM;
	BOOL bret;

	hPM = CreatePopupMenu();
	InsertMenu(hPM, -1, MF_STRING, 10001, D_("Export scan"));
	if (param) {
		//InsertMenu(hPM, -1, MF_STRING | (((XF06DAD_PSCAN *)param)->mm_enable ? MF_CHECKED : MF_UNCHECKED), 10002, TEXT("Show/hide global extrems"));
	}
	InsertMenu(hPM, -1, MF_SEPARATOR, 0, NULL);
	InsertMenu(hPM, -1, MF_STRING, 0, D_("hide menu"));

	bret = TrackPopupMenu(hPM, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, hwnd, NULL);

	if (bret == 1)
		return(TRUE);
	return(FALSE);
}

//*** function exports scen to file
DWORD F06DAD_ExportScan(HWND hwnd, TCHAR *filename, DWORD count, double *x, double *y, char *e, DWORD flags)
{
	FILE *fw;
	DWORD i;
	char *loc;

	if (count && (x == NULL || y == NULL))
		return(ERROR_INVALID_HANDLE);

	fw = __fopen(filename, TEXT("w"));
	if (fw == NULL)
		return(ERROR_FILE_NOT_FOUND);

	loc = SetCNumLocale("C");			//set localization

	//title
	__fprintf(fw, TEXT("Wavelength [nm]\t%s\terror\n"), (flags & 0x1) == 0 ? TEXT("Absorbance [mV]") : TEXT("Intensity [%]"));


	//go through scan
	for (i=0; i<count; i++) {
		__fprintf(fw, TEXT("%.0lf\t%.12lf\t%u\n"), x[i], y[i], e[i]);
	}

	setlocale(LC_NUMERIC, loc); 		//restore localization

	fclose(fw);

	return(NO_ERROR);
}

//-----------------------------------------------------------
//*** function exports graph control to EMF
DWORD F06DAD_ExportToEMF(HWND hGA, TCHAR *filename, int picw, int pich)
{
	MGCFEXPORT mfe;
//	MGGGRAPHPARAMS gparams = {0};

	//file export structure
	mfe.mask = MGC_FEXPMASK_TYPE |
						 MGC_FEXPMASK_FILEPATH |
						 MGC_FEXPMASK_BW;
	mfe.type = MGC_FILE_EMF;
	mfe.filepath = filename;
	mfe.bw = TRUE;

	//set graph parameters

	//export
	return(SendMessage(hGA, MGG_GAM_EXPORT, (WPARAM)&mfe, (LPARAM)NULL));
}


//==================================================================

//---------------------------------
#define F06DAD_TMP_FILENAME TEXT("f06dad_crr.tmp")
#define F06DAD_MAX_REPORTNAME_LEN 64
#define F06DAD_MAX_REPORTFLASH_LEN 256
#define F06DAD_MAX_REPORTRAM_LEN 128
typedef struct {
	DWORD flags;
	TCHAR name[F06DAD_MAX_REPORTNAME_LEN];
	TCHAR flash[F06DAD_MAX_REPORTFLASH_LEN];
	TCHAR ram[F06DAD_MAX_REPORTRAM_LEN];
} F06DADREPORTITEM;

//------------------------
//*** function processes IPROC function for working with strings
//IPTYPE_CREATE: iparam -> pointer to lparam of item, lparam -> LPARAM
//IPTYPE_DELETE: iparam -> pointer to lparam of item, lparam -> 0
//IPTYPE_PRINT: iparam -> pointer to lparam of item, lparam -> FILE *
BOOL F06DAD_ReportItemProcParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	if (iparam) {
		switch (type) {
			//--- processed after ITEM creation
			case IPTYPE_CREATE:
				if (lparam) {
					//allocation
					*iparam = (LPARAM)malloc(sizeof(F06DADREPORTITEM));
					if (*iparam == 0)
						return(FALSE);		//out of memory
					//copy data
					memcpy((F06DADREPORTITEM *)*iparam, (F06DADREPORTITEM *)lparam, sizeof(F06DADREPORTITEM));
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
						__fprintf((FILE *)lparam, TEXT("%s - %s (%s)"),
							((F06DADREPORTITEM *)*iparam)->name,
							((F06DADREPORTITEM *)*iparam)->flash,
							((F06DADREPORTITEM *)*iparam)->ram);
					}
					else
						__fprintf((FILE *)lparam, TEXT("(null)"));
				}
				break;
			//--- processed at ITEM compare
			case IPTYPE_COMPARE:
				return(0);		//don't sort
		}
	}
	return(TRUE);
}

//*** function searches text for F06DADREPORTITEM by name
BOOL F06DAD_SearchReportItemByName(char *str, const char *name, const char *flash, const char *ram, HLIST hlist)
{
	F06DADREPORTITEM item = {0};
	char *pt, *pt2, *ptend;
	int i = 0;

	if (str && *str && name && *name) {
		//--- flash ---
		if (flash && *flash && (pt = strstr(str, flash)) && *(pt+strlen(flash)) == ':') {
			i |= 0x1;
			//;
			if ((ptend = strchr(pt, ';'))) {
				*ptend = '\0';
				ptend++;
			}
			else
				ptend = pt+strlen(pt);
			//:
			pt2 = strchr(pt, ':');
			if (pt2) {
				//value
				w2strcpy(item.flash, pt2+1);
			}
			//!!!remove name + value
			strcpy(pt, ptend);
		}
		//--- ram ---
		if (ram && *ram && (pt = strstr(str, ram)) && *(pt+strlen(ram)) == ':') {
			i |= 0x2;
			//;
			if ((ptend = strchr(pt, ';'))) {
				*ptend = '\0';
				ptend++;
			}
			else
				ptend = pt+strlen(pt);
			//:
			pt2 = strchr(pt, ':');
			if (pt2) {
				//value
				w2strcpy(item.ram, pt2+1);
			}
			//!!!remove name + value
			strcpy(pt, ptend);
		}
		//--- name ---
		if (i) {
			w2strcpy(item.name, name);
			return(LIST_AppendItem(hlist, (LPARAM)&item));
		}
	}
	return(FALSE);
}

//*** function searches text for F06DADREPORTITEM
BOOL F06DAD_SearchReportItemRest(char *str, HLIST hlist)
{
	F06DADREPORTITEM item = {0};
	char *pt2, *ptend;

	if (str && *str) {
		//;
		if ((ptend = strchr(str, ';'))) {
			*ptend = '\0';
			ptend++;
		}
		else
			ptend = str+strlen(str);
		//:
		pt2 = strchr(str, ':');
		if (pt2) {
			*pt2 = '\0';
			//value
			if (strncmp(str, "F_", 2) == 0)
				w2strcpy(item.flash, pt2+1);
			else
				w2strcpy(item.ram, pt2+1);
		}
		//name
		w2strcpy(item.name, str);

		//!!!remove name + value
		strcpy(str, ptend);

		return(LIST_AppendItem(hlist, (LPARAM)&item));
	}
	return(FALSE);
}

//*** function searches text for F06DADREPORTITEM
BOOL F06DAD_InsertTitleItem(const char *title, HLIST hlist)
{
	F06DADREPORTITEM item = {0};

	if (title && *title) {
		w2strcpy(item.name, title);
		return(LIST_AppendItem(hlist, (LPARAM)&item));
	}
	return(FALSE);
}

//*** function gets report
DWORD F06DAD_GetReport(HDEVICE hD)
{
	D_DEVICE *pD;
	HLIST hreport;
	DWORD ret;
	char *ttext, *ptext;
	unsigned int tout;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);		//error
	pD = (D_DEVICE *)hD;

	hreport = pD->p[_XF06DAD_TEMP_CRR].val.h;
	if (hreport == NULL)
		return(ERROR_INVALID_HANDLE);
	LIST_RemoveAll(hreport);		//clear list

	strcpy(pD->com.sendbuf, "#CRr\n");
	//send and read
	tout = pD->com.timeout;		//store previous timeout
	pD->com.timeout = 3000;		//longer timeout
	ret = F06DAD_SendAndRcptStr(&pD->com);
	pD->com.timeout = tout;		//restore previous timout
	if (ret != NO_ERROR)
		return(ret);

	//decode response
	ttext = pD->com.rcptbuf;
	if (strstr(ttext, "CRrERR") != NULL)
		return(1);		//value not available
	else if (strncmp(ttext, "CRr", 3) == 0) {
		ptext = ttext+3;

		//main values
		F06DAD_SearchReportItemByName(ptext, "DEVICE_NAME", "DEVICE_NAME", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "MODEL", "MODEL", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "HARDWARE", "HARDWARE", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "SOFTWARE", "SOFTWARE", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "SERIALNO", "SNUMB", NULL, hreport);
		//calibration values
		F06DAD_InsertTitleItem("Calibration values", hreport);
		F06DAD_SearchReportItemByName(ptext, "CV_FD_254", "F_CV_FD_254", "CV_FD_254", hreport);
		F06DAD_SearchReportItemByName(ptext, "CV_QFD_254", "F_CV_QFD_254", "CV_QFD_254", hreport);
		F06DAD_SearchReportItemByName(ptext, "CV_WLC_COEF", "F_CV_WLC_COEF", "CV_WLC_COEF", hreport);
		F06DAD_SearchReportItemByName(ptext, "CV_HWLC_COEF", "F_CV_HWLC_COEF", "CV_HWLC_COEF", hreport);
		F06DAD_SearchReportItemByName(ptext, "CV_QWLC_COEF", "F_CV_QWLC_COEF", "CV_QWLC_COEF", hreport);
		F06DAD_SearchReportItemByName(ptext, "CV_ABS", "F_CV_ABS", "CV_ABS", hreport);
		F06DAD_SearchReportItemByName(ptext, "CV_TEXP", "F_CV_TEXP", "CV_TEXP", hreport);
		F06DAD_SearchReportItemByName(ptext, "CV_NMA_LAMP", "F_CV_NMA_LAMP", "NMA_LAMP", hreport);
		//last values
		F06DAD_InsertTitleItem("Last values", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_LIFE_TIME", NULL, "LIFE_TIME", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_LIFE_TIME_ARRAY", "F_LV_LIFE_TIME_ARRAY", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_ERRORS", NULL, "LV_ERRORS", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_ERRORS_ARRAY", "F_LV_ERRORS_ARRAY", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_WL_CHA", "F_LV_WL_CHA", "WL_CHA", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_WL_CHB", "F_LV_WL_CHB", "WL_CHB", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_WL_CHC", "F_LV_WL_CHC", "WL_CHC", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_WL_CHD", "F_LV_WL_CHD", "WL_CHD", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_WL_START", "F_LV_WL_START", "WL_START", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_WL_STOP", "F_LV_WL_STOP", "WL_STOP", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_WL_CAL", "F_LV_WL_CAL", "WL_CAL", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_RESP_CAL", "F_LV_RESP_CAL", "RESP_CAL", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_DES_EXPOSURE", "F_LV_DES_EXPOSURE", "DES_EXPOSURE", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_HALF_WIDTH", "F_LV_HALF_WIDTH", "HALF_WIDTH", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_TIME_CONST", "F_LV_TIME_CONST", "TIME_CONST", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_MDF_CHD", "F_LV_MDF_CHD", "MDF_CHD", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_MDF_ARTH", "F_LV_MDF_ARTH", "MDF_ARTH", hreport);
		F06DAD_SearchReportItemByName(ptext, "LV_SUB_FREQ", "F_LV_SUB_FREQ", NULL, hreport);
		//information values
		F06DAD_InsertTitleItem("Information values", hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_LAMP_IDF", "IV_F_LAMP_IDF", "IV_LAMP_IDF", hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_LAMP_STA", "IV_F_LAMP_STA", "IV_LAMP_STA", hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_PS36_STA", "IV_F_PS36_STA", "IV_PS36_STA", hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_VN_ARC", "F_IV_VN_ARC", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_UHG_LOW", "F_IV_UHG_LOW", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_UHG_HIGH", "F_IV_UHG_HIGH", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_UA_NOLD", "F_IV_UA_NOLD", NULL, hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_WL_MIN", "F_IV_WL_MIN", "WL_MIN", hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_WL_MAX", "F_IV_WL_MAX", "WL_MAX", hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_ACT_EXPOSURE", "F_IV_ACT_EXPOSURE", "ACT_EXPOSURE", hreport);
		F06DAD_SearchReportItemByName(ptext, "IV_FD_MAX", "F_IV_FD_MAX", "FD_MAX", hreport);
		//measure values
		F06DAD_InsertTitleItem("Measured values", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_I_ANODE", NULL, "MV_I_ANODE", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_U_ANODE", NULL, "MV_U_ANODE", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_U_HEAT", NULL, "MV_U_HEAT", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_I_BULB", NULL, "MV_I_BULB", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_U_BULB", NULL, "MV_U_BULB", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_I_FAN", NULL, "MV_I_FAN", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_U_FAN", NULL, "MV_U_FAN", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_U_INPUT", NULL, "MV_U_INPUT", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_U_ANAP", NULL, "MV_U_ANAP", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_U_ANAN", NULL, "MV_U_ANAN", hreport);
		F06DAD_SearchReportItemByName(ptext, "MV_U_DIG", NULL, "MV_U_DIG", hreport);
		//other values
		F06DAD_InsertTitleItem("Other values", hreport);
		F06DAD_SearchReportItemByName(ptext, "SV_FWRSTR", "F_SV_FWRSTR", NULL, hreport);

		while (F06DAD_SearchReportItemRest(ptext, hreport))
			;

	}
	else
		return(2);		//none or bad response
	return(NO_ERROR);
}

//*** function create CR report RTF-file
BOOL F06DAD_CreateCRFile(HLIST hreport, const TCHAR *filename)
{
	HITEM hitem;
	F06DADREPORTITEM *pdri;
	FILE *fw;
	DWORD i, t, e;

	if (filename == NULL)
		return(FALSE);

	fw = __fopen(filename, TEXT("w"));
	if (fw) {
		//--- create RTF header
		__fprintf(fw,
							TEXT("{\\rtf1\\ansi\\ansicpg1250\\deff0\\deflang1029"		/* header */
							"{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Microsoft Sans Serif;}"		/* fonts */
							"{\\f1\\fmodern\\fprq1\\fcharset238 Courier New CE;}}\n"
							"{\\colortbl ;"		/* colors */
							"\\red250\\green0\\blue0;"
							"\\red0\\green250\\blue0;"
							"\\red0\\green0\\blue250;"
							"\\red100\\green100\\blue100;}\n"
							));

		//read report
		if (LIST_GetSize(hreport)) {
			//--- get statistics
			hitem = LIST_GetItem(hreport, LPOS_FIRST, 0);
			e = 0;
			t = 0;
			while (hitem) {
				if (LIST_GetItemValue(hitem, (void *)&pdri) && pdri) {
					//set flags
					if (*pdri->ram == '\0' && *pdri->flash == '\0')
						;
					else if (*pdri->ram && *pdri->flash && __strcmp(pdri->flash, pdri->ram) != 0) {
						e++;
						t++;
					}
					else
						t++;
				}
				//get next item
				hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);
			}

			//--- create RTF title
			__fprintf(fw, TEXT("Total items / memory sync. errors: %lu / %s%lu%s\\par\\par\n"),
								t,
								e ? TEXT("\\cf1\\b ") : TEXT(""),
								e,
								e ? TEXT("\\b0\\cf0 ") : TEXT(""));

			__fprintf(fw,
								TEXT("\\pard\\tx3000\\tx5000\n"
										 "\\f0\\fs22\n"
										 "\\ul\\b Name\\tab FLASH value\\tab RAM value\\b0\\ul0\\par\\par\n"
										 "\\fs18\n"
										 ));


			i = 0;
			hitem = LIST_GetItem(hreport, LPOS_FIRST, 0);
			while (hitem) {
				//get item value
				if (LIST_GetItemValue(hitem, (void *)&pdri) && pdri) {
					//set flags
					if (*pdri->ram == '\0' && *pdri->flash == '\0')
						pdri->flags |= 0x1;		//title
					else if (*pdri->ram && *pdri->flash && __strcmp(pdri->flash, pdri->ram) != 0)
						pdri->flags |= 0x10;		//comparison error

					//--- write RTF line
					//line begin


					if (pdri->flags & 0x01)
						__fprintf(fw, TEXT("\\par\\b\\i "));
					if (pdri->flags & 0x10)
						__fprintf(fw, TEXT("\\cf1\\b "));
					//data itself
					__fprintf(fw, TEXT("%s\\tab %s\\tab %s"),
										pdri->name,
										pdri->flash,
										pdri->ram);
					//line end
					if (pdri->flags & 0x01)
						__fprintf(fw, TEXT("\\b0\\i0 "));
					if (pdri->flags & 0x10)
						__fprintf(fw, TEXT("\\b0\\cf0"));
					__fprintf(fw, TEXT("\\par\n"));
				}

				//get next item
				hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);
				i++;
			}

			__fprintf(fw, TEXT("\\par"));
		}
		else {
			//--- create RTF title
			__fprintf(fw,	TEXT("\\f0\\fs32\n\\par\\cf1\\b\\tab Communication error!\\cf0\\b0\n"));
		}

		//--- end RTF file
		__fprintf(fw, TEXT("}"));

		fclose(fw);

		return(TRUE);
	}
	return(FALSE);
}

//------------------------------------------------------------
//*** function processes report dialog
#define F06DAD_CRRDLG_BW 100
#define F06DAD_CRRDLG_BH 20
BOOL CALLBACK F06DAD_ReportDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	D_DEVICE *pD = NULL;
	HWND hctl;
	RECT rc;
	TCHAR *ptext = NULL;

	if (msg == WM_INITDIALOG) {
		pD = (D_DEVICE *)lparam;		//device handle
		if (pD == NULL || pD->n_p == 0 || pD->p == NULL)
			EndDialog(hdlg, -1);		//error

		SetWindowLong(hdlg, GWLP_USERDATA, (LONG)pD);		//store handle !!!!!

		//--- controls ---
		GetClientRect(hdlg, &rc);
		//editbox
		hctl = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Richedit"), TEXT(""),
													WS_CHILD | WS_VISIBLE | WS_TABSTOP |
													WS_HSCROLL | WS_VSCROLL | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE,
													rc.left, rc.top, rc.right, rc.bottom-F06DAD_CRRDLG_BH,
													hdlg, (HMENU)20, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		SetReadOnlySubclassing(hctl);

		//refresh button
		hctl = CreateWindow(CLASS_BUTTON, D_("Refresh"),
												WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												rc.left, rc.top+rc.bottom-F06DAD_CRRDLG_BH, F06DAD_CRRDLG_BW, F06DAD_CRRDLG_BH,
												hdlg, (HMENU)11, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//save button
		hctl = CreateWindow(CLASS_BUTTON, D_("Save to file"),
												WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												rc.left+F06DAD_CRRDLG_BW, rc.top+rc.bottom-F06DAD_CRRDLG_BH, F06DAD_CRRDLG_BW, F06DAD_CRRDLG_BH,
												hdlg, (HMENU)12, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//decode err. button
		hctl = CreateWindow(CLASS_BUTTON, D_("Decode errors"),
												WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												rc.left+2*F06DAD_CRRDLG_BW, rc.top+rc.bottom-F06DAD_CRRDLG_BH, F06DAD_CRRDLG_BW, F06DAD_CRRDLG_BH,
												hdlg, (HMENU)13, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		EnableWindow(hctl, FALSE);
		//cancel button
		hctl = CreateWindow(CLASS_BUTTON, D_("Close"),
												WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												rc.left+rc.right-F06DAD_CRRDLG_BW, rc.top+rc.bottom-F06DAD_CRRDLG_BH, F06DAD_CRRDLG_BW, F06DAD_CRRDLG_BH,
												hdlg, (HMENU)IDCANCEL, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));


		//center window
		CenterParentWindow(hdlg);
		//get report
		SendMessage(hdlg, WM_USER+106, 0, 0);
		//hide
		ShowWindow(hdlg, SW_HIDE);
	}
	else {
		pD = (D_DEVICE *)GetWindowLong(hdlg, GWLP_USERDATA);		//get device handle

		//process rest messages
		switch(msg) {
			//make report request
			case (WM_USER+105):
				D_ReadFunction((HDEVICE)pD, F06DAD_ReadProp, _XF06DAD_TEMP_CRR, hdlg, WM_USER+106);
				return TRUE;

			//update report
			case (WM_USER+106):
				ptext = D_TempDirAllocFilename((HDEVICE)pD, F06DAD_TMP_FILENAME);
				if (ptext) {
					if (F06DAD_CreateCRFile(pD->p[_XF06DAD_TEMP_CRR].val.h, ptext)) {
						F06DAD_FillRichEditFromFile(GetDlgItem(hdlg, 20), ptext);
						DeleteFile(F06DAD_TMP_FILENAME);
					}
					free((void *)ptext);
				}
				return TRUE;

			case WM_SIZE:
				rc.left = 0;
				rc.right = LOWORD(lparam);
				rc.top = 0;
				rc.bottom = HIWORD(lparam);
				//richedit
				MoveWindow(GetDlgItem(hdlg, 20), rc.left, rc.top, rc.right, rc.bottom-F06DAD_CRRDLG_BH, FALSE);
				//refresh button
				MoveWindow(GetDlgItem(hdlg, 11), rc.left, rc.top+rc.bottom-F06DAD_CRRDLG_BH, F06DAD_CRRDLG_BW, F06DAD_CRRDLG_BH, FALSE);
				//save button
				MoveWindow(GetDlgItem(hdlg, 12), rc.left+F06DAD_CRRDLG_BW, rc.top+rc.bottom-F06DAD_CRRDLG_BH, F06DAD_CRRDLG_BW, F06DAD_CRRDLG_BH, FALSE);
				//decode err. button
				MoveWindow(GetDlgItem(hdlg, 13), rc.left+2*F06DAD_CRRDLG_BW, rc.top+rc.bottom-F06DAD_CRRDLG_BH, F06DAD_CRRDLG_BW, F06DAD_CRRDLG_BH, FALSE);
				//cancel button
				MoveWindow(GetDlgItem(hdlg, IDCANCEL), rc.left+rc.right-F06DAD_CRRDLG_BW, rc.top+rc.bottom-F06DAD_CRRDLG_BH, F06DAD_CRRDLG_BW, F06DAD_CRRDLG_BH, FALSE);

				InvalidateRect(hdlg, NULL, TRUE);
				return TRUE;

			case WM_COMMAND:
				if (lparam) {
					switch (LOWORD(wparam)) {
						case IDCANCEL:
							ShowWindow(hdlg, SW_HIDE);
							return TRUE;
						case 11:		//refresh
							SendMessage(hdlg, WM_USER+105, 0, 0);
							return TRUE;
						case 12:		//save
#ifndef DEVICE_ALONE
							ptext = malloc(MAX_PATH*sizeof(TCHAR));
							if (ptext) {
								__strcpy(ptext, TEXT("f06dad_report.rtf"));
								if (DLG_GetSaveFile(g_hInst, hdlg, D_("Save to file"), g_chSetup.datadir, ptext,
																	  IDS_RICHEXTFILTER, NULL, &ptext)) {
									if (!F06DAD_CreateCRFile(pD->p[_XF06DAD_TEMP_CRR].val.h, ptext))
										ErrorBox(hdlg, TEXT("File not saved!"), 0);
								}
								free((void *)ptext);
							}
							ptext = NULL;
#endif
							return TRUE;
						case 13:		//decode errors
							//create error sub-dialog -> lparam = GetDlgItem(hdlg, IDC_RE_REPORT)
							//DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_DAD_CHECK_CRERR), hdlg, DlgCheckCRErrorsProc, (LPARAM)GetDlgItem(hdlg, IDC_RE_REPORT));
							return TRUE;
					}
				}
				break;

			case WM_CLOSE:
				//EndDialog(hdlg, 0);
				ShowWindow(hdlg, SW_HIDE);
				break;

			case WM_DESTROY:
				ClearReadOnlySubclassing(GetDlgItem(hdlg, 20));

				pD = NULL;
				break;
		}


	}
	return FALSE;
}


//------------------------------------------------------------
//*** function process richedit stream callback
DWORD CALLBACK F06DAD_EditStreamCallback(DWORD_PTR dwCookie, LPBYTE lpBuff,
                                  LONG cb, PLONG pcb)
{
	HANDLE hFile;

	hFile = (HANDLE)dwCookie;
	if (ReadFile(hFile, lpBuff, cb, (DWORD *)pcb, NULL))
		return 0;
	return -1;
}

//*** function fills richedit by file
BOOL F06DAD_FillRichEditFromFile(HWND hwnd, LPCTSTR pszFile)
{
	BOOL fSuccess = FALSE;
	HANDLE hFile;
	EDITSTREAM es = {0};

	hFile = CreateFile(pszFile,
										 GENERIC_READ,
										 FILE_SHARE_READ,
										 0,
										 OPEN_EXISTING,
										 FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
    es.pfnCallback = F06DAD_EditStreamCallback;
    es.dwCookie = (DWORD_PTR)hFile;
    if (SendMessage(hwnd, EM_STREAMIN, SF_RTF, (LPARAM)&es) && es.dwError == 0) {
			fSuccess = TRUE;
    }
    CloseHandle(hFile);
	}
	return fSuccess;
}


//----------------------------
//*** function decode error from string
unsigned int F06DAD_DecodeErrorA(const char *str)
{
	unsigned int err = 0;

	err |= HexStringNumNToIntA(str, 2);
	err |= (HexStringNumNToIntA(str+2, 2) << 8);
	err |= (HexStringNumNToIntA(str+4, 2) << 16);
	err |= (HexStringNumNToIntA(str+6, 2) << 24);
	return(err);
}


//*** function prints error
int F06DAD_PrintError(unsigned int err, int bytes, TCHAR *str)
{
	//test string pointer
	if (str == NULL)
		return(-1);

	//check bytes
	if (bytes > 4)
		bytes = 4;
	//print
	while (bytes) {
		__sprintf(str, TEXT("%02X"), err & 0xFF);
		err >>= 8;
		str += 2;
		bytes--;
	}
	*str = '\0';
	return(0);
}

//*** function gets top error mask
DWORD F06DAD_GetTopError(DWORD errs)
{
	DWORD e = 0;
	int i;

	if (errs) {
		i = 0x1;
		e++;
		while ((errs & i) == 0) {
			i <<= 1;
			e++;
		}
	}
	return(e);
}

//*** function gets description of errors
const TCHAR *F06DAD_GetErrorDescription(int err_no)
{
	const TCHAR *ptext = NULL;

	if (err_no == 0) {
		ptext = TEXT("No error.");
	}
	else {
		switch (1<<(err_no-1)) {
			//ERR0
			case 0x00000001: ptext = TEXT("ERR0 bit0: Bad block of High Voltage (HV) generator for lamp or "
																		"disconnected lamp or bad lamp."); break;
			case 0x00000002: ptext = TEXT("ERR0 bit1: Bad power supply of lamp heater voltage."); break;
			case 0x00000004: ptext = TEXT("ERR0 bit2: Bad power supply of lamp anodic voltage."); break;
			case 0x00000008: ptext = TEXT("ERR0 bit3: Bad power supply of detector analog voltage."); break;
			case 0x00000010: ptext = TEXT("ERR0 bit4: Fourth cycle of lamp ignition fails."); break;
			case 0x00000020: ptext = TEXT("ERR0 bit5: Lamp spontaneously douse during unit working."); break;
			case 0x00000040: ptext = TEXT("ERR0 bit6: Lamp ignition fails after short douse in AUTOZERO function and "
																		"repeated High Voltage impuls and heater cycle fails too."); break;
			case 0x00000080: ptext = TEXT("ERR0 bit7: Repetitive error of data receiving from power supply."); break;
			//ERR1
			case 0x00000100: ptext = TEXT("ERR1 bit0: Repetitive error of data transmitting to power supply."); break;
			case 0x00000200: ptext = TEXT("ERR1 bit1: Bad identification of maximum exposition value during automatic "
																		"max. exposition settings caused by unworkable value of light intensity."); break;
			case 0x00000400: ptext = TEXT("ERR1 bit2: Bad identification of maximum exposition value during automatic "
																		"max. exposition settings caused by unworkable value of light intensity."); break;
			case 0x00000800: ptext = TEXT("ERR1 bit3: Low light intensity was found on some photo elements of CCD "
																		"sensor during automatic setting of amplification."); break;
			case 0x00001000: ptext = TEXT("ERR1 bit4: Required wavelength is out of range of CCD sensor during "
																		"measurement of absorbance, intensity or scan."); break;
			case 0x00002000: ptext = TEXT("ERR1 bit5: Over flow of lamp lifetime counter (>9999.99 hours). "
																		"Counter works from zero after this errors occurs."); break;
			case 0x00004000: ptext = TEXT("ERR1 bit6: Spontaneously failure on heater power supply of lamp "
																		"during detector working."); break;
			case 0x00008000: ptext = TEXT("ERR1 bit7: Spontaneously failure on analog power supply "
																		"during detector working."); break;
			//ERR2
			case 0x00010000: ptext = TEXT("ERR2 bit0: Error during calibration of intensity caused by low base signal or "
																		"AUTOZERO was not proceeded before calibration."); break;
			case 0x00020000: ptext = TEXT("ERR2 bit1: Error during calibration of intensity caused by error of reference "
																		"background or AUTOZERO was not proceeded before calibration."); break;
			case 0x00040000: ptext = TEXT("ERR2 bit2: Error during calibration of absorbance, error of under flow, "
																		"over flow or reference background during base absorbance count."); break;
			case 0x00080000: ptext = TEXT("ERR2 bit3: Error during calibration of absorbance, negative base absorbance."); break;
			case 0x00100000: ptext = TEXT("ERR2 bit4: Error during calibration of absorbance, low base absorbance or "
																		"high requested response."); break;
			case 0x00200000: ptext = TEXT("ERR2 bit5: Error during calibration of absorbance, high base absorbance or "
																		"low requested response."); break;
			case 0x00400000: ptext = TEXT("ERR2 bit6: Error during automatic setting of base offset of unit, bad CCD "
																		"sensor, monochromator is open."); break;
			case 0x00800000: ptext = TEXT("ERR2 bit7: Error incompatible firmware used."); break;
			//ERR3
			case 0x01000000: ptext = TEXT("ERR3 bit0: Bulb current is out of range."); break;
			case 0x02000000: ptext = TEXT("ERR3 bit1: Any fan is not working."); break;
			case 0x04000000: ptext = TEXT("ERR3 bit2: Unknown error."); break;
			case 0x08000000: ptext = TEXT("ERR3 bit3: Unknown error."); break;
			case 0x10000000: ptext = TEXT("ERR3 bit4: Unknown error."); break;
			case 0x20000000: ptext = TEXT("ERR3 bit5: Unknown error."); break;
			case 0x40000000: ptext = TEXT("ERR3 bit6: Unknown error."); break;
			case 0x80000000: ptext = TEXT("ERR3 bit7: Unknown error."); break;
		}
	}
	return(ptext);
}

//*** function gets description of status
const TCHAR *F06DAD_GetStatusDescription(int status)
{
	const TCHAR *ptext = NULL;
	static TCHAR ttext[16];

	switch (status) {
		case _XF06DAD_ST_IDLE: ptext = D_("Idle"); break;
		case _XF06DAD_ST_INIT: ptext = D_("Init lamp"); break;
		case _XF06DAD_ST_MEAS: ptext = D_("Meas. mode"); break;
		case _XF06DAD_ST_ZERO: ptext = D_("Autozero"); break;
		case _XF06DAD_ST_SABS: ptext = D_("Scanning abs."); break;
		case _XF06DAD_ST_SINT: ptext = D_("Scanning int."); break;
		case _XF06DAD_ST_UMOD: ptext = D_("User cal."); break;
		case _XF06DAD_ST_SELF: ptext = D_("Self test"); break;
		case _XF06DAD_ST_FMOD: ptext = D_("Factory cal."); break;
		case _XF06DAD_ST_ASCA: ptext = D_("Cont. scan"); break;
		default:
			__sprintf(ttext, TEXT("Unknown %u"), status & 0xFF);
			ptext = ttext;
			break;
	}
	return(ptext);
}

/** @brief Decode errors to message box
 *
 * @param hwnd HWND
 * @param err_val BIGPROT_ERRVAL*
 * @return BOOL
 *
 */
BOOL F06DAD_ErrorsDecodeBox(HWND hwnd, unsigned int err_code, const TCHAR *title, const TCHAR *descr_fce(int))
{
	unsigned int err_no = 1, n = 0;
	unsigned int len, total_len;
	const TCHAR *ptext;
	TCHAR *pstr;

	if (descr_fce == NULL)
		return(FALSE);
	//
	total_len = 128;
	pstr = (TCHAR *)malloc(total_len*sizeof(TCHAR));
	if (pstr == NULL)
		return(FALSE);
	*pstr = '\0';

	//through bites
	while (err_code) {
		if (err_code & 0x1) {
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
		err_code >>= 1;
		err_no++;
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

//-----------------------
//*** special function (show spectrum)
BOOL F06DAD_Special(HDEVICE hD, DSPECIAL specidx, LPARAM lparam)
{
	switch ((unsigned int)specidx) {
		//show spectrum dialog
		case DSPEC_SHOWSPECTRUM:
			if (hD) {
				HANDLE h = ((D_DEVICE *)hD)->p[_XF06DAD_TEMP_ONLINE].val.h;
				if (h)
					SendMessage(h, WM_COMMAND, MAKEWPARAM(F06DAD_OLBT_SCAN, BN_CLICKED), 1);
			}
			return TRUE;
		//spectrum is available
		case DSPEC_ASK(DSPEC_SHOWSPECTRUM):
			return TRUE;

		//lamp control
		case DSPEC_LAMPCTRL:
			F06DAD_SetLamp(hD, 0, (BOOL)lparam);
			return TRUE;
		//lamp control is available
		case DSPEC_ASK(DSPEC_LAMPCTRL):
			return TRUE;

		default: break;
	}
	return(FALSE);
}

