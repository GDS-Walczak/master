/*
 * chdevice.c
 *
 * Devices - source file
 *
 * Author: Filip Kinovic
 * Version: 3.4
 * Date: 25.01.2016
*/

#ifdef DEBUG
	//#define D_SIMUL_ALL
#endif


/*
--- Tested devices [other languages] ---
//DETECTORS
Aquilon					YES	[cs]
LCD2070					YES	[cs]
LCD2071.x				YES	[cs] (LCD2071.3, LCD2071.4)
LCD2073					NO	[]
LCDA2073				YES	[cs]
LCD2083.6				YES	[cs]
LCD2083.7				YES	[cs]
LCD2084.x				YES (communication not stable)	[cs]
Opal						YES	[cs]
Opal100					NO	[]
Ruby						YES	[cs]
Sapphire				YES	[cs]
Topaz						YES	[cs]
Opal-Demo				YES	[cs] /sim
FLASH 06 DAD  	YES	[cs]
FLASH 06 SINGLE YES	[cs]
FLASH 06 DUAL		YES	[]
SHODEX RI-10X		YES (testing, errorous communication)	[cs]
MALACHITE 			YES [no]
RF-10AXL	 			YES [no]
YL9160	 			  YES (not completed) [no]
ECD2000					YES (not completed) /sim
TOYDAD					YES (not completed) /sim
ECDA2000				YES [cs] /sim

//PUMPS
Alpha10					YES	[cs]
Alpha50					YES	[cs]
Alpha100				YES	[cs]
Beta10					YES	[cs]
Beta50					YES	[cs]
LCP4100					YES	(communication stable from FW version >= 3.1) [cs]
LCP4050					YES (communication not stable)	[cs]
ATL_ALPHA				YES [no]
ATL_SAPPHIRE		YES [no]
ATL_KAPPA				YES [no]
PCPP2C					YES
SEPAR3					YES	[no]
LINAR1000				YES
PP03C-G					YES [cs] /sim (PP03)
KAPPA						YES	[cs]
THETA						YES	[cs]
IOTA						YES	[cs]
SEPAR-T					YES	[cs] /sim
SEPAR-T2				YES	[cs] /sim
ECP2000					YES [no] /sim
P6000						YES [no]

//THERMOSTATS
LCO102					YES	[cs]
TV10+						YES	[cs]
LCT5100					YES [cs] /sim
ECO2000					YES (not completed)

//THERMOMETERS
TM-917					YES [cs]
ALMEMO2490			YES [cs]

//MULTIMETERS
MS8218					YES [cs]
HP34401A				YES (not completed)	[cs]

//AUTOSAMPLERS
AS54						NO	[]
HT3X0L					YES [cs] (HT300L, HT310L, HT300LV)

//SOURCES
AC250K1D				NO	[]

//CONVERTERS
SM Loop					YES	[cs]
Panther1000			YES [cs]
Panda88					YES [cs]
Panda30					YES [cs]
Panda30HID			YES [cs]
ORCA2800				YES [cs]

//COUNTERS
(BK)1823A				YES	[cs]

//OTHERS
MIREK01					YES	[cs]
MIREK02					YES	[cs]
MIREK03					YES	[no]
MIREK04					YES	[no]
MIREK-CCD				YES	[no]
NIELS-BOHR			YES	[cs]
CENTOR-EASY			YES	[cs]
TEST-DEVICE			YES [cs] /sim
F06LAMPTEST			YES [no]
QSTART					YES [no]
AAA-SAMPLE			YES [no]
DT8852					YES [cs]
ECOM-DIPS				YES [no]
ECX2000_FIFO		YES [no]
USB-PH-M				YES []
HM8135					YES []
SYS-DEVICE			YES []
LOUIS-PASTEUR	  YES []
F06CCD					YES []

//BALANCES
PB3002					YES [cs]

//FRACTION COLLECTORS
FC203B					YES [cs]
CHF122SC				YES [cs]
FC20						YES [cs]
SEPARFLOW				YES [cs] /sim
SPIDER					YES	[cs]
VALCOV					YES []
ECF2000					YES [cs] /sim


*/


#include <windows.h>
#include <locale.h>
#include <commctrl.h>
#include <math.h>
#include <process.h>
#include "simplexml.h"
#include "mygctrls.h"
#include "chdevice.h"
#ifndef DEVICE_ALONE
	#include "chversion.h"
	#define D_PROGTITLE CH_PROGTITLE
	#define D_PROGVER CH_VER
#else
	#pragma message "Set version in device.c!"
	#define D_PROGTITLE TEXT("Prog")
	#define D_PROGVER TEXT("1.0")
#endif // DEVICE_ALONE
#include "commfce.h"
#include "res.h"
#include "memwatch.h"

/* devices */
//advantec
#include "devices/advantec/xchf122sc.h"
//agilent
#include "devices/agilent/xhp34401a.h"
//ahlborn
#include "devices/ahlborn/xalmemo2490.h"
//analtech
#include "devices/analtech/xatl_alpha.h"
#include "devices/analtech/xatl_sapphire.h"
#include "devices/analtech/xatl_kappa.h"
//andilog
#include "devices/andilog/xcentoreasy.h"
//bjcxth
#include "devices/bjcxth/xp6000.h"
//bk precision
#include "devices/bk_precision/xbk1823a.h"
//cem
#include "devices/cem/xdt8852.h"
//ecom
#include "devices/ecom/xa2073.h"
#include "devices/ecom/xaaasample.h"
#include "devices/ecom/xalpha.h"
#include "devices/ecom/xapatit.h"
#include "devices/ecom/xaquilon.h"
#include "devices/ecom/xbeta.h"
#include "devices/ecom/xecd2000.h"
#include "devices/ecom/xecda2000.h"
#include "devices/ecom/xecf2000.h"
#include "devices/ecom/xeco2000.h"
#include "devices/ecom/xecx2000t01.h"
#include "devices/ecom/xecom_dips.h"
#include "devices/ecom/xecp2000.h"
#include "devices/ecom/xf06ccd.h"
#include "devices/ecom/xf06dad.h"
#include "devices/ecom/xf06dual.h"
#include "devices/ecom/xf06lamptest.h"
#include "devices/ecom/xf06single.h"
#include "devices/ecom/xiota.h"
#include "devices/ecom/xkappa.h"
#include "devices/ecom/xlcd2070.h"
#include "devices/ecom/xlcd2071_x.h"
#include "devices/ecom/xlcd2083_6.h"
#include "devices/ecom/xlcd2083_7.h"
#include "devices/ecom/xlcd2083_x_sm.h"
#include "devices/ecom/xlcd2084.h"
#include "devices/ecom/xlco102.h"
#include "devices/ecom/xlcp4050.h"
#include "devices/ecom/xlcp4100.h"
#include "devices/ecom/xlouispasteur.h"
#include "devices/ecom/xloopsm.h"
#include "devices/ecom/xmalachite.h"
#include "devices/ecom/xmirek01.h"
#include "devices/ecom/xmirek02.h"
#include "devices/ecom/xmirek03.h"
#include "devices/ecom/xmirek04.h"
#include "devices/ecom/xmirek_ccd.h"
#include "devices/ecom/xnielsbohr.h"
#include "devices/ecom/xopal.h"
#include "devices/ecom/xopaldemo.h"
#include "devices/ecom/xorca2800.h"
//#include "devices/ecom/xpanda30.h"
//#include "devices/ecom/xpanda30hid.h"
#include "devices/ecom/xpanda88.h"
#include "devices/ecom/xpanther1000.h"
#include "devices/ecom/xruby.h"
#include "devices/ecom/xspider.h"
#include "devices/ecom/xsapphire.h"
#include "devices/ecom/xsysdev.h"
#include "devices/ecom/xtestdev.h"
#include "devices/ecom/xtheta.h"
#include "devices/ecom/xtopaz.h"
#include "devices/ecom/xtoydad.h"
#include "devices/ecom/xtv10p.h"
//gilson
#include "devices/gilson/xfc203b.h"
//hameg
#include "devices/hameg/xhm8135.h"
//hta
#include "devices/hta/xht3x0l.h"
//ingos
#include "devices/ingos/xlct5100.h"
//jai
#include "devices/jai/xri700ii.h"
//kromaton
#include "devices/kromaton/xfcpca.h"
//labio
#include "devices/labio/xpcpp2c.h"
#include "devices/labio/xfc20.h"
#include "devices/labio/xlinar1000.h"
//lutron
#include "devices/lutron/xtm917.h"
//mastech
#include "devices/mastech/xms8218.h"
//mettle toledo
#include "devices/mettler_toledo/xpb3002.h"
//rotachrom
#include "devices/rotachrom/xicpc.h"
//separlab
#include "devices/separlab/xsepar3.h"
#include "devices/separlab/xpp03.h"
#include "devices/separlab/xsepar510.h"
#include "devices/separlab/xsepar_t.h"
#include "devices/separlab/xsepar_t2.h"
//shimadzu
#include "devices/shimadzu/xrf10axl.h"
//shodex
#include "devices/shodex/xshodex_ri10x.h"
//sisw
#include "devices/sisw/xusbphm.h"
//valco
#include "devices/valco/xvici_act.h"
//young lin
#include "devices/young_lin/xyl9160.h"
//z_other
#include "devices/z_others/xdigimatic.h"



//**** Constants *********************************************************/
const DEVPOINTER DevicePointer[] = {
	//detectors
	{OPAL_GetDeviceInfo, OPAL_CreateDevParams},
	{SAPPHIRE_GetDeviceInfo, SAPPHIRE_CreateDevParams},
	{TOPAZ_GetDeviceInfo, TOPAZ_CreateDevParams},
	{RUBY_GetDeviceInfo, RUBY_CreateDevParams},
	{AQUILON_GetDeviceInfo, AQUILON_CreateDevParams},
	{OPALDEMO_GetDeviceInfo, OPALDEMO_CreateDevParams},
	{LCD2071_X_GetDeviceInfo, LCD2071_X_CreateDevParams},
	{A2073_GetDeviceInfo, A2073_CreateDevParams},
	{LCD2084_GetDeviceInfo, LCD2084_CreateDevParams},
	{LCD2070_GetDeviceInfo, LCD2070_CreateDevParams},
	{LCD2083_6_GetDeviceInfo, LCD2083_6_CreateDevParams},
	{F06DAD_GetDeviceInfo, F06DAD_CreateDevParams},
	{F06SGL_GetDeviceInfo, F06SGL_CreateDevParams},
	{SHODEXRI10X_GetDeviceInfo, SHODEXRI10X_CreateDevParams},		//shodex
	{F06DUAL_GetDeviceInfo, F06DUAL_CreateDevParams},
	{LCD2083_7_GetDeviceInfo, LCD2083_7_CreateDevParams},
	{ATL_SAPPHIRE_GetDeviceInfo, ATL_SAPPHIRE_CreateDevParams},		//analtech
	{TESTDEV_GetDeviceInfo, TESTDEV_CreateDevParams},
	{MALACHITE_GetDeviceInfo, MALACHITE_CreateDevParams},
	{RF10AXL_GetDeviceInfo, RF10AXL_CreateDevParams},
	{YL9160_GetDeviceInfo, YL9160_CreateDevParams},		//young-lin
	{APATIT_GetDeviceInfo, APATIT_CreateDevParams},
	{ECD2000_GetDeviceInfo, ECD2000_CreateDevParams},
	{RI700II_GetDeviceInfo, RI700II_CreateDevParams},		//jai
	{TOYDAD_GetDeviceInfo, TOYDAD_CreateDevParams},
	{ECDA2000_GetDeviceInfo, ECDA2000_CreateDevParams},
	//pumps
	{ALPHA_GetDeviceInfo, ALPHA_CreateDevParams},
	{BETA_GetDeviceInfo, BETA_CreateDevParams},
	{LCP4100_GetDeviceInfo, LCP4100_CreateDevParams},
	{LCP4050_GetDeviceInfo, LCP4050_CreateDevParams},
	{KAPPA_GetDeviceInfo, KAPPA_CreateDevParams},
	{ATL_ALPHA_GetDeviceInfo, ATL_ALPHA_CreateDevParams},		//analtech
	{ATL_KAPPA_GetDeviceInfo, ATL_KAPPA_CreateDevParams},		//analtech
	{PCPP2C_GetDeviceInfo, PCPP2C_CreateDevParams},		//labio
	{LINAR1000_GetDeviceInfo, LINAR1000_CreateDevParams},		//labio
	{THETA_GetDeviceInfo, THETA_CreateDevParams},
	{SEPAR3_GetDeviceInfo, SEPAR3_CreateDevParams},		//separlab
	{PP03_GetDeviceInfo, PP03_CreateDevParams},		//separlab
	{ECP2000_GetDeviceInfo, ECP2000_CreateDevParams},
	{IOTA_GetDeviceInfo, IOTA_CreateDevParams},
	{SEPAR_T_GetDeviceInfo, SEPAR_T_CreateDevParams},		//separlab
	{SEPAR_T2_GetDeviceInfo, SEPAR_T2_CreateDevParams},		//separlab
	{P6000_GetDeviceInfo, P6000_CreateDevParams},		//separlab
	//column ovens
	{LCO102_GetDeviceInfo, LCO102_CreateDevParams},
	{TV10P_GetDeviceInfo, TV10P_CreateDevParams},
	{LCT5100_GetDeviceInfo, LCT5100_CreateDevParams},		//ingos
	{ECO2000_GetDeviceInfo, ECO2000_CreateDevParams},
	//thermometers
	{TM917_GetDeviceInfo, TM917_CreateDevParams},		//lutron
	{ALMEMO2490_GetDeviceInfo, ALMEMO2490_CreateDevParams},	//ahlborn
	//multimeters
	{MS8218_GetDeviceInfo, MS8218_CreateDevParams},		//mastech
	{HP34401A_GetDeviceInfo, HP34401A_CreateDevParams},		//agilent
	//converter
	{LOOPSM_GetDeviceInfo, LOOPSM_CreateDevParams},
	{PANTHER1000_GetDeviceInfo, PANTHER1000_CreateDevParams},
	{PANDA88_GetDeviceInfo, PANDA88_CreateDevParams},
	//{PANDA30_GetDeviceInfo, PANDA30_CreateDevParams},
	{LCD2083_X_SM_GetDeviceInfo, LCD2083_X_SM_CreateDevParams},
	{ORCA2800_GetDeviceInfo, ORCA2800_CreateDevParams},
	//{PANDA30HID_GetDeviceInfo, PANDA30HID_CreateDevParams},
	//autosamplers
	{HT3X0L_GetDeviceInfo, HT3X0L_CreateDevParams},
	//fraction collectors
	{SPIDER_GetDeviceInfo, SPIDER_CreateDevParams},
	{FC203B_GetDeviceInfo, FC203B_CreateDevParams},		//gilson
	{CHF122SC_GetDeviceInfo, CHF122SC_CreateDevParams},		//advantech
	{FC20_GetDeviceInfo, FC20_CreateDevParams},		//labio
	{SEPAR510_GetDeviceInfo, SEPAR510_CreateDevParams},		//separlab
	{VICIACT_GetDeviceInfo, VICIACT_CreateDevParams},		//valco
	{ECF2000_GetDeviceInfo, ECF2000_CreateDevParams},
	//source
//	{AC250K1D_GetDeviceInfo, AC250K1D_CreateDevParams},
	//counter
	{BK1823A_GetDeviceInfo, BK1823A_CreateDevParams},		//bk precision
	//balance
	{PB3002_GetDeviceInfo, PB3002_CreateDevParams},		//mettler toledo
	//other/unknown
	{MIREK01_GetDeviceInfo, MIREK01_CreateDevParams},
	{MIREK02_GetDeviceInfo, MIREK02_CreateDevParams},
	{MIREK03_GetDeviceInfo, MIREK03_CreateDevParams},
	{CENTOREASY_GetDeviceInfo, CENTOREASY_CreateDevParams},		//andilog
	{NIELSBOHR_GetDeviceInfo, NIELSBOHR_CreateDevParams},
	{F06LAMPTEST_GetDeviceInfo, F06LAMPTEST_CreateDevParams},
	{AAASAMPLE_GetDeviceInfo, AAASAMPLE_CreateDevParams},
	{DT8852_GetDeviceInfo, DT8852_CreateDevParams},		//cem
	{DIGIMATIC_GetDeviceInfo, DIGIMATIC_CreateDevParams},		//z_others
	{FCPCA_GetDeviceInfo, FCPCA_CreateDevParams},		//kromaton
	{MIREK04_GetDeviceInfo, MIREK04_CreateDevParams},
	{ECOM_DIPS_GetDeviceInfo, ECOM_DIPS_CreateDevParams},
	{ECX2000T01_GetDeviceInfo, ECX2000T01_CreateDevParams},	//ECx2000 FIFO
	{USBPHM_GetDeviceInfo, USBPHM_CreateDevParams},
	{MIREKCCD_GetDeviceInfo, MIREKCCD_CreateDevParams},
	{HM8135_GetDeviceInfo, HM8135_CreateDevParams},
	{SYSDEV_GetDeviceInfo, SYSDEV_CreateDevParams},
	{LOUISPASTEUR_GetDeviceInfo, LOUISPASTEUR_CreateDevParams},
	{ICPC_GetDeviceInfo, ICPC_CreateDevParams},		//rotachrom
	{F06CCD_GetDeviceInfo, F06CCD_CreateDevParams},
	//last item
	{NULL, NULL}};

//default communication parameters
//rs232
#define D_DEFCOM_PORTNO 1
#define D_DEFCOM_BAUDRATE 9600
#define D_DEFCOM_DATABITS 8
#define D_DEFCOM_PARITY NOPARITY
#define D_DEFCOM_STOPBITS ONESTOPBIT
#define D_DEFCOM_HANDSHAKE 0		//none
#define D_DEFCOM_INBUFSIZE 1024
#define D_DEFCOM_OUTBUFSIZE 1024
//ethernet
#define D_DEFCOM_HOST TEXT("128.0.0.1")
#define D_DEFCOM_PORT 1000
#define D_DEFCOM_CNTIMEOUT 3500
//user id
#define D_DEFCOM_USERID TEXT("")
//
#define D_DEFCOM_TIMEOUT 1000		//ms
#define D_DEFCOM_PARAM 0
#define D_DEFCOM_REPEAT 0
#define D_DEFCOM_TERMINAL_CHAR '\r'

//#define D_COLOR_EMPHASIZED 		0x000000C0L		//red
#define D_COLOR_EMPHASIZED 		0x00C00000L		//blue
#define D_COLOR_GRAY 					0x00808080L		//gray
#define D_COLOR_WHITE 				0x00FFFFFFL		//white

#define D_BGCOLORRATION 0.7

#define D_TIMER1_STEP 125		//125 ms
#define D_TIMER2_STEP 250		//250 ms

const TCHAR *c_strUnknownDev = TEXT("???");

#define D_CB_ACTION_MAX 7		//none, mark, start, stop, zero, C/W, spec
#define D_CB_TRIGER_MAX 3		//down, up, both
#define D_CB_INPUT_MAX 16		//none, 1-15

#define D_STR_NONE 	"none"
#define D_STR_MARK 	"mark"
#define D_STR_START	"start"
#define D_STR_STOP 	"stop"
#define D_STR_ZERO 	"zero"
#define D_STR_CW	 	"C/W"
#ifdef JAISCAN
	#define D_STR_SPEC	"recycle"
#else
	#define D_STR_SPEC	"spec"
#endif

static const TCHAR *c_strSMActions[D_CB_ACTION_MAX] = {
	DC_(D_STR_NONE),
	DC_(D_STR_MARK),
	DC_(D_STR_START),
	DC_(D_STR_STOP),
	DC_(D_STR_ZERO),
	DC_(D_STR_CW),
	DC_(D_STR_SPEC)};

#define D_STR_DOWN 	"down"
#define D_STR_UP 		"up"
#define D_STR_BOTH 	"both"

static const TCHAR *c_strSMTriggers[D_CB_TRIGER_MAX] = {
	DC_(D_STR_DOWN),
	DC_(D_STR_UP),
	DC_(D_STR_BOTH)};
/*
static const TCHAR *c_strSMInputs[D_CB_INPUT_MAX] = {
	DC_(D_STR_NONE),
	TEXT("Input 1"),
	TEXT("Input 2"),
	TEXT("Input 3"),
	TEXT("Input 4"),
	TEXT("Input 5"),
	TEXT("Input 6"),
	TEXT("Input 7"),
	TEXT("Input 8"),
	TEXT("Input 9"),
	TEXT("Input 10"),
	TEXT("Input 11"),
	TEXT("Input 12"),
	TEXT("Input 13"),
	TEXT("Input 14"),
	TEXT("Input 15")};
*/

static TCHAR *cstr_empty = TEXT("");
static TCHAR *cstr_questmark = TEXT("?");


//**** Macros ************************************************************/

//**** Global variables **************************************/
BOOL g_e_proclogging = FALSE;
TCHAR g_e_proclogpath[MAX_PATH] = TEXT("");

static HIMAGELIST g_hImgList = NULL;
static BOOL g_bIsInternal = FALSE;
static CHUI_UNITS g_dev_units = {
	CHUI_TIME_MIN,
	CHUI_PRESSURE_MPA,

	};

//**** Variables *********************************************************/


//**** Function Declaration **********************************************/
const char *D_SaveCommentFce(XML_TAG *, void *);


//**** Function Definitions **********************************************/

/** @brief Absolute value (inline)
 *
 * @param x Value
 * @return Absolute value
 *
 */
double D_fabs(double x)
{
	return(x < 0.0 ? -x : x);
}

/** @brief Get application name
 *
 * @param void
 * @return const TCHAR*
 *
 */
const TCHAR *D_GetAppName(void)
{
	return(D_PROGTITLE);
}

/** @brief Get application version
 *
 * @param void
 * @return const TCHAR*
 *
 */
const TCHAR *D_GetAppVer(void)
{
	return(D_PROGVER);
}

//---------------------------
/* init gui handles */
BOOL D_InitGui(HWND hwnd)
{
	if (hwnd == NULL)
		return(FALSE);

#ifndef DEVICE_ALONE
	HINSTANCE hI;
	HICON hicon;

	//get instance
	hI = (HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE);

	//create image list
	g_hImgList = ImageList_Create(16, 16, ILC_COLORDDB | ILC_MASK, 1, 0);
	hicon = LoadIcon(hI, MAKEINTRESOURCE(IDI_EDGE_DOWN));		//0
	ImageList_AddIcon(g_hImgList, hicon);
	hicon = LoadIcon(hI, MAKEINTRESOURCE(IDI_EDGE_UP));		//1
	ImageList_AddIcon(g_hImgList, hicon);
	hicon = LoadIcon(hI, MAKEINTRESOURCE(IDI_EDGE_BOTH2));		//2
	ImageList_AddIcon(g_hImgList, hicon);

#endif // DEVICE_ALONE

	return(TRUE);
}

//*** function returns model text
//translate by _()
const TCHAR *D_ModelText(DWORD model)
{
	const TCHAR *tret;

	switch (model & D_MODEL_TYPE) {		//the lowest byte
		default:
		case D_MODEL_UNKNOWN: 			tret = C_("Not specified"); break;
		case D_MODEL_DETECTOR: 			tret = C_("Detector");	break;
		case D_MODEL_PUMP: 					tret = C_("Pump"); break;
		case D_MODEL_AUTOSAMPLER: 	tret = C_("Autosampler"); break;
		case D_MODEL_FRACCOLLECTOR: tret = C_("Fraction collector");  break;
		case D_MODEL_THERMOOVEN: 		tret = C_("Thermostatic oven"); break;
		case D_MODEL_STARTMARK: 		tret = C_("Starter & marker"); break;
		case D_MODEL_THERMOMETER: 	tret = C_("Thermometer"); break;
		case D_MODEL_OUTPUT: 				tret = C_("Output device"); break;
		case D_MODEL_VALVE: 				tret = C_("Valve"); break;
		case D_MODEL_COLUMN: 				tret = C_("Column"); break;
		case D_MODEL_MULTIMETER: 		tret = C_("Multimeter"); break;
		case D_MODEL_SOURCE: 				tret = C_("Source"); break;
		case D_MODEL_FLOWMETER: 		tret = C_("Flowmeter"); break;
		case D_MODEL_CONVERTER: 		tret = C_("Converter"); break;
		case D_MODEL_COUNTER: 			tret = C_("Counter"); break;
		case D_MODEL_BALANCE: 			tret = C_("Balance"); break;
		case D_MODEL_SYSDEVICE: 		tret = C_("System"); break;
		case D_MODEL_OTHER: 				tret = C_("Other device"); break;
		case D_MODEL_MULTIPLE: 			tret = C_("Multiple-function device"); break;
	}
	return(tret);
}

//*** function returns model abbreviation text
//translate by _()
const TCHAR *D_ModelAbbrText(DWORD model)
{
	const TCHAR *ptext;

	switch (model & D_MODEL_TYPE) {		//the lowest byte
		default:
		case D_MODEL_UNKNOWN: 			ptext = TEXT("Ukn."); break;
		case D_MODEL_DETECTOR: 			ptext = TEXT("Det."); break;
		case D_MODEL_PUMP: 					ptext = TEXT("Pump"); break;
		case D_MODEL_AUTOSAMPLER: 	ptext = TEXT("AS"); break;
		case D_MODEL_FRACCOLLECTOR: ptext = TEXT("FC"); break;
		case D_MODEL_THERMOOVEN: 		ptext = TEXT("TO"); break;
		case D_MODEL_STARTMARK: 		ptext = TEXT("SM"); break;
		case D_MODEL_THERMOMETER: 	ptext = TEXT("Ther."); break;
		case D_MODEL_OUTPUT: 				ptext = TEXT("Out."); break;
		case D_MODEL_VALVE: 				ptext = TEXT("Val."); break;
		case D_MODEL_COLUMN: 				ptext = TEXT("Col"); break;
		case D_MODEL_MULTIMETER: 		ptext = TEXT("MM"); break;
		case D_MODEL_SOURCE: 				ptext = TEXT("Src"); break;
		case D_MODEL_FLOWMETER: 		ptext = TEXT("FM"); break;
		case D_MODEL_CONVERTER: 		ptext = TEXT("Conv."); break;
		case D_MODEL_COUNTER: 			ptext = TEXT("Cnt."); break;
		case D_MODEL_BALANCE: 			ptext = TEXT("Bal."); break;
		case D_MODEL_SYSDEVICE:			ptext = TEXT("Sys"); break;
		case D_MODEL_OTHER: 				ptext = TEXT("Other."); break;
		case D_MODEL_MULTIPLE: 			ptext = TEXT("Multi"); break;
	}
	return(ptext);
}

//*** function returns model color
COLORREF D_ModelColor(DWORD model)
{
	COLORREF color;
#define D_LIGHTRATIO 0.9

	switch (model & D_MODEL_TYPE) {		//the lowest byte
		default:
		case D_MODEL_UNKNOWN: color = LIGHTCOLOR(0x00000000L, D_LIGHTRATIO); break;		//light gray
		case D_MODEL_CONVERTER:
		case D_MODEL_MULTIMETER:
		case D_MODEL_DETECTOR: color = LIGHTCOLOR(0x00FF0000L, D_LIGHTRATIO); break;		//light blue
		case D_MODEL_PUMP: color = LIGHTCOLOR(0x0000FF00L, D_LIGHTRATIO); break;		//light green
		case D_MODEL_AUTOSAMPLER: color = LIGHTCOLOR(0x00FF80FFL, D_LIGHTRATIO); break;		//light violet
		case D_MODEL_FRACCOLLECTOR: color = LIGHTCOLOR(0x0080FFFFL, D_LIGHTRATIO); break;		//light yellow
		case D_MODEL_THERMOMETER:
		case D_MODEL_THERMOOVEN: color = LIGHTCOLOR(0x000000FFL, D_LIGHTRATIO); break;		//light red
		case D_MODEL_STARTMARK: color = LIGHTCOLOR(0x00404080L, D_LIGHTRATIO); break;		//light brown
		case D_MODEL_VALVE:
		case D_MODEL_SOURCE:
		case D_MODEL_OUTPUT: color = LIGHTCOLOR(0x00FF8080L, D_LIGHTRATIO); break;		//light cyan
		case D_MODEL_BALANCE: color = LIGHTCOLOR(0x00DBC0FFL, D_LIGHTRATIO); break;		//light pink
		case D_MODEL_SYSDEVICE: color = LIGHTCOLOR(0x00000000L, 0.7); break;		//dark gray
	}
	return(color);
}

//*** function test if device is demo device
BOOL D_IsDemoDevice(HDEVICE hD)
{
	DEVICEINFO di = {0};

	if (hD) {
		if (D_GetDeviceInfo(hD, &di) && (di.model & D_MODEL_DEMO) == D_MODEL_DEMO)
			return TRUE;
	}
	return FALSE;
}

//*** function count devices (last item of dev. array must be null)
DWORD D_CountDevices(const DEVPOINTER *devs)
{
	DWORD d = 0;

	if (devs) {
		while (devs[d].info)
			d++;
	}
	return(d);
}

//*** function compares two info function pointers by name (alias)
int D_ListCompareByName(const void *p1, const void *p2)
{
	DEVINFO pf1, pf2;
	DEVICEINFO df1, df2;

	if (p1 && p2) {
		pf1 = *(DEVINFO *)p1;
		pf2 = *(DEVINFO *)p2;

		D_GetDeviceInfoProc(pf1, &df1);
		D_GetDeviceInfoProc(pf2, &df2);
		if (df1.alias && df2.alias) {
			//compare names
			return(lstrcmpi(df1.alias, df2.alias));
		}
	}
	return(0);
}

//*** function lists info functions all available devices
DWORD D_ListDevices(DEVINFO **ifce, const TCHAR *filter)
{
	DWORD i, c = 0, dcount;
	DEVINFO finfo;
	DEVICEINFO linfo;

	if (ifce) {
		//count devices
		dcount = D_CountDevices(DevicePointer);

		//allocate list
		if ((*ifce = (DEVINFO *)malloc(dcount*sizeof(DEVINFO))) == NULL)
			return(0);
		memset(*ifce, 0, sizeof(dcount*sizeof(DEVINFO)));		//zero memory

		//go through
		for (i=0,c=0; i<dcount; i++) {
			finfo = DevicePointer[i].info;
			if (finfo == NULL)
				break;		//end listing

			memset(&linfo, 0, sizeof(DEVICEINFO));		//zero info
			if (D_GetDeviceInfoProc(finfo, &linfo) && (linfo.filter == NULL || *linfo.filter == '\0' || SearchSubString(filter, linfo.filter, TRUE, NULL))) {
				(*ifce)[c] = finfo;
				c++;
			}
		}

		if (c == 0 && *ifce) {
			free((void *)*ifce);
			*ifce = NULL;
		}
		else {
			//sort by name
			qsort((void *)(*ifce), c, sizeof(DEVINFO), D_ListCompareByName);
		}

	}

	return(c);
}

//*** function lists info functions all available devices
DWORD D_ClearDeviceList(DEVINFO **ifce)
{
	if (ifce && *ifce) {
		free((void *)*ifce);
		*ifce = NULL;
	}
	return(NO_ERROR);
}

//*** function lists info functions all available devices
DWORD D_ListDevicesInfo(DEVICEINFO **pinfo, const TCHAR *filter)
{
	DWORD i, c = 0, dcount;
	DEVINFO finfo;
	DEVICEINFO linfo;

	if (pinfo) {
		//count devices
		dcount = D_CountDevices(DevicePointer);

		//allocate list
		if ((*pinfo = (DEVICEINFO *)malloc(dcount*sizeof(DEVICEINFO))) == NULL)
			return(0);
		memset(*pinfo, 0, sizeof(dcount*sizeof(DEVICEINFO)));		//zero memory

		//go through
		for (i=0,c=0; i<dcount; i++) {
			finfo = DevicePointer[i].info;
			if (finfo == NULL)
				break;		//end listing

			memset(&linfo, 0, sizeof(DEVICEINFO));		//zero info
			if (D_GetDeviceInfoProc(finfo, &linfo) && (linfo.filter == NULL || *linfo.filter == '\0' || SearchSubString(filter, linfo.filter, TRUE, NULL))) {
				(*pinfo)[c] = linfo;
				c++;
			}
		}

		if (c == 0 && *pinfo) {
			free((void *)*pinfo);
			*pinfo = NULL;
		}
	}

	return(c);
}

//*** function lists info functions all available devices
DWORD D_ClearDevicesInfoList(DEVICEINFO **pinfo)
{
	if (pinfo && *pinfo) {
		free((void *)*pinfo);
		*pinfo = NULL;
	}
	return(NO_ERROR);
}

//*** function searches device by name
DEVPARAMS D_SearchDeviceByName(const TCHAR *devname, DEVICEINFO *dinfo)
{
	DWORD i;
	DEVICEINFO di;
	DWORD dcount;

	dcount = D_CountDevices(DevicePointer);
	for (i=0; i<dcount; i++) {
		if (DevicePointer[i].info) {
			//get name
			D_GetDeviceInfoProc(DevicePointer[i].info, &di);
			if (di.name && lstrcmp(di.name, devname) == 0) {
				if (dinfo)
					*dinfo = di;
				return(DevicePointer[i].params);		//device found; returns pointer to param. function
			}
		}
	}

	return(NULL);
}

/* get state of ECOM_INTERNAL */
BOOL D_IsInternal(void)
{
	return(g_bIsInternal);
}

/* store state of ECOM_INTERNAL */
void D_StoreInternal(BOOL state)
{
	g_bIsInternal = state;
}

//*** function creates device by params
HDEVICE D_CreateDeviceByParams(DEVPARAMS params)
{
	D_DEVICE *pD;
	DWORD ret;

	if (params == NULL) {
		SetLastError(ERROR_INVALID_HANDLE);		//bad input
		return(NULL);
	}

	//allocate device
	if ((pD = (D_DEVICE *)malloc(sizeof(D_DEVICE))) == NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
		return(NULL);
	}

	//initiates device
	D_DefInitDevice(pD);

	//create device-specified parameters (serial params., properties, function pointers)
	ret = params((HDEVICE)pD, NULL);
	if (ret != NO_ERROR) {
		free((void *)pD);
		SetLastError(ret);
		return(NULL);
	}

	//set unit conversion
	D_SetUnitConversion((HDEVICE)pD, &g_dev_units);

	//set label
	D_SetDeviceLabel((HDEVICE)pD, NULL);

	//create signals
	D_CreateSignals((HDEVICE)pD);

	//make com. ID string
	D_MakeCommIDString((HDEVICE)pD);

	return((HDEVICE)pD);
}

//*** function creates device by name
HDEVICE D_CreateDeviceByName(const TCHAR *devname)
{
	DEVPARAMS pfce;

	if (devname == NULL) {
		SetLastError(ERROR_INVALID_HANDLE);		//bad input
		return(NULL);
	}

	//search for device
	pfce = D_SearchDeviceByName(devname, NULL);
	if (pfce == NULL) {
		SetLastError(ERROR_INVALID_HANDLE);		//?? device not found
		return(NULL);
	}

	return(D_CreateDeviceByParams(pfce));
}

//*** function initiates device
DWORD D_DefInitDevice(D_DEVICE *pD)
{

	if (pD == NULL)
		return(ERROR_INVALID_HANDLE);
	memset(pD, 0, sizeof(D_DEVICE));		//clear device

	//default label
	D_SetDefaultLabel((HDEVICE)pD);

	//set default comm. parameters
	//general
	pD->com.mode = COM_MODE_RS232;
	pD->com.log_format = COM_LOGFORMAT_TEXT;
	pD->com.com_flags.en_rs232 = 0;
	pD->com.com_flags.en_eth = 0;
	pD->com.com_flags.en_user = 0;
	pD->com.com_flags.en_timeout = 1;
	//rs232
	pD->com.ser_pars.portno = D_DEFCOM_PORTNO;
	pD->com.ser_pars.baudrate = D_DEFCOM_BAUDRATE;
	pD->com.ser_pars.databits = D_DEFCOM_DATABITS;
	pD->com.ser_pars.parity = D_DEFCOM_PARITY;
	pD->com.ser_pars.stopbits = D_DEFCOM_STOPBITS;
	pD->com.ser_pars.handshake = D_DEFCOM_HANDSHAKE;
	pD->com.ser_pars.insize = D_DEFCOM_INBUFSIZE;
	pD->com.ser_pars.outsize = D_DEFCOM_OUTBUFSIZE;
	//ethernet
	__strncpy(pD->com.eth_pars.ip, D_DEFCOM_HOST, COM_MAX_IP_LEN);
	pD->com.eth_pars.port = D_DEFCOM_PORT;
	pD->com.eth_pars.cn_timeout = D_DEFCOM_CNTIMEOUT;
	pD->com.eth_flags.en_ip = 1;		//IP should be enabled by default
	//user id
	__strncpy(pD->com.userid, D_DEFCOM_USERID, COM_MAX_USERID_LEN);
	//
	pD->com.timeout = D_DEFCOM_TIMEOUT;
	pD->com.repeat = D_DEFCOM_REPEAT;
	pD->com.terminal_char = D_DEFCOM_TERMINAL_CHAR;
	pD->com.lparam = D_DEFCOM_PARAM;

	pD->utimer1 = D_TIMER1_STEP;
	pD->utimer2 = D_TIMER2_STEP;

	return(NO_ERROR);
}

//*** function opens device (test device, open port, create comm. queue)
DWORD D_OpenDevice(HDEVICE hD)
{
	D_DEVICE *pD;
	DWORD ret = NO_ERROR;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//test device (if already opened => close)
	D_CloseDevice(hD);

	if (pD->ufce.pOpen) {
		ret = pD->ufce.pOpen(hD);
	}
	else if (!D_IsDemoDevice(hD)) {
		ret = CF_OpenAndConfPort(&pD->com);
		//ret = COM_OpenAndConf(&pD->com);
	}

	if (ret == NO_ERROR) {
		//create device basic parameters (thread, comm. queue, ...)
		pD->hqueue = MQ_CreateQueue(D_CommTQueueProc, (LPARAM)hD);
		pD->com.errs = 0;		//clear errors

		pD->_opened = 1;
	}

	return(ret);
}

//*** function closes device (destroy com. queue, close ports)
DWORD D_CloseDevice(HDEVICE hD)
{
	D_DEVICE *pD;
	DWORD ret = NO_ERROR;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//destroy comm. queue (wait for thread to quit)
	if (pD->hqueue)
		ret = MQ_DestroyQueue(pD->hqueue);
	pD->hqueue = NULL;

	if (pD->ufce.pClose) {
		ret = pD->ufce.pClose(hD);
	}
	else if (!D_IsDemoDevice(hD)) {
		//close port
		ret = CF_ClosePort(&pD->com);
		//ret = COM_Close(&pD->com);
	}

	pD->_opened = 0;

	return(ret);
}

/** @brief Test if device is opened
 *
 * @param hD HDEVICE
 * @return BOOL
 *
 */
BOOL D_IsOpened(HDEVICE hD)
{
	D_DEVICE *pD;
	BOOL bret = FALSE;

	if (hD) {
		pD = (D_DEVICE *)hD;
		bret = pD->_opened;
	}
	return(bret);
}

//*** function starts closing device (signal closing thread)
BOOL D_ClosingDevice(HDEVICE hD)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//start queue destroying
	return(MQ_StartDestroyQueue(pD->hqueue));
}

//*** fucntion destroys device
DWORD D_DestroyDevice(HDEVICE hD)
{
	D_DEVICE *pD;
	WORD p;

	if (hD) {
		pD = (D_DEVICE *)hD;		//device structure

		//--- stop communication ---
		D_CloseDevice(hD);

		//--- free memories ---
		//free properties
		if (pD->n_p && pD->p) {
			for (p=0; p<pD->n_p; p++) {
				//test if was allocated string
				if ((pD->p[p].type == DT_MEMORY || pD->p[p].type == DT_STRING) && pD->p[p].val.m.size && pD->p[p].val.m.buf) {
					free((void *)pD->p[p].val.m.buf);
					pD->p[p].val.m.buf = NULL;
					pD->p[p].val.m.size = 0;
				}
			}
			free((void *)pD->p);
		}
		pD->n_p = 0;
		pD->p = NULL;
		//free oper. properties
		if (pD->n_op && pD->op)
			free((void *)pD->op);
		pD->n_op = 0;
		pD->op = NULL;

		//destroy signals
		D_DestroySignals(hD);

		//destroy data
		if (pD->data)
			free((void *)pD->data);
		pD->data = NULL;

		//zero structure
		memset(pD, 0, sizeof(D_DEVICE));
		free((void *)pD);

	}
	return(NO_ERROR);
}

//---------------------
//*** function creates default measure signals for device
DWORD D_CreateSignals(HDEVICE hD)
{
	D_DEVICE *pD;
	D_DEV_SIGNAL *pS;
	WORD pidx, s, n_s, n;
	TCHAR *plabel, *psuffix;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//count measure signals
	n = D_GetDevPropertiesByFlags(hD, DFLAG_MEAS);
	if (n) {
		//allocation
		pS = (D_DEV_SIGNAL *)malloc(n*sizeof(D_DEV_SIGNAL));
		if (pS == NULL)
			return(ERROR_OUTOFMEMORY);
		//init
		memset(pS, 0, n*sizeof(D_DEV_SIGNAL));

		//fill
		n_s = 0;
		for (s=0; s<n; s++) {
			pidx = D_GetDevPropertyIndexByFlags(hD, DFLAG_MEAS, s);
			if (pidx < (WORD)-1) {
				//store index
				pS[n_s].pidx = pidx;
				//defaultly disabled
				if ((pD->p[pS[n_s].pidx].flags & DFLAG_NOSIGNAL) == DFLAG_NOSIGNAL)
					pS[n_s].disabled = 1;

				//get label
				plabel = (TCHAR *)D_GetDevPropertyTextVar(hD, pidx, 0);		//label
				if (plabel == NULL)
					plabel = _("Signal");
				//get suffix
				psuffix = (TCHAR *)D_GetDevPropertyTextVar(hD, pidx, 1);		//suffix

				//create name
				__sprintf(pS[n_s].name, TEXT("%.*s%.*s"),
									D_MAX_PROPNAME_LEN, D_ram(plabel),
									D_MAX_PROPSUFFIX_LEN, psuffix ? psuffix : cstr_empty);

				n_s++;
			}

			//default scale
			pS[s].sc.my = 1.0;
			pS[s].sc.oy = 0.0;
		}

		if (n_s == 0) {
			free((void *)pS);		//free memory
		}
		else {
			//pointer signals
			pD->n_s = n_s;
			pD->s = pS;
		}
	}

	return(NO_ERROR);
}

//*** function renames signals
DWORD D_RenameDevicesSignals(HDEVICES *hDs)
{
	D_DEVICE *pD;
	TCHAR *plabel, *psuffix;
	DWORD d, idx, total;
	WORD i;

	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount == 0 || hDs->hd == NULL)
		return(NO_ERROR);		//nothing to do

	//count device with signals first
	total = 0;
	for (d=0; d<hDs->dcount; d++) {
		pD = (D_DEVICE *)hDs->hd[d];
		if (pD && pD->n_s && pD->s && pD->n_p && pD->p)
			total++;
	}

	//count device with signals first
	idx = 0;
	for (d=0; d<hDs->dcount; d++) {
		pD = (D_DEVICE *)hDs->hd[d];
		if (pD && pD->n_s && pD->s && pD->n_p && pD->p) {
			//go though signals
			for (i=0; i<pD->n_s; i++) {
				if (pD->s[i].pidx < pD->n_p) {
					//get label
					plabel = (TCHAR *)D_GetDevPropertyTextVar((HDEVICE)pD, pD->s[i].pidx, 0);		//label
					if (plabel == NULL)
						plabel = _("Signal");
					//get suffix
					psuffix = (TCHAR *)D_GetDevPropertyTextVar((HDEVICE)pD, pD->s[i].pidx, 1);		//suffix

					//create name
					if (total < 2) {
						__sprintf(pD->s[i].name, TEXT("%.*s%.*s"),
											D_MAX_PROPNAME_LEN, D_ram(plabel),
											D_MAX_PROPSUFFIX_LEN, psuffix ? psuffix : cstr_empty);
					}
					else {
						__sprintf(pD->s[i].name, TEXT("%.*s%.*s%lu"),
											D_MAX_PROPNAME_LEN, D_ram(plabel),
											D_MAX_PROPSUFFIX_LEN, psuffix && *psuffix ? psuffix : TEXT(" "),
											idx+1);
					}
				}
			}
			idx++;
		}
	}
	return(NO_ERROR);
}

//*** function destroy signals
DWORD D_DestroySignals(HDEVICE hD)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (pD->n_s && pD->s) {
		free((void *)pD->s);
	}
	pD->n_s = 0;
	pD->s = NULL;

	return(NO_ERROR);
}

//*** function validates signals
BOOL D_ValidateSignals(HDEVICE hD)
{
	D_DEVICE *pD;
	WORD i;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->n_s && pD->s) {
			for (i=0; i<pD->n_s; i++) {
				if (pD->n_p == 0 ||
						pD->p == NULL ||
						pD->s[i].pidx >= pD->n_p ||
						(pD->p[pD->s[i].pidx].flags & DFLAG_MEAS) == 0) {
					pD->s[i].disabled = 1;		//disabled signal
				}
			}
		}
	}
	return(TRUE);
}

//*** function returns total number of valid signals
unsigned D_CountEnabledSignals(HDEVICE hD, unsigned *total)
{
	D_DEVICE *pD;
	unsigned i, n = 0, nt = 0;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->n_s && pD->s) {
			nt += pD->n_s;		//add to total counter
			for (i=0; i<pD->n_s; i++) {
				if (pD->s[i].disabled == 0)
					n++;		//count enabled signal
			}
		}
	}

	if (total)
		*total = nt;
	return(n);
}

//*** function returns total number of valid signals through all devices
unsigned D_CountAllEnabledSignals(HDEVICES *pDs, unsigned *total)
{
	DWORD d;
	unsigned n = 0, nt = 0, lnt;

	if (pDs && pDs->dcount && pDs->hd) {
		for (d=0; d<pDs->dcount; d++) {
			n += D_CountEnabledSignals(pDs->hd[d], &lnt);
			nt += lnt;
		}
	}

	if (total)
		*total = nt;
	return(n);
}

/** @brief Choose one channel only for a device (the first enabled)
 *
 * @param hDs HDEVICES
 * @param param int
 * @return DWORD
 *
 */
DWORD D_ChooseOneChannelForDevice(HDEVICES *pDs, int param)
{
	if (pDs == NULL || (pDs->dcount > 0 && pDs->hd == NULL))
		return(ERROR_INVALID_HANDLE);

	//through devices
	unsigned d, s, sel;
	D_DEVICE *pD;
	D_DEV_PROP *pprop;
	for (d=0; d<pDs->dcount; d++) {
		pD = (D_DEVICE*)pDs->hd[d];
		if (pD && pD->n_s && pD->s) {
			sel = 0;
			//through signals
			for (s=0; s<pD->n_s; s++) {
				//signal property (signal by default
				pprop = &pD->p[pD->s[s].pidx];
				if (pprop && (pprop->flags & DFLAG_NOSIGNAL) == 0 && sel == 0) {
					//the first?
					sel = 1;
					pD->s[s].disabled = 0;	//enable the first
				}
				else {
					//disable others
					pD->s[s].disabled = 1;
				}
			}
		}
	}

	return(NO_ERROR);
}

/** @brief Get signal ID by device index and signal index
 *
 * @param d_idx WORD
 * @param s_idx WORD
 * @return DWORD
 *
 */
DWORD D_GetSignalID(WORD d_idx, WORD s_idx)
{
	return(MAKELONG(s_idx, d_idx));
}

/** @brief Get signal ID by device and signal index
 *
 * @param pDs HDEVICES*
 * @param hD HDEVICE
 * @param s_idx WORD
 * @return DWORD
 *
 */
DWORD D_GetSignalIDByDevice(HDEVICES *pDs, HDEVICE hD, WORD s_idx)
{
	DWORD sid = (DWORD)-1;	//invalid

	if (pDs && pDs->hd && hD) {
		WORD d;
		for (d=0; d<pDs->dcount; d++)	{
			if (pDs->hd[d] == hD && s_idx < ((D_DEVICE*)hD)->n_s) {
				sid = D_GetSignalID(d, s_idx);
				break;
			}
		}
	}

	return(sid);
}

//--------------------
//*** function counts devices by demanded model
DWORD D_CountDevicesByModel(HDEVICES *pDs, DWORD model)
{
	DWORD d, nd = 0;
	D_DEVICE *pD;

	if (pDs && pDs->dcount && pDs->hd) {
		for (d=0; d<pDs->dcount; d++) {
			pD = (D_DEVICE *)pDs->hd[d];
			if (pD && (pD->model & model))
				nd++;
		}
	}
	return(nd);
}


//-------------------------------------

//*** function tests device
/*
DWORD D_TestDevice(HDEVICE hD)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return FALSE;
	pD = (D_DEVICE *)hD;

	return(NO_ERROR);
}
*/

//*** function set default device label
BOOL D_SetDefaultLabel(HDEVICE hD)
{
	return(D_SetDeviceLabel(hD, NULL));
}

//*** function set device label
BOOL D_SetDeviceLabel(HDEVICE hD, const TCHAR *label)
{
	DEVICEINFO di;

	if (hD) {
		if (label == NULL) {
			if (D_GetDeviceInfo(hD, &di))
				label = di.alias;
			else
				label = _("Device");
		}
		__strncpy(((D_DEVICE *)hD)->label, label, D_MAX_DEVNAME_LEN);
		return(TRUE);
	}
	return(FALSE);
}

//-------------------------------------
//*** function flushes communication buffers of device
BOOL D_FlushDevice(HDEVICE hD)
{
	if (hD) {
		if (PurgeBothQues(((D_DEVICE *)hD)->com.hComm) == NO_ERROR)
			return(TRUE);
	}
	return(FALSE);
}

//---------------------------------------------------------------------
//*** function returns comm. ID string in demanded format
//formats: 0=(COM1 or 128.0.0.1), 1=(COM1:9600:8:N:1 or 128.0.0.1:1000)
BOOL D_GetCommIDString(HDEVICE hD, TCHAR *str, int len, int format)
{
	D_DEVICE *pD;

	if (hD == NULL || str == NULL || len < 24)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//make id according to mode
	switch (pD->com.mode) {
		case COM_MODE_RS232:
			if (format == 1)
				__sprintf(str, TEXT("COM%d:%lu:%d:%s:%s"),
									pD->com.ser_pars.portno, pD->com.ser_pars.baudrate, pD->com.ser_pars.databits,
									CF_GetParityString(pD->com.ser_pars.parity),
									CF_GetStopbitsString(pD->com.ser_pars.stopbits));
			else
				__sprintf(str, TEXT("COM%d"), pD->com.ser_pars.portno);
			break;
		case COM_MODE_ETHERNET:
			if (format == 1)
				__sprintf(str, TEXT("%.*s:%d"), len-10, pD->com.eth_pars.ip, pD->com.eth_pars.port);
			else
				__sprintf(str, TEXT("%.*s"), len, pD->com.eth_pars.ip);
			break;
		case COM_MODE_USERID:
			__strncpy(str, pD->com.userid, len);
			break;
		default:
			*str = '?';
			*(str+1) = '\0';
			break;
	}

	return(TRUE);
}

//*** function makes comm. ID string (CID) for device
BOOL D_MakeCommIDString(HDEVICE hD)
{
	if (hD == NULL)
		return(FALSE);
	memset(((D_DEVICE *)hD)->cid, 0, D_MAX_CID_LEN*sizeof(TCHAR));
	return(D_GetCommIDString(hD, ((D_DEVICE *)hD)->cid, D_MAX_CID_LEN, 1));		//full format
}

//*** function returns comm. ID string in demanded format
TCHAR *D_RetCommIDString(HDEVICE hD, int format)
{
	static TCHAR cid[D_MAX_CID_LEN];

	if (D_GetCommIDString(hD, cid, D_MAX_CID_LEN, format))
		return(cid);
	else
		return(cstr_questmark);		//unknown
}

//---------------------------------------------------------------------
//*** function tests devices for communication errors
//if error counter > max_errors --> return TRUE + handle to device
BOOL D_TestCommErrors(HDEVICES *hDs, DWORD max_errs, DWORD *didx)
{
#ifndef D_SIMUL_ALL
	D_DEVICE *pD;
	DWORD d;

	if (hDs && hDs->dcount && hDs->hd) {
		//go through devices
		for (d=0; d<hDs->dcount; d++) {
			if (hDs->hd[d]) {
				pD = (D_DEVICE *)hDs->hd[d];
				if (pD->com.errs > max_errs) {
					if (didx)
						*didx = d;
					return TRUE;		//device overfull max comm errors
				}
			}
		}
	}
#endif
	return FALSE;
}

//*** function clears devices' error-counters
BOOL D_ClearCommErrors(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d;

	if (hDs && hDs->dcount && hDs->hd) {
		//go through devices
		for (d=0; d<hDs->dcount; d++) {
			if (hDs->hd[d]) {
				pD = (D_DEVICE *)hDs->hd[d];
				pD->com.errs = 0;
			}
		}
		return TRUE;
	}
	return FALSE;
}


//---------------------------------------------------------------------
//*** function adds new device to devices
DWORD D_AddNewDevice(HDEVICE hD, HDEVICES *hDs)
{
	HDEVICE *phD;
	DWORD i;

	if (hD == NULL || hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	//allocate new array
	if ((phD = (HDEVICE *)malloc((hDs->dcount+1)*sizeof(HDEVICE))) == NULL)
		return(ERROR_OUTOFMEMORY);

	//copy old dev. handles
	for (i=0; i<hDs->dcount && hDs->hd; i++)
		phD[i] = hDs->hd[i];
	//copy new
	phD[i] = hD;

	//free old memory
	if (hDs->hd)
		free((void *)hDs->hd);
	//associate new array
	hDs->hd = phD;
	hDs->dcount++;

	//rename signals
	D_RenameDevicesSignals(hDs);

	return(NO_ERROR);
}

//*** function removes device from devices by index
DWORD D_RemoveDevice(DWORD idx, HDEVICES *hDs)
{
	HDEVICE *phD = NULL;
	DWORD i;

	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount == 0 || hDs->hd == NULL)
		return(NO_ERROR);		//nothing to do

	if (idx >= hDs->dcount)
		return(ERROR_INVALID_DATA);

	if (hDs->dcount > 1) {
		//allocate new array
		if ((phD = (HDEVICE *)malloc((hDs->dcount-1)*sizeof(HDEVICE *))) == NULL)
			return(ERROR_OUTOFMEMORY);
	}

	//copy previous
	for (i=0; i<idx; i++)
		phD[i] = hDs->hd[i];
	//destroy device
	D_DestroyDevice(hDs->hd[idx]);

	//copy next
	for (; i<(hDs->dcount-1); i++)
		phD[i] = hDs->hd[i+1];

	//free old memory
	if (hDs->hd)
		free((void *)hDs->hd);
	//associate new array
	hDs->hd = phD;
	hDs->dcount--;

	//rename signals
	D_RenameDevicesSignals(hDs);

	return(NO_ERROR);
}

//*** function removes device from devices by handle
DWORD D_RemoveDeviceByHandle(HDEVICE hD, HDEVICES *hDs)
{
	DWORD i;

	if (hD == NULL || hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount == 0 || hDs->hd == NULL)
		return(NO_ERROR);		//nothing to do

	//search for handle
	for (i=0; i<hDs->dcount; i++) {
		if (hDs->hd[i] == hD)
			return(D_RemoveDevice(i, hDs));
	}

	return(ERROR_INVALID_HANDLE);		//device not found
}

//*** function removes all devices
DWORD D_RemoveAllDevices(HDEVICES *hDs)
{
	DWORD i;

	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount && hDs->hd) {
		for (i=0; i<hDs->dcount; i++)
			D_DestroyDevice(hDs->hd[i]);

		free((void *)hDs->hd);
	}
	hDs->hd = NULL;
	hDs->dcount = 0;

	return(NO_ERROR);
}

//*** function copies device values
DWORD D_CopyDeviceProperty(D_DEV_PROP *psrc, D_DEV_PROP *pdest)
{
	unsigned int size;

	if (psrc == NULL || pdest == NULL)
		return(ERROR_INVALID_HANDLE);

	if (psrc->flags & DFLAG_MEM) {		//do not realloc
		if (psrc->val.m.buf && pdest->val.m.buf) {
			//get min. size
			size = (psrc->val.m.size <= pdest->val.m.size ? psrc->val.m.size : pdest->val.m.size);
			//copy content
			memcpy(pdest->val.m.buf, psrc->val.m.buf, size);
		}
		return(NO_ERROR);
	}

	//test memory value (destroy)
	if (pdest->type == DT_MEMORY || pdest->type == DT_STRING) {
		if (pdest->val.m.size && pdest->val.m.buf)
			free((void *)pdest->val.m.buf);
		pdest->val.m.buf = NULL;
		pdest->val.m.size = 0;
	}

	//!!!backup guidata (when guidata pointered to other prop. buffer)
	TCHAR *guidata = pdest->guidata;

	//copy
	*pdest = *psrc;

	//!!!restore guidata
	pdest->guidata = guidata;

	//test memory value
	if ((psrc->type == DT_MEMORY || psrc->type == DT_STRING) &&
			psrc->val.m.size && psrc->val.m.buf) {
		pdest->val.m.buf = malloc(psrc->val.m.size);		//allocation
		if (pdest->val.m.buf)
			memcpy(pdest->val.m.buf, psrc->val.m.buf, psrc->val.m.size);		//copy data
	}

	return(NO_ERROR);
}


//*** function copies device values
DWORD D_CopyDeviceValues(HDEVICE src, HDEVICE dest)
{
	D_DEVICE *sd, *dd;
	WORD i;

	if (src == NULL || dest == NULL)
		return(ERROR_INVALID_HANDLE);
	sd = (D_DEVICE *)src;
	dd = (D_DEVICE *)dest;

	//dd->com = sd->com;
	dd->com.hComm = NULL;		//null com. handle
	dd->com.mode = sd->com.mode;
	dd->com.timeout = sd->com.timeout;
	dd->com.repeat = sd->com.repeat;
	dd->com.terminal_char = sd->com.terminal_char;
	dd->com.log_format = sd->com.log_format;
	dd->com.com_flags = sd->com.com_flags;
	dd->com.ser_pars = sd->com.ser_pars;
	dd->com.ser_flags = sd->com.ser_flags;
	dd->com.eth_pars = sd->com.eth_pars;
	dd->com.eth_flags = sd->com.eth_flags;
	memcpy(dd->com.userid, sd->com.userid, COM_MAX_USERID_LEN*sizeof(TCHAR));
	dd->com.var = sd->com.var;

	dd->model = sd->model;
	__strncpy(dd->label, sd->label, D_MAX_DEVNAME_LEN);
	memcpy(dd->id, sd->id, D_MAX_ID_LEN*sizeof(TCHAR));
	memcpy(dd->cid, sd->cid, D_MAX_CID_LEN*sizeof(TCHAR));
	dd->color = sd->color;
	dd->tmindelay = sd->tmindelay;
	dd->tdelay = sd->tdelay;
	dd->tdelay2 = sd->tdelay2;
	dd->utimer1 = sd->utimer1;
	dd->utimer2 = sd->utimer2;

	dd->n_p = sd->n_p;

	//prop. values
	for (i=0; i<sd->n_p; i++) {
		//if (sd->p[i].flags & (DFLAG_SAVE|DFLAG_MET|DFLAG_STAT|DFLAG_CTRL|DFLAG_MEAS)) {
			D_CopyDeviceProperty(&sd->p[i], &dd->p[i]);
		//}
	}

	//oper values
	dd->n_op = sd->n_op;
	for (i=0; i<sd->n_op; i++)
		dd->op[i] = sd->op[i];

	//signals
	dd->n_s = sd->n_s;
	for (i=0; i<sd->n_s; i++)
		dd->s[i] = sd->s[i];
	dd->dis_meas_int = sd->dis_meas_int;

	dd->ufce = sd->ufce;
	dd->detfce = sd->detfce;
	dd->fcfce = sd->fcfce;
	dd->asfce = sd->asfce;

	return(NO_ERROR);
}

//*** function copies devices from source (s) to destination (d)
DWORD D_CopyDevices(HDEVICES *s, HDEVICES *d)
{
	DWORD i;
	DEVICEINFO di;
	HDEVICE hd;
	DWORD ret;

	if (s == NULL || d == NULL)
		return(ERROR_INVALID_HANDLE);

	//clear dest. devices
	D_RemoveAllDevices(d);

//!!!!!!!!!!!!!
	if (s->dcount && s->hd) {
		//create new device handles
		for (i=0; i<s->dcount; i++) {
			//get device name
			if (D_GetDeviceInfo(s->hd[i], &di)) {
				hd = D_CreateDeviceByName(di.name);
				if (hd) {
					ret = D_AddNewDevice(hd, d);
					if (ret != NO_ERROR)
						return(ret);
					ret = D_CopyDeviceValues(s->hd[i], hd);
					if (ret != NO_ERROR)
						return(ret);
				}
			}
		}
	}
	return(NO_ERROR);
}

//*** function compares properties for modification
BOOL D_ArePropertiesModified(D_DEV_PROP *p1, D_DEV_PROP *p2)
{
	BOOL modified = FALSE;

	if (p1 == NULL || p2 == NULL)
		return(FALSE);

	while (1) {

		//label, quant. & unit ??, flags, value
		if (p1->flags != p2->flags ||
				p1->type != p2->type) {
				modified = TRUE;
				break;
		}

		//label, value if DFLAG_CONF is set
		if ((p1->flags & DFLAG_CONF) == DFLAG_CONF) {
			//label
			if (__strcmp(p1->label, p2->label) != 0) {
				modified = TRUE;
				break;
			}

			//value
			switch (p1->type) {
				case DT_BOOL:
				case DT_INT:
				case DT_UINT:
				case DT_DOUBLE:
				case DT_INT64:
				case DT_UINT64:
				case DT_HANDLE:
					if (memcmp(&p1->val, &p2->val, sizeof(D_VAL)) != 0)
						modified = TRUE;
					break;
				case DT_MEMORY:
				case DT_STRING:
					if (p1->val.m.size != p2->val.m.size ||
							(p1->val.m.size && memcmp(p1->val.m.buf, p2->val.m.buf, p1->val.m.size) != 0))
						modified = TRUE;
					break;
			}
//			if (modified)
//				break;
		}

		break;
	}

	return(modified);
}

//*** function compares devices for modification
BOOL D_AreDevicesModified(HDEVICES *d1, HDEVICES *d2)
{
	BOOL modified = FALSE;
	DWORD d, p;
	D_DEVICE *pd1, *pd2;

	if (d1 == NULL || d2 == NULL)
		return(FALSE);

	while (1) {			//only once
		//dev. counts
		if (d1->dcount != d2->dcount ||
				(d1->hd && !d2->hd) || (!d1->hd && d2->hd)) {
			modified = TRUE;
			break;
		}

		if (d1->dcount && d2->dcount && d1->hd && d2->hd) {
			//through devices
			for (d=0; d<d1->dcount; d++) {
				pd1 = (D_DEVICE *)d1->hd[d];
				pd2 = (D_DEVICE *)d2->hd[d];
				if (pd1 == NULL || pd2 == NULL) {
					if (pd1 == pd2)
						continue;
					modified = TRUE;
					break;
				}

				//dev. label
				if (__strncmp(pd1->label, pd2->label, D_MAX_DEVNAME_LEN)) {
					modified = TRUE;
					break;
				}

				//dev. comm.
				if (pd1->com.mode != pd2->com.mode ||
						//
						pd1->com.ser_pars.portno != pd2->com.ser_pars.portno ||
						pd1->com.ser_pars.baudrate != pd2->com.ser_pars.baudrate ||
						pd1->com.ser_pars.databits != pd2->com.ser_pars.databits ||
						pd1->com.ser_pars.parity != pd2->com.ser_pars.parity ||
						pd1->com.ser_pars.stopbits != pd2->com.ser_pars.stopbits ||
						pd1->com.ser_pars.handshake != pd2->com.ser_pars.handshake ||
						//
						__strncmp(pd1->com.eth_pars.ip, pd2->com.eth_pars.ip, COM_MAX_IP_LEN) != 0 ||
						__strncmp(pd1->com.eth_pars.mac_addr, pd2->com.eth_pars.mac_addr, COM_MAX_MACADDR_LEN) != 0 ||
						pd1->com.eth_pars.port != pd2->com.eth_pars.port ||
						pd1->com.eth_pars.cn_timeout != pd2->com.eth_pars.cn_timeout ||
						//
						__strncmp(pd1->com.userid, pd2->com.userid, COM_MAX_USERID_LEN) != 0 ||
						//
						pd1->com.timeout != pd2->com.timeout ||
						pd1->com.lparam != pd2->com.lparam ||
						pd1->com.repeat != pd2->com.repeat) {
					modified = TRUE;
					break;
				}
				//model & color
				if (pd1->model != pd2->model ||
						pd1->color != pd2->color) {
					modified = TRUE;
					break;
				}
				//timers
				if (pd1->tmindelay != pd2->tmindelay ||
						pd1->tdelay != pd2->tdelay ||
						pd1->tdelay2 != pd2->tdelay2) {
					modified = TRUE;
					break;
				}

				//dev. properties
				if (pd1->n_p != pd2->n_p ||
						(pd1->p && !pd2->p) || (!pd1->p && pd2->p)) {
					modified = TRUE;
					break;
				}
				if (pd1->n_p && pd2->n_p && pd1->p && pd2->p) {
					//through properties
					for (p=0; p<pd1->n_p; p++) {
						if (D_ArePropertiesModified(&pd1->p[p], &pd2->p[p])) {
							modified = TRUE;
							break;
						}
					}
				}

				//dev. oper. properties
				if (pd1->n_op != pd2->n_op ||
						(pd1->op && !pd2->op) || (!pd1->op && pd2->op)) {
					modified = TRUE;
					break;
				}
				if (pd1->n_op && pd2->n_op && pd1->op && pd2->op) {
					//through oper. properties
					for (p=0; p<pd1->n_op; p++) {
						//label
						if (__strcmp(pd1->op[p].label, pd2->op[p].label) != 0) {
							modified = TRUE;
							break;
						}
					}
				}

				//dev. signals
				if (pd1->n_s != pd2->n_s ||
						(pd1->s && !pd2->s) || (!pd1->s && pd2->s)) {
					modified = TRUE;
					break;
				}
				if (pd1->n_s && pd2->n_s && pd1->s && pd2->s) {
					//through signals
					for (p=0; p<pd1->n_s; p++) {
						//name
						if (__strcmp(pd1->s[p].name, pd2->s[p].name) != 0) {
							modified = TRUE;
							break;
						}
						//rename
						if (__strcmp(pd1->s[p].rename, pd2->s[p].rename) != 0) {
							modified = TRUE;
							break;
						}
						//values
						if (pd1->s[p].pidx != pd2->s[p].pidx ||
							pd1->s[p].disabled != pd2->s[p].disabled ||
							pd1->s[p].hidden != pd2->s[p].hidden ||
							memcmp(&pd1->s[p].sc, &pd2->s[p].sc, sizeof(D_DEV_SCALE)) != 0) {
							modified = TRUE;
							break;
						}
					}
				}


			}
		}

		break;
	}

	return(modified);
}

//----

/** @brief Create List with available port numbers (no ITEMPROC, call LIST_Discard() to free list)
 *
 * @param void
 * @return HLIST
 *
 */
HLIST D_CreateComList(unsigned int max_port)
{
	HLIST hcomlist = NULL;
	unsigned i;

	//create comlist
	hcomlist = LIST_Create(NULL);
	if (hcomlist) {
		//add available ports
		for (i=D_MIN_COMPORT; i<=max_port; i++) {
			//test openning of comport
			if (RS232_TestPortNo(i)) {
				//add port
				LIST_AppendItem(hcomlist, i);
			}
		}
	}
	return(hcomlist);
}

#ifdef DEBUG
	//#define DEBUG_SEARCHDEVPORTS
#endif // DEBUG

typedef struct {
	HDEVICE hD;
	HLIST hlist;
} S_SSTH_PARAM;

/** @brief Function testing serial port on thread
 *
 * @param param void*
 * @return unsigned __stdcall
 *
 */
unsigned __stdcall D_SearchSerialPortsThreadProc(void *param)
{
	unsigned bret = 0;
#ifdef DEBUG_SEARCHDEVPORTS
	FILE *fw;
#endif

	S_SSTH_PARAM *sp = (S_SSTH_PARAM*)param;
	if (sp && sp->hD) {
		D_DEVICE *pD = (D_DEVICE*)sp->hD;

#ifdef DEBUG_SEARCHDEVPORTS
		fw = fopen("SearchDevicePortsTh.log", "a");
		if (fw) {
			fprintf(fw, "SearchSerialPortsThreadProc; start, name=%S, com=%d\n", pD->label, pD->com.ser_pars.portno);
			fclose(fw);
		}
#endif

		//set serial mode
		pD->com.mode = COM_MODE_RS232;
		//open port
		if (D_OpenDevice(sp->hD) == NO_ERROR) {
			//port opened successfully
			if (D_TestID(sp->hD, NULL, NULL, NULL, NULL)) {

				D_DEV_SEARCH dsearch = {0};
				dsearch.status = D_SEARCHSTATUS_ONLINE;

				//mode
				dsearch.mode = COM_MODE_RS232;
				//comport
				dsearch.portno = pD->com.ser_pars.portno;
				//sid
				__sprintf(dsearch.spec_id, TEXT("%lu:%d:%s:%s"),
							pD->com.ser_pars.baudrate, pD->com.ser_pars.databits,
							CF_GetParityString(pD->com.ser_pars.parity),
							CF_GetStopbitsString(pD->com.ser_pars.stopbits));
				//id
				memcpy(dsearch.id, pD->id, D_MAX_ID_LEN*sizeof(TCHAR));

				//add into list
				LIST_AppendItem(sp->hlist, (LPARAM)&dsearch);

				bret = 1;		//found
			}

			//close port
			D_CloseDevice(sp->hD);
		}

#ifdef DEBUG_SEARCHDEVPORTS
		fw = fopen("SearchDevicePortsTh.log", "a");
		if (fw) {
			fprintf(fw, "SearchSerialPortsThreadProc; stop, name=%S, com=%d\n", pD->label, pD->com.ser_pars.portno);
			fclose(fw);
		}
#endif

	}

	_endthreadex(bret);
	return 0;
}

/** @brief Function searing other ports (TCP/IP, UserID)
 *
 * @param param void*
 * @return unsigned __stdcall
 *
 */
unsigned __stdcall D_SearchOtherPortsThreadProc(void *param)
{
	unsigned bret = 0;
#ifdef DEBUG_SEARCHDEVPORTS
	FILE *fw;
#endif

	S_SSTH_PARAM *sp = (S_SSTH_PARAM*)param;
	if (sp && sp->hD) {
		D_DEVICE *pD = (D_DEVICE*)sp->hD;

#ifdef DEBUG_SEARCHDEVPORTS
		fw = fopen("SearchDevicePortsTh.log", "a");
		if (fw) {
			fprintf(fw, "SearchOtherPortsThreadProc; start, name=%S\n", pD->label);
			fclose(fw);
		}
#endif

		//search function
		if (pD->ufce.pSearch) {
			bret = pD->ufce.pSearch(sp->hD, sp->hlist);
		}

#ifdef DEBUG_SEARCHDEVPORTS
		fw = fopen("SearchDevicePortsTh.log", "a");
		if (fw) {
			fprintf(fw, "SearchOtherPortsThreadProc; stop, name=%S\n", pD->label);
			fclose(fw);
		}
#endif

	}
	_endthreadex(bret);
	return 0;
}

/** @brief Search ports (by comlist) for device (each port in its thread)
 *
 * @param hD HDEVICE
 * @param comlist HLIST
 * @param hlist HLIST
 * @param percent WORD*
 * @param tfound unsigned int*
 * @return unsigned int
 *
 */
unsigned int D_SearchDevicePortsTh(HDEVICE hD, HLIST comlist, HLIST hlist, WORD *percent, unsigned int *tfound)
{
	D_DEVICE *pD;
	unsigned int fcnt = 0;
	unsigned int s_cnt = 0;	//serial threads
	unsigned int o_cnt = 0;	//other search thread
#ifdef DEBUG_SEARCHDEVPORTS
	FILE *fw;
#endif


	if (hD == NULL || hlist == NULL || comlist == NULL)
		return(0);

	if (percent)
		*percent = 0;

	while (1) {
		//dev. pointer
		pD = (D_DEVICE *)hD;

		//--- search by RS232 ---
		if (pD->com.com_flags.en_rs232) {
			s_cnt = LIST_GetSize(comlist);
		}
		//--- search by search function (Ethernet, UserID)
		if (pD->ufce.pSearch /* && pD->com.mode != COM_MODE_RS232*/) {
			o_cnt = 1;
		}

#ifdef DEBUG_SEARCHDEVPORTS
		fw = fopen("SearchDevicePortsTh.log", "a");
		if (fw) {
			fprintf(fw, "name=%S, s-cnt=%u, o_cnt=%u\n", pD->label, s_cnt, o_cnt);
			fclose(fw);
		}
#endif

		unsigned int cnt = s_cnt+o_cnt;
		if (cnt) {
			//allocate
			HANDLE *pthread = (HANDLE)malloc((cnt)*(sizeof(HANDLE)+sizeof(S_SSTH_PARAM)));
			if (pthread == NULL)
				break;	//error
			memset(pthread, 0, (cnt)*(sizeof(HANDLE)+sizeof(S_SSTH_PARAM)));
			S_SSTH_PARAM *sp = (S_SSTH_PARAM*)(pthread+cnt);
			const TCHAR *devname = D_GetDeviceName(hD);
			unsigned id, portno, i;
			COMH *pcom;
			HITEM hitem;
			//create devices + serial threads
			hitem = LIST_GetItem(comlist, LPOS_FIRST, 0);
			for (i=0; i<s_cnt && hitem && LIST_GetItemValue(hitem, (void *)&portno); i++) {
				sp[i].hD = D_CreateDeviceByName(devname);
				sp[i].hlist = hlist;
				if (sp[i].hD) {
					pcom = &((D_DEVICE*)sp[i].hD)->com;
					pcom->mode = COM_MODE_RS232;
					pcom->ser_pars = pD->com.ser_pars;
					pcom->ser_pars.portno = portno;
					pcom->timeout = pD->com.timeout;
					pcom->repeat = 0;
				}
				pthread[i] = (HANDLE)_beginthreadex(NULL, 0, D_SearchSerialPortsThreadProc, (void*)&sp[i], 0, &id);
				hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);		//next
			}
			//create devices + other threads
			if (o_cnt) {
				sp[i].hD = D_CreateDeviceByName(devname);
				sp[i].hlist = hlist;
				if (sp[i].hD) {
					pcom = &((D_DEVICE*)sp[i].hD)->com;
					pcom->mode = pD->com.mode;
					pcom->eth_pars = pD->com.eth_pars;
					memcpy(pcom->userid, pD->com.userid, sizeof(pcom->userid));
					pcom->timeout = pD->com.timeout;
					pcom->repeat = 0;
				}
				pthread[i] = (HANDLE)_beginthreadex(NULL, 0, D_SearchOtherPortsThreadProc, (void*)&sp[i], 0, &id);
			}

			//monitor threads
			double t0, t;
			int scnt;
			t0 = timer_GetTime();
			do {
				Sleep(100);
				//test for found ports and stopped threads
				scnt = 0;
				for (i=0; i<cnt; i++) {
					if (WaitForSingleObject(pthread[i], 0) == WAIT_OBJECT_0) {		//signaled
						scnt++;		//stopped
					}
				}
				//found
				fcnt = LIST_GetSize(hlist);
				if (tfound)
					*tfound = fcnt;
				//percentage
				if (percent)
					*percent = scnt*100/cnt;
				t = timer_GetTime();
			} while (scnt<cnt && (t-t0) < 10000.0);		//10s

			//wait for threads
			if (WaitForMultipleObjects(cnt, pthread, TRUE, 2000) != WAIT_OBJECT_0) {	//wait for threads
				//threads not stopped
				i = 0;
			}

			//add to list + clear memory
			for (i=0; i<cnt; i++) {
				CloseHandle(pthread[i]);
				D_DestroyDevice(sp[i].hD);
			}
			free((void *)pthread);

#ifdef DEBUG_SEARCHDEVPORTS
			fw = fopen("SearchDevicePortsTh.log", "a");
			if (fw) {
				fprintf(fw, "name=%S, threads=%d/%d, found=%u\n", pD->label, scnt, cnt, LIST_GetSize(hlist));
				fclose(fw);
			}
#endif

		}

		//sort list
		LIST_BubbleSort(hlist, LIST_SORTMODE_ASCEND);

		//set percentage
		if (percent)
			*percent = 100;
		//counter
		fcnt = LIST_GetSize(hlist);
		if (tfound) {
			*tfound = fcnt;
		}

		break;		//run only once
	}

	return(fcnt);		//number of found devices
}

//*** function searches for devices by autodetection
DWORD D_AutodetectDevices(HDEVICES *hDs, BOOL *run, WORD *percent, TCHAR **devname, unsigned *tfound, TCHAR *sel, TCHAR *filter)
{
	DWORD dcount, d, count = 0, ret;
	HDEVICE hD = NULL;
	D_DEVICE *pD;
	DEVICEINFO di;
	HLIST hcomlist = NULL, hlist = NULL;
	HITEM hitem;
	D_DEV_SEARCH *psearch;
#ifdef DEBUG_SEARCHDEVPORTS
	FILE *fw;
#endif

	if (hDs == NULL)
		return(count);

	if (percent)
		*percent = 0;

	while (1) {
		//create comlist
		hcomlist = D_CreateComList(D_MAX_COMPORT);
		if (hcomlist == NULL)
			break;		//error
		//create search list
		hlist = LIST_Create(D_ListIProcDevSearchParam);
		if (hlist == NULL)
			break;		//error


		//count devices
		dcount = D_CountDevices(DevicePointer);
#ifdef DEBUG_SEARCHDEVPORTS
		//get total available ports
		unsigned int ports = LIST_GetSize(hcomlist);

		fw = fopen("SearchDevicePortsTh.log", "w");
		if (fw) {
			fprintf(fw, "AutodetectDevices; start, dcount=%u, ports=%u\n", dcount, ports);
			fclose(fw);
		}
#endif


		for (d=0; d<dcount; d++) {		//go though devices
			//test run
			if (run && !(*run))
				break;		//stop searching

			memset(&di, 0, sizeof(DEVICEINFO));		//zero variable
			//test device searchability
			if (D_GetDeviceInfoProc(DevicePointer[d].info, &di) == FALSE ||
					(di.model & D_MODEL_NOAUTO) == D_MODEL_NOAUTO ||		//no demo devices
					(sel && !SearchSubString(sel, TEXT("All"), TRUE, NULL) &&
									!SearchSubString(sel, di.name, TRUE, NULL)) ||
					(di.filter && *di.filter != '\0' && !SearchSubString(filter, di.filter, TRUE, NULL)))
				continue;		//next device

			//get pointer device name
			if (devname)
				*devname = (TCHAR *)di.name;
			//get total fount
			if (tfound)
				*tfound = count;

			//create device
			hD = D_CreateDeviceByParams(DevicePointer[d].params);
			if (hD == NULL)
				break;		//some error
			pD = (D_DEVICE *)hD;

#ifdef DEBUG_SEARCHDEVPORTS
			fw = fopen("SearchDevicePortsTh.log", "a");
			if (fw) {
				fprintf(fw, "AutodetectDevices; created device=%S\n", pD->label);
				fclose(fw);
			}
#endif

			//reset list
			LIST_RemoveAll(hlist);
			//search for ports
			ret = NO_ERROR;
			if (D_SearchDevicePortsTh(hD, hcomlist, hlist, NULL, NULL)) {
				//go though list
				hitem = LIST_GetItem(hlist, LPOS_FIRST, 0);
				while (hitem && hD) {
					pD = (D_DEVICE *)hD;
					if (LIST_GetItemValue(hitem, (LPARAM*)&psearch) && psearch) {
						if (psearch->mode == COM_MODE_RS232 && psearch->portno) {
							//set port
							pD->com.mode = psearch->mode;
							pD->com.ser_pars.portno = psearch->portno;
							pD->com.ser_pars.baudrate = __atoi(psearch->spec_id);
						}
						else if (psearch->mode == COM_MODE_ETHERNET && *(psearch->ip)) {
							pD->com.mode = psearch->mode;
							__strncpy(pD->com.eth_pars.ip, psearch->ip, COM_MAX_IP_LEN);
						}
						else if (psearch->mode == COM_MODE_USERID && *(psearch->uid)) {
							pD->com.mode = psearch->mode;
							__strncpy(pD->com.userid, psearch->uid, COM_MAX_USERID_LEN);
						}
						else {
							//invalid -> next
							hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);
							continue;
						}

						//test ID (for model)
						if (D_OpenDevice(hD) == NO_ERROR) {
#ifdef DEBUG_SEARCHDEVPORTS
							fw = fopen("SearchDevicePortsTh.log", "a");
							if (fw) {
								fprintf(fw, "AutodetectDevices; TestID\n");
								fclose(fw);
							}
#endif
							D_TestID(hD, NULL, NULL, NULL, NULL);
							D_CloseDevice(hD);
						}

#ifdef DEBUG_SEARCHDEVPORTS
						fw = fopen("SearchDevicePortsTh.log", "a");
						if (fw) {
							fprintf(fw, "AutodetectDevices; idx=%u, add=%S, cmode=%d\n", count, pD->label, pD->com.mode);
							fclose(fw);
						}
#endif
						//add device
						ret = D_AddNewDevice(hD, hDs);
						if (ret != NO_ERROR) {
							break;		//error adding device
						}

						//increment counter
						count++;

						//recreate new device
						hD = D_CreateDeviceByParams(DevicePointer[d].params);
						if (hD == NULL) {
							ret = ERROR_INVALID_HANDLE;
							break;
						}

#ifdef DEBUG_SEARCHDEVPORTS
						fw = fopen("SearchDevicePortsTh.log", "a");
						if (fw) {
							fprintf(fw, "AutodetectDevices; device recreated\n");
							fclose(fw);
						}
#endif

					}
					//next item
					hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);
				}
			}
			//test errors
			if (ret != NO_ERROR)
				break;

#ifdef DEBUG_SEARCHDEVPORTS
			fw = fopen("SearchDevicePortsTh.log", "a");
			if (fw) {
				fprintf(fw, "AutodetectDevices; destroy device, ret=%u\n", ret);
				fclose(fw);
			}
#endif

			//destroy device
			D_DestroyDevice(hD);

			//set percentage
			if (percent)
				*percent = ((d+1)*100)/dcount;
		}

		break;		//run only once
	}

#ifdef DEBUG_SEARCHDEVPORTS
	fw = fopen("SearchDevicePortsTh.log", "a");
	if (fw) {
		fprintf(fw, "Done\n", ret);
		fclose(fw);
	}
#endif

	//free memories
	if (hlist)
		LIST_Discard(hlist);
	if (hcomlist)
		LIST_Discard(hcomlist);

	return(count);		//number of found devices
}

//*** function searches for device comport/IP/user-id
//note: HLIST should be created with D_ListIProcDevSearchParam
DWORD D_SearchDevicePortTh(HDEVICE hD, HLIST hlist, BOOL *run, WORD *percent, unsigned int *tfound)
{
	unsigned int fcnt = 0;

	if (hD == NULL || hlist == NULL)
		return(0);

	//clear list
	LIST_RemoveAll(hlist);

	if (percent)
		*percent = 0;

	while (1) {
		//available com-ports
		HLIST hcomlist = D_CreateComList(D_MAX_COMPORT);
		if (hcomlist) {
			//search ports on threads
			fcnt = D_SearchDevicePortsTh(hD, hcomlist, hlist, percent, tfound);

			//free comlist
			LIST_Discard(hcomlist);
		}

		//set percentage
		if (percent)
			*percent = 100;
		//counter
		if (tfound)
			*tfound = fcnt;

		break;		//run only once
	}

	return(LIST_GetSize(hlist));		//number of found devices
}

//*** function find nearest next unused port
BYTE D_FindNextComport(HDEVICES *hDs, HDEVICE *hD)
{
	DWORD d;
	BYTE portno = 1;
	D_DEVICE *pD;
	BOOL done = FALSE;

	if (hDs == NULL)
		return(portno);

	if (hDs->dcount && hDs->hd) {
		while (!done) {
			done = TRUE;
			for (d=0; d<hDs->dcount; d++) {
				pD = (D_DEVICE *)hDs->hd[d];
				if (pD && pD->com.mode == COM_MODE_RS232 && pD->com.ser_pars.portno == portno) {
					portno++;
					done = FALSE;
				}
			}
		}
	}
	//validate com index range
	if (portno < D_MIN_COMPORT)
		portno = D_MIN_COMPORT;
	else if (portno > D_MAX_COMPORT)
		portno = D_MAX_COMPORT;
	//comport
	if (hD) {
		((D_DEVICE *)hD)->com.ser_pars.portno = portno;
	}
	return(portno);		//port number
}

//*** function compares commsettings (returns true when matches)
BOOL D_CompareCommSettings(COMH *pC1, COMH*pC2)
{
	if (pC1 && pC2) {
		//test modes
		if (pC1->mode == pC2->mode) {
			switch (pC1->mode) {
				case COM_MODE_RS232:
					if (pC1->ser_pars.portno == pC2->ser_pars.portno)
						return(TRUE);
					break;
				case COM_MODE_ETHERNET:
					if (__strncmp(pC1->eth_pars.ip, pC2->eth_pars.ip, COM_MAX_IP_LEN) == 0 && pC1->eth_pars.port == pC2->eth_pars.port)
						return(TRUE);
					break;
				case COM_MODE_USERID:
					if (__strncmp(pC1->userid, pC2->userid, COM_MAX_USERID_LEN) == 0)
						return(TRUE);
					break;
			}
		}
	}
	return(FALSE);
}

/** @brief Increment used port
 *
 * @param pC COMH*
 * @return BOOL
 *
 */
BOOL D_IncrementCommSettings(COMH *pC)
{
	if (pC == NULL)
		return(FALSE);

	uint32_t val;
	TCHAR *pt;

	switch (pC->mode) {
		default:
		case COM_MODE_RS232:
			val = pC->ser_pars.portno+1;
			if (val >= D_MIN_COMPORT && val <= D_MAX_COMPORT) {
				pC->ser_pars.portno = val;
				return TRUE;
			}
			break;

		case COM_MODE_ETHERNET:
			if (!ETH_IpToAddress(pC->eth_pars.ip, &val))
				break;
			val++;
			if (ETH_AddressToIp(val, pC->eth_pars.ip)) {
				return TRUE;
			}
			break;

		case COM_MODE_USERID:
			{
			pt = pC->userid;
			while (*pt && *(pt+1))
				pt++;
			if (*pt >= '0' && *pt < '9') {
				(*pt)++;
				return TRUE;
			}
			}
			break;
	}

	return FALSE;
}

//***
/** @brief Correct com settings according to existring devices
 *
 * @param hDs HDEVICES*
 * @param hD HDEVICE*
 * @return BOOL
 *
 */
BOOL D_CorrectCommSettings(HDEVICES *hDs, HDEVICE *hD)
{
	DWORD d;
	D_DEVICE *pD, *pDD;
	BOOL done = FALSE;

	if (hDs == NULL || hD == NULL)
		return(FALSE);

	pD = (D_DEVICE *)hD;

	if (hDs->dcount && hDs->hd) {
		while (!done) {
			done = TRUE;
			for (d=0; d<hDs->dcount; d++) {
				pDD = (D_DEVICE *)hDs->hd[d];
				if (pD->com.mode == pDD->com.mode) {
					if (D_CompareCommSettings(&pD->com, &pDD->com)) {
						//the same -> increment
						done = FALSE;
						if (!D_IncrementCommSettings(&pD->com)) {
							return(FALSE);
						}
					}
				}
			}
		}
	}

	return(done);
}

//*** function runs validation function on device
BOOL D_ValidateProps(HDEVICE hD, DWORD pidx)
{
	BOOL valid = TRUE;
	D_DEVICE *pD;
	unsigned int p;

	pD = (D_DEVICE *)hD;
	if (pD) {
		//validate function
		if (pD->ufce.pValidate && !pD->ufce.pValidate((HDEVICE)pD, pidx)) {
			valid = FALSE;
		}

		//validate prop. ranges
		if (pD->p) {
			//validate one property range
			if (pidx < pD->n_p) {
				if ((pD->p[pidx].flags & DFLAG_CONF) == DFLAG_CONF && !D_ValidatePropRange(&pD->p[pidx]))
					valid = FALSE;
			}
			//validate all
			else {
				for (p=0; p<pD->n_p; p++) {
					if ((pD->p[p].flags & DFLAG_CONF) == DFLAG_CONF) {
						if (!D_ValidatePropRange(&pD->p[p]))
							valid = FALSE;
					}
				}
			}
		}

		//set unit convertion after validation
		D_SetUnitConversion(hD, &g_dev_units);
	}
	return(valid);
}

//*** function validates devices (different comports, ...)
BOOL D_ValidateDevices(HDEVICES *hDs)
{
	DWORD d, dd;
	D_DEVICE *pD;
	COMH *pC;

	if (hDs == NULL)
		return(FALSE);

	if (hDs->dcount && hDs->hd) {
		//test maximum number of devices
		if (hDs->dcount > D_MAX_ADDDEVICES)
			return(FALSE);

		//test device handles & validate
		for (d=0; d<hDs->dcount; d++) {
			if (hDs->hd[d] == NULL)
				return(FALSE);
			pD = (D_DEVICE *)hDs->hd[d];

			//validate function
			D_ValidateProps((HDEVICE)pD, (DWORD)-1);

			//validate signals
			D_ValidateSignals((HDEVICE)pD);
		}

		//test for the same comunication parameters
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			for (dd=0; dd<hDs->dcount; dd++) {
				pC = &((D_DEVICE *)hDs->hd[dd])->com;
				if (d != dd && D_CompareCommSettings(pC, &pD->com)) {
					return(FALSE);		//cannot be two device with the same port
				}
			}
		}


	}
	return(TRUE);
}

//*** function returns device index in device structure
//returns: -1 when device not found
DWORD D_GetDeviceIndex(HDEVICE hD, HDEVICES *hDs)
{
	DWORD d;

	if (hD && hDs) {
		if (hDs->dcount && hDs->hd) {
			for (d=0; d<hDs->dcount; d++) {
				if (hDs->hd[d] == hD)
					return(d);
			}
		}
	}
	return((DWORD)-1);
}

//*** function validates device index by model
//returns: handle to device or NULL if invalid
HDEVICE D_IsModelValid(DWORD idx, DWORD model_flags, HDEVICES *pDs)
{
	if (pDs && pDs->dcount && pDs->hd && idx < pDs->dcount &&
			(((D_DEVICE *)pDs->hd[idx])->model & model_flags) == model_flags) {
		return(pDs->hd[idx]);
	}
	return(NULL);
}


//---------------------------------------------------------------------
//*** function stores value to backup
DWORD D_StoreBackupValue(D_DEV_PROP *prop)
{

	if (prop == NULL)
		return(ERROR_INVALID_HANDLE);

	//type
	if ((prop->type == DT_MEMORY || prop->type == DT_STRING) && prop->val.m.size && prop->val.m.buf) {
		//allocation
		if ((prop->vbackup.m.buf = malloc(prop->val.m.size)) == NULL)
			return(ERROR_OUTOFMEMORY);
		//copy memory
		prop->vbackup.m.size = prop->val.m.size;
		memcpy(prop->vbackup.m.buf, prop->val.m.buf, prop->val.m.size);
	}
	else {
		//store value
		prop->vbackup = prop->val;
	}
	return(NO_ERROR);
}

//*** function restores value from backup
DWORD D_RestoreBackupValue(D_DEV_PROP *prop)
{

	if (prop == NULL)
		return(ERROR_INVALID_HANDLE);

	//type
	if ((prop->type == DT_MEMORY || prop->type == DT_STRING) && prop->vbackup.m.size && prop->vbackup.m.buf) {
		//copy memory
		if (prop->vbackup.m.size==prop->val.m.size && prop->val.m.buf)
			memcpy(prop->val.m.buf, prop->vbackup.m.buf, prop->val.m.size);
		//free memory
		free((void *)prop->vbackup.m.buf);
		prop->vbackup.m.buf = NULL;
		prop->vbackup.m.size = 0;
	}
	else {
		//restore value
		prop->val  = prop->vbackup;
	}
	return(NO_ERROR);
}

//*** function sets value to backup
DWORD D_SetBackupValue(D_DEV_PROP *prop, D_VAL *val)
{

	if (prop == NULL || val == NULL)
		return(ERROR_INVALID_HANDLE);

	//type
	if ((prop->type == DT_MEMORY || prop->type == DT_STRING) && prop->vbackup.m.size && prop->vbackup.m.buf) {
		//copy memory
		if (prop->vbackup.m.size==val->m.size && val->m.buf)
			memcpy(prop->vbackup.m.buf, val->m.buf, prop->vbackup.m.size);
	}
	else {
		//copy value
		prop->vbackup = *val;
	}
	return(NO_ERROR);
}

//*** function gets value from backup
DWORD D_GetBackupValue(D_DEV_PROP *prop, D_VAL *val)
{

	if (prop == NULL || val == NULL)
		return(ERROR_INVALID_HANDLE);

	//type
	if ((prop->type == DT_MEMORY || prop->type == DT_STRING) && prop->vbackup.m.size && prop->vbackup.m.buf) {
		//copy memory
		if (prop->vbackup.m.size==val->m.size && val->m.buf)
			memcpy(val->m.buf, prop->vbackup.m.buf, prop->vbackup.m.size);
	}
	else {
		//copy value
		*val = prop->vbackup;
	}
	return(NO_ERROR);
}

//*** function restores value from backup
BOOL D_IsBackupValueModify(D_DEV_PROP *prop, D_VAL *val)
{
	if (prop == NULL || val == NULL)
		return(FALSE);

	//type
	if ((prop->type == DT_MEMORY || prop->type == DT_STRING) && prop->vbackup.m.size && prop->vbackup.m.buf) {
		//copy memory
		if (memcmp(prop->vbackup.m.buf, val->m.buf, prop->vbackup.m.size) != 0)
			return(TRUE);
	}
	else {
		//copy value
		if (memcmp(&prop->vbackup, val, sizeof(D_VAL)) != 0)
			return(TRUE);
	}
	return(FALSE);
}

//*** function sets backup values through devices (from source (s) to destination (d))
DWORD D_SetDevicesBackupValues(HDEVICES *s, HDEVICES *d)
{
	DWORD i, p;
	D_DEVICE *sd, *dd;
	DWORD ret;

	if (s == NULL || d == NULL || s->dcount != d->dcount)
		return(ERROR_INVALID_HANDLE);

	for (i=0; i<s->dcount; i++) {
		sd = (D_DEVICE *)s->hd[i];
		dd = (D_DEVICE *)d->hd[i];
		if (sd->n_p == dd->n_p && sd->p && dd->p) {
			//prop. conf. values
			for (p=0; p<sd->n_p; p++) {
				//copy to backup
				if ((dd->p[p].flags & DFLAG_CONF) == DFLAG_CONF) {
					ret = D_SetBackupValue(&dd->p[p], &sd->p[p].val);
					if (ret != NO_ERROR)
						return(ret);
				}
			}
		}
	}
	return(FALSE);
}

//*** function gets backup values through devices (from source (s) to destination (d))
DWORD D_GetDevicesBackupValues(HDEVICES *s, HDEVICES *d)
{
	DWORD i, p;
	D_DEVICE *sd, *dd;
	DWORD ret;

	if (s == NULL || d == NULL || s->dcount != d->dcount)
		return(ERROR_INVALID_HANDLE);

	for (i=0; i<s->dcount; i++) {
		sd = (D_DEVICE *)s->hd[i];
		dd = (D_DEVICE *)d->hd[i];
		if (sd->n_p == dd->n_p && sd->p && dd->p) {
			//prop. conf. values
			for (p=0; p<sd->n_p; p++) {
				//copy to backup
				if ((dd->p[p].flags & DFLAG_CONF) == DFLAG_CONF) {
					ret = D_GetBackupValue(&sd->p[p], &dd->p[p].val);
					if (ret != NO_ERROR)
						return(ret);
				}
			}
		}
	}
	return(FALSE);
}

//*** function checks changes in backup values through devices (from source (s) to destination (d))
DWORD D_AreDevicesBackupValuesModified(HDEVICES *s, HDEVICES *d)
{
	DWORD i, p;
	D_DEVICE *sd, *dd;

	if (s == NULL || d == NULL || s->dcount != d->dcount)
		return(ERROR_INVALID_HANDLE);

	for (i=0; i<s->dcount; i++) {
		sd = (D_DEVICE *)s->hd[i];
		dd = (D_DEVICE *)d->hd[i];
		if (sd->n_p == dd->n_p && sd->p && dd->p) {
			//prop. conf. values
			for (p=0; p<sd->n_p; p++) {
				//copy to backup
				if ((dd->p[p].flags & DFLAG_CONF) == DFLAG_CONF) {
					if (D_IsBackupValueModify(&dd->p[p], &sd->p[p].val))
						return(TRUE);
				}
			}
		}
	}
	return(NO_ERROR);
}


//---------------------

//*** function initiates device
DWORD D_InitDevice(HDEVICE hD, HWND hwnd, int prepare, D_STAT *dstat)
{
	D_DEVICE *pD;
	DWORD ret;
	WORD p;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);

	if (dstat && dstat->run == FALSE)
		return(NO_ERROR);		//stop

	pD = (D_DEVICE *)hD;
	//clear communication error-counters
	pD->com.errs = 0;
	//set window handle
	pD->_hwnd = hwnd;

	//backup + property's data-queue
	if (pD->n_p && pD->p) {
		for (p=0; p<pD->n_p; p++) {
			//backup conf. value
			if ((pD->p[p].flags & DFLAG_CONF) == DFLAG_CONF) {
				D_StoreBackupValue(&pD->p[p]);
			}
			//test data queue
			if ((pD->p[p].flags & DFLAG_QUEUE))
				pD->p[p].dhq = D_CreateDataQueue(0);		//create data queue
		}
	}

	//open device
	ret = D_OpenDevice(hD);
#ifndef D_SIMUL_ALL
	if (ret != NO_ERROR)
		return(ret);
#endif

	//preset properties
	if (pD->ufce.pPreSetProp && pD->p) {
		for (p=0; p<pD->n_p; p++) {
			if ((pD->p[p].flags & DFLAG_CONF) == DFLAG_CONF && pD->p[p].group) {
				pD->ufce.pPreSetProp(hD, p, pD->p[p].vbackup);
			}
		}
	}

	//initiate device
	if (pD->ufce.pInit) {
		//log operation
		if (g_e_proclogging)
			D_LogOperation(hD, DT_FCE_I, (DWORD)TEXT("Init. device"), NULL);

		ret = pD->ufce.pInit(hD, dstat);
#ifndef D_SIMUL_ALL
		if (ret != NO_ERROR) {
			D_CloseDevice(hD);
			return(ret);
		}
#endif
	}

	//prepare device (prep. mode init)
	if (prepare) {
		D_PrepareDevice(hD, 1, dstat);
	}

	//run measure timer
	D_RunMeasureTimer(hD);

	//inited now
	pD->status = D_STATUS_NOTREADY;

	return(NO_ERROR);
}

//*** function deinitiates device
DWORD D_PostInitDevice(HDEVICE hD, D_STAT *dstat)
{
	D_DEVICE *pD;
	DWORD ret;
	WORD p;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//test if inited
	if (pD->status == D_STATUS_IDLE)
		return(NO_ERROR);

	//stop measure timer
	D_StopMeasureTimer(hD);

	//destroy comm. queue (wait for thread to quit)
	ret = MQ_DestroyQueue(pD->hqueue);
	pD->hqueue = NULL;

	//deinitiate device
	if (pD->ufce.pPostInit) {
		//log operation
		if (g_e_proclogging)
			D_LogOperation(hD, DT_FCE_I, (DWORD)TEXT("Deinit. device"), NULL);

		pD->ufce.pPostInit(hD, dstat);
	}

	//restore backup + property's data-queue
	if (pD->n_p && pD->p) {
		for (p=0; p<pD->n_p; p++) {
			//destroy data queue
			if ((pD->p[p].flags & DFLAG_QUEUE) && pD->p[p].dhq)
				D_DestroyDataQueue(pD->p[p].dhq);
			//restore backup conf. value
			if ((pD->p[p].flags & DFLAG_CONF) == DFLAG_CONF) {
				D_RestoreBackupValue(&pD->p[p]);
			}
		}
	}

	//close device
	ret = D_CloseDevice(hD);

	//idle now
	pD->status = D_STATUS_IDLE;

	return(ret);
}

//-------
//*** function measures devices
BOOL D_MeasureDevice(HDEVICE hD, DWORD idx, HWND hwnd, UINT postmsg)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	return(MQ_SendQueueMessage(pD->hqueue, MQM_READVOLTAGE,
														 (DPARAM)hD, (MPARAM)idx, (WPARAM)hwnd, (LPARAM)postmsg,
														 MQM_PRIORITY_NORMAL));
}

//*** function processes sending WMD_MEASURE messages
void CALLBACK D_MeasureTimerProc(UINT tid, UINT msg, DWORD hD, DWORD dw1, DWORD dw2)
{
	D_DEVICE *pD;
	unsigned i;

	if (hD) {
		pD = (D_DEVICE *)hD;

		if (pD->n_s && pD->s && !pD->stop_measure) {
			if (pD->multi_measure) {
				//search for the first enabled signal
				for (i=0; i<pD->n_s; i++) {
					if (!pD->s[i].disabled) {
						break;
					}
				}
				if (i < pD->n_s) {
					D_MeasureDevice((HDEVICE)hD, i, pD->_hwnd, WMD_MEASURES);		// 1x MEASURES
				}
			}
			else {
				for (i=0; i<pD->n_s; i++) {
					if (!pD->s[i].disabled) {
						//send measure message
						D_MeasureDevice((HDEVICE)hD, i, pD->_hwnd, WMD_MEASURE);		// nx MEASURE
					}
				}
			}
		}
	}
}

//*** function runs device measure timer
DWORD D_RunMeasureTimer(HDEVICE hD)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (pD->_tid)
		timeKillEvent(pD->_tid);		//stop timer first
	pD->_tid = 0;

	//test for any enabled signal
	if (D_CountEnabledSignals(hD, NULL)) {
		//set new timer
		pD->_tid = timeSetEvent(pD->tdelay, pD->tdelay/20, D_MeasureTimerProc, (DWORD)hD, TIME_PERIODIC);
	}

	return(NO_ERROR);
}

//*** function stops device timer
DWORD D_StopMeasureTimer(HDEVICE hD)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (pD->_tid)
		timeKillEvent(pD->_tid);		//stop timer
	pD->_tid = 0;

	return(NO_ERROR);
}


//----------

//*** function presets property
DWORD D_PresetProp(HDEVICE hD, WORD pidx, D_VAL val)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//log operation
	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_P, pidx, &val);

	//test if fce defined
	if (pD->ufce.pPreSetProp) {
		return(pD->ufce.pPreSetProp(hD, pidx, val));
	}
	return(NO_ERROR);
}

//*** function prepares device (set parameters before analysis)
//mode: 0=any time, 1=after init, 2-after stop, 3-don't use pPrepare
DWORD D_PrepareDevice(HDEVICE hD, int mode, D_STAT *dstat)
{
	D_DEVICE *pD;
	WORD p;
	D_STAT st = {1, 0, NULL};
	DWORD twait;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//test state
	if (dstat == NULL)
		dstat = &st;
	if (dstat->run == FALSE)
		return(NO_ERROR);		//stop


	//log operation
	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_I, (DWORD)TEXT("Prep. device"), NULL);

	//test if fce defined
	if (pD->ufce.pPrepare && mode != 3) {
		return(pD->ufce.pPrepare(hD, dstat));		//run other function
	}

	//set wait
	//twait = 2*pD->tmindelay;
	twait = pD->tmindelay;
	if (twait < 20)
		twait = 20;

	//go through "sendable" parameters
	if (pD->n_p && pD->p) {
		//write parameters
		for (p=0; p<pD->n_p; p++) {
			if (!dstat->run)
				break;
			if (dstat->perc < 100)
				dstat->perc = (p*45)/pD->n_p;

			//write backup conf. value
			if ((pD->p[p].flags & DFLAG_SEND) == DFLAG_SEND) {
				if (pD->p[p].group && p < (pD->n_p-1) && pD->p[p].group == pD->p[p+1].group && (pD->p[p+1].flags & DFLAG_SEND) == DFLAG_SEND) {
					D_PresetProp(hD, p, pD->p[p].vbackup);
					//D_WriteFunctionPrior(hD, pD->ufce.pPresetProp, p, pD->p[p].vbackup, NULL, 0, MQM_PRIORITY_HIGHER);
				}
				else
					D_WriteFunctionPrior(hD, pD->ufce.pWriteProp, p, pD->p[p].vbackup, NULL, 0, MQM_PRIORITY_HIGHER);
				Sleep(twait);
			}
			else if ((pD->p[p].flags & DFLAG_CONF) == DFLAG_CONF) {
				D_GetBackupValue(&pD->p[p], &pD->p[p].val);
			}
		}

		//read parameters
		for (p=0; p<pD->n_p; p++) {
			if (!dstat->run)
				break;
			if (dstat->perc < 100)
				dstat->perc = (p*45)/pD->n_p + 45;
			//read value (save or meas)
			if ((pD->p[p].flags & DFLAG_RECV) == DFLAG_RECV) {
				D_ReadFunction(hD, pD->ufce.pReadProp, p, NULL, 0);
				Sleep(twait);
			}
		}
	}

	//go through "sendable" oper. parameters
	if (pD->n_op && pD->op) {
		//write parameters
		for (p=0; p<pD->n_op; p++) {
			if (!dstat->run)
				break;
			if (dstat->perc < 100)
				dstat->perc = (p*10)/pD->n_p + 90;

			//write backup conf. value
			if ((pD->op[p].flags & DFLAG_SEND) == DFLAG_SEND) {
				D_MakeOperFunctionPrior(hD, pD->ufce.pSetOper, p, NULL, 0, MQM_PRIORITY_HIGHER);
				Sleep(twait);
			}
		}
	}

	//log operation
	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_I, (DWORD)TEXT("Prep. device finished"), NULL);

	if (dstat->perc < 100)
		dstat->perc = 100;
	return(NO_ERROR);
}


//----------

//*** function makes function on device (uses com. queue)
BOOL D_MakeFunction(HDEVICE hD, DEVFCE fce, HWND hwnd, UINT post_msg, UINT post_idx, const TCHAR *logname)
{
	D_DEVICE *pD;

	if (hD == NULL || fce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_I, (DWORD)logname, NULL);

	return(MQ_SendQueueMessage(pD->hqueue, MQM_MAKEFCE,
														 (DPARAM)hD, (MPARAM)fce, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, post_idx),
														 MQM_PRIORITY_HIGHER));
}

//*** function init starter devices
BOOL D_MakeFunctionParam(HDEVICE hD, DEVPARFCE parfce, LPARAM param, HWND hwnd, UINT post_msg, UINT post_idx,
												 const TCHAR *logname)
{
	D_DEVICE *pD;
	unsigned char *pbuf;
	D_VAL dval;

	if (hD == NULL || parfce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVPARFCE)+sizeof(LPARAM))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &parfce, sizeof(DEVPARFCE));
	memcpy(pbuf+sizeof(DEVPARFCE), &param, sizeof(LPARAM));

	if (g_e_proclogging) {
		dval.u = param;
		D_LogOperation(hD, DT_FCE_IPAR, (DWORD)logname, &dval);
	}

	return(MQ_SendQueueMessage(pD->hqueue, MQM_MAKEPARFCE,
														 (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, 0),
														 MQM_PRIORITY_HIGHER));
}

//*** function sends reading function to communication queue
BOOL D_ReadFunction(HDEVICE hD, DEVRFCE rfce, WORD idx, HWND hwnd, UINT post_msg)
{
	D_DEVICE *pD;
	unsigned char *pbuf;

	if (hD == NULL || rfce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVRFCE)+sizeof(WORD))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &rfce, sizeof(DEVRFCE));
	memcpy(pbuf+sizeof(DEVRFCE), &idx, sizeof(WORD));

	return(MQ_SendQueueMessage(pD->hqueue, MQM_READFCE,
														 (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, 0),
														 MQM_PRIORITY_NORMAL));
}

//*** function sends reading function to communication queue (with def. priority)
BOOL D_ReadFunctionPrior(HDEVICE hD, DEVRFCE rfce, WORD idx, HWND hwnd, UINT post_msg,
												 int priority)
{
	D_DEVICE *pD;
	unsigned char *pbuf;

	if (hD == NULL || rfce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVRFCE)+sizeof(WORD))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &rfce, sizeof(DEVRFCE));
	memcpy(pbuf+sizeof(DEVRFCE), &idx, sizeof(WORD));

	return(MQ_SendQueueMessage(pD->hqueue, MQM_READFCE,
														 (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, 0),
														 priority));
}

//*** function sends writing function to communication queue
BOOL D_WriteFunction(HDEVICE hD, DEVWFCE wfce, WORD idx, D_VAL val, HWND hwnd, UINT post_msg)
{
	D_DEVICE *pD;
	unsigned char *pbuf;

	if (hD == NULL || wfce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVWFCE)+sizeof(WORD)+sizeof(D_VAL))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &wfce, sizeof(DEVWFCE));
	memcpy(pbuf+sizeof(DEVWFCE), &idx, sizeof(WORD));
	memcpy(pbuf+sizeof(DEVWFCE)+sizeof(WORD), &val, sizeof(D_VAL));

	//log operation
	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_W, idx, &val);

	return(MQ_SendQueueMessage(pD->hqueue, MQM_WRITEFCE,
														 (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, 0),
														 MQM_PRIORITY_NORMAL));
}

//*** function sends writing function to communication queue (with priority)
BOOL D_WriteFunctionPrior(HDEVICE hD, DEVWFCE wfce, WORD idx, D_VAL val, HWND hwnd, UINT post_msg,
													int priority)
{
	D_DEVICE *pD;
	unsigned char *pbuf;

	if (hD == NULL || wfce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVWFCE)+sizeof(WORD)+sizeof(D_VAL))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &wfce, sizeof(DEVWFCE));
	memcpy(pbuf+sizeof(DEVWFCE), &idx, sizeof(WORD));
	memcpy(pbuf+sizeof(DEVWFCE)+sizeof(WORD), &val, sizeof(D_VAL));

	//log operation
	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_W, idx, &val);

	return(MQ_SendQueueMessage(pD->hqueue, MQM_WRITEFCE,
														 (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, 0),
														 priority));
}

//*** function sends writing function to communication queue (on top, with priority)
BOOL D_WriteFunctionTopPrior(HDEVICE hD, DEVWFCE wfce, WORD idx, D_VAL val, HWND hwnd, UINT post_msg,
														 int priority)
{
	D_DEVICE *pD;
	unsigned char *pbuf;

	if (hD == NULL || wfce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVWFCE)+sizeof(WORD)+sizeof(D_VAL))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &wfce, sizeof(DEVWFCE));
	memcpy(pbuf+sizeof(DEVWFCE), &idx, sizeof(WORD));
	memcpy(pbuf+sizeof(DEVWFCE)+sizeof(WORD), &val, sizeof(D_VAL));

	//log operation
	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_W, idx, &val);

	return(MQ_SendTopQueueMessage(pD->hqueue, MQM_WRITEFCE,
															  (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
																(LPARAM)MAKELONG(post_msg, 0),
																priority));
}

//*** function sends writing function to communication queue (no logging)
BOOL D_WriteFunctionNoLog(HDEVICE hD, DEVWFCE wfce, WORD idx, D_VAL val, HWND hwnd, UINT post_msg)
{
	D_DEVICE *pD;
	unsigned char *pbuf;

	if (hD == NULL || wfce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVWFCE)+sizeof(WORD)+sizeof(D_VAL))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &wfce, sizeof(DEVWFCE));
	memcpy(pbuf+sizeof(DEVWFCE), &idx, sizeof(WORD));
	memcpy(pbuf+sizeof(DEVWFCE)+sizeof(WORD), &val, sizeof(D_VAL));

	//no log operation
	return(MQ_SendQueueMessage(pD->hqueue, MQM_WRITEFCE,
														 (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, 0),
														 MQM_PRIORITY_NORMAL));
}

//*** function sends make-oper function to communication queue
BOOL D_MakeOperFunction(HDEVICE hD, DEVOFCE ofce, WORD idx, HWND hwnd, UINT post_msg)
{
	D_DEVICE *pD;
	unsigned char *pbuf;

	if (hD == NULL || ofce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVOFCE)+sizeof(WORD)+sizeof(LPARAM))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &ofce, sizeof(DEVOFCE));
	memcpy(pbuf+sizeof(DEVOFCE), &idx, sizeof(WORD));
	memset(pbuf+sizeof(DEVOFCE)+sizeof(WORD), 0, sizeof(LPARAM));

	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_O, idx, NULL);

	return(MQ_SendQueueMessage(pD->hqueue, MQM_OPERFCE,
														 (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, 0),
														 MQM_PRIORITY_NORMAL));
}

//*** function sends make-oper function to communication queue (with priority)
BOOL D_MakeOperFunctionPrior(HDEVICE hD, DEVOFCE ofce, WORD idx, HWND hwnd, UINT post_msg, int priority)
{
	D_DEVICE *pD;
	unsigned char *pbuf;

	if (hD == NULL || ofce == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVOFCE)+sizeof(WORD))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &ofce, sizeof(DEVOFCE));
	memcpy(pbuf+sizeof(DEVOFCE), &idx, sizeof(WORD));

	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_O, idx, NULL);

	return(MQ_SendQueueMessage(pD->hqueue, MQM_OPERFCE,
														 (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, 0),
														 priority));
}

//*** function sends make-oper function with parameter to communication queue
BOOL D_MakeOperFunctionParam(HDEVICE hD, DEVOFCEPAR ofcepar, WORD idx, LPARAM param, HWND hwnd, UINT post_msg)
{
	D_DEVICE *pD;
	unsigned char *pbuf;

	if (hD == NULL || ofcepar == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//allocation
	if ((pbuf = (unsigned char *)malloc(sizeof(DEVOFCE)+sizeof(WORD)+sizeof(LPARAM))) == NULL)
		return FALSE;		//out of memory
	//store parameters
	memcpy(pbuf, &ofcepar, sizeof(DEVOFCEPAR));
	memcpy(pbuf+sizeof(DEVOFCEPAR), &idx, sizeof(WORD));
	memcpy(pbuf+sizeof(DEVOFCEPAR)+sizeof(WORD), &param, sizeof(LPARAM));

	if (g_e_proclogging)
		D_LogOperation(hD, DT_FCE_O, idx, NULL);

	return(MQ_SendQueueMessage(pD->hqueue, MQM_OPERFCEPAR,
														 (DPARAM)hD, (MPARAM)pbuf, (WPARAM)hwnd,
														 (LPARAM)MAKELONG(post_msg, 0),
														 MQM_PRIORITY_NORMAL));
}

//---------------------------------------------------------------------

//*** function logs set-operation to log file (when maked autozero, ...)
void D_LogOperation(HDEVICE hD, DTFCE type, DWORD prop, D_VAL *val)
{
	const TCHAR *cstrInternal = TEXT("internal");					//internal operation
	const TCHAR *cstrUserAction = TEXT("user action");					//user's action
	const TCHAR *cstrExternal = TEXT("external");					//external operation
	const TCHAR *cstrReadProp = TEXT("read prop.");				//read operation
	const TCHAR *cstrWriteProp = TEXT("write prop.");			//write operation
	const TCHAR *cstrPresetProp = TEXT("preset prop.");		//preset operation
	const TCHAR *cstrMakeOper = TEXT("make oper.");				//make operation
	const TCHAR *cstrUnknown = TEXT("unknown");						//unkown operation
	const TCHAR *cstrEmpty = TEXT("---");						//unkown operation
	const TCHAR *cstrQuest = TEXT("?");						//unkown operation
	//
	static DWORD count = 0;
	D_DEVICE *pD = NULL;
	FILE *fw = NULL;
	SYSTEMTIME st;
	TCHAR *ptext;
	int bp, bo;

	if (hD)
		pD = (D_DEVICE *)hD;

	if (count == 0) {
		count++;
		//do first work (begin file)

		//file head
		if ((fw = __fopen(g_e_proclogpath, TEXT("w"))) != NULL) {
			fprintf(fw, "Process log file\n\n");
			fprintf(fw, "no.\ttime & date\tprocess\tdevice (port)\tproperty\tvalue\n");
			fclose(fw);
		}

	}

	if ((fw = __fopen(g_e_proclogpath, TEXT("a"))) != NULL) {		//add to file
		//--- index, date & time ---
		GetLocalTime(&st);
		fprintf(fw, "%ld\t%02d:%02d:%02d.%03d %02d.%02d.%04d\t",
						count++,
						st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, st.wDay, st.wMonth, st.wYear);

		//--- type of operation ---
		switch (type) {
			case DT_FCE_I: ptext = (TCHAR *)cstrInternal; break;
			case DT_FCE_IPAR: ptext = (TCHAR *)cstrInternal; break;
			case DT_FCE_U: ptext = (TCHAR *)cstrUserAction; break;
			case DT_FCE_E: ptext = (TCHAR *)cstrExternal; break;
			case DT_FCE_R: ptext = (TCHAR *)cstrReadProp; break;
			case DT_FCE_W: ptext = (TCHAR *)cstrWriteProp; break;
			case DT_FCE_P: ptext = (TCHAR *)cstrPresetProp; break;
			case DT_FCE_O: ptext = (TCHAR *)cstrMakeOper; break;
			default: ptext = (TCHAR *)cstrUnknown; break;
		}
		fprintf(fw, CSTR_S "\t", ptext);

		//--- device (name+port) ---
		if (pD)
			fprintf(fw, CSTR_S " (" CSTR_S ")", pD->id, pD->cid);
		else
			fprintf(fw, CSTR_S, TEXT("ECOMAC"));		//no device -> ECOMAC
		fputc('\t', fw);

		//--- property ---
		bp = (pD && pD->n_p && pD->p && prop < pD->n_p);
		bo = (pD && pD->n_op && pD->op && prop < pD->n_op);
		switch (type) {
			case DT_FCE_I: fprintf(fw, CSTR_S, prop ? (const TCHAR *)prop : cstrEmpty); break;
			case DT_FCE_IPAR: fprintf(fw, CSTR_S, prop ? (const TCHAR *)prop : cstrEmpty); break;
			case DT_FCE_E: fprintf(fw, CSTR_S, prop ? (const TCHAR *)prop : cstrEmpty); break;
			case DT_FCE_R:
			case DT_FCE_W:
			case DT_FCE_P:
				fprintf(fw, CSTR_S, bp ? pD->p[prop].label : cstrQuest);
				if (bp && *pD->p[prop].suffix)
					fprintf(fw, CSTR_S, pD->p[prop].suffix);
				break;
			case DT_FCE_O:
				fprintf(fw, CSTR_S, bo ? pD->op[prop].label : cstrQuest);
				if (bo && *pD->op[prop].suffix)
					fprintf(fw, CSTR_S, pD->op[prop].suffix);
				break;
			default: fprintf(fw, CSTR_S, cstrEmpty); break;
		}
		fputc('\t', fw);

		//--- value ---
		if ((type == DT_FCE_W || type == DT_FCE_P) && bp && val) {
			switch (pD->p[prop].type) {
				case DT_BOOL: fprintf(fw, "%s", val->b ? "True" : "False"); break;
				case DT_INT: fprintf(fw, "%d",val->i); break;
				case DT_UINT: fprintf(fw, "%u", val->u); break;
				case DT_DOUBLE: fprintf(fw, "%g", val->d); break;
				case DT_INT64: fprintf(fw, "%I64d", val->i64); break;
				case DT_UINT64: fprintf(fw, "%I64u", val->u64); break;
				case DT_HANDLE: fprintf(fw, "%u", (unsigned)val->h); break;
				case DT_MEMORY:
				case DT_STRING:
					fprintf(fw, "(%u)", val->m.size);
					break;
				default: fprintf(fw, CSTR_S, cstrEmpty); break;
			}
		}
		else if (type == DT_FCE_IPAR && val) {
			fprintf(fw, "%u", val->u);
		}
		else
			fprintf(fw, CSTR_S, cstrEmpty);
		fputc('\n', fw);

		fclose(fw);
	}
}

//---------------------------------------------------------------------
//*** function sets property parameters
DWORD D_SetProperty(D_DEV_PROP *prop, const TCHAR *label,
										 const TCHAR *quan, const TCHAR *unit,
										 DWORD flags, DTYPE type)
{

	if (prop == NULL)
		return(ERROR_INVALID_HANDLE);

	//zero
	memset(prop, 0, sizeof(D_DEV_PROP));

	//label
	if (label) {
		__strncpy(prop->label, label, D_MAX_PROPNAME_LEN-1);
		*(prop->label+D_MAX_PROPNAME_LEN-1) = '\0';
	}
	//unit
	if (quan) {
		__strncpy(prop->unit.q, quan, D_MAX_PROPQUAN_LEN-1);
		*(prop->unit.q+D_MAX_PROPQUAN_LEN-1) = '\0';
	}
	if (unit) {
		__strncpy(prop->unit.u, unit, D_MAX_PROPUNIT_LEN-1);
		*(prop->unit.u+D_MAX_PROPUNIT_LEN-1) = '\0';
	}
	//flags
	prop->flags = flags;
	//type
	prop->type = type;

	return(NO_ERROR);
}

//*** function sets property parameters
DWORD D_SetPropertySuffix(D_DEV_PROP *prop, const TCHAR *label, const TCHAR *suffix,
												  const TCHAR *quan, const TCHAR *unit,
													DWORD flags, DTYPE type)
{

	if (prop == NULL)
		return(ERROR_INVALID_HANDLE);

	//zero
	memset(prop, 0, sizeof(D_DEV_PROP));

	//label
	if (label) {
		__strncpy(prop->label, label, D_MAX_PROPNAME_LEN-1);
		*(prop->label+D_MAX_PROPNAME_LEN-1) = '\0';
	}
	//suffix
	if (suffix) {
		__strncpy(prop->suffix, suffix, D_MAX_PROPSUFFIX_LEN-1);
		*(prop->suffix+D_MAX_PROPSUFFIX_LEN-1) = '\0';
	}
	//unit
	if (quan) {
		__strncpy(prop->unit.q, quan, D_MAX_PROPQUAN_LEN-1);
		*(prop->unit.q+D_MAX_PROPQUAN_LEN-1) = '\0';
	}
	if (unit) {
		__strncpy(prop->unit.u, unit, D_MAX_PROPUNIT_LEN-1);
		*(prop->unit.u+D_MAX_PROPUNIT_LEN-1) = '\0';
	}
	//flags
	prop->flags = flags;
	//type
	prop->type = type;

	return(NO_ERROR);
}

//*** function sets oper. property parameters
//sed: s/D_SetOperProperty|D_SetOperPropertySuffix(&pD->op\[\(.*\)\],\(.*\)));/D_SetOperProperty(&pD->op[\1],\2), 0);/g
DWORD D_SetOperProperty(D_DEV_OPERPROP *oprop, const TCHAR *label, DWORD flags)
{

	if (oprop == NULL)
		return(ERROR_INVALID_HANDLE);

	//label
	if (label) {
		__strncpy(oprop->label, label, D_MAX_PROPNAME_LEN-1);
		*(oprop->label+D_MAX_PROPNAME_LEN-1) = '\0';
	}
	else
		*oprop->label = '\0';
	//suffix
	*oprop->suffix = '\0';
	//flags
	oprop->flags = flags;

	return(NO_ERROR);
}

//*** function sets oper. property parameters (with suffix)
DWORD D_SetOperPropertySuffix(D_DEV_OPERPROP *oprop, const TCHAR *label, const TCHAR *suffix, DWORD flags)
{

	if (oprop == NULL)
		return(ERROR_INVALID_HANDLE);

	//label
	if (label) {
		__strncpy(oprop->label, label, D_MAX_PROPNAME_LEN-1);
		*(oprop->label+D_MAX_PROPNAME_LEN-1) = '\0';
	}
	else
		*oprop->label = '\0';
	//suffix
	if (suffix) {
		__strncpy(oprop->suffix, suffix, D_MAX_PROPSUFFIX_LEN-1);
		*(oprop->suffix+D_MAX_PROPSUFFIX_LEN-1) = '\0';
	}
	else
		*oprop->suffix = '\0';
	//flags
	oprop->flags = flags;

	return(NO_ERROR);
}

//*** function sets device status
BOOL D_SetState(WORD perc, const TCHAR *text, D_STAT *stat)
{
	if (stat) {
		stat->perc = perc;
		if (text)
			stat->text = text;
		else
			stat->text = cstr_empty;
		return TRUE;
	}
	return FALSE;
}

//*** function sets device status
BOOL D_SetStatePerc(WORD perc, D_STAT *stat)
{
	if (stat) {
		stat->perc = perc;
		return TRUE;
	}
	return FALSE;
}

//*** function test status running parameter
BOOL D_IsStateRun(D_STAT *stat)
{
	if (stat && stat->run == FALSE)
		return FALSE;
	return TRUE;
}

//*** function sets property parameters
BOOL D_ComparePropVal(D_DEV_PROP *prop, D_VAL *val)
{

	if (prop == NULL || val == NULL)
		return(FALSE);

	//compare
	switch (prop->type) {
		default:
		case DT_BOOL:
			if (prop->val.b == val->b)
				return(TRUE);
			break;
		case DT_INT:
			if (prop->val.i == val->i)
				return(TRUE);
			break;
		case DT_UINT:
			if (prop->val.u == val->u)
				return(TRUE);
			break;
		case DT_DOUBLE:
			if (D_fabs(prop->val.d - val->d) < __DBL_EPSILON__)		//is the same
				return(TRUE);
			break;
		case DT_INT64:
			if (prop->val.i64 == val->i64)
				return(TRUE);
			break;
		case DT_UINT64:
			if (prop->val.u64 == val->u64)
				return(TRUE);
			break;
		case DT_HANDLE:
			if (prop->val.h == val->h)
				return(TRUE);
			break;
		case DT_MEMORY:
		case DT_STRING:
			if (prop->val.m.size == val->m.size &&
					prop->val.m.buf && val->m.buf &&
					memcmp(prop->val.m.buf, val->m.buf, val->m.size) == 0)
				return(TRUE);
			break;
	}
	return(FALSE);
}


//-----------------------------
//*** function validates property range
//returns: valid range -> true; range had to be validated -> false
BOOL D_ValidatePropRange(D_DEV_PROP *prop)
{
	unsigned int min, max, zer;
	TCHAR *ptext;

	if (prop == NULL)
		return(FALSE);


	//validate CB
	if (prop->guitype == DGUIT_COMBOBOX && prop->guidata) {
		//count indexes
		max = 0;
		ptext = (TCHAR *)prop->guidata;
		while (*ptext) {
			max++;
			ptext += lstrlen(ptext)+1;
		}
		if ((prop->flags & DFLAG_MIN) && max > prop->max.u)
			max = prop->max.u;
		if (prop->val.u > max) {
			prop->val.u = max;
			return(FALSE);
		}
		return(TRUE);
	}
	else if (prop->guitype == DGUIT_RANGE) {
		//low
		min = (prop->flags & DFLAG_MIN) ? LOWORD(prop->min.u) : 0;
		max = (prop->flags & DFLAG_MAX) ? LOWORD(prop->max.u) : 0xFFFF;
		ptext = NULL;
		if (LOWORD(prop->val.u) < min) {
			prop->val.u = (prop->val.u & ~0xFFFF) | min;
			ptext = (TCHAR *)1;
		}
		else if (LOWORD(prop->val.u) > max) {
			prop->val.u = (prop->val.u & ~0xFFFF) | max;
			ptext = (TCHAR *)1;
		}
		//high
		min = (prop->flags & DFLAG_MIN) ? HIWORD(prop->min.u) : 0;
		max = (prop->flags & DFLAG_MAX) ? HIWORD(prop->max.u) : 0xFFFF;
		ptext = NULL;
		if (HIWORD(prop->val.u) < min) {
			prop->val.u = (prop->val.u & 0xFFFF) | (min << 16);
			ptext = (TCHAR *)1;
		}
		else if (HIWORD(prop->val.u) > max) {
			prop->val.u = (prop->val.u & 0xFFFF) | (max << 16);
			ptext = (TCHAR *)1;
		}
		//min > max
		if (LOWORD(prop->val.u) > HIWORD(prop->val.u)) {
			prop->val.u = MAKELONG(LOWORD(prop->val.u), LOWORD(prop->val.u));
			ptext = (TCHAR *)1;
		}
		//result
		if (ptext)
			return(FALSE);
		return(TRUE);
	}

	min = (prop->flags & DFLAG_MIN) ? 1 : 0;
	max = (prop->flags & DFLAG_MAX) ? 1 : 0;
	zer = (prop->flags & DFLAG_ZER) ? 1 : 0;

	//validate ranges
	switch (prop->type) {
		default:
		case DT_BOOL:
			if (min && prop->val.b < prop->min.b) {
				prop->val.b = prop->min.b;
				return(FALSE);
			}
			else if (max && prop->val.b > prop->max.b) {
				prop->val.b = prop->max.b;
				return(FALSE);
			}
			break;
		case DT_INT:
			if (zer && prop->val.i == 0)
				break;
			else if (min && prop->val.i < prop->min.i) {
				prop->val.i = prop->min.i;
				return(FALSE);
			}
			else if (max && prop->val.i > prop->max.i) {
				prop->val.i = prop->max.i;
				return(FALSE);
			}
			break;
		case DT_UINT:
			if (zer && prop->val.u == 0)
				break;
			else if (min && prop->val.u < prop->min.u) {
				prop->val.u = prop->min.u;
				return(FALSE);
			}
			else if (max && prop->val.u > prop->max.u) {
				prop->val.u = prop->max.u;
				return(FALSE);
			}
			break;
		case DT_DOUBLE:
			if (zer && prop->val.d == 0.0)
				break;
			else if (min && prop->val.d < prop->min.d) {
				prop->val.d = prop->min.d;
				return(FALSE);
			}
			else if (max && prop->val.d > prop->max.d) {
				prop->val.d = prop->max.d;
				return(FALSE);
			}
			break;
		case DT_INT64:
			if (zer && prop->val.i64 == 0)
				break;
			else if (min && prop->val.i64 < prop->min.i64) {
				prop->val.i64 = prop->min.i64;
				return(FALSE);
			}
			else if (max && prop->val.i64 > prop->max.i64) {
				prop->val.i64 = prop->max.i64;
				return(FALSE);
			}
			break;
		case DT_UINT64:
			if (zer && prop->val.u64 == 0)
				break;
			else if (min && prop->val.u64 < prop->min.u64) {
				prop->val.u64 = prop->min.u64;
				return(FALSE);
			}
			else if (max && prop->val.u64 > prop->max.u64) {
				prop->val.u64 = prop->max.u64;
				return(FALSE);
			}
			break;
		case DT_HANDLE:
		case DT_MEMORY:
		case DT_STRING:
			break;
	}
	return(TRUE);
}

//------------------------- Universal -------------------------------------
//*** function gets device off-line information
BOOL D_GetDeviceInfoProc(DEVINFO infoproc, DEVICEINFO *dinfo)
{
	//zero data
	memset(dinfo, 0, sizeof(DEVICEINFO));

	if (infoproc && infoproc(NULL, dinfo)) {
		//check name and alias
		if (dinfo->name == NULL)
			dinfo->name = c_strUnknownDev;
		if (dinfo->alias == NULL)
			dinfo->alias = dinfo->name;
		return(TRUE);
	}
	return(FALSE);		//unknown device
}


//*** function gets device off-line information
BOOL D_GetDeviceInfo(HDEVICE hD, DEVICEINFO *dinfo)
{
	D_DEVICE *pD;

	if (hD && dinfo) {
		pD = (D_DEVICE *)hD;
		return(D_GetDeviceInfoProc(pD->ufce.pGetDeviceInfo, dinfo));
	}
	return(FALSE);		//unknown device
}

//*** function returns device name
const TCHAR *D_GetDeviceName(HDEVICE hD)
{
	DEVICEINFO di;

	if (D_GetDeviceInfo(hD, &di))
		return(di.name);
	return(c_strUnknownDev);		//unknown device
}

//*** function returns device alias name
const TCHAR *D_GetDeviceAliasName(HDEVICE hD)
{
	DEVICEINFO di;

	if (D_GetDeviceInfo(hD, &di))
		return(di.alias);
	return(c_strUnknownDev);		//unknown device
}

//*** function tests ID (gets name, suffix, sw version, sn)
BOOL D_TestID(HDEVICE hD, TCHAR *name, TCHAR *suffix, TCHAR *sw, TCHAR *sn)
{
	D_DEVICE *pD;

	if (hD) {
		pD = (D_DEVICE *)hD;
		if (pD->ufce.pTestID)
			return(pD->ufce.pTestID(hD, name, suffix, sw, sn));
	}
	return FALSE;
}

//----------
//*** function reads all parameters (no thread, open/close device!)
DWORD D_SetupReadAllParameters(HDEVICE hD, unsigned int *err_cnt, HSTRING hs)
{
	D_DEVICE *pD;
	WORD p;
	DWORD ret, pret;
	unsigned int n = 0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//open device
	ret = D_OpenDevice(hD);
	if (ret != NO_ERROR)
		return(ret);

	//thorugh oper.
	while (1) {
		//test ID
		if (!D_TestID(hD, NULL, NULL, NULL, NULL)) {
			ret = ERROR_FILE_NOT_FOUND;
			break;
		}
		//read all parameters
		if (pD->ufce.pReadProp && pD->p) {
			for (p=0; p<pD->n_p; p++) {
				//through "receivable" parameters
				if (pD->p[p].flags & DFLAG_RECV) {
					pret = pD->ufce.pReadProp(hD, p);
					if (pret != NO_ERROR && (pD->p[p].flags & DFLAG_IGNORE) == 0) {
						n++;		//!!! debug which property
						if (hs) {
							HSTR_AppendPrintf(hs, TEXT("\n%d: %s%s"), n, pD->p[p].label, pD->p[p].suffix);
						}
					}
				}
			}
		}
		//read oper. parameters
		if (pD->ufce.pSetOper) {
			for (p=0; p<pD->n_op; p++) {
				if (pD->op[p].flags & DFLAG_RECV) {
					pret = pD->ufce.pSetOper(hD, p);
					if (pret != NO_ERROR && (pD->p[p].flags & DFLAG_IGNORE) == 0) {
						n++;
						if (hs) {
							HSTR_AppendPrintf(hs, TEXT("\n%d: %s%s"), n, pD->p[p].label, pD->p[p].suffix);
						}
					}
				}
			}
		}

		break;	//1x
	}

  //validate props
  D_ValidateProps(hD, (DWORD)-1);

	//close device
	D_CloseDevice(hD);

	//return number of read errors
	if (err_cnt)
		*err_cnt = n;

	return(ret);
}

//*** function write all parameters (no thread, open/close device!)
DWORD D_SetupWriteAllParameters(HDEVICE hD, unsigned int *err_cnt, HSTRING hs)
{
	D_DEVICE *pD;
	WORD p;
	DWORD ret, pret;
	unsigned int n = 0;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//open device
	ret = D_OpenDevice(hD);
	if (ret != NO_ERROR)
		return(ret);

	//thorugh oper.
	while (1) {
		//test ID
		if (!D_TestID(hD, NULL, NULL, NULL, NULL)) {
			ret = ERROR_FILE_NOT_FOUND;
			break;
		}

		//validate props?
		D_ValidateProps(hD, (DWORD)-1);

		//write all parameters
		if (pD->ufce.pWriteProp && pD->p) {
			for (p=0; p<pD->n_p; p++) {
				//go through "sendable" parameters
				if ((pD->p[p].flags & DFLAG_SEND) == DFLAG_SEND) {
					if (pD->p[p].group && p < (pD->n_p-1) && pD->p[p].group == pD->p[p+1].group && (pD->p[p+1].flags & DFLAG_SEND) == DFLAG_SEND) {
						D_PresetProp(hD, p, pD->p[p].val);		//preset
					}
					else {
						pret = pD->ufce.pWriteProp(hD, p, pD->p[p].val);
						if (pret != NO_ERROR && (pD->p[p].flags & DFLAG_IGNORE) == 0) {
							n++;	//!!! debug which property
							if (hs) {
								HSTR_AppendPrintf(hs, TEXT("\n%d: %s%s"), n, pD->p[p].label, pD->p[p].suffix);
							}
						}
					}
				}
			}
		}

		break;	//1x
	}

	//close device
	D_CloseDevice(hD);

	//return number of read errors
	if (err_cnt)
		*err_cnt = n;

	return(ret);
}

//-----------------------------------------
//*** function processes setup communication-dialog
DEVICEDLGPROC D_SetupCommDlgProc(HDEVICE hD)
{
	if (hD)
		return(((D_DEVICE *)hD)->ufce.pSetupCommDlgProc);
	return(NULL);
}

//*** function processes setup parameter-dialog
DEVICEDLGPROC D_SetupParamDlgProc(HDEVICE hD)
{
	if (hD)
		return(((D_DEVICE *)hD)->ufce.pSetupParamDlgProc);
	return(NULL);
}

//*** function processes setup init-dialog
DEVICEDLGPROC D_SetupInitDlgProc(HDEVICE hD)
{
	if (hD)
		return(((D_DEVICE *)hD)->ufce.pSetupInitDlgProc);
	return(NULL);
}

//*** function processes on-line (ready) dialog
DEVICEDLGPROC D_OnLineDlgProc(HDEVICE hD)
{
	if (hD)
		return(((D_DEVICE *)hD)->ufce.pOnLineDlgProc);
	return(NULL);
}

//----------------------------- Useful -----------------------------------
//*** function sends start message (rel. time) to device window
BOOL D_SendExErrorMsg(HDEVICE hD, DWORD err)
{
	D_DEVICE *pd;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->_hwnd) {
			//send WMD_EX_ERROR msg. + error
			SendNotifyMessage(pd->_hwnd, WMD_EX_ERROR, 0, (LPARAM)err);
			return(TRUE);
		}
	}
	return(FALSE);
}

//*** function sends start message (rel. time) to device window
BOOL D_SendExStartMsg(HDEVICE hD, double tstart)
{
	D_DEVICE *pd;
	double *pt;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->_hwnd) {
			//allocate time, must be freed during reception
			pt = (double *)malloc(sizeof(double));
			if (pt) {
				*pt = tstart;
				//send WMD_EX_START msg. + start time
				SendNotifyMessage(pd->_hwnd, WMD_EX_START, 0, (LPARAM)pt);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

//*** function sends stop message (rel. time) to device window
BOOL D_SendExStopMsg(HDEVICE hD, double tstop)
{
	D_DEVICE *pd;
	double *pt;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->_hwnd) {
			//allocate time, must be freed during reception
			pt = (double *)malloc(sizeof(double));
			if (pt) {
				*pt = tstop;
				//send WMD_EX_STOP msg. + stop time
				SendNotifyMessage(pd->_hwnd, WMD_EX_STOP, 0, (LPARAM)pt);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

//*** function sends pause message (rel. time) to device window
BOOL D_SendExPauseMsg(HDEVICE hD, double tpause)
{
	D_DEVICE *pd;
	double *pt;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->_hwnd) {
			//allocate time, must be freed during reception
			pt = (double *)malloc(sizeof(double));
			if (pt) {
				*pt = tpause;
				//send WMD_EX_PAUSE msg. + pause time
				SendNotifyMessage(pd->_hwnd, WMD_EX_PAUSE, 0, (LPARAM)pt);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

//*** function sends resume message (rel. time) to device window
BOOL D_SendExResumeMsg(HDEVICE hD, double tresume)
{
	D_DEVICE *pd;
	double *pt;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->_hwnd) {
			//allocate time, must be freed during reception
			pt = (double *)malloc(sizeof(double));
			if (pt) {
				*pt = tresume;
				//send WMD_EX_RESUME msg. + resume time
				SendNotifyMessage(pd->_hwnd, WMD_EX_RESUME, 0, (LPARAM)pt);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

//*** function sends mark message (rel. time) to device window
BOOL D_SendExMarkMsg(HDEVICE hD, double tmark, TCHAR *descr)
{
	D_DEVICE *pd;
	double *pt;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->_hwnd) {
			//allocate time, must be freed during reception
			pt = (double *)malloc(sizeof(double));
			if (pt) {
				*pt = tmark;
				//send WMD_EX_MARK msg. + mark time
				SendNotifyMessage(pd->_hwnd, WMD_EX_MARK_E, (WPARAM)descr, (LPARAM)pt);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

//*** function sends zero message to device window
BOOL D_SendExZeroMsg(HDEVICE hD)
{
	D_DEVICE *pd;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->_hwnd) {
			//send WMD_EX_ZERO msg.
			SendNotifyMessage(pd->_hwnd, WMD_EX_ZERO, 0, (LPARAM)0);
			return(TRUE);
		}
	}
	return(FALSE);
}

//*** function sends frac. C/W message to device window
BOOL D_SendExFracCWMsg(HDEVICE hD)
{
	D_DEVICE *pd;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->_hwnd) {
			//send WMD_EX_FCW msg.
			SendNotifyMessage(pd->_hwnd, WMD_EX_FCW, 0, (LPARAM)0);
			return(TRUE);
		}
	}
	return(FALSE);
}

void CALLBACK D_DelayedStopTimerProc(UINT tid, UINT msg, DWORD hD, DWORD dw1, DWORD dw2)
{
	D_SendExStopMsg((HDEVICE)hD, timer_GetTime());
}

//*** function sends delayed stop message to device window
BOOL D_SendDelayedStopMsg(HDEVICE hD, DWORD msdelay)
{
	UINT tid;

	tid = timeSetEvent(msdelay, msdelay/20, D_DelayedStopTimerProc, (DWORD)hD, TIME_ONESHOT);
	return(tid > 0 ? TRUE : FALSE);
}

/** @brief Return real time from LPARAM pointer of SendNotify proc message
 *
 * @param lparam LPARAM
 * @return double
 *
 */
double D_GetExTime(LPARAM lparam)
{
	double t;

	if (lparam) {
		t = *(double *)lparam;
		free((void *)lparam);
	}
	else
		t = timer_GetTime();

	return(t);
}

/** @brief Check EX-time to be in valid deviation
 *
 * @param t double
 * @return int
 *
 */
void D_TestExTime(double *t)
{
	if (t) {
		double tnow = timer_GetTime();
		if (D_fabs(*t-tnow) > D_MAX_EXTIME_DEV) {
			*t = tnow;	//replace by actual time
		}
	}
}


//-----------------------------------
//*** function gets number of properties with certain flags
WORD D_GetDevPropertiesByFlags(HDEVICE hD, DWORD flags)
{
	WORD p, count = 0;
	D_DEVICE *pd;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->n_p && pd->p) {
			for (p=0; p<pd->n_p; p++) {
				if ((pd->p[p].flags & flags) == flags)
					count++;
			}
		}
	}
	return(count);
}

//*** function gets index of property with certain flags and offset (order)
//offset: means relative index from the first such property
WORD D_GetDevPropertyIndexByFlags(HDEVICE hD, DWORD flags, WORD offset)
{
	WORD p, count = 0;
	D_DEVICE *pd;

	if (hD) {
		pd = (D_DEVICE *)hD;

		if (pd->n_p && pd->p) {
			for (p=0; p<pd->n_p; p++) {
				if ((pd->p[p].flags & flags) == flags) {
					count++;
					if (count > offset)
						return(p);
				}
			}
		}
	}
	return((WORD)-1);
}

//*** function returns device property text variable
//type: 0=label, 1=suffix, 2=quantity, 3=unit,
const TCHAR *D_GetDevPropertyTextVar(HDEVICE hD, WORD pid, int type)
{
	D_DEVICE *pd;

	//test dev. handle
	if (hD) {
		pd = (D_DEVICE *)hD;
		//test for property
		if (pd->n_p && pd->p && pid < pd->n_p) {
			switch (type) {
				default:
				case 0: return(pd->p[pid].label);
				case 1: return(pd->p[pid].suffix);
				case 2: return(pd->p[pid].unit.q);
				case 3: return(pd->p[pid].unit.u);
			}
		}
	}
	return(NULL);
}

//-------------

//*** get device label
TCHAR *D_GetDevLabel(HDEVICE hD)
{
	TCHAR *pstr = NULL;

	if (hD) {
		pstr = ((D_DEVICE *)hD)->label;
	}
	return(pstr ? pstr : cstr_empty);
}

//*** get device name from identification
TCHAR *D_GetDevName(HDEVICE hD)
{
	TCHAR *pstr = NULL;

	if (hD) {
		pstr = GetSubString(((D_DEVICE *)hD)->id, 0);
	}
	return(pstr ? pstr : cstr_empty);
}

//*** get defice FW version from identification
TCHAR *D_GetDevType(HDEVICE hD)
{
	TCHAR *pstr = NULL;

	if (hD) {
		pstr = GetSubString(((D_DEVICE *)hD)->id, 1);
	}
	return(pstr ? pstr : cstr_empty);
}

//*** get defice FW version from identification
TCHAR *D_GetDevFirmware(HDEVICE hD)
{
	TCHAR *pstr = NULL;

	if (hD) {
		pstr = GetSubString(((D_DEVICE *)hD)->id, 2);
	}
	return(pstr ? pstr : cstr_empty);
}

//*** get device serial number from identification
TCHAR *D_GetDevSerialNumber(HDEVICE hD)
{
	TCHAR *pstr = NULL;

	if (hD) {
		pstr = GetSubString(((D_DEVICE *)hD)->id, 3);
	}
	return(pstr ? pstr : cstr_empty);
}

//---------------------------------
//*** function runs oper. function for all devices
DWORD D_RunOperFunc(HDEVICES *hDs, WORD op_index)
{
	D_DEVICE *pD;
	DWORD d;


	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && op_index < pD->n_op) {
				//make oper. funcion
				D_MakeOperFunction((HDEVICE)pD, pD->ufce.pSetOper, op_index, NULL, 0);
			}
		}
	}
	return(NO_ERROR);
}

//-------------------------------------
//*** function shows identification of found device
void D_ShowFoundDeviceID(HWND hwnd, HDEVICE hD, int found)
{
	TCHAR ttext[512];
	TCHAR *dname = NULL, *dtype = NULL, *dsw = NULL, *dsn = NULL;

	dname = D_GetDevName(hD);
	dtype = D_GetDevType(hD);
	dsw = D_GetDevFirmware(hD);
	dsn = D_GetDevSerialNumber(hD);

	if (found) {
		__sprintf(ttext, TEXT("%s: %s (type: %s, fw: %s, sn: %s)."),
							_("Found"),
							dname && *dname ? dname : c_strUnknownDev,
							dtype && *dtype ? dtype : cstr_questmark,
							dsw && *dsw ? dsw : cstr_questmark,
							dsn && *dsn ? dsn : cstr_questmark);
		MessageBox(hwnd, ttext, _("Device identification"), MB_OK);
	}
	else
		ErrorBox(hwnd, _("Communication error!"), 0);
}


//---------

//*** function make critical stop for devices
DWORD D_CriticalStop(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d;


	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->ufce.pCriticalStop) {
				D_MakeFunctionParam(hDs->hd[d], pD->ufce.pCriticalStop, 0, NULL, 0, 0, TEXT("Critical_stop"));
			}
		}
	}
	return(NO_ERROR);
}

//*** function make pause for devices
DWORD D_PauseDevices(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d;


	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->ufce.pPauseResume) {
				D_MakeFunctionParam(hDs->hd[d], pD->ufce.pPauseResume, D_PRMODE_PAUSE, NULL, 0, 0, TEXT("Pause"));
			}
		}
	}
	return(NO_ERROR);
}

//*** function make resume for devices
DWORD D_ResumeDevices(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d;


	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->ufce.pPauseResume) {
				D_MakeFunctionParam(hDs->hd[d], pD->ufce.pPauseResume, D_PRMODE_RESUME, NULL, 0, 0, TEXT("Resume"));
			}
		}
	}
	return(NO_ERROR);
}


//------------------------- Device Dependent -----------------------------
//----------------------------- detectors --------------------------------
//*** function tests if autozero is available (returns number of avail. autozeros)
BOOL D_IsAutozero(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d, count = 0;

	if (hDs && hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->detfce.pAutozero) {
				count++;
			}
		}
		return((BOOL)count);
	}
	return(FALSE);
}

//*** function autozeros devices
DWORD D_RunAutozeros(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d;


	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->detfce.pAutozero) {
				D_MakeFunction(hDs->hd[d], pD->detfce.pAutozero, NULL, 0, 0, TEXT("Autozero"));
			}
		}
	}
	return(NO_ERROR);
}

//*** function sets voltage
DWORD D_GetSignal(HDEVICE hD, int how, DWORD idx, double *abs, double *time, D_DHQUEUE *dhq)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	if (pD->detfce.pGetSignal == NULL)
		return(ERROR_INVALID_HANDLE);
	return(pD->detfce.pGetSignal(hD, how, idx, abs, time, dhq));
}


//------------------------------- pumps ----------------------------------

//------------------------- fraction collectors --------------------------
//*** function process FC function on FC
DWORD D_ProcFunctionFC(HDEVICE hD, D_FCFCE fcfce, unsigned int val)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//test fc-fce
	switch (fcfce) {
		case D_FCFCE_INFO:
			if (pD->fcfce.pGetFCInfo)
				return(pD->fcfce.pGetFCInfo(hD, (D_FC_INFO *)val));
			return(ERROR_INVALID_HANDLE);
			break;
		case D_FCFCE_START:
			if (pD->fcfce.pStart)
				D_MakeFunctionParam(hD, pD->fcfce.pStart, val, NULL, 0, 0, TEXT("Fraction start"));
			break;
		case D_FCFCE_COLLECT:
			if (pD->fcfce.pCollectWaste)
				D_MakeFunctionParam(hD, pD->fcfce.pCollectWaste, 1, NULL, 0, 0, TEXT("Fraction collect"));
			break;
		case D_FCFCE_WASTE:
			if (pD->fcfce.pCollectWaste)
				D_MakeFunctionParam(hD, pD->fcfce.pCollectWaste, 0, NULL, 0, 0, TEXT("Fraction waste"));
			break;
		case D_FCFCE_NEXT:
			if (pD->fcfce.pNext)
				D_MakeFunction(hD, pD->fcfce.pNext, NULL, 0, 0, TEXT("Fraction next"));
			break;
		case D_FCFCE_STOP:
			if (pD->fcfce.pStop)
				D_MakeFunction(hD, pD->fcfce.pStop, NULL, 0, 0, TEXT("Fraction stop"));
			break;
/*
		case D_FCFCE_SETPARS:
			if (pD->fcfce.pSetPars)
				D_MakeFunction(hD, pD->fcfce.pSetPars, (D_FC_PARS*)val, 0, 0, TEXT("Fraction set pars"));
			break;
*/
		default:
			return(ERROR_INVALID_DATA);
	}
	return(NO_ERROR);
}

//------------------------- autosamplers --------------------------
//*** function process AS function on AS
DWORD D_ProcFunctionAS(HDEVICE hD, D_ASFCE asfce, unsigned int val)
{
	D_DEVICE *pD;
	TCHAR ttext[128];

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//test as-fce
	switch (asfce) {
		case D_ASFCE_INFO:
			if (pD->asfce.pGetASInfo)
				return(pD->asfce.pGetASInfo(hD, (D_AS_INFO *)val));
			return(ERROR_INVALID_HANDLE);
		case D_ASFCE_STATUS:
			break;
		case D_ASFCE_INJECT:
			if (pD->asfce.pMakeInjection) {
				__sprintf(ttext, TEXT("AS inject vial %u"), val);
				D_MakeFunctionParam(hD, pD->asfce.pMakeInjection, val, NULL, 0, 0, ttext);
			}
			break;
		case D_ASFCE_STOP:
			if (pD->asfce.pStop) {
				D_MakeFunction(hD, pD->asfce.pStop, NULL, 0, 0, TEXT("AS stop"));
			}
			break;
		default:
			return(ERROR_INVALID_DATA);
	}
	return(NO_ERROR);
}


//----------------------------- special --------------------------------
//*** function tests if show spectrum is available
BOOL D_IsSpectrum(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d, count = 0;

	if (hDs && hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->ufce.pSpecial && pD->ufce.pSpecial(hDs->hd[d], DSPEC_ASK(DSPEC_SHOWSPECTRUM), 0)) {
				count++;
			}
		}
		return((BOOL)count);
	}
	return(FALSE);
}

//*** function show spectrums
DWORD D_ShowSpectrums(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d;


	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->ufce.pSpecial && pD->ufce.pSpecial(hDs->hd[d], DSPEC_ASK(DSPEC_SHOWSPECTRUM), 0)) {
				pD->ufce.pSpecial(hDs->hd[d], DSPEC_SHOWSPECTRUM, 0);
			}
		}
	}
	return(NO_ERROR);
}

//*** function check device after method load
DWORD D_CheckLoadedDevice(HDEVICE hD, unsigned int dver)
{
	D_DEVICE *pD;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;
	if (pD->ufce.pSpecial) {
		pD->ufce.pSpecial(hD, DSPEC_LOADCHECK, dver);
	}
	return(NO_ERROR);
}

//*** function tests if lamp control is available
BOOL D_IsLampControl(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d, count = 0;

	if (hDs && hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->ufce.pSpecial && pD->ufce.pSpecial(hDs->hd[d], DSPEC_ASK(DSPEC_LAMPCTRL), 0)) {
				count++;
			}
		}
		return((BOOL)count);
	}
	return(FALSE);
}

//*** function set lamps
DWORD D_SetLamp(HDEVICES *hDs, BOOL lamp_state)
{
	D_DEVICE *pD;
	DWORD d;


	if (hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	if (hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->ufce.pSpecial && pD->ufce.pSpecial(hDs->hd[d], DSPEC_ASK(DSPEC_LAMPCTRL), 0)) {
				pD->ufce.pSpecial(hDs->hd[d], DSPEC_LAMPCTRL, (LPARAM)lamp_state);
			}
		}
	}
	return(NO_ERROR);
}

//*** function tests if IPAUSE is needed
BOOL D_IsIPause(HDEVICES *hDs)
{
	D_DEVICE *pD;
	DWORD d, count = 0;

	if (hDs && hDs->dcount && hDs->hd) {
		//through devices
		for (d=0; d<hDs->dcount; d++) {
			pD = (D_DEVICE *)hDs->hd[d];
			if (pD && pD->ufce.pSpecial && pD->ufce.pSpecial(hDs->hd[d], DSPEC_IPAUSE, 0)) {
				count++;
			}
		}
		return((BOOL)count);
	}
	return(FALSE);
}

//********************** Thread Function *************************************
//*** function process queue communication messages running on separate thread
//dparam ~ device handle
//mparam ~ signal index
//wparam ~ window handle for notification
//lparam ~ notification message id
BOOL D_CommTQueueProc(UINT msg, DPARAM dparam, MPARAM mparam, WPARAM wparam, LPARAM lparam, int run)
{
	DWORD ret;
	unsigned char *pbuf;

	switch (msg) {
		//after queue creation
		case MQM_CREATE_QUEUE:
			break;

		//message for reading device voltage & notify reading
		case MQM_READVOLTAGE:
			if (run) {
				ret = D_GetSignal((HDEVICE)dparam, 1, mparam, NULL, NULL, NULL);
				if (ret != D_ERROR_DONTNOTIFY && wparam && lparam)
					SendNotifyMessage((HWND)wparam, (UINT)lparam, dparam, mparam);
			}
			return TRUE;

		case MQM_MAKEFCE:
			if (run) {
				if (dparam && mparam) {
					mparam = ((DEVFCE)mparam)((HDEVICE)dparam);
					dparam = (DWORD)HIWORD(lparam);
					lparam = (DWORD)LOWORD(lparam);
					if (wparam && lparam)
						SendNotifyMessage((HWND)wparam, (UINT)lparam, dparam, mparam);
				}
			}
			return TRUE;

		case MQM_MAKEPARFCE:
			if (dparam && mparam) {
				pbuf = (unsigned char *)mparam;
				if (pbuf) {
					if (run) {
						mparam = (*(DEVPARFCE *)pbuf)((HDEVICE)dparam, *(LPARAM *)(pbuf+sizeof(DEVPARFCE)));
						dparam = *(DWORD *)(pbuf+sizeof(DEVPARFCE));
						if (wparam && lparam)
							SendNotifyMessage((HWND)wparam, (UINT)lparam, dparam, mparam);
					}
					free((void *)pbuf);		//free memory
				}
			}
			return TRUE;

		case MQM_READFCE:
			if (dparam && mparam) {
				pbuf = (unsigned char *)mparam;
				if (pbuf) {
					if (run) {
						mparam = (*(DEVRFCE *)pbuf)((HDEVICE)dparam, *(WORD *)(pbuf+sizeof(DEVRFCE)));
						dparam = (DWORD)*(WORD *)(pbuf+sizeof(DEVRFCE));
						if (wparam && lparam)
							SendNotifyMessage((HWND)wparam, (UINT)lparam, dparam, mparam);
					}
					free((void *)pbuf);		//free memory
				}
			}
			return TRUE;

		case MQM_WRITEFCE:
			if (dparam && mparam) {
				pbuf = (unsigned char *)mparam;
				if (pbuf) {
					if (run) {
						mparam = (*(DEVWFCE *)pbuf)((HDEVICE)dparam,
																				*(WORD *)(pbuf+sizeof(DEVWFCE)),
																				*(D_VAL *)(pbuf+sizeof(DEVWFCE)+sizeof(WORD)));
						dparam = (DWORD)*(WORD *)(pbuf+sizeof(DEVWFCE));
						if (wparam && lparam)
							SendNotifyMessage((HWND)wparam, (UINT)lparam, dparam, mparam);
					}
					free((void *)pbuf);		//free memory
				}
			}
			return TRUE;

		case MQM_OPERFCE:
			if (dparam && mparam) {
				pbuf = (unsigned char *)mparam;
				if (pbuf) {
					if (run) {
						mparam = (*(DEVOFCE *)pbuf)((HDEVICE)dparam,
																				*(WORD *)(pbuf+sizeof(DEVOFCE)));
						dparam = (DWORD)*(WORD *)(pbuf+sizeof(DEVOFCE));
						if (wparam && lparam)
							SendNotifyMessage((HWND)wparam, (UINT)lparam, dparam, mparam);
					}
					free((void *)pbuf);		//free memory
				}
			}
			return TRUE;

		case MQM_OPERFCEPAR:
			if (dparam && mparam) {
				pbuf = (unsigned char *)mparam;
				if (pbuf) {
					if (run) {
						mparam = (*(DEVOFCEPAR *)pbuf)((HDEVICE)dparam,
																					 *(WORD *)(pbuf+sizeof(DEVOFCEPAR)),
																					 *(LPARAM *)(pbuf+sizeof(DEVOFCEPAR)+sizeof(WORD)));
						dparam = (DWORD)*(WORD *)(pbuf+sizeof(DEVOFCEPAR));
						if (wparam && lparam)
							SendNotifyMessage((HWND)wparam, (UINT)lparam, dparam, mparam);
					}
					free((void *)pbuf);		//free memory
				}
			}
			return TRUE;

		//before queue destruction
		case MQM_DESTROY_QUEUE:
			break;
	}

	return FALSE;
}

//------------------------------------------------------------
//*** function creates property data queue
D_DHQUEUE D_CreateDataQueue(DWORD lparam)
{
	D_DATAQUEUE *dq;

	//allocate queue
	if ((dq = (D_DATAQUEUE *)malloc(sizeof(D_DATAQUEUE))) == NULL)
		return(NULL);		//out of memory
	memset(dq, 0, sizeof(D_DATAQUEUE));		//zero memory

	//init. critical section
	InitializeCriticalSection(&dq->cs);

	return((D_DHQUEUE)dq);
}

//*** function destroys property data queue
DWORD D_DestroyDataQueue(D_DHQUEUE hQ)
{
	D_DATAQUEUE *dq;
	D_DATAQUEUEITEM *di;

	if (hQ == NULL)
		return(ERROR_INVALID_HANDLE);
	dq = (D_DATAQUEUE *)hQ;

	//destroy queue memory
	while (dq->size && dq->front) {
		di = dq->front;
		//removes from queue
		if (di->next) {
			dq->front = (D_DATAQUEUEITEM *)di->next;
			di->next = NULL;
			dq->size--;
		}
		else {
			//was last one
			dq->front = NULL;
			dq->back = NULL;
			dq->size = 0;
		}

		//discard data
		D_DiscardDataQueueContent(&di->content);

		//free item
		free((void *)di);
	}

	//delete critical section
	DeleteCriticalSection(&dq->cs);
	//zero memory
	memset(dq, 0, sizeof(D_DATAQUEUE));
	//free queue
	free((void *)dq);
	dq = NULL;

	return(NO_ERROR);
}

//*** function allocates data queue content
DWORD D_AllocateDataQueueContent(DWORD size, D_DATAQUEUECONTENT *dqc)
{

	if (dqc == NULL)
		return(ERROR_INVALID_HANDLE);

	memset(dqc, 0, sizeof(D_DATAQUEUECONTENT));		//zero contents

	if (size) {
		//double size
		if ((dqc->dtime = (double *)malloc(size*sizeof(double))) == NULL)
			return(ERROR_OUTOFMEMORY);
		//allocate value memory
		if ((dqc->value = (double *)malloc(size*sizeof(double))) == NULL)
			return(ERROR_OUTOFMEMORY);
		dqc->count = size;
		dqc->preserve = FALSE;		//can be discarded
	}
	return(NO_ERROR);
}

//*** function discards data queue content (if enabled = no preservation)
DWORD D_DiscardDataQueueContent(D_DATAQUEUECONTENT *dqc)
{

	if (dqc == NULL)
		return(ERROR_INVALID_HANDLE);

	if (!dqc->preserve) {
		//test if time memory
		if (dqc->count && dqc->dtime)
			free((void *)dqc->dtime);		//free time memory
		//test if value memory
		if (dqc->count && dqc->value)
			free((void *)dqc->value);		//free value memory
		//no data any more
		dqc->count = 0;
		dqc->dtime = NULL;
		dqc->value = NULL;
	}
	return(NO_ERROR);
}

//*** function pushes data queue item (adds it at the end)
//if item pushed => returns TRUE
BOOL D_PushDataQueueItem(D_DHQUEUE hQ, D_DATAQUEUECONTENT *dqc)
{
	D_DATAQUEUE *dq;
	D_DATAQUEUEITEM *di, *dt;

	if (hQ == NULL)
		return(ERROR_INVALID_HANDLE);

	dq = (D_DATAQUEUE *)hQ;
	//allocate item
	if ((di = (D_DATAQUEUEITEM *)malloc(sizeof(D_DATAQUEUEITEM))) == NULL)
		return(ERROR_OUTOFMEMORY);		//out of memory
	memset(di, 0, sizeof(D_DATAQUEUEITEM));		//clear new item
	if (dqc)
		di->content = *dqc;

	//ENTER CRITICAL SECTION
	EnterCriticalSection(&dq->cs);

	if (dq->size == 0) {
		//1st item
		dq->front = di;
		dq->back = di;
		dq->size++;
	}
	else {
		//next items
		dt = dq->back;
		if (dt == NULL) {
			free((void *)di);		//free memory
			return(FALSE);		//?no last item
		}
		dt->next = di;
		dq->back = di;
		dq->size++;		//count
	}

	//LEAVE CRITICAL SECTION
	LeaveCriticalSection(&dq->cs);

	return(NO_ERROR);
}

//*** function pops data queue item (removes it from the beginning)
//if pops any item => returns TRUE
BOOL D_PopDataQueueItem(D_DHQUEUE hQ, D_DATAQUEUECONTENT *dqc)
{
	D_DATAQUEUE *dq;
	D_DATAQUEUEITEM *di;
	BOOL ret = FALSE;

	if (hQ == NULL)
		return(FALSE);

	dq = (D_DATAQUEUE *)hQ;

	//ENTER CRITICAL SECTION
	EnterCriticalSection(&dq->cs);

	if (dq->size && dq->front) {
		di = dq->front;
		//removes from queue
		if (di->next) {
			dq->front = (D_DATAQUEUEITEM *)di->next;
			di->next = NULL;
			dq->size--;
		}
		else {
			//was last one
			dq->front = NULL;
			dq->back = NULL;
			dq->size = 0;
		}
		//get data
		if (dqc) {
			*dqc = di->content;

		}
		//free item
		free((void *)di);
		ret = TRUE;
	}

	//LEAVE CRITICAL SECTION
	LeaveCriticalSection(&dq->cs);

	return(ret);
}

//*** function get 1st data queue item
//if any item => returns TRUE
BOOL D_GetDataQueueItem(D_DHQUEUE hQ, D_DATAQUEUECONTENT *dqc)
{
	D_DATAQUEUE *dq;
	D_DATAQUEUEITEM *di;
	BOOL ret = FALSE;

	if (hQ == NULL)
		return(FALSE);

	dq = (D_DATAQUEUE *)hQ;

	//ENTER CRITICAL SECTION
	EnterCriticalSection(&dq->cs);

	if (dq->size && dq->front) {
		di = dq->front;
		//get data
		if (dqc) {
			*dqc = di->content;
		}
		ret = TRUE;
	}

	//LEAVE CRITICAL SECTION
	LeaveCriticalSection(&dq->cs);

	return(ret);
}

//*** function remove 1st data queue item
//if any item => returns TRUE
BOOL D_RemoveDataQueueItem(D_DHQUEUE hQ)
{
	D_DATAQUEUE *dq;
	D_DATAQUEUEITEM *di;
	BOOL ret = FALSE;

	if (hQ == NULL)
		return(FALSE);

	dq = (D_DATAQUEUE *)hQ;

	//ENTER CRITICAL SECTION
	EnterCriticalSection(&dq->cs);

	if (dq->size && dq->front) {
		di = dq->front;
		//removes from queue
		if (di->next) {
			dq->front = (D_DATAQUEUEITEM *)di->next;
			di->next = NULL;
			dq->size--;
		}
		else {
			//was last one
			dq->front = NULL;
			dq->back = NULL;
			dq->size = 0;
		}
		//discard data
		D_DiscardDataQueueContent(&di->content);

		//free item
		free((void *)di);
		ret = TRUE;
	}

	//LEAVE CRITICAL SECTION
	LeaveCriticalSection(&dq->cs);

	return(ret);
}

//*** function returns total count of data inside data queue
DWORD D_GetDataQueueContentTotalCount(D_DHQUEUE hQ)
{
	D_DATAQUEUE *dq;
	D_DATAQUEUEITEM *di;
	DWORD total = 0;

	if (hQ) {
		dq = (D_DATAQUEUE *)hQ;

		//ENTER CRITICAL SECTION
		EnterCriticalSection(&dq->cs);

		if (dq->size && dq->front) {
			di = dq->front;
			while (di) {
				total += di->content.count;
				di = di->next;
			}
		}

		//LEAVE CRITICAL SECTION
		LeaveCriticalSection(&dq->cs);
	}

	return(total);
}

//*** function clears data queue item
BOOL D_ClearDataQueue(D_DHQUEUE hQ)
{
	D_DATAQUEUECONTENT dqc;

	if (hQ == NULL)
		return(FALSE);

	//pop all data
	while (D_PopDataQueueItem(hQ, &dqc)) {
		D_DiscardDataQueueContent(&dqc);
	}

	return(TRUE);
}



//------------------------------------------------------------
/* constants */
static const char *D_xmlstrDEVICES = "Devices";
static const char *D_xmlstrDEVICE = "Device";
static const char *D_xmlstrNAME = "Name";
static const char *D_xmlstrVERSION = "Version";
static const char *D_xmlstrLABEL = "Label";
static const char *D_xmlstrCOMMSETTINGS = "CommSettings";
static const char *D_xmlstrCMODE = "Mode";
static const char *D_xmlstrCCOM = "COM";
static const char *D_xmlstrCBAUDRATE = "Baudrate";
static const char *D_xmlstrCDATABITS = "Databits";
static const char *D_xmlstrCPARITY = "Parity";
static const char *D_xmlstrCSTOPBITS = "Stopbits";
static const char *D_xmlstrCHANDSHAKE = "Handshake";
static const char *D_xmlstrCIP = "IP";
static const char *D_xmlstrCPORT = "Port";
static const char *D_xmlstrCUSERID = "UserID";
static const char *D_xmlstrCTIMEOUT = "Timeout";
static const char *D_xmlstrCPARAM = "Param";
static const char *D_xmlstrMEASURESETTINGS = "MeasureSettings";
static const char *D_xmlstrINTERVAL = "Interval";
static const char *D_xmlstrINTERVAL2 = "Interval2";
static const char *D_xmlstrPROPERTIES = "Properties";
static const char *D_xmlstrPROPERTY = "Property";
static const char *D_xmlstrPNAME = "name";
static const char *D_xmlstrPTYPE = "type";
static const char *D_xmlstrPUNIT = "unit";
static const char *D_xmlstrPDISABLED = "disabled";
static const char *D_xmlstrPHIDDEN = "hidden";
static const char *D_xmlstrPIDX = "pidx";
static const char *D_xmlstrTRUE = "True";
static const char *D_xmlstrFALSE = "False";
static const char *D_xmlstrSIGNALS = "Signals";
static const char *D_xmlstrSIGNAL = "Signal";
static const char *D_xmlstrSCALE = "Scale";
static const char *D_xmlstrSCMY = "my";
static const char *D_xmlstrSCOY = "oy";


//*** function loads devices' XML configuration file
DWORD D_LoadRawDevices(const char *src_charset, XML_TAGLIST *list, HDEVICES *hDs)
{
	XML_TAG *rtag, *dtag, *itag, *ptag;
	DWORD ret = NO_ERROR, d, dcount, dwval;
	#define D_XMLTEXT_LEN 128
	char ttext[D_XMLTEXT_LEN], *pstr;
	TCHAR wtext[D_XMLTEXT_LEN], *pwstr;
	DEVPARAMS dparams;
	HDEVICE hD;
	D_DEVICE *pD;
	COMH *pC;
	WORD p;
	unsigned int size;
	unsigned int dver;

	if (list == NULL || hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	while (1) {
		//test format
		rtag = XML_GetRootByName(list, D_xmlstrDEVICES);
		if (rtag) {
			//clear devices
			D_RemoveAllDevices(hDs);

			//count devices
			dcount = XML_CountChildrenByName(rtag, D_xmlstrDEVICE);
			if (dcount) {
				//through devices
				dtag = XML_GetChildByName(rtag, D_xmlstrDEVICE);
				for (d=0; d<dcount && dtag; d++,dtag = XML_GetNextByName(dtag, D_xmlstrDEVICE)) {
					//get name
					itag = XML_GetChildByName(dtag, D_xmlstrNAME);
					if (XML_GetDataFromTagToBuffer(src_charset, itag, wtext, D_XMLTEXT_LEN) == 1) {
						dparams = D_SearchDeviceByName(wtext, NULL);
						if (dparams == NULL)
							continue;		//next device

						//create device
						hD = D_CreateDeviceByParams(dparams);
						if (hD == NULL)
							continue;		//next device
						//add device
						ret = D_AddNewDevice(hD, hDs);
						if (ret != NO_ERROR)
							break;		//error

						//--- load other parameters ---
						pD = (D_DEVICE *)hD;

						//load device label
						itag = XML_GetChildByName(dtag, D_xmlstrLABEL);
						if (XML_GetDataFromTagToBuffer(src_charset, itag, pD->label, D_MAX_DEVNAME_LEN) == -2) {
							ret = ERROR_INVALID_DATA;
							break;
						}
						if (*pD->label == '\0')
							D_SetDefaultLabel(hD);		//default label

						//load device driver version
						itag = XML_GetChildByName(dtag, D_xmlstrVERSION);
						if (XML_GetDataFromTagToBufferA(src_charset, itag, ttext, D_XMLTEXT_LEN) == 1) {
							dver = atoi(ttext) * 100;
							pstr = strchr(ttext, '.');
							dver += (pstr ? (atoi(pstr+1)%100) : 0);
						}
						else {
							dver = 100;		//1.00
						}


						pC = &pD->com;
						//subtag CommSettings
						itag = XML_GetChildByName(dtag, D_xmlstrCOMMSETTINGS);
						if (pC && itag) {
							//subsubtag Mode
							ptag = XML_GetChildByName(itag, D_xmlstrCMODE);
							if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
								pC->mode = atoi(ttext);
							}
							//check mode
							if ((pC->mode == COM_MODE_RS232 && !pC->com_flags.en_rs232) ||
									(pC->mode == COM_MODE_ETHERNET && !pC->com_flags.en_eth) ||
									(pC->mode == COM_MODE_USERID && !pC->com_flags.en_user)) {
								//invalid mode -> set valid
								if (pC->com_flags.en_rs232)
									pC->mode = COM_MODE_RS232;
								else if (pC->com_flags.en_eth)
									pC->mode = COM_MODE_ETHERNET;
								else if (pC->com_flags.en_user)
									pC->mode = COM_MODE_USERID;
								else
									pC->mode = COM_MODE_RS232;
							}

							//-- rs232
							if (pC->com_flags.en_rs232) {
								//subsubtag COM
								ptag = XML_GetChildByName(itag, D_xmlstrCCOM);
								if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
									pC->ser_pars.portno = atoi(ttext);
									if (pC->ser_pars.portno < D_MIN_COMPORT)
										pC->ser_pars.portno = D_MIN_COMPORT;
									else if (pC->ser_pars.portno > D_MAX_COMPORT)
										pC->ser_pars.portno = D_MAX_COMPORT;

								}
								//subsubtag Baudrate
								ptag = XML_GetChildByName(itag, D_xmlstrCBAUDRATE);
								if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
									dwval = atoi(ttext);
									if (RS232_ValidateBaudrate(pC->ser_flags.en_baudrate, dwval))
										pC->ser_pars.baudrate = dwval;
								}
								//subsubtag Databits
								ptag = XML_GetChildByName(itag, D_xmlstrCDATABITS);
								if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
									dwval = atoi(ttext);
									if (RS232_ValidateDatabits(pC->ser_flags.en_databits, dwval))
										pC->ser_pars.databits = dwval;
								}
								//subsubtag Parity
								ptag = XML_GetChildByName(itag, D_xmlstrCPARITY);
								if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
									dwval = atoi(ttext);
									if (RS232_ValidateParity(pC->ser_flags.en_parity, dwval))
										pC->ser_pars.parity = dwval;
								}
								//subsubtag Stopbits
								ptag = XML_GetChildByName(itag, D_xmlstrCSTOPBITS);
								if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
									dwval = atoi(ttext);
									if (RS232_ValidateStopbits(pC->ser_flags.en_stopbits, dwval))
										pC->ser_pars.stopbits = dwval;
								}
								//subsubtag Handshake
								ptag = XML_GetChildByName(itag, D_xmlstrCHANDSHAKE);
								if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
									dwval = atoi(ttext);
									if (RS232_ValidateHandshake(pC->ser_flags.en_handshake, dwval))
										pC->ser_pars.handshake = dwval;
								}
							}
							//-- ethernet
							if (pC->com_flags.en_eth) {
								//subsubtag IP
								ptag = XML_GetChildByName(itag, D_xmlstrCIP);
								XML_GetDataFromTagToBuffer(src_charset, ptag, pC->eth_pars.ip, COM_MAX_IP_LEN);
								//subsubtag Port
								ptag = XML_GetChildByName(itag, D_xmlstrCPORT);
								if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
									pC->eth_pars.port = atoi(ttext);
								}
							}
							//-- user-id
							if (pC->com_flags.en_user) {
								//subsubtag UserID
								ptag = XML_GetChildByName(itag, D_xmlstrCUSERID);
								XML_GetDataFromTagToBuffer(src_charset, ptag, pC->userid, COM_MAX_USERID_LEN);
							}
							//---
							//subsubtag Timeout
							ptag = XML_GetChildByName(itag, D_xmlstrCTIMEOUT);
							if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
								pC->timeout = atoi(ttext);
								if (pC->timeout < 100)
									pC->timeout = 100;		//min. 100 ms
								else if (pC->timeout > 30000)
									pC->timeout = 30000;		//max. 30 s
							}
							//subsubtag Param
							ptag = XML_GetChildByName(itag, D_xmlstrCPARAM);
							if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
								pC->lparam = atoi(ttext);
							}

							//--- make comm. ID string
							D_MakeCommIDString(hD);
						}

						//subtag Measure Settings
						itag = XML_GetChildByName(dtag, D_xmlstrMEASURESETTINGS);
						if (pD && itag) {
							//subsubtag Interval
							ptag = XML_GetChildByName(itag, D_xmlstrINTERVAL);
							if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
								pD->tdelay = atoi(ttext);
								if (pD->tdelay < pD->tmindelay)
									pD->tdelay = pD->tmindelay;		//min. interval
								else if (pD->tdelay > 600000)
									pD->tdelay = 600000;
							}
							//subsubtag Interval2
							ptag = XML_GetChildByName(itag, D_xmlstrINTERVAL2);
							if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
								pD->tdelay2 = atoi(ttext);
								if (pD->tdelay2 < pD->tmindelay)
									pD->tdelay2 = pD->tmindelay;		//min. interval
								else if (pD->tdelay2 > 600000)
									pD->tdelay2 = 600000;
							}
						}

						//subtag Properties
						itag = XML_GetChildByName(dtag, D_xmlstrPROPERTIES);
						if (pD->n_p && pD->p && itag) {
							//through properties
							for (p=0; p<pD->n_p; p++) {
								if ((pD->p[p].flags & DFLAG_CONF) != DFLAG_CONF)
									continue;		//next property

								//search conf. property by attribute name
								lstrcpy(wtext, pD->p[p].label);		//label+suffix
								if (*pD->p[p].suffix)
									lstrcat(wtext, pD->p[p].suffix);
								ptag = XML_GetChildByAttribNameAndValue(src_charset, itag, D_xmlstrPROPERTY,
																												D_xmlstrPNAME, wtext);
								if (pD->p[p].type == DT_STRING) {		//is string
									//load string
									if (XML_GetDataFromTag(src_charset, ptag, &pwstr) == 1) {
										if ((pD->p[p].flags & DFLAG_MEM)) {
											//copy content only
											if (pD->p[p].val.m.size && pD->p[p].val.m.buf) {
												//min. length
												size = sizeof(TCHAR)*(lstrlen(pwstr)+1);
												if (pD->p[p].val.m.size < size)
													size = pD->p[p].val.m.size;
												//copy
												memcpy(pD->p[p].val.m.buf, pwstr, size);
											}
											free((void *)pwstr);
										}
										else {
											//deallocation
											if (pD->p[p].val.m.buf)
												free((void *)pD->p[p].val.m.buf);
											//point new string
											pD->p[p].val.m.size = sizeof(TCHAR)*(lstrlen(pwstr)+1);
											pD->p[p].val.m.buf = (unsigned char *)pwstr;
										}
									}
								}
								else if (pD->p[p].type == DT_MEMORY) {		//is memory
									//load memory
									if (XML_GetDataFromTagA(src_charset, ptag, &pstr) == 1) {
										if ((pD->p[p].flags & DFLAG_MEM)) {
											//copy content only
											if (pD->p[p].val.m.size && pD->p[p].val.m.buf)
												ret = XML_GetMemTextIntoBuf(pstr, pD->p[p].val.m.size, pD->p[p].val.m.buf);
											else
												ret = NO_ERROR;		//nothing to load
										}
										else {
											//deallocation
											if (pD->p[p].val.m.buf)
												free((void *)pD->p[p].val.m.buf);
											pD->p[p].val.m.size = 0;
											pD->p[p].val.m.buf = NULL;
											//allocation
											ret = XML_GetMemText(pstr, &pD->p[p].val.m.size, &pD->p[p].val.m.buf);
										}
										free((void *)pstr);		//free memory
										if (ret != NO_ERROR)
											return(ret);
									}
								}
								else {		//is other
									if (XML_GetDataFromTagToBufferA(src_charset, ptag, ttext, D_XMLTEXT_LEN) == 1) {
										//load value
										switch (pD->p[p].type) {
											case DT_BOOL:
												pD->p[p].val.b = (strcmp(ttext, D_xmlstrTRUE) == 0 ? TRUE : FALSE);
												break;
											case DT_INT:
												pD->p[p].val.i = atoi(ttext);
												break;
											default:
											case DT_UINT:
												pD->p[p].val.u = atoi(ttext);
												break;
											case DT_DOUBLE:
												pD->p[p].val.d = astr2dbl(ttext);
												break;
											case DT_INT64:
												pD->p[p].val.i64 = _atoi64(ttext);
												break;
											case DT_UINT64:
												pD->p[p].val.u64 = _atoi64(ttext);
												break;
											case DT_HANDLE:
												pD->p[p].val.h = (HANDLE)HexStringNumToIntA(ttext);		//???
												break;
										}
									}
								}
							}
						}

						//subtag Signals
						itag = XML_GetChildByName(dtag, D_xmlstrSIGNALS);
						if (pD->n_s && pD->s && itag) {
							XML_TAG *sc_tag;
							XML_APAIR *sc_pair;
							//D_DEV_SCALE sc;
							//through signals
							for (p=0; p<pD->n_s; p++) {
								sprintf(ttext, "%u", pD->s[p].pidx);
								ptag = XML_GetChildByAttribNameAndValueA(src_charset, itag, D_xmlstrSIGNAL, D_xmlstrPIDX, ttext);
								if (ptag) {
									//rename
									XML_GetDataFromTagToBuffer(src_charset, ptag, pD->s[p].rename, D_MAX_SIGNALNAME_LEN);
									//disabled
									if (XML_GetDataFromAttribToBufferA(src_charset,
																										 XML_GetAttribByName(ptag, D_xmlstrPDISABLED),
																										 ttext, D_XMLTEXT_LEN) == 1) {
										 pD->s[p].disabled = (strcmp(ttext, D_xmlstrTRUE) == 0 ? TRUE : FALSE);
									}
									//hidden
									if (XML_GetDataFromAttribToBufferA(src_charset,
																										 XML_GetAttribByName(ptag, D_xmlstrPHIDDEN),
																										 ttext, D_XMLTEXT_LEN) == 1) {
										 pD->s[p].hidden = (strcmp(ttext, D_xmlstrTRUE) == 0 ? TRUE : FALSE);
									}
									//scale
									sc_tag = XML_GetChildByName(ptag, D_xmlstrSCALE);
									if (sc_tag) {
										D_DEV_SCALE sc = {1.0, 0.0};
										//my
										sc_pair = XML_GetAttribByName(sc_tag, D_xmlstrSCMY);
										if (sc_pair) {
											sc.my = atof(sc_pair->value);
										}
										//oy
										sc_pair = XML_GetAttribByName(sc_tag, D_xmlstrSCOY);
										if (sc_pair) {
											sc.oy = atof(sc_pair->value);
										}
										//check
										if (sc.my != 0.0) {
											pD->s[p].sc = sc;
										}
									}

								}
							}
							//correct signals ???
							/*
							if (pD->dis_meas_int)
								pD->s[0].disabled = 0;		//enable 1st
							*/
						}

						//------ check device for backward compatibility
						D_CheckLoadedDevice(hD, dver);

					}
				}

				//update sys-devices
				D_SYSD_ListSignalsTo(hDs);

				//validate devices
				if (!D_ValidateDevices(hDs)) {
					//clear devices
					D_RemoveAllDevices(hDs);
					ret = ERROR_BAD_FORMAT;
				}

				if (ret != NO_ERROR)
					break;

			}
		}

		break;
	}

	return(ret);
}

//*** function saves devices' XML configuration file
DWORD D_SaveRawDevices(const char *trg_charset, XML_TAGLIST *list, HDEVICES *hDs)
{
	XML_TAG *rtag, *dtag, *itag, *ptag;
	DWORD ret, d;
	char ttext[128], *pstr;
	TCHAR wtext[128];
	DEVICEINFO dinfo;
	D_DEVICE *pD;
	COMH *pC;
	WORD p;

	if (list == NULL || hDs == NULL)
		return(ERROR_INVALID_HANDLE);

	while (1) {
		//--- create XML structure ---
		//root tag "Devices"
		ret = XML_AddDataTagA(trg_charset, list, &rtag, D_xmlstrDEVICES, NULL);
		if (ret != NO_ERROR)
			break;

		//through devices
		if (hDs->dcount && hDs->hd) {
			for (d=0; d<hDs->dcount; d++) {
				if (hDs->hd[d] == NULL || !D_GetDeviceInfo(hDs->hd[d], &dinfo))
					continue;
				pD = (D_DEVICE *)hDs->hd[d];

				//tag "Device"
				ret = XML_AddDataTagA(trg_charset, &rtag->list, &dtag, D_xmlstrDEVICE, NULL);
				if (ret != NO_ERROR)
					break;
				//subtag "Name"
				ret = XML_AddDataTag(trg_charset, &dtag->list, NULL, D_xmlstrNAME, dinfo.name);
				if (ret != NO_ERROR)
					break;
				//subtag "Version"
				ret = XML_AddDataTag(trg_charset, &dtag->list, NULL, D_xmlstrVERSION, dinfo.ver);
				if (ret != NO_ERROR)
					break;
				//subtag "Label"
				ret = XML_AddDataTag(trg_charset, &dtag->list, NULL, D_xmlstrLABEL, pD->label);
				if (ret != NO_ERROR)
					break;

				//--- communication settings ---
				pC = &(pD->com);
				if (pC) {
					//subtag "CommSettings"
					ret = XML_AddDataTagA(trg_charset, &dtag->list, &itag, D_xmlstrCOMMSETTINGS, NULL);
					if (ret != NO_ERROR)
						break;
					//subtag "CommSettings" subsub tag "Mode"
					sprintf(ttext, "%d", pC->mode);
					ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCMODE, ttext);
					if (ret != NO_ERROR)
						break;
					//-- rs232
					if (pC->com_flags.en_rs232) {
						//subtag "CommSettings" subsub tag "COM"
						sprintf(ttext, "%d", pC->ser_pars.portno);
						ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCCOM, ttext);
						if (ret != NO_ERROR)
							break;
						//subtag "CommSettings" subsub tag "Baudrate"
						sprintf(ttext, "%u", pC->ser_pars.baudrate);
						ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCBAUDRATE, ttext);
						if (ret != NO_ERROR)
							break;
						//subtag "CommSettings" subsub tag "Databits"
						sprintf(ttext, "%d", pC->ser_pars.databits);
						ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCDATABITS, ttext);
						if (ret != NO_ERROR)
							break;
						//subtag "CommSettings" subsub tag "Parity"
						sprintf(ttext, "%d", pC->ser_pars.parity);
						ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCPARITY, ttext);
						if (ret != NO_ERROR)
							break;
						//subtag "CommSettings" subsub tag "Stopbits"
						sprintf(ttext, "%d", pC->ser_pars.stopbits);
						ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCSTOPBITS, ttext);
						if (ret != NO_ERROR)
							break;
						//subtag "CommSettings" subsub tag "Handshake"
						sprintf(ttext, "%d", pC->ser_pars.handshake);
						ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCHANDSHAKE, ttext);
						if (ret != NO_ERROR)
							break;
					}
					//-- ethernet
					if (pC->com_flags.en_eth) {
						//subtag "CommSettings" subsub tag "IP"
						ret = XML_AddDataTag(trg_charset, &itag->list, NULL, D_xmlstrCIP, pC->eth_pars.ip);
						if (ret != NO_ERROR)
							break;
						//subtag "CommSettings" subsub tag "Port"
						sprintf(ttext, "%d", pC->eth_pars.port);
						ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCPORT, ttext);
						if (ret != NO_ERROR)
							break;
					}
					//-- user-id
					if (pC->com_flags.en_user) {
						//subtag "CommSettings" subsub tag "UserID"
						ret = XML_AddDataTag(trg_charset, &itag->list, NULL, D_xmlstrCUSERID, pC->userid);
						if (ret != NO_ERROR)
							break;
					}
					//--
					//subtag "CommSettings" subsub tag "Timeout"
					sprintf(ttext, "%u", pC->timeout);
					ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCTIMEOUT, ttext);
					if (ret != NO_ERROR)
						break;
					//subtag "CommSettings" subsub tag "Param"
					sprintf(ttext, "%lu", pC->lparam);
					ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrCPARAM, ttext);
					if (ret != NO_ERROR)
						break;
				}

				//--- measure settings ---
				if (pD) {
					//subtag "MeasureSettings"
					ret = XML_AddDataTagA(trg_charset, &dtag->list, &itag, D_xmlstrMEASURESETTINGS, NULL);
					if (ret != NO_ERROR)
						break;
					//subtag "MeasureSetting" subsub tag "Interval"
					sprintf(ttext, "%ld", pD->tdelay);
					ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrINTERVAL, ttext);
					if (ret != NO_ERROR)
						break;
					//subtag "MeasureSetting" subsub tag "Interval2"
					sprintf(ttext, "%ld", pD->tdelay2);
					ret = XML_AddDataTagA(trg_charset, &itag->list, NULL, D_xmlstrINTERVAL2, ttext);
					if (ret != NO_ERROR)
						break;
				}

				//--- properties ---
				//subtag "Properties"
				ret = XML_AddDataTagA(trg_charset, &dtag->list, &itag, D_xmlstrPROPERTIES, NULL);
				if (ret != NO_ERROR)
					break;

				if (pD->n_p && pD->p) {
					//through properties
					for (p=0; p<pD->n_p; p++) {
						if ((pD->p[p].flags & DFLAG_CONF) != DFLAG_CONF)
							continue;		//not configuration property

						//subsub tag "Property" (+value)
						if (pD->p[p].type == DT_MEMORY) {
							//data --> "<ascii hex data>" (2x size)
							ret = XML_CreateMemText(pD->p[p].val.m.buf, pD->p[p].val.m.size, &pstr);
							if (ret != NO_ERROR)
								break;
							ret = XML_AddDataTagA(trg_charset, &itag->list, &ptag, D_xmlstrPROPERTY, pstr);
							free((void *)pstr);
						}
						else if (pD->p[p].type == DT_STRING) {
							ret = XML_AddDataTag(trg_charset, &itag->list, &ptag, D_xmlstrPROPERTY, (TCHAR *)pD->p[p].val.m.buf);
						}
						else {
							switch (pD->p[p].type) {
								case DT_BOOL: strcpy(ttext, pD->p[p].val.b ? D_xmlstrTRUE : D_xmlstrFALSE); break;
								case DT_INT: sprintf(ttext, "%d", pD->p[p].val.i); break;
								default:
								case DT_UINT: sprintf(ttext, "%u", pD->p[p].val.u); break;
								case DT_DOUBLE: sprintf(ttext, "%g", pD->p[p].val.d); break;
								case DT_INT64: sprintf(ttext, "%I64d", pD->p[p].val.i64); break;
								case DT_UINT64: sprintf(ttext, "%I64u", pD->p[p].val.u64); break;
								case DT_HANDLE: sprintf(ttext, "%08X", (unsigned)pD->p[p].val.h); break;
							}
							ret = XML_AddDataTagA(trg_charset, &itag->list, &ptag, D_xmlstrPROPERTY, ttext);
						}
						if (ret != NO_ERROR)
							break;

						//property pidx pair (informatively)
						sprintf(ttext, "%u", p);
						ret = XML_AddDataAttribA(trg_charset, &ptag->alist, D_xmlstrPIDX, ttext);
						if (ret != NO_ERROR)
							break;
						//property name pair
						lstrcpy(wtext, pD->p[p].label);		//label+suffix
						if (*pD->p[p].suffix)
							lstrcat(wtext, pD->p[p].suffix);
						ret = XML_AddDataAttrib(trg_charset, &ptag->alist, D_xmlstrPNAME, wtext);
						if (ret != NO_ERROR)
							break;
						//property type pair
						sprintf(ttext, "%d", pD->p[p].type);
						ret = XML_AddDataAttribA(trg_charset, &ptag->alist, D_xmlstrPTYPE, ttext);
						if (ret != NO_ERROR)
							break;
						//property unit pair
						if (pD->p[p].unit.u && *pD->p[p].unit.u) {
							ret = XML_AddDataAttrib(trg_charset, &ptag->alist, D_xmlstrPUNIT, pD->p[p].unit.u);
							if (ret != NO_ERROR)
								break;
						}

					}
					if (ret != NO_ERROR)
						break;
				}

				//--- signals ---
				//subtag "Signals"
				ret = XML_AddDataTagA(trg_charset, &dtag->list, &itag, D_xmlstrSIGNALS, NULL);
				if (ret != NO_ERROR)
					break;

				if (pD->n_s && pD->s) {
					//through signals
					for (p=0; p<pD->n_s; p++) {
						ret = XML_AddDataTag(trg_charset, &itag->list, &ptag, D_xmlstrSIGNAL, pD->s[p].rename);
						if (ret != NO_ERROR)
							break;

						//signal name pair (infomatively)
						ret = XML_AddDataAttrib(trg_charset, &ptag->alist, D_xmlstrPNAME, pD->s[p].name);
						if (ret != NO_ERROR)
							break;
						//signal pidx pair
						sprintf(ttext, "%u", pD->s[p].pidx);
						ret = XML_AddDataAttribA(trg_charset, &ptag->alist, D_xmlstrPIDX, ttext);
						if (ret != NO_ERROR)
							break;
						//signal disabled pair
						strcpy(ttext, pD->s[p].disabled ? D_xmlstrTRUE : D_xmlstrFALSE);
						ret = XML_AddDataAttribA(trg_charset, &ptag->alist, D_xmlstrPDISABLED, ttext);
						if (ret != NO_ERROR)
							break;
						//signal hidden pair
						strcpy(ttext, pD->s[p].hidden ? D_xmlstrTRUE : D_xmlstrFALSE);
						ret = XML_AddDataAttribA(trg_charset, &ptag->alist, D_xmlstrPHIDDEN, ttext);
						if (ret != NO_ERROR)
							break;
						//signal scale
						if (pD->s[p].sc.my != 1.0 || pD->s[p].sc.oy != 0.0) {
							XML_TAG *sc_tag;
							ret = XML_AddDataTag(trg_charset, &ptag->list, &sc_tag, D_xmlstrSCALE, NULL);
							if (ret != NO_ERROR)
								break;
							//signal scale my pair
							sprintf(ttext, "%.8g", pD->s[p].sc.my);
							ret = XML_AddDataAttribA(trg_charset, &sc_tag->alist, D_xmlstrSCMY, ttext);
							if (ret != NO_ERROR)
								break;
							//signal scale oy pair
							sprintf(ttext, "%.8g", pD->s[p].sc.oy);
							ret = XML_AddDataAttribA(trg_charset, &sc_tag->alist, D_xmlstrSCOY, ttext);
							if (ret != NO_ERROR)
								break;
						}

					}
					if (ret != NO_ERROR)
						break;
				}



			}
		}
		if (ret != NO_ERROR)
			break;		//error

		break;
	}

	return(ret);
}

//---------------------- UNITS ---------------------------

/** @brief Set default units
 *
 * @param punits Pointer to demanded unit structure
 * @return BOOL
 *
 */
BOOL D_SetDefaultUnits(CHUI_UNITS *punits)
{
	if (punits == NULL)
		return(FALSE);

	g_dev_units = *punits;
	return(TRUE);
}

/** @brief Set unit conversion for all device properties
 *
 * @param hD Device handle
 * @param punits Pointer to demanded unit structure
 * @return BOOL
 *
 */
BOOL D_SetUnitConversion(HDEVICE hD, CHUI_UNITS *punits)
{
	D_DEVICE *pD;
	int p;

	if (hD == NULL)
		return(FALSE);
	pD = (D_DEVICE *)hD;

	//through properties
	if (pD->n_p && pD->p) {
		for (p=0; p<pD->n_p; p++) {
			D_SetPropUnitConversion(&pD->p[p], punits);
		}
	}

	return(TRUE);
}

/** @brief Set property unit conversion from origin unit to demand unit
 *
 * @param prop Pointer to property structure
 * @param punits Pointer to demanded unit structure
 * @return Result (TRUE=ok, FALSE=failed)
 *
 */
BOOL D_SetPropUnitConversion(D_DEV_PROP *prop, CHUI_UNITS *punits)
{
	CHUI_UTYPE utype = CHUI_UTYPE_ORIGIN;

	if (prop == NULL)
		return(FALSE);

	//convert only double!!!
	if ((prop->flags & DFLAG_NOCONVUNIT) == 0 && prop->type == DT_DOUBLE) {
		//search for known property (search for type)
		utype = CHUI_GetTypeByQuantityName(prop->unit.q);
	}

	//set unit
	CHUI_SetUnitConversion(utype, prop->unit.u, punits, &prop->_uconv);

	return(TRUE);
}

/** @brief Convert property value to demanded unit value
 *
 * @param prop Pointer to property structure
 * @return Succesful conversion=true (otherwise value is not changed)
 *
 */
BOOL D_ConvertValueToDemandUnit(D_DEV_PROP *prop, double *dval)
{

	if (prop == NULL || prop->_uconv.use == 0  || dval == NULL)
		return(FALSE);

	//convert to demand
	switch (prop->type) {
/*
		case DT_INT:
			nval->i = (int)round((double)nval->i * prop->_uconv.scale + prop->_uconv.offset);
			return(TRUE);
		case DT_UINT:
			nval->u = (unsigned int)round((double)nval->u * prop->_uconv.scale + prop->_uconv.offset);
			return(TRUE);
*/
		case DT_DOUBLE:
			*dval = *dval * prop->_uconv.scale + prop->_uconv.offset;
			return(TRUE);
		default:
			return(FALSE);
	}
}

/** @brief Convert property value to origin unit value
 *
 * @param prop Pointer to property structure
 * @return Succesful conversion=true (otherwise value is not changed)
 *
 */
BOOL D_ConvertValueToOriginUnit(D_DEV_PROP *prop, double *dval)
{

	if (prop == NULL || prop->_uconv.use == 0 || dval == NULL)
		return(FALSE);

	//convert to origin
	switch (prop->type) {
/*
		case DT_INT:
			nval->i = (int)round(((double)nval->i - prop->_uconv.offset) / prop->_uconv.scale);
			return(TRUE);
		case DT_UINT:
			nval->u = (unsigned int)round(((double)nval->u - prop->_uconv.offset) / prop->_uconv.scale);
			return(TRUE);
*/
		case DT_DOUBLE:
			*dval = (*dval - prop->_uconv.offset) / prop->_uconv.scale;
			return(TRUE);
		default:
			return(FALSE);
	}
}

/** @brief Convert precision by scale
 *
 * @param prop D_DEV_PROP*
 * @param precision int*
 * @return BOOL
 *
 */
BOOL D_ConvertUnitDemandPrecision(D_DEV_PROP *prop, int *precision)
{
	if (prop == NULL || prop->_uconv.use == 0 || precision == NULL)
		return(FALSE);

	*precision += prop->_uconv.precadd;
	if (*precision < 0)
		*precision = 0;
	else if (*precision  > 5)
		*precision = 5;

	return(TRUE);
}

/** @brief Return property unit label (using unit conversion)
 *
 * @param prop Pointer to property structure
 * @return Unit string pointer (can be null)
 *
 */
TCHAR *D_GetUnitLabel(D_DEV_PROP *prop)
{
	TCHAR *punit = NULL;

	if (prop) {
		if (prop->_uconv.use)
			punit = (TCHAR *)prop->_uconv.unit;
		else
			punit = prop->unit.u;
	}
		return(punit);
}

//---------------------- GUI -----------------------------

//--------------------------------------------------------
//*** function creates label for property (+ units)
//format: 0-label+suffix, 1-unit, 2-semicolon
BOOL D_MakePropLabel(D_DEV_PROP *prop, TCHAR *str, unsigned format)
{
	TCHAR *pt;

	if (prop && str) {

		//label
		__strcpy(str, *prop->label ? D_ram(prop->label) : cstr_empty);
		//suffix
		if (*prop->suffix)
			__strcat(str, prop->suffix);
		//unit
		if ((format & 0x1)) {
			pt = D_GetUnitLabel(prop);
			if (pt && *pt) {
				__strcat(str, TEXT(" ["));
				__strcat(str, pt);
				__strcat(str, TEXT("]"));
			}
		}
		//semicolon
		if ((format & 0x2)) {
			__strcat(str, TEXT(":"));
		}
		return TRUE;
	}
	return FALSE;
}

//*** function creates label for oper. property
//format: 0-label+suffix
BOOL D_MakeOperPropLabel(D_DEV_OPERPROP *oprop, TCHAR *str, unsigned format)
{
	if (oprop && str) {

		//label
		__strcpy(str, *oprop->label ? D_ram(oprop->label) : cstr_empty);
		//suffix
		if (*oprop->suffix)
			__strcat(str, oprop->suffix);
		return TRUE;
	}
	return FALSE;
}

//*** function tests & sets user data of the control
BOOL D_TestUserData(HWND hctl, LONG newp)
{
	if (hctl) {
		if (GetWindowLong(hctl, GWLP_USERDATA) != newp) {
			SetWindowLong(hctl, GWLP_USERDATA, newp);
			RedrawWindow(hctl, NULL, NULL, RDW_INVALIDATE);		//update now
			return TRUE;
		}
	}
	return FALSE;
}

//----------------------------------
//*** function processes child window enumeration
BOOL CALLBACK D_EnableEnumChildProc(HWND hwnd, LPARAM lparam)
{
	TCHAR ttext[128];

	if (hwnd) {
		if (GetClassName(hwnd, ttext, 128) > 0 &&
				lstrcmpi(ttext, CLASS_STATIC) != 0 &&
				GetProp(hwnd, STR_READONLY) == NULL &&
				GetProp(hwnd, D_STR_PROPENABLE) == NULL) {
			EnableWindow(hwnd, (BOOL)lparam);
		}
	}
	return TRUE;
}


//*** function processes rest of dialog messages
BOOL D_RestDevDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam, D_DEVICE *pD)
{
	static HBRUSH sbrush = NULL;
	static D_VAL dval;
	unsigned int i, j;

	switch(msg) {
		case WM_INITDIALOG:

      //scale window
      ScaleChildWindowsByScreen(hdlg);

			if (pD && pD->p) {
        //start timer
        pD->counter = 0;

        //update used parameters
        for (i=0; i<pD->n_p; i++) {
          if (D_GETOLH_VAL(hdlg, i) || pD->p[i].guitype == DGUIT_RADIOSW) {
            //control exist -> update
            SendMessage(hdlg, D_OL_WM_UPDATE, i, pD->p[i].lerr);
          }
        }
      }
			return TRUE;

		case WMD_ENABLE:
			if ((BOOL)lparam) {
				//enable
				KillTimer(hdlg, 2);
				if (pD) {
#ifdef D_SIMUL_ALL
					SetTimer(hdlg, 1, 1000, NULL);
#else
					SetTimer(hdlg, 1, pD->utimer1, NULL);
#endif
				}
				EnumChildWindows(hdlg, D_EnableEnumChildProc, TRUE);
			}
			else {
				//disable
				KillTimer(hdlg, 1);
				if (pD) {
#ifdef D_SIMUL_ALL
					SetTimer(hdlg, 2, 2000, NULL);
#else
					SetTimer(hdlg, 2, pD->utimer2, NULL);
#endif
				}
				EnumChildWindows(hdlg, D_EnableEnumChildProc, FALSE);
			}
			return TRUE;

		case WMD_MAIN:
		case WMD_APPLY:
		case WMD_START:
		case WMD_STOP:
		case WMD_PAUSE:
		case WMD_RESUME:
		case WMD_SYNC:
		case WMD_SPECIAL:
			return TRUE;

		case D_OL_WM_UPDATE:
			//update value
			if (pD && pD->p && wparam < pD->n_p) {
				D_SetOnLineControl(hdlg, wparam, pD, lparam);
			}
			return TRUE;

		case WM_COMMAND:
			if (lparam && pD) {
				i = LOWORD(wparam);

				//CLICKED
				if (HIWORD(wparam) == BN_CLICKED) {
					//oper. property
					if (i == D_OL_IDOBT) {
						i = SendMessage(GetDlgItem(hdlg, D_OL_IDOCB), CB_GETCURSEL, 0, 0);
						if (i != CB_ERR)
							D_MakeOperFunction((HDEVICE)pD, pD->ufce.pSetOper, i, GetParent(hdlg), WMD_RESPONSE);
						return TRUE;
					}
					//property
					else if (pD->p) {
						//ON button
						if (i >= D_OL_IDON && ((j = i-D_OL_IDON) < pD->n_p) && (pD->p[j].guitype == DGUIT_SWITCH || pD->p[j].guitype == DGUIT_RADIOSW)) {
							dval.b = TRUE;
							D_WriteFunctionPrior((HDEVICE)pD, pD->ufce.pWriteProp, j, dval, GetParent(hdlg), WMD_RESPONSE, MQM_PRIORITY_HIGHER);
							D_ReadFunctionPrior((HDEVICE)pD, pD->ufce.pReadProp, j, hdlg, D_OL_WM_UPDATE, MQM_PRIORITY_HIGHER);
							return TRUE;
						}
						//OFF button
						else if (i >= D_OL_IDOFF && ((j = i-D_OL_IDOFF) < pD->n_p) && (pD->p[j].guitype == DGUIT_SWITCH || pD->p[j].guitype == DGUIT_RADIOSW)) {
							dval.b = FALSE;
							D_WriteFunction((HDEVICE)pD, pD->ufce.pWriteProp, j, dval, GetParent(hdlg), WMD_RESPONSE);
							D_ReadFunction((HDEVICE)pD, pD->ufce.pReadProp, j, hdlg, D_OL_WM_UPDATE);
							return TRUE;
						}
					}
				}

				//CB SELCHANGED
				else if (HIWORD(wparam) == CBN_SELCHANGE) {
					if (i < pD->n_p && pD->p && pD->p[i].guitype == DGUIT_COMBOBOX) {
						dval.u = (unsigned int)SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
						D_WriteFunction((HDEVICE)pD, pD->ufce.pWriteProp, i, dval, GetParent(hdlg), WMD_RESPONSE);
						D_ReadFunction((HDEVICE)pD, pD->ufce.pReadProp, i, hdlg, D_OL_WM_UPDATE);
						return TRUE;
					}
				}

				//EDITBOX KILLFOCUS
				else if (HIWORD(wparam) == EN_KILLFOCUS) {
					if (i < pD->n_p && pD->p && (pD->p[i].flags & DFLAG_CONF) == DFLAG_CONF) {
						if (D_TestOnLineEBControl((HWND)lparam, &pD->p[i], &dval)) {
							D_WriteFunction((HDEVICE)pD, pD->ufce.pWriteProp, i, dval, GetParent(hdlg), WMD_RESPONSE);
							D_ReadFunction((HDEVICE)pD, pD->ufce.pReadProp, i, hdlg, D_OL_WM_UPDATE);
							return TRUE;
						}
					}
				}
      }
			break;

		case WM_NOTIFY:
			if (lparam && ((NMHDR *)lparam)->code == UDN_DELTAPOS) {
				i = ((NMHDR *)lparam)->idFrom;
				if (i >= D_OL_IDUD && ((j = i-D_OL_IDUD) < pD->n_p) && pD->p[j].guitype == DGUIT_SPINVAL) {
					D_NotifyOnlineUDControl(hdlg, j, pD, ((NMUPDOWN *)lparam)->iDelta, 1.0);
					return TRUE;
				}
			}
			break;

		case WM_KILLFOCUS:
			break;


		case WM_TIMER:
			break;

		case WM_CTLCOLORBTN:
			return((BOOL)SetTransparency((HDC)wparam));

		case WM_CTLCOLOREDIT:
			//text color
			if (GetWindowLong((HWND)lparam, GWLP_USERDATA) != 0)
				SetTextColor((HDC)wparam, D_COLOR_GRAY);
			else if (GetProp((HWND)lparam, STR_EMPHASIS) != NULL)
				SetTextColor((HDC)wparam, D_COLOR_EMPHASIZED);

			//background
			if (pD && GetProp((HWND)lparam, STR_READONLY) != NULL) {
				SetBackgroundColor((HDC)wparam,
													 MixTwoColors(D_ModelColor(pD->model), D_COLOR_WHITE, D_BGCOLORRATION),
													 &sbrush);
			}
			else
				SetBackgroundColor((HDC)wparam, D_COLOR_WHITE, &sbrush);
			return((BOOL)sbrush);
			break;

		case WM_CTLCOLORSTATIC:
			//return((BOOL)SetTransparency((HDC)wparam));
			if (pD != NULL) {
				SetBackgroundColor((HDC)wparam, D_ModelColor(pD->model), &sbrush);
				return((BOOL)sbrush);
			}
			break;

		case WM_CTLCOLORDLG:
			if (pD != NULL) {
				SetBackgroundColor((HDC)wparam, D_ModelColor(pD->model), &sbrush);
				return((BOOL)sbrush);
			}
			break;

		case WM_PAINT:
			//ED_DrawPropLines(hdlg, eD);
			//return TRUE;
			break;

		case WM_CLOSE:
			return TRUE;

		case WM_DESTROY:
			KillTimer(hdlg, 1);

			//destroy controls -> through properties
			if (pD && pD->p) {
				for (i=0; i<pD->n_p; i++) {
					D_DestroyOnLineCtrl(hdlg, i, pD);
				}
			}

			if (sbrush)
				DeleteObject(sbrush);
			sbrush = NULL;
			break;
	}

	return FALSE;
}

//*** function processes rest of setup dialog messages
//note: using for: pSetupParamDlgProc(), pSetupInitDlgProc()
BOOL D_RestSetupDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam, D_DEVICE *pD)
{
	switch(msg) {
		case WM_INITDIALOG:

      //scale window
			ScaleChildWindowsByScreen(hdlg);
			return TRUE;

		case WMD_ENABLE:
			return TRUE;

		case WMD_UPDATE:
      D_SetAllParamCtrls(hdlg, pD);
      return TRUE;

		case WMD_APPLY:
      D_GetAllParamCtrls(hdlg, pD);
      return TRUE;

		case WM_COMMAND:
			if (pD && lparam) {
				//range
				if (HIWORD(wparam) == EN_KILLFOCUS) {
					if (LOWORD(wparam) < pD->n_p) {
						//validate range controls
						D_CheckValidateParamRangeCtrl(hdlg, 0, LOWORD(wparam), pD);
						return TRUE;
					}
					else if (LOWORD(wparam) > D_OL_IDON && (LOWORD(wparam)-D_OL_IDON) < pD->n_p) {
						D_CheckValidateParamRangeCtrl(hdlg, 1, LOWORD(wparam)-D_OL_IDON, pD);
						return TRUE;
					}
				}

			}
			break;

		case WM_MEASUREITEM:
			{
				MEASUREITEMSTRUCT *pmeas = (MEASUREITEMSTRUCT *)lparam;
				if (pmeas) {
					HDC hdc = GetDC(hdlg);
					if (hdc) {
						pmeas->itemHeight = (16 * GetDeviceCaps(hdc, LOGPIXELSY))/96;
						ReleaseDC(hdlg, hdc);
					}
				}
			}
			break;
		case WM_DRAWITEM:
			DrawComboboxIcon(hdlg, (DRAWITEMSTRUCT *)lparam);
			break;

		case WM_NOTIFY:
			break;

		case WM_CLOSE:
			break;

		case WM_DESTROY:
			D_DestroyAllParamCtrls(hdlg, pD);
			return TRUE;
	}

	return FALSE;
}

//-------------------------------------------------------------
//*** function prepares property Combobox control
BOOL D_PrepareOperCombobox(HWND hctl, D_DEVICE *pD, WORD def_idx)
{
	WORD p;
	TCHAR ttext[D_MAX_PROPNAME_LEN+D_MAX_PROPSUFFIX_LEN+1];

	if (hctl == NULL)
		return(FALSE);

	//reset content
	SendMessage(hctl, CB_RESETCONTENT, 0, 0);

	//insert oper. labels
	if (pD && pD->n_op && pD->op) {
		for (p=0; p<pD->n_op; p++) {
			if (__strncmp(pD->op[p].label, TEXT("t_"), 2) == 0)
				break;

			__sprintf(ttext, TEXT("%.*s%.*s"),
								D_MAX_PROPNAME_LEN, D_ram(pD->op[p].label),
								D_MAX_PROPSUFFIX_LEN, *pD->op[p].suffix ? pD->op[p].suffix : cstr_empty);
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)ttext);
		}
		//correct index
		if (def_idx >= pD->n_op)
			def_idx = pD->n_op-1;
		SendMessage(hctl, CB_SETCURSEL, (WPARAM)def_idx, 0);
	}
	return(TRUE);
}

//*** function prepares property Combobox control
BOOL D_PrepareCombobox(HWND hctl, D_DEV_PROP *prop)
{
	unsigned i, max;
	TCHAR *ptext;

	if (hctl == NULL)
		return(FALSE);

	//reset content
	SendMessage(hctl, CB_RESETCONTENT, 0, 0);

	//test if gui combobox data
	if (prop /*&& prop->guitype == DGUIT_COMBOBOX*/ && prop->guidata) {		//data combobox
		//get max
		switch (prop->type) {
			case DT_BOOL:	max = (unsigned)prop->max.b; break;
			case DT_INT: max = (unsigned)prop->max.i; break;
			default:
			case DT_UINT: max = prop->max.u; break;
			case DT_INT64: max = (unsigned)prop->max.i64; break;
			case DT_UINT64: max = (unsigned)prop->max.u64; break;
		}
		//go through data string
		i = 0;
		ptext = (TCHAR *)prop->guidata;
		while (*ptext) {
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_ram(ptext));
			i++;
			ptext += lstrlen(ptext)+1;
			//limit by max.
			if ((prop->flags & DFLAG_MAX) && i > max)
				break;
		}
		if (i) {
			i--;		//make index
			//limit value
			switch (prop->type) {
				case DT_BOOL:
					if (prop->val.b > (BOOL)i)
						prop->val.b = i;
					i = (unsigned)prop->val.b;
					break;
				case DT_INT:
					if (prop->val.i > (int)i)
						prop->val.i = i;
					i = (unsigned)prop->val.i;
					break;
				default:
				case DT_UINT:
					if (prop->val.u > i)
						prop->val.u = i;
					i = prop->val.u;
					break;
				case DT_INT64:
					if (prop->val.i64 > (__int64)i)
						prop->val.i64 = i;
					i = (unsigned)prop->val.i64;
					break;
				case DT_UINT64:
					if (prop->val.u64 > (unsigned __int64)i)
						prop->val.u64 = i;
					i = (unsigned)prop->val.u64;
					break;
			}

			//select default item by value
			SendMessage(hctl, CB_SETCURSEL, i, 0);
		}
	}
	else if (prop && ((prop->guitype == DGUIT_COMBOBOX && prop->type == DT_BOOL) || (prop->guitype == DGUIT_SWITCH))) {		//boolean combobox
		SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("Off"));
		SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("On"));
		SendMessage(hctl, CB_SETCURSEL, prop->val.b ? 1 : 0, 0);
	}

	return(TRUE);
}

//-----
//*** function prepares edit control subclassing by property
BOOL D_PrepareValidNumSubclassing(HWND hctl, D_DEV_PROP *prop)
{
	U_NSUBCLASSPARAM param = {0};
	U_NVAL nval;

	if (hctl == NULL || prop == NULL)
		return(FALSE);

	//type
	param.mask = U_NMASK_TYPE;
	switch (prop->type) {
		default:
		case DT_BOOL:
		case DT_INT:
		case DT_INT64:
			param.type = HIWORD(U_NFLAGINT);
			param.min.i = prop->min.i;
			param.max.i = prop->max.i;
			nval.i = prop->val.i;
			break;
		case DT_UINT:
		case DT_UINT64:
		case DT_HANDLE:
			param.type = ((prop->type == DT_HANDLE || prop->guitype == DGUIT_HEX) ? HIWORD(U_NFLAGHEX) : HIWORD(U_NFLAGUINT));
			param.min.u = prop->min.u;
			param.max.u = prop->max.u;
			nval.u = prop->val.u;
			break;
		case DT_DOUBLE:
			if ((prop->flags & DFLAG_MIN) && prop->min.d >= 0.0)
				param.type = HIWORD(U_NFLAGUDBL);
			else
				param.type = HIWORD(U_NFLAGDBL);
			param.min.d = prop->min.d;
			param.max.d = prop->max.d;
			nval.d = prop->val.d;
			//apply unit conversion
			if (prop->_uconv.use) {
				D_ConvertValueToDemandUnit(prop, &nval.d);
				D_ConvertValueToDemandUnit(prop, &param.min.d);
				D_ConvertValueToDemandUnit(prop, &param.max.d);
				D_ConvertUnitDemandPrecision(prop, (int *)&param.prec);
			}
			break;
	}
	//precision
	param.mask |= U_NMASK_PREC;
	param.prec = DFLAG_GETPREC(prop->flags);
	//min
	if (prop->flags & DFLAG_MIN)
		param.mask |= U_NMASK_MIN;
	//max
	if (prop->flags & DFLAG_MAX)
		param.mask |= U_NMASK_MAX;
	//zero
	if (prop->flags & DFLAG_ZER)
		param.mask |= U_NMASK_ZER;


	//subclass
	if (SetValidNumStrSubclassingParam(hctl, &param) != NO_ERROR)
		return(FALSE);

	//set value
	SetEditUval(hctl, nval);

	return(TRUE);
}

//*** function modifies edit control subclassing by property
BOOL D_ModifyValidNumSubclassing(HWND hctl, D_DEV_PROP *prop, D_VAL *min, D_VAL *max, unsigned short *prec)
{
	U_NSUBCLASSPARAM param = {0};

	if (hctl == NULL || prop == NULL)
		return(FALSE);

	//type
	param.mask = U_NMASK_TYPE;
	switch (prop->type) {
		default:
		case DT_BOOL:
		case DT_INT:
		case DT_INT64:
			param.type = HIWORD(U_NFLAGINT);
			param.min.i = min ? min->i : prop->min.i;
			param.max.i = max ? max->i : prop->max.i;
			break;
		case DT_UINT:
		case DT_UINT64:
		case DT_HANDLE:
			param.type = (prop->type == DT_HANDLE ? HIWORD(U_NFLAGHEX) : HIWORD(U_NFLAGUINT));
			param.min.u = min ? min->u : prop->min.u;
			param.max.u = max ? max->u : prop->max.u;
			break;
		case DT_DOUBLE:
			if ((prop->flags & DFLAG_MIN) && prop->min.d >= 0.0)
				param.type = HIWORD(U_NFLAGUDBL);
			else
				param.type = HIWORD(U_NFLAGDBL);
			param.min.d = min ? min->d : prop->min.d;
			param.max.d = max? max->d : prop->max.d;
			//apply unit conversion
			if (prop->_uconv.use) {
				D_ConvertValueToDemandUnit(prop, &param.min.d);
				D_ConvertValueToDemandUnit(prop, &param.max.d);
				D_ConvertUnitDemandPrecision(prop, (int *)&param.prec);
			}
			break;
	}
	//precision
	param.mask |= U_NMASK_PREC;
	param.prec = prec ? *prec : DFLAG_GETPREC(prop->flags);
	//min
	if (prop->flags & DFLAG_MIN)
		param.mask |= U_NMASK_MIN;
	//max
	if (prop->flags & DFLAG_MAX)
		param.mask |= U_NMASK_MAX;
	//zero
	if (prop->flags & DFLAG_ZER)
		param.mask |= U_NMASK_ZER;

	//subclass
	if (UpdateValidNumStrSubclassingParam(hctl, &param) != NO_ERROR)
		return(FALSE);

	return(TRUE);
}

//*** function prepares range edit control subclassing by property
BOOL D_PrepareRangeValidNumSubclassing(HWND hctl, int level, D_DEV_PROP *prop)
{
	U_NSUBCLASSPARAM param = {0};
	U_NVAL nval;

	if (hctl == NULL || prop == NULL)
		return(FALSE);

	//setup parameters
	param.mask = U_NMASK_TYPE | U_NMASK_MIN | U_NMASK_MAX;
	switch (prop->guitype) {
		case DGUIT_HEX: param.type = HIWORD(U_NFLAGHEX); break;
		default: param.type = HIWORD(U_NFLAGUINT); break;
	}
	//for low range
	if (level == 0) {
		param.min.u = (prop->flags & DFLAG_MIN) ? LOWORD(prop->min.u) : 0;
		param.max.u = (prop->flags & DFLAG_MAX) ? LOWORD(prop->max.u) : 0xFFFF;
		nval.u = LOWORD(prop->val.u);
	}
	//for high range
	else {
		param.min.u = (prop->flags & DFLAG_MIN) ? HIWORD(prop->min.u) : 0;
		param.max.u = (prop->flags & DFLAG_MAX) ? HIWORD(prop->max.u) : 0xFFFF;
		nval.u = HIWORD(prop->val.u);
	}

	//subclass
	if (SetValidNumStrSubclassingParam(hctl, &param) != NO_ERROR)
		return(FALSE);

	//set value
	SetEditUval(hctl, nval);

	return(TRUE);
}

//*** function modifies range edit control subclassing by parameters
BOOL D_ModifyRangeValidNumSubclassing(HWND hctl, int level, D_DEV_PROP *prop, D_VAL *min, D_VAL *max)
{
	U_NSUBCLASSPARAM param = {0};

	if (hctl == NULL || prop == NULL)
		return(FALSE);

	//for low range
	if (level == 0) {
		param.min.u = min ? LOWORD(min->u) : LOWORD(prop->min.u);
		param.max.u = max ? LOWORD(max->u) : LOWORD(prop->max.u);
	}
	//for high range
	else {
		param.min.u = min ? HIWORD(min->u) : HIWORD(prop->min.u);
		param.max.u = max ? HIWORD(max->u) : HIWORD(prop->max.u);
	}

	//min
	if (prop->flags & DFLAG_MIN)
		param.mask |= U_NMASK_MIN;
	//max
	if (prop->flags & DFLAG_MAX)
		param.mask |= U_NMASK_MAX;

	//subclass
	if (UpdateValidNumStrSubclassingParam(hctl, &param) != NO_ERROR)
		return(FALSE);

	return(TRUE);
}

//*** function modifies range edit controls
BOOL D_ModifyRangeControls(HWND hdlg, D_DEVICE *pD, int pidx, D_VAL *min, D_VAL *max)
{
	HWND hlo, hhi;
	D_DEV_PROP *prop;
	U_NVAL nlo, nhi;

	if (pD == NULL || pD->p == NULL)
		return(FALSE);

	prop = &pD->p[pidx];
	hlo = D_GETOLH_VAL(hdlg, pidx);
	hhi = D_GETOLH_VAL2(hdlg, pidx);

	D_ModifyRangeValidNumSubclassing(hlo, 0, prop, min, max);
	D_ModifyRangeValidNumSubclassing(hhi, 1, prop, min, max);

	//validate value
	GetEditUval(hlo, &nlo);
	GetEditUval(hhi, &nhi);

	//correct high limit
	if (nhi.u < nlo.u) {
		nhi.u = nlo.u;
		SetEditUval(hhi, nhi);
	}

	return(TRUE);
}


//*** function prepares S+M combobox control
//note: mode=0 -> edge; mode=1 -> action
/*
BOOL D_PrepareStartMarkCB(HWND hctl, int mode)
{

	if (hctl == NULL)
		return(FALSE);

	//reset content
	SendMessage(hctl, CB_RESETCONTENT, 0, 0);

	switch (mode) {
		default:
		case 0:
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("none"));
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("down"));
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("up"));
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("both"));
			break;
		case 1:
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("none"));
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("mark"));
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("start"));
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("stop"));
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("zero"));
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_("C/W"));
			break;
	}
	return(TRUE);
}
*/

//------
#define D_GUITABLE_SEP1 '|'
#define D_GUITABLE_SEP2 ';'
#define D_GUITABLE_SEP3 ','
#define D_MAX_TABLECBDATA_LEN 128
typedef struct {
	TCHAR name[D_MAX_PROPNAME_LEN];
	TCHAR unit[D_MAX_PROPUNIT_LEN];
	unsigned int bytes;
	unsigned int div;
	TCHAR format[D_MAX_PROPNAME_LEN];
	TCHAR *cb;
} D_GUITABLEITEM;

typedef struct {
	unsigned int count;
	unsigned int rsize;
	D_GUITABLEITEM *col;
	//
	int row_id;
	int chdr_lines;
} D_GUITABLE;

int D_GetDivPrecision(int div)
{
	int prec = 0;

	while ((div /= 10) > 0) {
		prec++;
	};
	return(prec);
}

//*** function decodes gui table
//"C<cols>;<row_id>;<ch_lines>|<name> [unit];<bytes>;<divider>;<printf>;CB_item1:CN_item2|..."
BOOL D_GuiDecodeTable(TCHAR *format, D_GUITABLE *guitable)
{
	D_GUITABLEITEM *col;
	unsigned int n, i, len, rsize = 0, x;
	TCHAR c_mask[3] = {D_GUITABLE_SEP2, D_GUITABLE_SEP1, 0};
	TCHAR *ptext, *pt;

	if (format == NULL || guitable == NULL || *format != 'C')
		return(FALSE);	//invalid format
	format++;

	//get number of columns
	n = __atoi(format);
	if (n == 0)
		return(FALSE);		//no columns

	//reset guitable
	memset(guitable, 0, sizeof(D_GUITABLE));

	//search for row-id after size
	ptext = __strpbrk(format, c_mask);
	if (ptext && *ptext == D_GUITABLE_SEP2) {
		ptext++;
		if (*ptext != D_GUITABLE_SEP2 && *ptext != D_GUITABLE_SEP1) {
			guitable->row_id = *ptext;
		}
		format = ptext;
	}

	//search for col-header lines
	ptext = __strpbrk(format, c_mask);
	if (ptext && *ptext == D_GUITABLE_SEP2) {
		ptext++;
		if (*ptext != D_GUITABLE_SEP2 && *ptext != D_GUITABLE_SEP1) {
			guitable->chdr_lines = __atoi(ptext);
		}
		format = ptext;
	}
	if (guitable->chdr_lines < 0)
		guitable->chdr_lines = 1;

	//allocation
	if ((col = malloc(n*sizeof(D_GUITABLEITEM))) == NULL)
		return(FALSE);		//out of memory
	memset(col, 0, n*sizeof(D_GUITABLEITEM));

	//decode columns format
	for (i=0; i<n; i++) {
		ptext = __strchr(format, D_GUITABLE_SEP1);
		if (ptext == NULL) {
			//defaults
			__sprintf(col[i].name, TEXT("Col%d"), i+1);
		}
		else {
			//name
			format = ptext+1;
			ptext = __strpbrk(format, c_mask);
			len = (ptext ? ptext-format : lstrlen(format)) + 1;
			//unit
			pt = __strstr(format, TEXT(" ["));
			if (pt && (pt-format) < len) {
				x = pt-format+1;
				if (x > D_MAX_PROPNAME_LEN)
					x = D_MAX_PROPNAME_LEN;
				lstrcpyn(col[i].name, format, x);
				pt += 2;
				x = len-(pt-format+1);
				if (*(pt+x-1) != ']')
					x++;
				if (x > D_MAX_PROPUNIT_LEN)
					x = D_MAX_PROPUNIT_LEN;
				lstrcpyn(col[i].unit, pt, x);
			}
			else {
				if (len > D_MAX_PROPNAME_LEN)
					len = D_MAX_PROPNAME_LEN;
				lstrcpyn(col[i].name, format, len);
			}
			if (ptext == NULL || *ptext != D_GUITABLE_SEP2)
				continue;
			//bytes
			format = ptext+1;
			ptext = __strpbrk(format, c_mask);
			col[i].bytes = __atoi(format);
			rsize += col[i].bytes;
			if (ptext == NULL || *ptext != D_GUITABLE_SEP2)
				continue;
			//div
			format = ptext+1;
			ptext = __strpbrk(format, c_mask);
			col[i].div = __atoi(format);
			if (ptext == NULL || *ptext != D_GUITABLE_SEP2)
				continue;
			//printf
			format = ptext+1;
			ptext = __strpbrk(format, c_mask);
			if (ptext)
				len = ptext-format;
			else
				len = lstrlen(format);
			if (len > D_MAX_PROPNAME_LEN)
				len = D_MAX_PROPNAME_LEN;
			lstrcpyn(col[i].format, format, len);
			if (ptext == NULL || *ptext != D_GUITABLE_SEP2)
				continue;
			//cb-data
			format = ptext+1;
			if (*format && __strncmp(format, TEXT("CB:"), 3) == 0) {
				format += 3;
				TCHAR mask2[4];
				mask2[0] = D_GUITABLE_SEP3;
				mask2[1] = D_GUITABLE_SEP2;
				mask2[2] = D_GUITABLE_SEP1;
				mask2[3] = 0;
				int n = 1;
				int nc = 2;
				ptext = format;
				while (1) {
					pt = __strpbrk(ptext, mask2);
					if (pt)
						nc += pt-ptext+1;
					else
						nc += __strlen(ptext)+1;
					if (pt == NULL || *pt != D_GUITABLE_SEP3)
						break;
					n++;
					ptext = pt+1;
				}
				//alloc
				col[i].cb = malloc(nc*sizeof(TCHAR));
				if (col[i].cb == NULL)
					continue;	//memory error
				//set
				memset(col[i].cb, 0, nc*sizeof(TCHAR));
				pt = col[i].cb;
				while (1) {
					ptext = __strpbrk(format, mask2);
					if (ptext)
						nc = ptext-format;
					else
						nc = __strlen(format);
					__strncpy(pt, format, nc);
					pt[nc] = '\0';
					pt = pt+nc+1;
					if (ptext == NULL || *ptext != D_GUITABLE_SEP3)
						break;
					n++;
					format = ptext+1;
				}
			}

		}
	}
	//result
	guitable->count = n;
	guitable->col = col;
	guitable->rsize = rsize;
	return(TRUE);
}


//*** function destroys gui table
BOOL D_GuiDestroyTable(D_GUITABLE *guitable)
{

	if (guitable == NULL)
		return(FALSE);	//invalid handle

	if (guitable->count && guitable->col) {
		//through columns
		int i;
		D_GUITABLEITEM *pcol;
		for (i=0; i<guitable->count; i++) {
			pcol = &guitable->col[i];
			//cb-data
			if (pcol->cb) {
				free((void*)pcol->cb);
				pcol->cb = NULL;
			}
		}
		free((void*)guitable->col);
	}
	guitable->col = NULL;
	guitable->count = 0;
	return(TRUE);
}

//*** function prints gui table
BOOL D_GuiPrintTable(TCHAR *text, unsigned int maxlen, TCHAR *format, unsigned char *buf, unsigned int size, int rtf)
{
	unsigned int i, j;
	TCHAR ttext[D_MAX_PROPNAME_LEN+D_MAX_PROPUNIT_LEN+3], *ptext;
	int val;
	unsigned char *pbuf, rlen;
	double dval;
	int prec;
	D_GUITABLE gtable;

	if (format == NULL || text == NULL || buf == NULL || size == 0)
		return(FALSE);

	if (!D_GuiDecodeTable(format, &gtable))
		return(FALSE);

	TCHAR *c_tab = rtf ? TEXT("\\tab ") : TEXT("\t");
	TCHAR *c_crlf = rtf ? TEXT("\\par ") : TEXT("\r\n");

	//print header
	ptext = text;
	__strcpy(ptext, TEXT("No."));
	for (i=0; i<gtable.count; i++) {
		lstrcpyn(ttext, D_ram(gtable.col[i].name), D_MAX_PROPNAME_LEN);
		if (*gtable.col[i].unit)
			__sprintf(ttext, TEXT("%s [%s]"), ttext, gtable.col[i].unit);
		__sprintf(ptext, rtf ? TEXT("%s  %s%s") : TEXT("%s  %s%-10s"), ptext, c_tab, ttext);
	}
	__strcat(ptext, c_crlf);
	maxlen -= __strlen(text)+10;

	//values
	j = 0;
	pbuf = buf;
	rlen = 10*(gtable.count+1);
	while (size >= gtable.rsize && maxlen >= rlen) {
		if (gtable.row_id)
			__sprintf(ptext, TEXT("%s  %s%c"), ptext, c_tab, j+gtable.row_id);
		else
			__sprintf(ptext, TEXT("%s  %s%d"), ptext, c_tab, j+1);
		for (i=0; i<gtable.count; i++) {
			if (gtable.col[i].bytes>=8) {
				dval = 0.0;
				memcpy(&dval, pbuf, 8);
				prec = gtable.col[i].div;
			}
			else {
				val = 0;
				memcpy(&val, pbuf, gtable.col[i].bytes>4 ? 4 : gtable.col[i].bytes);
				if (gtable.col[i].div > 1)
					dval = (double)val/(double)gtable.col[i].div;
				else
					dval = (double)val;
				prec = D_GetDivPrecision(gtable.col[i].div);
			}
			pbuf += gtable.col[i].bytes;
			if (gtable.col[i].cb) {
				TCHAR *pt = GetSubString(gtable.col[i].cb, (unsigned int)dval);
				__sprintf(ptext, TEXT("%s  %s%s"), ptext, c_tab, pt ? pt : TEXT("?"));
			}
			else {
				__sprintf(ptext, TEXT("%s  %s%.*lf"), ptext, c_tab, prec, dval);
			}
		}
		__strcat(ptext, c_crlf);
		size -= gtable.rsize;
		maxlen -= rlen;
		j++;
	}

	//not enough space
	if (size) {
		__sprintf(ptext, TEXT("%s%s...%s"), ptext, c_tab, c_crlf);
	}

	//
	D_GuiDestroyTable(&gtable);

	return(TRUE);
}

//---
#define D_GUISHOWMEM_SEP1 '|'
#define D_GUISHOWMEM_SEP2 ';'
#define D_GUISHOWMEM_SEP3 '['
#define D_GUISHOWMEM_SEP4 ']'
#define D_GUISHOWMEM_SEP5 ','
#define D_GUISHOWMEM_SEPSTR TEXT("|;")
#define D_GUISHOWMEM_SEPSTRUNIT TEXT("[|;")

/* get substring by length (-1), separator and index */
TCHAR *D_GetSepSubString(TCHAR *str, int str_len, int separ, int index, int *substr_len)
{
	int i;
	TCHAR *pmax, *pt;

	if (str) {

		//max size
		if (str_len == (unsigned int)-1)
			str_len = __strlen(str);

		//no length
		if (str_len == 0)
			return(NULL);

		//correct index
		if (index < 0)
			index = 0;

		//through separators
		i = 0;
		pmax = str + str_len;
		while (i < index) {
			while (str < pmax && *str != separ)
				str++;
			if (str >= pmax)
				return(NULL);
			i++;
			str++;
		}

		//get substring length
		if (substr_len) {
			pt = str;
			while (pt < pmax && *pt != separ)
				pt++;
			*substr_len = pt-str;
		}
		return(str);
	}
	return(NULL);
}

/* print formated memory data (data has to be in system endian) */
//format for one item: |name[unit];VALTYPE;offset in bytes;divisor (optional);combobox/data (optional);style (optional)
//style:0=none,1=combobox,2=bitmask,3=ip,4=hex
BOOL D_GuiPrintShowMem(TCHAR *text, unsigned int maxlen, TCHAR *format, unsigned char *buf, unsigned int size)
{
	TCHAR *fi, *pt, *name, *unit, *cb;
	TCHAR ttext[512];
	unsigned char *pbuf;
	VALTYPE vtype;
	int name_len, unit_len, offset, div, prec, cb_len, count, vcount, vsize, v, c, style;
	double dval, ddiv;

	if (text == NULL || buf == NULL || size == 0 || format == NULL)
		return(FALSE);

	*text = '\0';
	count = 0;

	//init
	fi = format;
	while ((fi = __strchr(fi, D_GUISHOWMEM_SEP1)) != NULL) {
		//decode item format
		fi++;
		name = cstr_questmark;
		name_len = __strlen(name);
		unit = cstr_empty;
		unit_len = 0;
		vtype = VALTYPE_U8;
		offset = 0;
		div = 1;
		ddiv = 1.0;
		style = 0;
		cb = NULL;
		cb_len = 0;

		while (1) {
			//name
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt == NULL || *pt == D_GUISHOWMEM_SEP1 || (pt-fi) == 0) {
				break;
			}
			name = fi;
			name_len = pt-fi;
			fi = pt+1;
			//unit
			pt = __strpbrk(name, D_GUISHOWMEM_SEPSTRUNIT);
			if (pt && *pt == D_GUISHOWMEM_SEP3 && pt < fi) {
				pt++;
				unit = pt;
				unit_len = name_len-(pt-name);
				name_len -= unit_len+1;
				if (unit[unit_len-1] == D_GUISHOWMEM_SEP4)
					unit_len--;		//remove ']'
				if (name[name_len-1] == ' ')
					name_len--;
			}
			//type
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt == NULL || *pt == D_GUISHOWMEM_SEP1 || (pt-fi) == 0) {
				break;
			}
			if (*fi == 'x')	//hexadecimal value
				vtype = HexStringNumToInt(fi+1);
			else
				vtype = __atoi(fi);	//separator stops conversion
			fi = pt+1;
			//offset
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt && (pt-fi) == 0) {
				break;
			}
			if (*fi == 'x')	//hexadecimal value
				offset = HexStringNumToInt(fi+1);
			else
				offset = __atoi(fi);	//separator stops conversion
			if (pt == NULL || *pt == D_GUISHOWMEM_SEP1)
				break;
			fi = pt+1;
			//divisor
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt && (pt-fi) == 0) {
				break;
			}
			ddiv = __atof(fi);	//separator stops conversion
			div = (int)(ddiv+0.5);
			if (div <= 0) {
				div = 1;
			}
			if (pt == NULL || *pt == D_GUISHOWMEM_SEP1 || *pt == '\0')
				break;
			fi = pt+1;
			//combobox/other data
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt == NULL) {
				pt = fi + __strlen(fi);	//last item
			}
			cb = fi;
			cb_len = pt-fi;
			if (cb && cb_len) {
				style = 1;	//default combobox
			}
			if (*pt == D_GUISHOWMEM_SEP1 || *pt == '\0')
				break;
			fi = pt+1;
			//style
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt == NULL) {
				pt = fi + __strlen(fi);	//last item
			}
			style = __atoi(fi);
			if (*pt == D_GUISHOWMEM_SEP1 || *pt == '\0')
				break;
			fi = pt+1;

			//done
			break;
		}

		//get type
		vcount = VALTYPE_GETARRAYLEN(vtype);
		if (vcount <= 0)
			vcount = 1;
		vtype = VALTYPE_TYPE(vtype);
		vsize = VALTYPE_SIZE(vtype);

		//pointer to data
		if (offset < 0  || (offset+vsize) > size)
			pbuf = NULL;	//error
		else
			pbuf = buf+offset;
		//check array size


		//precision from div
		prec = 0;
		offset = div;
		while ((offset = (offset+8)/10) > 0) {
			prec++;
		}

		//--- print
		//separator
		if (count) {
			__strcpy(ttext, TEXT("; "));
		}
		else
			*ttext = '\0';

		//name
		__sprintf(ttext, TEXT("%s%.*s: "), ttext, name_len, name);

		//values
		for (v=0; v<vcount; v++) {
			if (pbuf) {

				//char -> string
				if (vtype == VALTYPE_CHAR || vtype == VALTYPE_UCHAR) {
					if (vtype == VALTYPE_CHAR)
						c = (int)((unsigned char *)pbuf)[v];
					else
						c = (int)((unsigned short *)pbuf)[v];
					if (c) {
						__sprintf(ttext, TEXT("%s%c"), ttext, (int)((unsigned char *)pbuf)[v]);
						continue;		//next character
					}
					else
						break;		//end for cycle
				}

				//comma
				if (v > 0)
					__strcat(ttext, TEXT(", "));

				//value -> dval
				switch (vtype) {
					default:
					case VALTYPE_U8: dval = (double)((unsigned char *)pbuf)[v]; break;
					case VALTYPE_U16: dval = (double)((unsigned short *)pbuf)[v]; break;
					case VALTYPE_U32: dval = (double)((unsigned int *)pbuf)[v]; break;
					case VALTYPE_U64: dval = (double)((unsigned __int64 *)pbuf)[v]; break;
					case VALTYPE_I8: dval = (double)((char *)pbuf)[v]; break;
					case VALTYPE_I16: dval = (double)((short *)pbuf)[v]; break;
					case VALTYPE_I32: dval = (double)((int *)pbuf)[v]; break;
					case VALTYPE_I64: dval = (double)((__int64 *)pbuf)[v]; break;
					case VALTYPE_FLT: dval = (double)((float *)pbuf)[v]; break;
					case VALTYPE_DBL: dval = (double)((double *)pbuf)[v]; break;
				}
				//division
				dval /= ddiv;

				pt = NULL;
				if (style) {
					switch (style) {
					//combobox
					case 1:
						//combobox value
						pt = D_GetSepSubString(cb, cb_len, D_GUISHOWMEM_SEP5, (unsigned int)dval, &c);
						if (pt) {
							__sprintf(ttext, TEXT("%s%.*s"), ttext, c, pt);
						}
						break;
					//bitmask
					case 2:
						{
							unsigned int bits = (unsigned int)dval;
							unsigned int n = 0;
							unsigned int bn = 0;
							while (bits) {
								if (bits & 0x01) {
									//separator
									if (bn) {
										__strcat(ttext, TEXT("|"));
									}
									//bitmask value
									pt = D_GetSepSubString(cb, cb_len, D_GUISHOWMEM_SEP5, n, &c);
									if (pt)
										__sprintf(ttext, TEXT("%s%.*s"), ttext, c, pt);
									else
										__sprintf(ttext, TEXT("%s0x%X"), ttext, 1<<n);
									bn++;
								}
								bits >>= 1;
								n++;
							}
							if (bn == 0) {
								__strcat(ttext, TEXT("---"));
							}
							pt = (TCHAR*)1;
						}
						break;
					//ip
					case 3:
						{
							if (vtype == VALTYPE_U32) {
								unsigned int ip = ((unsigned int *)pbuf)[v];
								unsigned char *pip = (unsigned char *)&ip;
								__sprintf(ttext, TEXT("%s%d.%d.%d.%d"), ttext, pip[0], pip[1], pip[2], pip[3]);
								pt = (TCHAR*)1;
							}
						}
						break;
					//hex
					case 4:
						{
							unsigned int hex = (unsigned int)dval;
							__sprintf(ttext, TEXT("%s0x%02X"), ttext, hex);
							pt = (TCHAR*)1;
						}
						break;
					}
				}
				if (pt) {
					//by style
				}
				else if (vtype == VALTYPE_FLT || vtype == VALTYPE_DBL) {
					//value -> string
					//__sprintf(ttext, TEXT("%s%.*lg"), ttext, 6, dval);
					__sprintf(ttext, TEXT("%s%lg"), ttext, dval);
				}
				else {
					//value -> string
					__sprintf(ttext, TEXT("%s%.*lf"), ttext, prec, dval);
				}
			}
			else
				__strcat(ttext, cstr_questmark);
		}

		//unit
		if (unit && unit_len) {
			__sprintf(ttext, TEXT("%s %.*s"), ttext, unit_len, unit);
		}

		//test size
		offset = __strlen(ttext);
		if (offset < maxlen) {
			__strcat(text, ttext);
			maxlen -= offset;
			count++;
		}
		else {
			//not enought space
			if (maxlen > 4)
				__strcat(text, TEXT("..."));
			break;
		}
	}

	return(TRUE);
}

/* RTF print formated memory data (data has to be in system endian) */
//format for one item: |name[unit];VALTYPE;offset in bytes;divisor (optional);combobox/data (optional);style (optional)
//style:0=none,1=combobox,2=bitmask,3=ip,4=hex
BOOL D_GuiRTFPrintShowMem(HSTRING hs, TCHAR *format, unsigned char *buf, unsigned int size)
{
	TCHAR *fi, *pt, *name, *unit, *cb;
	unsigned char *pbuf;
	VALTYPE vtype;
	int name_len, unit_len, offset, div, prec, count, cb_len, vcount, vsize, v, c, style;
	double dval, ddiv;

	if (hs == NULL || buf == NULL || size == 0 || format == NULL)
		return(FALSE);

	//init
	count = 0;
	fi = format;
	while ((fi = __strchr(fi, D_GUISHOWMEM_SEP1)) != NULL) {
		//decode item format
		fi++;
		name = cstr_questmark;
		name_len = __strlen(name);
		unit = cstr_empty;
		unit_len = 0;
		vtype = VALTYPE_U8;
		offset = 0;
		div = 1;
		ddiv = 1.0;
		style = 0;
		cb = NULL;
		cb_len = 0;

		while (1) {
			//name
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt == NULL || *pt == D_GUISHOWMEM_SEP1 || (pt-fi) == 0) {
				break;
			}
			name = fi;
			name_len = pt-fi;
			fi = pt+1;
			//unit
			pt = __strpbrk(name, D_GUISHOWMEM_SEPSTRUNIT);
			if (pt && *pt == D_GUISHOWMEM_SEP3 && pt < fi) {
				pt++;
				unit = pt;
				unit_len = name_len-(pt-name);
				name_len -= unit_len+1;
				if (unit[unit_len-1] == D_GUISHOWMEM_SEP4)
					unit_len--;		//remove ']'
				if (name[name_len-1] == ' ')
					name_len--;
			}
			//type
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt == NULL || *pt == D_GUISHOWMEM_SEP1 || (pt-fi) == 0) {
				break;
			}
			if (*fi == 'x')	//hexadecimal value
				vtype = HexStringNumToInt(fi+1);
			else
				vtype = __atoi(fi);	//separator stops conversion
			fi = pt+1;
			//offset
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt && (pt-fi) == 0) {
				break;
			}
			if (*fi == 'x')	//hexadecimal value
				offset = HexStringNumToInt(fi+1);
			else
				offset = __atoi(fi);	//separator stops conversion
			if (pt == NULL || *pt == D_GUISHOWMEM_SEP1)
				break;
			fi = pt+1;
			//divisor
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt && (pt-fi) == 0) {
				break;
			}
			ddiv = __atof(fi);	//separator stops conversion
			div = (int)(ddiv + 0.5);
			if (div <= 0) {
				div = 1;
			}
			if (pt == NULL || *pt == D_GUISHOWMEM_SEP1 || *pt == '\0')
				break;
			fi = pt+1;
			//combobox/other data
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt == NULL) {
				pt = fi + __strlen(fi);		//last item
			}
			cb = fi;
			cb_len = pt-fi;
			if (cb && cb_len) {
				style = 1;	//default combobox
			}
			if (*pt == D_GUISHOWMEM_SEP1 || *pt == '\0')
				break;
			fi = pt+1;
			//style
			pt = __strpbrk(fi, D_GUISHOWMEM_SEPSTR);
			if (pt == NULL) {
				pt = fi + __strlen(fi);		//last item
			}
			style = __atoi(fi);
			if (*pt == D_GUISHOWMEM_SEP1 || *pt == '\0')
				break;
			fi = pt+1;

			//done
			break;
		}

		//get type
		vcount = VALTYPE_GETARRAYLEN(vtype);
		if (vcount <= 0)
			vcount = 1;
		vtype = VALTYPE_TYPE(vtype);
		vsize = VALTYPE_SIZE(vtype);

		//pointer to data
		if (offset < 0  || (offset+vsize) > size)
			pbuf = NULL;	//error
		else
			pbuf = buf+offset;
		//check array size


		//precision from div
		if (vtype == VALTYPE_FLT || vtype == VALTYPE_DBL) {
			prec = 6;
		}
		else {
			prec = 0;
			offset = div;
			while ((offset = (offset+8)/10) > 0) {
				prec++;
			}
		}

		//--- print

		//name
		HSTR_AppendPrintf(hs, TEXT("%.*s: \\tab "), name_len, name);

		//values
		for (v=0; v<vcount; v++) {
			if (pbuf) {

				//char -> string
				if (vtype == VALTYPE_CHAR || vtype == VALTYPE_UCHAR) {
					if (vtype == VALTYPE_CHAR)
						c = (int)((unsigned char *)pbuf)[v];
					else
						c = (int)((unsigned short *)pbuf)[v];
					if (c) {
						HSTR_AppendPrintf(hs, TEXT("%c"), (int)((unsigned char *)pbuf)[v]);
						continue;		//next character
					}
					else
						break;		//end for cycle
				}

				//comma
				if (v > 0)
					HSTR_AppendPrintf(hs, TEXT(", "));

				//value -> dval
				switch (vtype) {
					default:
					case VALTYPE_U8: dval = (double)((unsigned char *)pbuf)[v]; break;
					case VALTYPE_U16: dval = (double)((unsigned short *)pbuf)[v]; break;
					case VALTYPE_U32: dval = (double)((unsigned int *)pbuf)[v]; break;
					case VALTYPE_U64: dval = (double)((unsigned __int64 *)pbuf)[v]; break;
					case VALTYPE_I8: dval = (double)((char *)pbuf)[v]; break;
					case VALTYPE_I16: dval = (double)((short *)pbuf)[v]; break;
					case VALTYPE_I32: dval = (double)((int *)pbuf)[v]; break;
					case VALTYPE_I64: dval = (double)((__int64 *)pbuf)[v]; break;
					case VALTYPE_FLT: dval = (double)((float *)pbuf)[v]; break;
					case VALTYPE_DBL: dval = (double)((double *)pbuf)[v]; break;
				}
				//division
				dval /= ddiv;

				//style
				pt = NULL;
				if (style) {
					switch (style) {
					//combobox
					case 1:
						//combobox value
						pt = D_GetSepSubString(cb, cb_len, D_GUISHOWMEM_SEP5, (unsigned int)dval, &c);
						if (pt) {
							HSTR_AppendPrintf(hs, TEXT("%.*s"), c, pt);
						}
						break;
					//bitmask
					case 2:
						{
							unsigned int bits = (unsigned int)dval;
							unsigned int n = 0;
							unsigned int bn = 0;
							while (bits) {
								if (bits & 0x01) {
									//separator
									if (bn) {
										HSTR_AppendPrintf(hs, TEXT("|"));
									}
									//bitmask value
									pt = D_GetSepSubString(cb, cb_len, D_GUISHOWMEM_SEP5, n, &c);
									if (pt)
										HSTR_AppendPrintf(hs, TEXT("%.*s"), c, pt);
									else
										HSTR_AppendPrintf(hs, TEXT("0x%X"), 1<<n);
									bn++;
								}
								bits >>= 1;
								n++;
							}
							if (bn == 0) {
								HSTR_AppendPrintf(hs, TEXT("---"));
							}
							pt = (TCHAR*)1;
						}
						break;
					//ip
					case 3:
						{
							if (vtype == VALTYPE_U32) {
								unsigned int ip = ((unsigned int *)pbuf)[v];
								unsigned char *pip = (unsigned char *)&ip;
								HSTR_AppendPrintf(hs, TEXT("%d.%d.%d.%d"), pip[0], pip[1], pip[2], pip[3]);
								pt = (TCHAR*)1;
							}
						}
						break;
					//hex
					case 4:
						{
							unsigned int hex = (unsigned int)dval;
							HSTR_AppendPrintf(hs, TEXT("%02Xh"), hex);
							pt = (TCHAR*)1;
						}
						break;
					}
				}
				if (pt) {
					//by style
				}
				else {
					//value -> string
					if (vtype == VALTYPE_FLT || vtype == VALTYPE_DBL)
						HSTR_AppendPrintf(hs, TEXT("%.6lg"), dval);
					else
						HSTR_AppendPrintf(hs, TEXT("%.*lf"), prec, dval);
				}
			}
			else
				HSTR_AppendPrintf(hs, cstr_questmark);
		}

		//unit
		if (unit && unit_len) {
			HSTR_AppendPrintf(hs, TEXT(" %.*s"), unit_len, unit);
		}
		HSTR_AppendPrintf(hs, TEXT("\\par "));

		count++;
	}

	return(TRUE);
}

//-----
//*** function returns number of characters for gui data
size_t D_GuiDataLen(const TCHAR *guidata)
{
	TCHAR *ptext;

	if (guidata == NULL)
		return(0);

	ptext = (TCHAR *)guidata;
	while (*ptext)
		ptext += lstrlen(ptext)+1;
	return((ptext-guidata)+1);		//calculate
}

//*** function returns gui data string by value
//note: return NULL when no such data
#define D_MAX_SHOW_DTMEMORY 32
const TCHAR *D_GetGuiDataValue(D_DEV_PROP *prop, D_VAL val)
{
	static TCHAR ttext[4096+128];
	unsigned i, j, n;
	TCHAR *ptext;
	unsigned char *pval;
	U_NVAL nval;

	if (prop == NULL)
		return(NULL);

	//test if gui combobox data
	if (prop->guidata && *prop->guidata &&
			(prop->guitype == DGUIT_COMBOBOX ||
			 //prop->guitype == DGUIT_SWITCH ||
			 prop->guitype == DGUIT_RADIOSW ||
			 prop->guitype == DGUIT_READ ||
			 prop->guitype == DGUIT_CHECKBITS)) {
		//get value
		switch (prop->type) {
			case DT_BOOL:
				i = (unsigned)val.b;
				break;
			case DT_INT:
				i = (unsigned)val.i;
				break;
			default:
			case DT_UINT:
				i = val.u;
				break;
			case DT_INT64:
				i = (unsigned)val.i64;
				break;
			case DT_UINT64:
				i = (unsigned)val.u64;
				break;
		}

		if (prop->guitype == DGUIT_CHECKBITS) {
			//through bits
			*ttext = '\0';
			n = 0;
			unsigned int nb = 0;
			ptext = (TCHAR *)prop->guidata;
			while (i) {
				if (i & 0x1) {
					if (nb) {
						__strcat(ttext, TEXT("|"));
					}
					if (*ptext)
						__sprintf(ttext, TEXT("%s%s"), ttext, D_ram(ptext));
					else
						__sprintf(ttext, TEXT("%s0x%X"), ttext, 1<<n);
					nb++;
				}
				i >>= 1;
				n++;
				if (*ptext) {
					ptext += lstrlen(ptext)+1;
				}
			}
			if (nb == 0) {
				__strcat(ttext, TEXT("---"));
			}
			return(ttext);
		}
		else {
			//count guidata string
			n = 0;
			ptext = (TCHAR *)prop->guidata;
			while (*ptext) {
				n++;
				ptext += lstrlen(ptext)+1;
			}
			//correct index
			if (i >= n)
				i = n-1;

			//get begginning of string by index
			ptext = (TCHAR *)prop->guidata;
			while (i > 0 && *ptext) {
				i--;
				ptext += lstrlen(ptext)+1;
			}
			return(D_ram(ptext));
		}
	}
	else if (prop->guitype == DGUIT_SWITCH) {
		return(D_GetBoolString(prop->guidata, 0, prop->val.b));
	}
	else if (prop->guitype == DGUIT_RANGE) {
		switch (prop->type) {
			case DT_INT:
				__sprintf(ttext, TEXT("%d--%d"), val.i & 0xFFFF, (val.i >> 16) & 0xFFFF);
				break;
			default:
			case DT_UINT:
				__sprintf(ttext, TEXT("%u--%u"), val.u & 0xFFFF, (val.u >> 16) & 0xFFFF);
				break;
		}
		return(ttext);
	}
	else if (prop->guitype == DGUIT_HEX || prop->guitype == DGUIT_BIGHEX /*|| prop->guitype == DGUIT_BIN*/) {
		//get value
		switch (prop->type) {
			case DT_BOOL:
				pval = (unsigned char *)&val.b;
				i = 4;		//4 bytes
				break;
			case DT_INT:
				pval = (unsigned char *)&val.i;
				i = 4;		//4 bytes
				break;
			default:
			case DT_UINT:
				pval = (unsigned char *)&val.u;
				i = 4;		//4 bytes
				break;
			case DT_DOUBLE:
				pval = (unsigned char *)&val.d;
				i = 8;		//8 bytes
				break;
			case DT_INT64:
				pval = (unsigned char *)&val.i64;
				i = 8;		//8 bytes
				break;
			case DT_UINT64:
				pval = (unsigned char *)&val.u64;
				i = 8;		//8 bytes
				break;
			case DT_MEMORY:
			case DT_STRING:
				pval = (unsigned char *)val.m.buf;
				i = val.m.size;
				break;
		}
		//limit
		if (i > 80)
			i = 80;
		//big indian
		if (prop->guitype == DGUIT_BIGHEX) {
			ptext = ttext;
			pval = pval+i-1;
			while (i) {
				__sprintf(ptext, TEXT("%02X"), *pval);
				ptext += 2;
				pval--;
				i--;
			}
		}
/*
		//little indian (bin)
		else if (prop->guitype == DGUIT_BIN) {
			ptext = ttext;
			while (i) {
				__sprintf(ptext, TEXT("%08b"), *pval);
				ptext += 2;
				pval++;
				i--;
			}
		}
*/
		//little indian
		else {
			ptext = ttext;
			while (i) {
				__sprintf(ptext, TEXT("%02X"), *pval);
				ptext += 2;
				pval++;
				i--;
			}
		}
/*
		if (prop->guitype == DGUIT_BIN)
			__strcat(ptext, TEXT("b"));
		else
*/
			__strcat(ptext, TEXT("h"));
		return(ttext);
	}
	else if (prop->guitype == DGUIT_SMACTION) {
		//action
		i = D_CFLAG_AGET(prop->val.u);
		if (i >= D_CB_ACTION_MAX)
			i = D_CB_ACTION_MAX-1;
		//trigger
		n = D_CFLAG_TGET(prop->val.u);
		if (n)
			n--;
		if (n >= D_CB_TRIGER_MAX)
			n = D_CB_TRIGER_MAX-1;
		__sprintf(ttext, TEXT("%s (%s)"), D_ram(c_strSMActions[i]), D_ram(c_strSMTriggers[n]));
		return(ttext);
	}
	else if (prop->guitype == DGUIT_TABLE && prop->type == DT_MEMORY && prop->guidata) {
		//print g-table
		if (D_GuiPrintTable(ttext, 4096, prop->guidata, prop->val.m.buf, prop->val.m.size, 0))
			ptext = ttext;
		else
			ptext = NULL;
		return(ptext);
	}
	else if (prop->guitype == DGUIT_SHOWMEM && prop->type == DT_MEMORY && prop->guidata) {
		//print g-table
		if (D_GuiPrintShowMem(ttext, 4096, prop->guidata, prop->val.m.buf, prop->val.m.size))
			ptext = ttext;
		else
			ptext = NULL;
		return(ptext);
	}
	else {
		switch (prop->type) {
			case DT_BOOL:
				__sprintf(ttext, TEXT("%s"), prop->val.b ? D_("On") : D_("Off"));
				break;
			case DT_INT: __sprintf(ttext, TEXT("%d"), prop->val.i); break;
			case DT_UINT: __sprintf(ttext, TEXT("%u"), prop->val.u); break;
			case DT_DOUBLE:
				nval.d = prop->val.d;
				D_ConvertValueToDemandUnit(prop, &nval.d);
				//precision
				if (DFLAG_GETPREC(prop->flags) < 6)
					__sprintf(ttext, TEXT("%.*lf"), DFLAG_GETPREC(prop->flags), nval.d);
				else
					__sprintf(ttext, TEXT("%g"), nval.d);
				break;
			case DT_INT64: __sprintf(ttext, TEXT("%I64d"), prop->val.i64); break;
			case DT_UINT64: __sprintf(ttext, TEXT("%0I64u"), prop->val.u64); break;
			case DT_HANDLE: __sprintf(ttext, TEXT("%u"), (unsigned)prop->val.h); break;
			case DT_MEMORY:
				*ttext = '\0';
				if (prop->val.m.buf) {
					__strcat(ttext, TEXT(" \'"));
					for (j=0; j<D_MAX_SHOW_DTMEMORY && j<prop->val.m.size; j++) {
						__sprintf(ttext, TEXT("%s%02X"), ttext, prop->val.m.buf[j]);
					}
					if (prop->val.m.size > D_MAX_SHOW_DTMEMORY)
						__strcat(ttext, TEXT("..."));
					__strcat(ttext, TEXT("\'"));
				}
				if (prop->val.m.size > D_MAX_SHOW_DTMEMORY) {
					__sprintf(ttext, TEXT("%s (%u)"), ttext, prop->val.m.size);
				}
				break;
			case DT_STRING:
				__sprintf(ttext, TEXT("\'%s\'"), prop->val.m.buf ? (TCHAR *)prop->val.m.buf : TEXT(""));
				break;
			default: return(NULL);
		}
		return(ttext);
	}

	return(NULL);
}

//*** function returns gui data string by actual prop. value
//note: return NULL when no such data
const TCHAR *D_GetGuiData(D_DEV_PROP *prop)
{
	return(D_GetGuiDataValue(prop, prop->val));
}



//------------------------------------

//*** function creates param. control according to property
BOOL D_CreateParamGroup(HWND hwnd, TCHAR *title, int col, int row, int col_size, int row_size)
{
	HWND hctl;

	//--- groupbox
	hctl = CreateWindow(CLASS_BUTTON, title ? title : cstr_empty, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
											D_PA_COLGP(col), D_PA_ROWGP(row), D_PA_GW(col_size), D_PA_GH(row_size),
											hwnd, (HMENU)(D_OL_IDGB+col*25+row), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	return(TRUE);
}

//*** function creates param. control according to property
BOOL D_CreateParamCtrlParam(HWND hwnd, WORD pidx, D_DEVICE *pD, D_CTRLSTYLE style, TCHAR *title, int col, int row)
{
	HWND hctl;
	D_DEV_PROP *prop;
	TCHAR ttext[128];
	int i, cols = 1, rows = 1;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	//cols
	if (col & D_GUICTRL_RC_MASK) {
		cols = (col & D_GUICTRL_RC_MASK) >> 24;
	}
	col &= ~D_GUICTRL_RC_MASK;
	//rows
	if (row & D_GUICTRL_RC_MASK) {
		rows = (row & D_GUICTRL_RC_MASK) >> 24;
	}
	row &= ~D_GUICTRL_RC_MASK;

	//prop. gui style
	if (style == D_CTRLSTYLE_PROP) {
		switch (prop->guitype) {
			case DGUIT_COMBOBOX: style = D_CTRLSTYLE_ST_CB; break;
			case DGUIT_SWITCH: style = D_CTRLSTYLE_ST_CB; break;
			case DGUIT_BUTTON: style = D_CTRLSTYLE_BT; break;
			case DGUIT_RANGE: style = D_CTRLSTYLE_RNG; break;
			case DGUIT_SMACTION: style = D_CTRLSTYLE_SMACT; break;
			//case DGUIT_RADIOSW: style = D_CTRLSTYLE_ST_RS; break;
			case DGUIT_TABLE: style = D_CTRLSTYLE_TAB; break;
			default: style = D_CTRLSTYLE_ST_EB; break;
		}
	}

	//--- static
	if (style != D_CTRLSTYLE_BT && style != D_CTRLSTYLE_TAB) {
    if (title)
      __sprintf(ttext, TEXT("%s:"), title);
    else
      D_MakePropLabel(prop, ttext, 0x3);
		hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE,
												D_PA_COLST(col), D_PA_ROWST(row), D_PA_W1, D_PA_H,
												hwnd, (HMENU)D_GETOLID_ST(pidx), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
	}

	//--- combobox
	if (style == D_CTRLSTYLE_ST_CB) {
		hctl = CreateWindow(CLASS_COMBOBOX, cstr_empty,
												WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
												D_PA_COLVAL(col), D_PA_ROWVAL(row), D_PA_W2*cols, D_PA_HCB,
												hwnd, (HMENU)D_GETOLID_VAL(pidx), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		SendMessage(hctl, CB_SETITEMHEIGHT, -1, D_PA_HCBI);
		D_PrepareCombobox(hctl, prop);
	}
	//--- check button
	else if (style == D_CTRLSTYLE_BT) {
    if (title)
      lstrcpyn(ttext, title, 128);
    else
      D_MakePropLabel(prop, ttext, 0x1);
		hctl = CreateWindow(CLASS_BUTTON, ttext, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
												D_PA_COLST(col), D_PA_ROWVAL(row), D_PA_GW1IN, D_PA_HBT,
												hwnd, (HMENU)D_GETOLID_VAL(pidx), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		SendMessage(hctl, BM_SETCHECK, (WPARAM)(prop->val.b), 0);
	}
	//--- push button
	else if (style == D_CTRLSTYLE_PBT) {
    if (title)
      lstrcpyn(ttext, title, 128);
    else
      D_MakePropLabel(prop, ttext, 0x1);
		hctl = CreateWindow(CLASS_BUTTON, ttext, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
												D_PA_COLST(col), D_PA_ROWVAL(row), D_PA_GW1IN, D_PA_HBT,
												hwnd, (HMENU)D_GETOLID_VAL(pidx), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
	}
	//--- range
	else if (style == D_CTRLSTYLE_RNG) {
		//low
		hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, cstr_empty,
													WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT,
													D_PA_COLVAL(col), D_PA_ROWVAL(row), D_PA_W2W, D_PA_HEB,
													hwnd, (HMENU)D_GETOLID_VAL(pidx), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		D_PrepareRangeValidNumSubclassing(hctl, 0, prop);
		//static
		hctl = CreateWindow(CLASS_STATIC, TEXT("---"),
												WS_CHILD | WS_VISIBLE | SS_CENTER,
												D_PA_COLVAL(col)+D_PA_W2W, D_PA_ROWST(row), D_PA_W2S, D_PA_H,
												hwnd, (HMENU)0x7FFF /* IDC_STATIC*/, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//high
		hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, cstr_empty,
													WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT,
													D_PA_COLVAL(col)+D_PA_W2W+D_PA_W2S, D_PA_ROWVAL(row), D_PA_W2W, D_PA_HEB,
													hwnd, (HMENU)D_GETOLID_VAL2(pidx), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		D_PrepareRangeValidNumSubclassing(hctl, 1, prop);
	}
	else if(style == D_CTRLSTYLE_SMACT) {
		//action
		#define D_SMACT_WICON 42
		hctl = CreateWindow(CLASS_COMBOBOX, cstr_empty,
												WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
												D_PA_COLVAL(col), D_PA_ROWVAL(row), D_PA_W2-D_SMACT_WICON-1, D_PA_HCB,
												hwnd, (HMENU)D_GETOLID_VAL(pidx), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//SendMessage(hctl, CB_SETITEMHEIGHT, -1, D_PA_HCBI);
		SendMessage(hctl, CB_RESETCONTENT, 0, 0);
		for (i=0; i<D_CB_ACTION_MAX; i++) {
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)D_ram(c_strSMActions[i]));
		}
		//trigger
		hctl = CreateWindow(CLASS_COMBOBOX, cstr_empty,
												WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_OWNERDRAWFIXED,
												D_PA_COLVAL(col)+D_PA_W2-D_SMACT_WICON, D_PA_ROWVAL(row), D_SMACT_WICON, D_PA_HCB,
												hwnd, (HMENU)D_GETOLID_UD(pidx), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//SendMessage(hctl, CB_SETITEMHEIGHT, -1, D_PA_HCBI);
		SendMessage(hctl, CB_RESETCONTENT, 0, 0);
		for (i=0; i<D_CB_TRIGER_MAX; i++) {
			SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)ImageList_GetIcon(g_hImgList, i, ILD_TRANSPARENT	));
		}
		D_SetParamCtrl(hwnd, pidx, pD);
	}
	//--- table
	else if (style == D_CTRLSTYLE_TAB) {
		D_CreateParamTable(hwnd, D_GETOLID_VAL(pidx), NULL, prop->guidata, cols, rows, col, row);
		D_SetParamTable(hwnd, pidx, pD);
	}
	//--- editbox
	else {
		//left/right
		if (style == D_CTRLSTYLE_ST_EBL)
			i = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT;
		else
			i = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT;

		hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, cstr_empty,
													i,
													D_PA_COLVAL(col), D_PA_ROWVAL(row), D_PA_W2, D_PA_HEB,
													hwnd, (HMENU)D_GETOLID_VAL(pidx), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		if (prop->type == DT_STRING) {
			if (prop->val.m.buf)
				SetWindowText(hctl, (TCHAR *)prop->val.m.buf);
		}
		else {
			D_PrepareValidNumSubclassing(hctl, prop);
		}
	}

	return(TRUE);
}

//*** function destroys param. control according to property
BOOL D_DestroyParamCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD)
{
	HWND hctl;
	D_DEV_PROP *prop;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

 	hctl = D_GETOLH_VAL(hwnd, pidx);
 	if (hctl == NULL)
    return(FALSE);

	if (CompareWindowClass(hctl, CLASS_EDIT)) {
    //clear subclassing
    ClearValidNumStrSubclassing(hctl);

    //range subclassing
    if (prop->guitype == DGUIT_RANGE) {
			ClearValidNumStrSubclassing(D_GETOLH_VAL2(hwnd, pidx));
    }
  }
  else if (prop->guitype == DGUIT_TABLE) {
		//table
		D_DestroyParamTable(hwnd, pidx, pD);
	}


	return(TRUE);
}

//*** function sets parameter to control
BOOL D_SetParamCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD)
{
	HWND hctl;
	D_DEV_PROP *prop;
	U_NVAL nval;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

 	hctl = D_GETOLH_VAL(hwnd, pidx);
 	if (hctl == NULL)
    return(FALSE);

	//sm-action
  if (prop->guitype == DGUIT_SMACTION) {
  	//action
		SendMessage(hctl, CB_SETCURSEL, D_CFLAG_AGET(prop->val.u), 0);
		//trigger
		nval.u = D_CFLAG_TGET(prop->val.u);
		if (nval.u)
			nval.u--;
		SendMessage(GetDlgItem(hwnd, D_GETOLID_UD(pidx)), CB_SETCURSEL, nval.u, 0);
  }
 	//combobox
  else if (CompareWindowClass(hctl, CLASS_COMBOBOX)) {
    SendMessage(hctl, CB_SETCURSEL, prop->val.u, 0);
  }
  //button
  else if (CompareWindowClass(hctl, CLASS_BUTTON)) {
    SendMessage(hctl, BM_SETCHECK, prop->val.b ? BST_CHECKED : BST_UNCHECKED, 0);
  }
  //range
  else if (prop->guitype == DGUIT_RANGE) {
  	//low
  	nval.u = prop->val.u & 0xFFFF;
  	SetEditUval(hctl, nval);
  	//high
  	nval.u = prop->val.u >> 16;
  	SetEditUval(D_GETOLH_VAL2(hwnd, pidx), nval);
  }
  //table
  else if (prop->guitype == DGUIT_TABLE) {
		return(D_SetParamTable(hwnd, pidx, pD));
  }
  //edit
  else {
    switch (prop->type) {
      case DT_BOOL: nval.i = prop->val.b; break;
      default:
      case DT_INT: nval.i = prop->val.i; break;
      case DT_UINT: nval.u = prop->val.u; break;
      case DT_DOUBLE:
      	nval.d = prop->val.d;
				//convert to demand
				D_ConvertValueToDemandUnit(prop, &nval.d);
      	break;
      case DT_HANDLE: nval.u = (unsigned int)prop->val.h; break;
    }
    SetEditUval(hctl, nval);
  }
  return(TRUE);
}

//*** function gets parameter from control
BOOL D_GetParamCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD)
{
	HWND hctl;
	D_DEV_PROP *prop;
	U_NVAL nval;
	int i;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

 	hctl = D_GETOLH_VAL(hwnd, pidx);
 	if (hctl == NULL)
    return(FALSE);

	//sm-action
  if (prop->guitype == DGUIT_SMACTION) {
  	//action
		i = SendMessage(hctl, CB_GETCURSEL, 0, 0);
    if (i == CB_ERR)
      i = 0;
		prop->val.u = D_CFLAG_ASET(i);
		//trigger
		i = SendMessage(GetDlgItem(hwnd, D_GETOLID_UD(pidx)), CB_GETCURSEL, 0, 0);
    if (i == CB_ERR)
      i = 0;
		prop->val.u |= D_CFLAG_TSET(i+1);
  }
 	//combobox
  else if (CompareWindowClass(hctl, CLASS_COMBOBOX)) {
    i = SendMessage(hctl, CB_GETCURSEL, 0, 0);
    if (i == CB_ERR)
      return(FALSE);
    prop->val.u = i;
  }
  //button
  else if (CompareWindowClass(hctl, CLASS_BUTTON)) {
    prop->val.b = (SendMessage(hctl, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE);
  }
  //range
  else if (prop->guitype == DGUIT_RANGE) {
  	//low
  	nval.u = 0;
  	if (GetEditUval(hctl, &nval) == NO_ERROR)
			prop->val.u = nval.u & 0xFFFF;
  	//high
  	if (GetEditUval(D_GETOLH_VAL2(hwnd, pidx), &nval) == NO_ERROR) {
			prop->val.u |= ((nval.u & 0xFFFF) << 16);
  	}
  }
  //table
  else if (prop->guitype == DGUIT_TABLE) {
		return(D_GetParamTable(hwnd, pidx, pD));
  }
  //edit
  else {
  	//string
  	if (prop->type == DT_STRING) {
  		i = GetWindowTextLength(hctl)+1;
  		//reallocate
			if (prop->val.m.buf)
				free((void *)prop->val.m.buf);
			prop->val.m.buf = malloc(i*sizeof(TCHAR));
			if (prop->val.m.buf) {
				//read text
				GetWindowText(hctl, (TCHAR *)prop->val.m.buf, i);
				prop->val.m.size = i*sizeof(TCHAR);
			}
  	}
  	else {
			if (GetEditUval(hctl, &nval) != NO_ERROR)
				return(FALSE);
			switch (prop->type) {
				case DT_BOOL: prop->val.b = nval.i; break;
				default:
				case DT_INT: prop->val.i = nval.i; break;
				case DT_UINT: prop->val.u = nval.u; break;
				case DT_DOUBLE:
					//convert to origin
					D_ConvertValueToOriginUnit(prop, &nval.d);
					prop->val.d = nval.d;
					break;
				case DT_HANDLE: prop->val.h = (HANDLE)nval.u; break;
			}
  	}
	}
  return(TRUE);
}

//*** function sets all parameters to control
BOOL D_SetAllParamCtrls(HWND hwnd, D_DEVICE *pD)
{
	int p;

	if (pD == NULL)
		return(FALSE);

  if (pD->p) {
    for (p=0; p<pD->n_p; p++) {
      if ((pD->p[p].flags & DFLAG_CONF) == DFLAG_CONF)
        D_SetParamCtrl(hwnd, p, pD);
    }
  }
  return(TRUE);
}

//*** function gets all parameters from control
BOOL D_GetAllParamCtrls(HWND hwnd, D_DEVICE *pD)
{
	int p;

	if (pD == NULL)
		return(FALSE);

  if (pD->p) {
    for (p=0; p<pD->n_p; p++) {
      if ((pD->p[p].flags & DFLAG_CONF) == DFLAG_CONF)
        D_GetParamCtrl(hwnd, p, pD);
    }
  }
  return(TRUE);
}

//*** function destroys all parameters
BOOL D_DestroyAllParamCtrls(HWND hwnd, D_DEVICE *pD)
{
	int p;

	if (pD == NULL)
		return(FALSE);

  if (pD->p) {
    for (p=0; p<pD->n_p; p++) {
      D_DestroyParamCtrl(hwnd, p, pD);
    }
  }
  return(TRUE);
}


//*** function checks and validate range property controls
BOOL D_CheckValidateParamRangeCtrl(HWND hwnd, int level, WORD pidx, D_DEVICE *pD)
{
	HWND hctl;
	D_DEV_PROP *prop;
	U_NVAL nval;
	unsigned short low;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];
	if (prop->guitype != DGUIT_RANGE)		//valid gui
		return(FALSE);
	if ((prop->flags & DFLAG_MINMAX) == 0)	//if not min or max -> nothing to do
		return(TRUE);

	//get low
 	hctl = D_GETOLH_VAL(hwnd, pidx);
 	if (hctl == NULL || GetEditUval(hctl, &nval) != NO_ERROR)
    return(FALSE);
	low = nval.u;
	//get high
 	hctl = GetDlgItem(hwnd, D_GETOLID_ON(pidx));
 	if (hctl == NULL || GetEditUval(hctl, &nval) != NO_ERROR)
    return(FALSE);

	//low > high
	if (low > nval.u) {
		if (level == 0) {
			//corect high
			nval.u = low;
			SetEditUval(hctl, nval);
		}
		else {
			//correct low
			hctl = D_GETOLH_VAL(hwnd, pidx);
			SetEditUval(hctl, nval);
		}
		return(FALSE);
	}

	//ok
	return(TRUE);
}

//*** function creates check button
BOOL D_CreateParamCheckBox(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int col, int row)
{
	HWND hctl;
	int bwidth;

  if (title == NULL)
    title = cstr_empty;


	bwidth = D_PA_GW1IN/bt_per_row;

	hctl = CreateWindow(CLASS_BUTTON, title, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
											D_PA_COLST(col)+(index%bt_per_row)*bwidth, D_PA_ROWVAL(row), bwidth, D_PA_HBT,
											hwnd, (HMENU)(id), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	return(TRUE);
}

//*** function creates bits check buttons (using guidata)
BOOL D_CreateParamCheckBoxBits(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id, int bt_per_row, int col, int row)
{
	D_DEV_PROP *prop;
	int i, n;
	TCHAR ttext[128], *pdata, *ptext;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	//count data
	if (prop->guidata) {
		//count guidata string
		n = 0;
		pdata = (TCHAR *)prop->guidata;
		while (*pdata) {
			n++;
			pdata += lstrlen(pdata)+1;
		}
	}
	else if (prop->max.u > 0 && prop->max.u <= 32) {
		//by max. value
		n = prop->max.u;
	}
	else
		return(FALSE);

	//create buttons
	pdata = prop->guidata;
	for (i=0; i<n; i++) {
		//name
		if (pdata) {
			ptext = D_ram(pdata);
			pdata += __strlen(pdata)+1;
		}
		else {
			__sprintf(ttext, TEXT("%d"), i);
			ptext = ttext;
		}
		D_CreateParamCheckBox(hwnd, start_id+i, D_ram(ptext), i, bt_per_row, col, row+i/bt_per_row);
		CheckDlgButton(hwnd, start_id+i, prop->val.u & (1<<i) ? TRUE : FALSE);
	}

	return(TRUE);
}

//*** function sets check buttons (using guidata)
BOOL D_SetParamCheckBoxBits(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id)
{
	D_DEV_PROP *prop;
	int i, n;
	TCHAR *pdata;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	//count data
	if (prop->guidata) {
		//count guidata string
		n = 0;
		pdata = (TCHAR *)prop->guidata;
		while (*pdata) {
			n++;
			pdata += lstrlen(pdata)+1;
		}
	}
	else if (prop->max.u > 0 && prop->max.u <= 32) {
		//by max. value
		n = prop->max.u;
	}
	else
		return(FALSE);

	//create buttons
	for (i=0; i<n; i++) {
		CheckDlgButton(hwnd, start_id+i, prop->val.u & (1<<i) ? TRUE : FALSE);
	}
	return(TRUE);
}

//*** function reads check buttons (using guidata)
BOOL D_GetParamCheckBoxBits(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id, unsigned int *uval)
{
	D_DEV_PROP *prop;
	int i, n, val;
	TCHAR *pdata;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	//count data
	if (prop->guidata) {
		//count guidata string
		n = 0;
		pdata = (TCHAR *)prop->guidata;
		while (*pdata) {
			n++;
			pdata += lstrlen(pdata)+1;
		}
	}
	else if (prop->max.u > 0 && prop->max.u <= 32) {
		//by max. value
		n = prop->max.u;
	}
	else
		return(FALSE);

	//create buttons
	val = 0;
	for (i=0; i<n; i++) {
		if (IsDlgButtonChecked(hwnd, start_id+i))
			val |= 1<<i;
	}

	prop->val.u = val;

	//output
	if (uval) {
		*uval = val;
	}

	return(TRUE);
}

//---

//*** function creates radio button
BOOL D_CreateParamRadioBox(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int col, int row)
{
	HWND hctl;
	int bwidth;

  if (title == NULL)
    title = cstr_empty;


	bwidth = D_PA_GW1IN/bt_per_row;

	hctl = CreateWindow(CLASS_BUTTON, title, WS_CHILD | WS_VISIBLE | WS_TABSTOP /*| BS_RADIOBUTTON*/ | BS_AUTORADIOBUTTON,
											D_PA_COLST(col)+(index%bt_per_row)*bwidth, D_PA_ROWVAL(row), bwidth, D_PA_HBT,
											hwnd, (HMENU)(id), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	return(TRUE);
}

//*** function creates radio buttons (using guidata)
BOOL D_CreateParamRadioBoxes(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id, int bt_per_row, int col, int row)
{
	D_DEV_PROP *prop;
	int i, n;
	TCHAR ttext[128], *pdata, *ptext;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	//count data
	if (prop->guidata) {
		//count guidata string
		n = 0;
		pdata = (TCHAR *)prop->guidata;
		while (*pdata) {
			n++;
			pdata += lstrlen(pdata)+1;
		}
		if (n)
			n--;
	}
	else if (prop->max.u > 0 && prop->max.u <= 32) {
		//by max. value
		n = prop->max.u;
	}
	else
		return(FALSE);

	//limit
	if ((prop->flags & DFLAG_MAX) &&  n > prop->max.u) {
		n = prop->max.u;
	}

	//create buttons
	pdata = prop->guidata;
	for (i=0; i<=n; i++) {
		//name
		if (pdata) {
			ptext = D_ram(pdata);
			pdata += __strlen(pdata)+1;
		}
		else {
			__sprintf(ttext, TEXT("%d"), i);
			ptext = ttext;
		}
		D_CreateParamRadioBox(hwnd, start_id+i, D_ram(ptext), i, bt_per_row, col, row+i/bt_per_row);
		CheckDlgButton(hwnd, start_id+i, prop->val.u == i ? TRUE : FALSE);
	}

	return(TRUE);
}

//*** function sets radio buttons (using guidata)
BOOL D_SetParamRadioBoxes(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id)
{
	D_DEV_PROP *prop;
	int n;
	TCHAR *pdata;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	//count data
	if (prop->guidata) {
		//count guidata string
		n = 0;
		pdata = (TCHAR *)prop->guidata;
		while (*pdata) {
			n++;
			pdata += lstrlen(pdata)+1;
		}
		if (n)
			n--;
	}
	else if (prop->max.u > 0 && prop->max.u <= 32) {
		//by max. value
		n = prop->max.u;
	}
	else
		return(FALSE);

	//limit
	if ((prop->flags & DFLAG_MAX) &&  n > prop->max.u) {
		n = prop->max.u;
	}

	//check buttons
	CheckRadioButton(hwnd, start_id+0, start_id+n, start_id+prop->val.u);

	return(TRUE);
}

//*** function reads radio buttons (using guidata)
BOOL D_GetParamRadioBoxes(HWND hwnd, WORD pidx, D_DEVICE *pD, int start_id, unsigned int *uval)
{
	D_DEV_PROP *prop;
	int i, n, val;
	TCHAR *pdata;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	//count data
	if (prop->guidata) {
		//count guidata string
		n = 0;
		pdata = (TCHAR *)prop->guidata;
		while (*pdata) {
			n++;
			pdata += lstrlen(pdata)+1;
		}
		if (n)
			n--;
	}
	else if (prop->max.u > 0 && prop->max.u <= 32) {
		//by max. value
		n = prop->max.u;
	}
	else
		return(FALSE);

	//limit
	if ((prop->flags & DFLAG_MAX) &&  n > prop->max.u) {
		n = prop->max.u;
	}

	//create buttons
	val = 0;
	for (i=0; i<=n; i++) {
		if (IsDlgButtonChecked(hwnd, start_id+i))
			val = i;
	}

	prop->val.u = val;

	//output
	if (uval) {
		*uval = val;
	}

	return(TRUE);
}

/** @brief Create dev. param table
 *
 * @param hwnd HWND
 * @param id int
 * @param title TCHAR*
 * @param format TCHAR*
 * @param show_rows int
 * @param col int
 * @param row int
 * @return BOOL
 *
 */
BOOL D_CreateParamTable(HWND hwnd, int id, TCHAR *title, TCHAR *format, int cols, int show_rows, int col, int row)
{
	HWND hctl;
	int width;
	unsigned int i, n_r = 20;
	D_GUITABLE guitable;
	double scalex, scaley;

  if (title == NULL)
    title = cstr_empty;

	//get screen scaling
	GetScreenScaling(&scalex, &scaley);

	//width
	width = D_PA_GWIN(cols);

	//table
	hctl = CreateWindow(MGC_TABLECLASSNAME, title, WS_CHILD | WS_VISIBLE | MGT_TAS_NOTIFY|MGT_TAS_SYSKEYDOWN,
											D_PA_COLST(col), D_PA_ROWVAL(row), width, show_rows*D_PA_H, hwnd, (HMENU)id, NULL, NULL);
	SendMessage(hctl, MGT_TAM_SETPARAM, MGT_TPAR_BORDER_BGCOLOR, (LPARAM)MGC_COLOR_BNTFACE);		//bg. color
//	SendMessage(hctl, MGT_TAM_SETPARAM, MGT_TPAR_BORDER_VISIBLE, (LPARAM)FALSE);		//bg. off
	SendMessage(hctl, MGT_TAM_SETPARAM, MGT_TPAR_BORDER_SPACE, (LPARAM)0);		//bg. space=0
	SendMessage(hctl, MGT_TAM_SETPARAM, MGT_TPAR_TITLE_VISIBLE, (LPARAM)FALSE);		//title off
	SendMessage(hctl, MGT_TAM_SETPARAM, MGT_TPAR_MODE, (LPARAM)MGT_MODE_EDITBYTAB);		//move by tab after edit
	SendMessage(hctl, MGT_TAM_SETPARAM, MGT_TPAR_EMPHASIS_ROWMODE, (LPARAM)1);		//emphasis rows

	//reset
	SendMessage(hctl, MGT_TAM_RESETCONTENT, 0, 0);

	//decode format
	if (D_GuiDecodeTable(format, &guitable)) {
		//init table
		SendMessage(hctl, MGT_TAM_INSERTCOLS, 0, guitable.count);		//number of columns
		SendMessage(hctl, MGT_TAM_INSERTROWS, 0, n_r);		//number of rows

		//column header
		MGTCHEADERPARAMS cheader;
		memset(&cheader, 0, sizeof(MGTCHEADERPARAMS));
		cheader.mask = MGT_CHEADERMASK_VIS |
									 MGT_CHEADERMASK_RESIZE |
									 MGT_CHEADERMASK_SEL |
									 MGT_CHEADERMASK_HEIGHT |
									 MGT_CHEADERMASK_COLMASK;
		cheader.visible = TRUE;
		cheader.selectable = FALSE;
		cheader.resizeable = FALSE;
		cheader.height = SY(16) * guitable.chdr_lines;
		cheader.colmask = MGT_COLMASK_LABEL | MGT_COLMASK_WIDTH;
		SendMessage(hctl, MGT_TAM_SETCOLHEADER, 0, (LPARAM)&cheader);

		//row header
		MGTRHEADERPARAMS rheader;
		SendMessage(hctl, MGT_TAM_GETROWHEADER, 0, (LPARAM)&rheader);
		rheader.mask = MGT_RHEADERMASK_VIS |
									 MGT_RHEADERMASK_SEL |
								   MGT_CHEADERMASK_RESIZE |
								   MGT_RHEADERMASK_WIDTH |
									 MGT_RHEADERMASK_FONT |
									 MGT_RHEADERMASK_ROWMASK;
		rheader.visible = TRUE;
		rheader.selectable = FALSE;
		rheader.resizeable = FALSE;
		rheader.width = 30;
		rheader.font.tflag = MGC_FONT_NORMAL;
		rheader.rowmask = MGT_ROWMASK_LABEL;
		SendMessage(hctl, MGT_TAM_SETROWHEADER, 0, (LPARAM)&rheader);
		//set row names
		MGTRHEADPARAMS rhead;
		for (i=0; i<n_r; i++) {
			rhead.mask = MGT_RHEADMASK_LABEL;
			if (guitable.row_id)
				__sprintf(rhead.label, TEXT("%c"), i+guitable.row_id);
			else
				__sprintf(rhead.label, TEXT("%d"), i+1);
			SendMessage(hctl, MGT_TAM_SETROWHEAD, i, (LPARAM)&rhead);		//
		}
		width -= rheader.width;
		width -= 18;

		//set columns
		MGTCHEADPARAMS chead;
		for (i=0; i<guitable.count; i++) {
			memset(&chead, 0, sizeof(MGTCHEADPARAMS));
			chead.mask = MGT_CHEADMASK_LABEL |
									MGT_CHEADMASK_WIDTH |
									MGT_CHEADMASK_COLOR |
									MGT_CHEADMASK_SPEC |
									MGT_CHEADMASK_LPARAM;
			lstrcpyn(chead.label, guitable.col[i].name, MGT_CHEADLABEL_LEN-8);
			if (*guitable.col[i].unit) {
				__sprintf(chead.label, TEXT("%s%c[%.*s]"), chead.label, guitable.chdr_lines ? '\n' : ' ', 6, guitable.col[i].unit);
			}
			chead.width = width/guitable.count;
			chead.color = MGC_COLOR_BLACK;
			chead.spec.type = MGC_TYPE_DOUBLE;
			chead.spec.align = MGC_VALALIGN_RT;
			chead.spec.prec = guitable.col[i].bytes >= 8 ? guitable.col[i].div : D_GetDivPrecision(guitable.col[i].div);
			chead.spec.mask = MGC_VALMASK_NONE;
			chead.spec.ctr = MGC_CTR_EDITBOX;
			chead.lparam = 0;

			//cb-data
			while (guitable.col[i].cb) {
				MGTCTRCBDATA *pcbdata;
				pcbdata = (MGTCTRCBDATA*)malloc(sizeof(MGTCTRCBDATA));
				if (pcbdata == NULL)
					break;
				memset(pcbdata, 0, sizeof(MGTCTRCBDATA));
				unsigned int n, j;
				n = CountSubStrings(guitable.col[i].cb);
				//alloc items
				pcbdata->item = malloc(n*sizeof(MGTCTRCBITEM));
				if (pcbdata->item == NULL) {
					free((void*)pcbdata);
					break;
				}
				memset(pcbdata->item, 0, n*sizeof(MGTCTRCBITEM));
				pcbdata->count = n;
				//through items
				TCHAR *pcb = guitable.col[i].cb;
				int len;
				for (j=0; j<n; j++) {
					len = __strlen(pcb);
					pcbdata->item[j].val.d = (double)j;
					pcbdata->item[j].hicon = NULL;
					pcbdata->item[j].name = malloc((len+1)*sizeof(TCHAR));
					if (pcbdata->item[j].name) {
						__strncpy(pcbdata->item[j].name, pcb, len);
						pcbdata->item[j].name[len] = '\0';
					}
					pcb += len+1;
				}
				chead.spec.align = MGC_VALALIGN_CT;
				chead.spec.ctr = MGC_CTR_COMBOBOX;
				chead.spec.data = pcbdata;
				chead.lparam = (LPARAM)pcbdata;

				break;	//only once
			}

			SendMessage(hctl, MGT_TAM_SETCOLHEAD, i, (LPARAM)&chead);
		}

		D_GuiDestroyTable(&guitable);
	}

	return(TRUE);
}

/** @brief Set dev. param. table
 *
 * @param hwnd HWND
 * @param pidx WORD
 * @param pD D_DEVICE*
 * @return BOOL
 *
 */
BOOL D_SetParamTable(HWND hwnd, WORD pidx, D_DEVICE *pD)
{
	D_DEV_PROP *prop;
	D_GUITABLE guitable;
	HWND hctl;
	int n_r, c, r;
	DPOINT p;
	MGTCELL cell;
	int val;
	double dval;
	unsigned char *pdata;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	if (prop->type != DT_MEMORY ||
			prop->guitype != DGUIT_TABLE ||
			prop->guidata == NULL ||
			prop->val.m.buf == NULL) {
		return(FALSE);
	}

	//handle
	hctl = D_GETOLH_VAL(hwnd, pidx);

	//decode format
	if (D_GuiDecodeTable(prop->guidata, &guitable)) {
		//rows
		n_r = prop->val.m.size / guitable.rsize;

		//set rows
		SendMessage(hctl, MGT_TAM_DELETEROWS, 0, -1);
		SendMessage(hctl, MGT_TAM_INSERTROWS, 0, n_r);		//number of rows

		//set row names
		if (guitable.row_id) {
			MGTRHEADPARAMS rhead;
			for (r=0; r<n_r; r++) {
				rhead.mask = MGT_RHEADMASK_LABEL;
				__sprintf(rhead.label, TEXT("%c"), r+guitable.row_id);
				SendMessage(hctl, MGT_TAM_SETROWHEAD, r, (LPARAM)&rhead);
			}
		}

		//fill rows
		pdata = prop->val.m.buf;
		for (r=0; r<n_r; r++) {
			//through columns
			for (c=0; c<guitable.count; c++) {
				if (guitable.col[c].bytes >= 8) {
					dval = 0.0;
					memcpy(&dval, pdata, 8);
				}
				else {
					val = 0;
					memcpy(&val, pdata, guitable.col[c].bytes>4 ? 4 : guitable.col[c].bytes);
					if (guitable.col[c].div)
						dval = (double)val / (double)guitable.col[c].div;
					else
						dval = (double)val;
				}

				memset(&cell, 0, sizeof(MGTCELL));
				p.x = c;
				p.y = r;
				cell.val.d = dval;
				SendMessage(hctl, MGT_TAM_SETCELL, (WPARAM)&p, (LPARAM)&cell);

				pdata += guitable.col[c].bytes;
			}
		}

		//clear memory
		D_GuiDestroyTable(&guitable);

		//refresh
		SendMessage(hctl, MGT_TAM_REFRESH, 0, 0);
	}


	return(TRUE);
}

/** @brief Get dev. param table
 *
 * @param hwnd HWND
 * @param pidx WORD
 * @param pD D_DEVICE*
 * @return BOOL
 *
 */
BOOL D_GetParamTable(HWND hwnd, WORD pidx, D_DEVICE *pD)
{
	D_DEV_PROP *prop;
	D_GUITABLE guitable;
	HWND hctl;
	int n_r, c, r;
	DPOINT p;
	MGTCELL cell;
	int val;
	unsigned char *pdata;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	if (prop->type != DT_MEMORY ||
			prop->guitype != DGUIT_TABLE ||
			prop->guidata == NULL ||
			prop->val.m.buf == NULL) {
		return(FALSE);
	}

	//handle
	hctl = D_GETOLH_VAL(hwnd, pidx);

	//decode format
	if (D_GuiDecodeTable(prop->guidata, &guitable)) {
		//rows
		n_r = prop->val.m.size / guitable.rsize;

		//get rows/columns
		r = 0;
		c = 0;
		SendMessage(hctl, MGT_TAM_GETROWSCOLS, (WPARAM)&r, (LPARAM)&c);
		//check
		if (r < n_r)
			n_r = r;
		if (c < guitable.count)
			guitable.count = c;

		//read rows
		pdata = prop->val.m.buf;
		for (r=0; r<n_r; r++) {
			//through columns
			for (c=0; c<guitable.count; c++) {
				memset(&cell, 0, sizeof(MGTCELL));
				p.x = c;
				p.y = r;
				SendMessage(hctl, MGT_TAM_GETCELL, (WPARAM)&p, (LPARAM)&cell);

				if (guitable.col[c].bytes >= 8) {
					memcpy(pdata, &cell.val.d, 8);
				}
				else {
					if (guitable.col[c].div > 1)
						val = (int)(cell.val.d * (double)guitable.col[c].div);
					else
						val = (int)(cell.val.d);
					memcpy(pdata, &val, guitable.col[c].bytes>4 ? 4 : guitable.col[c].bytes);
				}

				pdata += guitable.col[c].bytes;
			}
		}

		//clear memory
		D_GuiDestroyTable(&guitable);
	}

	return(TRUE);
}

/** @brief Set min/max limits for table column
 *
 * @param hwnd HWND
 * @param pidx WORD
 * @param cidx int
 * @param min_val double
 * @param max_val double
 * @return BOOL
 *
 */
BOOL D_ModifyParamTableMinMax(HWND hwnd, WORD pidx, int cidx, double min_val, double max_val)
{
	HWND hctl;
	int c, r;

	//handle
	hctl = D_GETOLH_VAL(hwnd, pidx);

	//get rows/columns
	r = 0;
	c = 0;
	SendMessage(hctl, MGT_TAM_GETROWSCOLS, (WPARAM)&r, (LPARAM)&c);

	MGTCHEADPARAMS chead;
	if (SendMessage(hctl, MGT_TAM_GETCOLHEAD, (WPARAM)cidx, (LPARAM)&chead) == NO_ERROR) {
		chead.mask = MGT_CHEADMASK_SPEC;
		chead.spec.mask = MGC_VALMASK_MINMAX;
		chead.spec.min.d = min_val;
		chead.spec.max.d = max_val;
		SendMessage(hctl, MGT_TAM_SETCOLHEAD, (WPARAM)cidx, (LPARAM)&chead);

		//refresh
		SendMessage(hctl, MGT_TAM_REFRESH, 0, 0);
	}

	return(TRUE);
}


/** @brief Destroy param table (CB data)
 *
 * @param hwnd HWND
 * @param pidx WORD
 * @param pD D_DEVICE*
 * @return BOOL
 *
 */
BOOL D_DestroyParamTable(HWND hwnd, WORD pidx, D_DEVICE *pD)
{
	D_DEV_PROP *prop;
	HWND hctl;
	int c, r, ci;
	DWORD ret;

	if (pD == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	if (prop->guitype != DGUIT_TABLE ||
			prop->guidata == NULL ||
			prop->val.m.buf == NULL) {
		return(FALSE);
	}

	//handle
	hctl = D_GETOLH_VAL(hwnd, pidx);

	//get rows/columns
	r = 0;
	c = 0;
	SendMessage(hctl, MGT_TAM_GETROWSCOLS, (WPARAM)&r, (LPARAM)&c);

	//through column heads
	MGTCHEADPARAMS chead;
	for (ci=0; ci<c; ci++) {
		ret = SendMessage(hctl, MGT_TAM_GETCOLHEAD, (WPARAM)ci, (LPARAM)&chead);
		//search for CB data
		if (ret == NO_ERROR && chead.spec.ctr == MGC_CTR_COMBOBOX && chead.spec.data && chead.lparam == (LPARAM)chead.spec.data) {
			MGTCTRCBDATA *pcbdata = chead.spec.data;
			//remove from table
			chead.spec.ctr = MGC_CTR_EDITBOX;
			chead.spec.data = NULL;
			chead.lparam = 0;
			SendMessage(hctl, MGT_TAM_SETCOLHEAD, (WPARAM)ci, (LPARAM)&chead);
			//free CB memory
			if (pcbdata->count && pcbdata->item) {
				//through items
				for (r=0; r<pcbdata->count; r++) {
					if (pcbdata->item[r].name) {
						free((void*)pcbdata->item[r].name);
						pcbdata->item[r].name = NULL;
					}
				}
				free((void*)pcbdata->item);
				pcbdata->item = NULL;
			}
			free((void*)pcbdata);
		}
	}

	return(TRUE);
}

/** @brief Return bool (switch) text according to boolean value and placement (status/button)
 *
 * @param prop_guidata TCHAR*
 * @param on_button int
 * @param state BOOL
 * @return TCHAR*
 *
 */
TCHAR *D_GetBoolString(TCHAR *prop_guidata, int on_button, BOOL bstate)
{
	TCHAR *ptext = NULL;
	int val;

	val = bstate ? 1 : 0;

	//guidata
	if (prop_guidata && *prop_guidata) {
		//status
		ptext = GetSubString(prop_guidata, val);	//state
		//button
		if (on_button && ptext && lstrlen(ptext) > 4) {	//button and text too long
			ptext = NULL;	//use on/off
		}
	}
	//no or invalid guidata
	if (ptext == NULL || *ptext == '\0')
		ptext = val ? DC_("On") : DC_("Off");	//off/on
	//resutl
	return(D_ram(ptext));
}

//------------------------------------

//*** function creates online group
BOOL D_CreateOnLineGroup(HWND hwnd, TCHAR *title, int pos, int row_size)
{
	HWND hctl;
	int r, c, lof = D_OL_LOF, w;

	//position
	r = pos & 0xFFFF;
	if ((pos & D_OL_POS_C12)) {
		w = D_OL_STW+D_OL_W+2*D_OL_SBW+D_OL_MOF+D_OL_STW+D_OL_W+20;
		c = 0;
	}
	else if ((pos & D_OL_POS_C2)) {
		lof = D_OL_WLOF;
		w = D_OL_STW+D_OL_W+20;
		c = 1;
	}
	else {
		w = D_OL_STW+D_OL_W+2*D_OL_SBW+20;
		c = 0;
	}

	//--- groupbox
	hctl = CreateWindow(CLASS_BUTTON, title ? title : cstr_empty, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
											lof-10, D_OL_TOF+(r-1)*D_OL_HOF, w, D_OL_HOF*row_size+30,
											hwnd, (HMENU)(D_OL_IDGB+c*25+r), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	return(TRUE);
}

//*** function creates operation CB
BOOL D_CreateOnLineOperCB(HWND hwnd, D_DEVICE *pD, int pos)
{
	HWND hctl;
	int i, c, lof = D_OL_LOF;

	//position
	i = pos & 0xFFFF;
	if ((pos & D_OL_POS_C2)) {
		lof = D_OL_WLOF;
		c = 1;
	}
	else
		c = 0;

	if (pD->n_op && pD->op) {
		//group box
		hctl = CreateWindow(CLASS_BUTTON, D_("Make operation"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
												lof-10, D_OL_TOF+(i-1)*D_OL_HOF, D_OL_STW+D_OL_W+2*D_OL_SBW+20, D_OL_HOF*1+30,
												hwnd, (HMENU)(D_OL_IDGB+c*25+i), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//oper. combobox
		hctl = CreateWindow(CLASS_COMBOBOX, cstr_empty,
												WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
												lof, D_OL_TOF+i*D_OL_HOF-1, D_OL_STW+D_OL_W-10, D_CB_DEFHEIGHT,
												hwnd, (HMENU)D_OL_IDOCB, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		D_PrepareOperCombobox(hctl, pD, 0);
		//oper. button
		hctl = CreateWindow(CLASS_BUTTON, D_("Set"),
												WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												lof+D_OL_STW+D_OL_W, D_OL_TOF+i*D_OL_HOF-1, 2*D_OL_SBW, D_OL_H+2,
												hwnd, (HMENU)D_OL_IDOBT, NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
	}
	return(TRUE);
}

//*** function creates online control according to property
BOOL D_CreateOnLineCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD, int pos)
{
	HWND hctl;
	D_DEV_PROP *prop;
	TCHAR ttext[128], *ptext;
	int i, lof = D_OL_LOF;
	int style;

	if (pD == NULL || pD->p == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	//position
	i = pos & 0xFFFF;
	if ((pos & D_OL_POS_C2))
		lof = D_OL_WLOF;

	//--- static
	D_MakePropLabel(prop, ttext, 0x3);
	hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE,
											lof, D_OL_TOF+i*D_OL_HOF+2, D_OL_STW, D_OL_H,
											hwnd, (HMENU)(pidx+D_OL_IDST), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	//--- combobox
	if (prop->guitype == DGUIT_COMBOBOX) {
		hctl = CreateWindow(CLASS_COMBOBOX, cstr_empty,
												WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
												lof+D_OL_STW+D_OL_SOF, D_OL_TOF+i*D_OL_HOF-1, D_OL_W, D_CB_DEFHEIGHT,
												hwnd, (HMENU)(pidx+D_OL_IDVAL), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		D_PrepareCombobox(hctl, prop);
	}
	//--- radio switch
	else if (prop->guitype == DGUIT_RADIOSW) {
		//off
		ptext = D_GetBoolString(prop->guidata, 1, FALSE);
		hctl = CreateWindow(CLASS_BUTTON, (ptext && *ptext) ? ptext : D_("Off"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RADIOBUTTON | WS_GROUP,
												lof+D_OL_STW+D_OL_SOF+(D_OL_W/2+D_OL_SBW), D_OL_TOF+i*D_OL_HOF-1, D_OL_W/2+D_OL_SBW, D_OL_H,
												hwnd, (HMENU)(pidx+D_OL_IDOFF), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//on
		ptext = D_GetBoolString(prop->guidata, 1, TRUE);
		hctl = CreateWindow(CLASS_BUTTON, (ptext && *ptext) ? ptext : D_("On"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RADIOBUTTON,
												lof+D_OL_STW+D_OL_SOF, D_OL_TOF+i*D_OL_HOF-1, D_OL_W/2+D_OL_SBW, D_OL_H,
												hwnd, (HMENU)(pidx+D_OL_IDON), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
	}
	else {
		//editbox
		if (prop->guitype == DGUIT_READ || prop->guitype == DGUIT_SWITCH)
			style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
		else
			style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_RIGHT;
		hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, cstr_empty,
													style,
													lof+D_OL_STW+D_OL_SOF, D_OL_TOF+i*D_OL_HOF-1, D_OL_W, D_OL_HEB,
													hwnd, (HMENU)(pidx+D_OL_IDVAL), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		if ((prop->flags & (DFLAG_CONF|DFLAG_READONLY)) == DFLAG_CONF && prop->guitype != DGUIT_SWITCH) {
			D_PrepareValidNumSubclassing(hctl, prop);
			if (prop->guitype == DGUIT_SPINVAL) {
				hctl = CreateUpDownControl(WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS,
																	 lof+D_OL_STW+2*D_OL_SOF+D_OL_W, D_OL_TOF+i*D_OL_HOF-2, D_OL_UDW, D_OL_HEB+1,
																	 hwnd, (pidx+D_OL_IDUD), NULL, hctl, 1, -1, 0);
				SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
			}
		}
		else
			SetReadOnlySubclassing(hctl);

		//emphasis
		if ((prop->guiflags & DGFLAG_EMPH)) {
			SetProp(hctl, STR_EMPHASIS, (HANDLE)1);
		}
	}

	//buttons
	if (prop->guitype == DGUIT_SWITCH) {
		//off
		ptext = D_GetBoolString(prop->guidata, 1, FALSE);
		hctl = CreateWindow(CLASS_BUTTON, (ptext && *ptext) ? ptext : D_("Off"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												lof+D_OL_STW+2*D_OL_SOF+D_OL_W+D_OL_SBW, D_OL_TOF+i*D_OL_HOF-1, D_OL_SBW, D_OL_H,
												hwnd, (HMENU)(pidx+D_OL_IDOFF), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
		//on
		ptext = D_GetBoolString(prop->guidata, 1, TRUE);
		hctl = CreateWindow(CLASS_BUTTON, (ptext && *ptext) ? ptext : D_("On"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
												lof+D_OL_STW+2*D_OL_SOF+D_OL_W, D_OL_TOF+i*D_OL_HOF-1, D_OL_SBW, D_OL_H,
												hwnd, (HMENU)(pidx+D_OL_IDON), NULL, NULL);
		SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
	}

	return(TRUE);
}

//*** function destroys online control according to property
BOOL D_DestroyOnLineCtrl(HWND hwnd, WORD pidx, D_DEVICE *pD)
{
	HWND hctl;
	D_DEV_PROP *prop;
	//int readonly;

	if (pD == NULL || pD->p == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	hctl = GetDlgItem(hwnd, (pidx+D_OL_IDVAL));
	if (hctl == NULL)
    return(FALSE);

	//clear subclassing
	if (prop->guitype != DGUIT_COMBOBOX && prop->guitype != DGUIT_RADIOSW) {
		//readonly
		//readonly = ((prop->flags & DFLAG_MEAS) || prop->guitype == DGUIT_READ || prop->guitype == DGUIT_SWITCH);

		if ((prop->flags & DFLAG_CONF) == DFLAG_CONF && (prop->guitype != DGUIT_SWITCH))
			ClearValidNumStrSubclassing(hctl);
		else
			ClearReadOnlySubclassing(hctl);

		//emphasis
		if ((prop->guiflags & DGFLAG_EMPH)) {
			RemoveProp(hctl, STR_EMPHASIS);
		}
	}

	//remove prop. enable
	RemoveProp(hctl, D_STR_PROPENABLE);
	return(TRUE);
}

//*** function creates online readonly editbox
BOOL D_CreateOnLineReadEB(HWND hwnd, WORD pidx, TCHAR *title, int right, int pos)
{
	HWND hctl;
	TCHAR ttext[128];
	int i, lof = D_OL_LOF;
	int style;

	//position
	i = pos & 0xFFFF;
	if ((pos & D_OL_POS_C2))
		lof = D_OL_WLOF;

	//title
  if (title == NULL)
    title = cstr_empty;
	else {
		//test for device pointer
		D_DEVICE *pD = (D_DEVICE*)title;
		if (pD->status >= D_STATUS_IDLE && pD->status <= D_STATUS_READY && pD->p != NULL && pidx < pD->n_p) {
			//is device pointer
			D_MakePropLabel(&pD->p[pidx], ttext, 0x3);
		}
		else {
			__sprintf(ttext, TEXT("%s:"), title);
		}
	}

	//position
	i = pos & 0xFFFF;
	if ((pos & D_OL_POS_C2))
		lof = D_OL_WLOF;

	//--- static
	hctl = CreateWindow(CLASS_STATIC, ttext, WS_CHILD | WS_VISIBLE,
											lof, D_OL_TOF+i*D_OL_HOF, D_OL_STW, D_OL_H,
											hwnd, (HMENU)(pidx+D_OL_IDST), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	//editbox
	style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	if (right)
    style |= ES_RIGHT;
	hctl = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_EDIT, cstr_empty,
												style,
												lof+D_OL_STW+D_OL_SOF, D_OL_TOF+i*D_OL_HOF-1, D_OL_W, D_OL_HEB,
												hwnd, (HMENU)(pidx+D_OL_IDVAL), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
	SetReadOnlySubclassing(hctl);

	return(TRUE);
}

//*** function destroys online readonly EB
BOOL D_DestroyOnLineReadEB(HWND hwnd, WORD pidx)
{
	HWND hctl;

	hctl = GetDlgItem(hwnd, (pidx+D_OL_IDVAL));
	ClearReadOnlySubclassing(hctl);
	return(TRUE);
}

//*** function creates online radio button
BOOL D_CreateOnLineRadioBT(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int pos)
{
	HWND hctl;
	int i, lof = D_OL_LOF, bwidth;

  if (title == NULL)
    title = cstr_empty;

	//position
	i = pos & 0xFFFF;
	if ((pos & D_OL_POS_C2)) {
		lof = D_OL_WLOF;
	}
	//button width
	bwidth = (D_OL_STW+3*D_OL_SOF+D_OL_W+2*D_OL_SBW)/bt_per_row;

	//radio button
	hctl = CreateWindow(CLASS_BUTTON, title, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RADIOBUTTON,
											lof+(index%bt_per_row)*bwidth, D_OL_TOF+i*D_OL_HOF-1,
											bwidth, D_OL_H,
											hwnd, (HMENU)(id), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	return(TRUE);
}

//*** function creates online push button
BOOL D_CreateOnLineButtonPar(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int pos, int style)
{
	HWND hctl;
	int i, lof = D_OL_LOF, w, bwidth, xheight = 1;

  if (title == NULL)
    title = cstr_empty;

	//position
	i = pos & 0xFFFF;

	if ((pos & D_OL_POS_C12)) {
		w = D_OL_STW+D_OL_W+2*D_OL_SBW+D_OL_MOF+D_OL_STW+D_OL_W;
	}
	else if ((pos & D_OL_POS_C2)) {
		lof = D_OL_WLOF;
		w = D_OL_STW+D_OL_W;
	}
	else {
		//w = D_OL_STW+3*D_OL_SOF+D_OL_W+2*D_OL_SBW;
		w = D_OL_STW+D_OL_W+2*D_OL_SBW;
	}

	if (pos & D_OL_XHEIGHT_MASK) {
		xheight += (pos >> 20) & 0xF;
	}
	//button width
	bwidth = w/bt_per_row;

	//style
	if (style == 0)
		style = BS_PUSHBUTTON;

	//button
	hctl = CreateWindow(CLASS_BUTTON, title, WS_CHILD | WS_VISIBLE | WS_TABSTOP | style,
											lof+(index%bt_per_row)*bwidth, D_OL_TOF+i*D_OL_HOF-1,
											bwidth, D_OL_H*xheight+(D_OL_HOF-D_OL_H)*(xheight-1),
											hwnd, (HMENU)(id), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	return(TRUE);
}

//*** function creates online checkbox button (index mask 0x8000 -> no autocheckbox)
BOOL D_CreateOnLineCheckbox(HWND hwnd, int id, TCHAR *title, int index, int bt_per_row, int pos)
{
	HWND hctl;
	int i, lof = D_OL_LOF, bwidth, xheight = 1;
	int checkstyle = 0;

  if (title == NULL)
    title = cstr_empty;

	//position
	i = pos & 0xFFFF;
	if ((pos & D_OL_POS_C2)) {
		lof = D_OL_WLOF;
	}
	if (pos & D_OL_XHEIGHT_MASK) {
		xheight += (pos >> 20) & 0xF;
	}
	//button width
	bwidth = (D_OL_STW+3*D_OL_SOF+D_OL_W+2*D_OL_SBW)/bt_per_row;

	//check style
	if (index & 0x8000)
		checkstyle = BS_CHECKBOX;
	else
		checkstyle = BS_AUTOCHECKBOX;
	index &= 0x7FFF;

	//button
	hctl = CreateWindow(CLASS_BUTTON, title, WS_CHILD | WS_VISIBLE | WS_TABSTOP | checkstyle,
											lof+(index%bt_per_row)*bwidth, D_OL_TOF+i*D_OL_HOF-1,
											bwidth, D_OL_H*xheight+(D_OL_HOF-D_OL_H)*(xheight-1),
											hwnd, (HMENU)(id), NULL, NULL);
	SendMessage(hctl, WM_SETFONT,	(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

	return(TRUE);
}

//*** function sets control by property value
BOOL D_SetOnLineControl(HWND hwnd, WORD pidx, D_DEVICE *pD, LPARAM lparam)
{
	static TCHAR ttext[128];
	HWND hctl;
	D_DEV_PROP *prop;
	U_NVAL nval;
	int prec;
	TCHAR *ptext, *pt;

	if (pD == NULL || pD->p == NULL || pidx >= pD->n_p)
		return(FALSE);
	prop = &pD->p[pidx];

	if (prop->guitype == DGUIT_RADIOSW)
		hctl = GetDlgItem(hwnd, D_GETOLID_OFF(pidx));
	else
		hctl = D_GETOLH_VAL(hwnd, pidx);
	if (hctl == NULL)
		return(FALSE);

	//test and store response
	D_TestUserData(hctl, (LONG)lparam);

	//combobox
	if (prop->guitype == DGUIT_COMBOBOX) {
		//is CB
		if (CompareWindowClass(hctl, CLASS_COMBOBOX)) {
			if (!SendMessage(hctl, CB_GETDROPPEDSTATE, 0, 0)) {
				SendMessage(hctl, CB_SETCURSEL, (WPARAM)prop->val.u, 0);
				return(2);
			}
		}
		else if (prop->guidata) {
			//text only
			ptext = GetSubString(prop->guidata, prop->val.u);
			if (ptext) {
				SetWindowText(hctl, ptext);
			}
		}
	}
	//switch
	else if (prop->guitype == DGUIT_SWITCH) {
		//ptext = prop->val.b ? D_("Running") : D_("Stopped");
		ptext = D_GetBoolString(prop->guidata, 0, prop->val.b);
		if (IsCtrlTextDiff(hctl, ptext)) {
			SetWindowText(hctl, ptext);
			return(2);
		}
	}
	//radio switch
	else if (prop->guitype == DGUIT_RADIOSW) {
		//off
		CheckDlgButton(hwnd, D_GETOLID_OFF(pidx), prop->val.b ? FALSE : TRUE);
		//on
		CheckDlgButton(hwnd, D_GETOLID_ON(pidx), prop->val.b ? TRUE : FALSE);
	}
	//control value
	else if ((prop->flags & (DFLAG_CONF|DFLAG_READONLY)) == DFLAG_CONF && !IsReadOnlySubclassing(hctl)) {
		if (!SendMessage(hctl, EM_GETMODIFY, 0, 0)) {
			GetEditUval(hctl, &nval);
			switch (prop->type) {
				case DT_INT:
					if (nval.i != prop->val.i) {
						nval.i = prop->val.i;
						SetEditUval(hctl, nval);
						return(2);
					}
					break;
				default:
				case DT_UINT:
					if (nval.u != prop->val.u) {
						nval.u = prop->val.u;
						SetEditUval(hctl, nval);
						return(2);
					}
					break;
				case DT_DOUBLE:
					if (D_fabs(nval.d - prop->val.d) >= 0.0001) {
						nval.d = prop->val.d;
						D_ConvertValueToDemandUnit(prop, &nval.d);
						SetEditUval(hctl, nval);
						return(2);
					}
					break;
			}
		}
	}
	//checkbits
	else if (prop->guitype == DGUIT_CHECKBITS && prop->guidata && *prop->guidata) {
		//get value
		switch (prop->type) {
			case DT_BOOL: prec = prop->val.b; break;
			case DT_INT: prec = prop->val.i; break;
			default:
			case DT_UINT: prec = (int)prop->val.u; break;
			case DT_INT64: prec = (int)prop->val.i64; break;
			case DT_UINT64: prec = (int)prop->val.u64; break;
		}
		//through bits
		*ttext = '\0';
		unsigned int n = 0;
		unsigned int nb = 0;
		ptext = (TCHAR *)prop->guidata;
		while (prec) {
			if (prec & 0x1) {
				if (nb) {
					__strcat(ttext, TEXT(","));		//?|
				}
				if (*ptext)
					__sprintf(ttext, TEXT("%s%s"), ttext, D_ram(ptext));
				else
					__sprintf(ttext, TEXT("%s0x%X"), ttext, 1<<n);
				nb++;
			}
			prec >>= 1;
			n++;
			if (*ptext) {
				ptext += lstrlen(ptext)+1;
			}
		}
		if (nb == 0) {
			__strcat(ttext, TEXT("---"));
		}
 		if (IsCtrlTextDiff(hctl, ttext)) {
			SetWindowText(hctl, ttext);
			return(2);
 		}
	}
	//read value + guidata
	else if (prop->guitype == DGUIT_READ && prop->guidata && *prop->guidata) {
		//get value
		switch (prop->type) {
			case DT_BOOL: prec = prop->val.b; break;
			case DT_INT: prec = prop->val.i; break;
			default:
			case DT_UINT: prec = (int)prop->val.u; break;
			case DT_INT64: prec = (int)prop->val.i64; break;
			case DT_UINT64: prec = (int)prop->val.u64; break;
		}
		//through data string
		ptext = (TCHAR *)prop->guidata;
		while (prec) {
			pt = ptext + (lstrlen(ptext)+1);
			if (*pt) {
				ptext = pt;
				prec--;
			}
			else
				break;
		}
		ptext = D_ram(ptext);
 		if (IsCtrlTextDiff(hctl, ptext)) {
			SetWindowText(hctl, ptext);
			return(2);
 		}
	}
	//read value
	else {
		prec = DFLAG_GETPREC(prop->flags);
		switch (prop->type) {
			case DT_BOOL: __strcpy(ttext, prop->val.b ? D_("On") : D_("Off")); break;
			case DT_INT:
				__sprintf(ttext, TEXT("%d"), prop->val.i);
				break;
			default:
			case DT_UINT:
				if (prop->guitype == DGUIT_HEX)
					__sprintf(ttext, TEXT("%0*X"), prec, prop->val.u);
/*
				else if (prop->guitype == DGUIT_BIN)
					__sprintf(ttext, TEXT("%0*b"), prec, prop->val.u);
*/
				else
					__sprintf(ttext, TEXT("%u"), prop->val.u);
				break;
			case DT_DOUBLE:
				nval.d = prop->val.d;
				D_ConvertValueToDemandUnit(prop, &nval.d);
				D_ConvertUnitDemandPrecision(prop, &prec);
				if (prec < U_NFLAGAUTOPREC && !IsBigExponent(nval.d))
					__sprintf(ttext, TEXT("%.*lf"), prec, nval.d);
				else
					__sprintf(ttext, TEXT("%lg"), nval.d);
				break;
			case DT_INT64:
				__sprintf(ttext, TEXT("%I64d"), prop->val.i64);
				break;
			case DT_UINT64:
				__sprintf(ttext, TEXT("%I64u"), prop->val.u64);
				break;
			case DT_STRING:
				if (prop->val.m.buf)
					__sprintf(ttext, TEXT("%.*s"), 32, prop->val.m.buf);
				else
					__sprintf(ttext, TEXT("(%u)"), prop->val.m.size);
				break;
			case DT_MEMORY:
				__sprintf(ttext, TEXT("(%u)"), prop->val.m.size);
				break;
		}
 		if (IsCtrlTextDiff(hctl, ttext)) {
			SetWindowText(hctl, ttext);
			return(2);
 		}
	}
	return(TRUE);
}

//*** function read value from EB and test for difference
BOOL D_TestOnLineEBControl(HWND hctl, D_DEV_PROP *prop, D_VAL *val)
{
	U_NVAL nval;

	//get value
	if (prop && val && GetEditUval(hctl, &nval) == NO_ERROR) {
		//test difference
		switch (prop->type) {
			case DT_INT:
				if (nval.i != prop->val.i) {
					val->i = nval.i;
					return(TRUE);
				}
				break;
			default:
			case DT_UINT:
				if (nval.u != prop->val.u) {
					val->u = nval.u;
					return(TRUE);
				}
				break;
			case DT_DOUBLE:
				//nval.d = prop->val.d;
				D_ConvertValueToOriginUnit(prop, &nval.d);
				if (D_fabs(nval.d - prop->val.d) >= __DBL_EPSILON__) {		//not the same
					val->d = nval.d;
					return(TRUE);
				}
				break;
		}
	}
	return(FALSE);
}

//*** function processes UD-control notify message
BOOL D_NotifyOnlineUDControl(HWND hwnd, WORD pidx, D_DEVICE *pD, int dir, double step)
{
	D_DEV_PROP *prop;
	U_NVAL nval;
	D_VAL dval;

	//test pointers
	if (pD == NULL || pidx >= pD->n_p || pD->p == NULL)
		return(FALSE);
	prop = &pD->p[pidx];

	//test if conf. property
	if ((prop->flags & DFLAG_CONF) != DFLAG_CONF)
		return(FALSE);

	//get value
	if (GetEditUval(D_GETOLH_VAL(hwnd, pidx), &nval) != NO_ERROR)
		return(FALSE);

	//up-down value
	switch (prop->type) {
		case DT_INT:
			if (dir < 0)
				nval.i -= (int)step;		//decrease
			else
				nval.i += (int)step;		//increase
			break;
		default:
		case DT_UINT:
			if (dir < 0) {
				nval.i -= (int)step;		//decrease
				if (nval.i < 0)
					nval.u = 0;
			}
			else
				nval.i += (int)step;		//increase
			break;
		case DT_DOUBLE:
			if (dir < 0)
				nval.d -= step;		//decrease
			else
				nval.d += step;		//increase
			break;
	}

	//validate range
	D_ValidatePropRange(prop);

	//set value
	switch (prop->type) {
		case DT_INT: dval.i = nval.i; break;
		default:
		case DT_UINT: dval.u = nval.u; break;
		case DT_DOUBLE:
			D_ConvertValueToOriginUnit(prop, &nval.d);
			dval.d = nval.d;
			break;
	}

	//write/read new value
	D_WriteFunction((HDEVICE)pD, pD->ufce.pWriteProp, pidx, dval, GetParent(hwnd), WMD_RESPONSE);
	D_ReadFunction((HDEVICE)pD, pD->ufce.pReadProp, pidx, hwnd, D_OL_WM_UPDATE);

	return(TRUE);
}


//--------------------------------------------------------------
//*** function filters unsigned value by last changed value
/*
unsigned D_UFilterByUTime(unsigned time, unsigned value, D_UTIMEFILT *filt, unsigned filttime)
{
	if (filt) {
		//first value
		if (!filt->is_first) {
			filt->is_first = 1;
			filt->prev_value = value;
		}
		else {
			//test for change
			if (value != filt->prev_value)
					filt->t0 = time;		//t0
			//set previous
			filt->prev_value = value;
			//test pulse width
			if ((time-filt->t0) >= filttime)
				filt->filt_value = value;		//new value
			else
				value = filt->filt_value;
		}
	}
	return(value);
}
*/

//*** function filters unsigned value by last changed value
unsigned D_UFilterByDTime(double time, unsigned value, D_DTIMEFILT *filt, double filttime)
{
	if (filt) {
		//first value
		if (!filt->is_first) {
			filt->is_first = 1;
			filt->prev_value = value;
		}
		else {
			//test for change
			if (value != filt->prev_value)
					filt->t0 = time;		//t0
			//set previous
			filt->prev_value = value;
			//test pulse width
			if ((time-filt->t0) >= filttime)
				filt->filt_value = value;		//new value
			else
				value = filt->filt_value;
		}
	}
	return(value);
}

//----------------------------------------------------------------
//*** function processes IPROC function for working with D_DEV_SEARCH structure
//IPTYPE_CREATE: iparam -> pointer to lparam of item, lparam -> LPARAM
//IPTYPE_DELETE: iparam -> pointer to lparam of item, lparam -> 0
//IPTYPE_PRINT: iparam -> pointer to lparam of item, lparam -> FILE *
BOOL D_ListIProcDevSearchParam(LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	if (iparam) {
		switch (type) {
			//--- processed after ITEM creation
			case IPTYPE_CREATE:
				if (lparam) {
					*iparam = (LPARAM)malloc(sizeof(D_DEV_SEARCH));
					if (*iparam == 0)
						return(FALSE);
					memcpy((D_DEV_SEARCH *)*iparam, (D_DEV_SEARCH *)lparam, sizeof(D_DEV_SEARCH));
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
				{
				if (*iparam && lparam) {
					D_DEV_SEARCH *pitem = (D_DEV_SEARCH *)*iparam;
					D_DEV_SEARCH *pitem2 = (D_DEV_SEARCH *)lparam;
					//by mode
					if (pitem->mode > pitem2->mode)
						return(1);
					else if (pitem->mode < pitem2->mode)
						return(-1);
					else {
						switch (pitem->mode) {
						//by comport
						case COM_MODE_RS232:
							if (pitem->portno > pitem2->portno)
								return(1);
							else if (pitem->portno < pitem2->portno)
								return(-1);
							else
								return(0);
							break;

						//by ip
						case COM_MODE_ETHERNET:
							return(__strcmp(pitem->ip, pitem2->ip));

						//by userid
						case COM_MODE_USERID:
							return(__strcmp(pitem->uid, pitem2->uid));
						}
					}
				}
				}
				return(0);		//don't sort
		}
	}
	return(TRUE);
}

//----------------------------------------------------------------
//*** function compare software version of device (if any)
int D_GetSWIntNumber(HDEVICE hD)
{
	TCHAR *ptext;

	ptext = D_GetDevFirmware(hD);
	if (ptext)
		return(__atoi(ptext));
	return(-1);
}


//------------ SP Table -------------------------------------------
static MGTCTRCBITEM d_cbt_item[D_CB_TRIGER_MAX];
static MGTCTRCBDATA d_cbt_data = {D_CB_TRIGER_MAX, d_cbt_item};
static MGTCTRCBITEM d_cba_item[D_CB_ACTION_MAX];
static MGTCTRCBDATA d_cba_data = {D_CB_ACTION_MAX, d_cba_item};
static MGTCTRCBITEM d_cbi_item[D_CB_INPUT_MAX];
static MGTCTRCBDATA d_cbi_data = {D_CB_INPUT_MAX, d_cbi_item};

//*** function creates SP-table
HWND D_CreateSPTable(HWND hparent, int id, int inputs, const TCHAR **names, int col, int row)
{
	HWND hga;
	//table
	DWORD r;
	MGTCHEADERPARAMS cheader;
	MGTRHEADERPARAMS rheader;
	MGTCHEADPARAMS chead;
	MGTRHEADPARAMS rhead;
	int i;

	hga = CreateWindow(MGC_TABLECLASSNAME, TEXT("SPTable"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
											D_PA_COLST(col), D_PA_ROWST(row), D_PA_GWIN(2), D_PA_HOF*(inputs+1),
											hparent, (HMENU)id, NULL, NULL);
	if (hga == NULL)
		return(hga);

	SendMessage(hga, MGT_TAM_SETPARAM, MGT_TPAR_BORDER_BGCOLOR, (LPARAM)MGC_COLOR_BNTFACE);		//bg. color
	//SendMessage(hga, MGT_TAM_SETPARAM, MGT_TPAR_BORDER_VISIBLE, (LPARAM)FALSE);		//bg. off
	SendMessage(hga, MGT_TAM_SETPARAM, MGT_TPAR_BORDER_SPACE, (LPARAM)0);		//bg. space=0
	SendMessage(hga, MGT_TAM_SETPARAM, MGT_TPAR_TITLE_VISIBLE, (LPARAM)FALSE);		//title off

	//triger combobox names
	memset(&d_cbt_item, 0, sizeof(d_cbt_item));
	for (i=0; i<D_CB_TRIGER_MAX; i++) {
		d_cbt_item[i].val.u = i;
		d_cbt_item[i].name = D_ram(c_strSMTriggers[i]);
		d_cbt_item[i].hicon = ImageList_GetIcon(g_hImgList, i, ILD_TRANSPARENT);
	}
	//c_data.item[0].hicon = LoadImage(g_hInst, MAKEINTRESOURCE(APPL_ICONS), IMAGE_ICON, 0, 0,	LR_DEFAULTCOLOR);


	//action combobox names
	memset(&d_cba_item, 0, sizeof(d_cba_item));
	for (i=0; i<D_CB_ACTION_MAX; i++) {
		d_cba_item[i].val.u = i;
		d_cba_item[i].name = D_ram(c_strSMActions[i]);
		d_cba_item[i].hicon = NULL;
	}

	//init table
	SendMessage(hga, MGT_TAM_RESETCONTENT, 0, 0);

	SendMessage(hga, MGT_TAM_INSERTCOLS, 0, 3);		//number of columns
	SendMessage(hga, MGT_TAM_INSERTROWS, 0, inputs);		//number of rows

	//column header
	memset(&cheader, 0, sizeof(MGTCHEADERPARAMS));
	cheader.mask = MGT_CHEADERMASK_VIS |
								 MGT_CHEADERMASK_RESIZE |
								 MGT_CHEADERMASK_SEL |
								 MGT_CHEADERMASK_COLMASK;
	cheader.visible = TRUE;
	cheader.selectable = FALSE;
	cheader.resizeable = FALSE;
	cheader.colmask = MGT_COLMASK_LABEL | MGT_COLMASK_WIDTH;
	SendMessage(hga, MGT_TAM_SETCOLHEADER, 0, (LPARAM)&cheader);

	//row header
	SendMessage(hga, MGT_TAM_GETROWHEADER, 0, (LPARAM)&rheader);
	rheader.mask = MGT_RHEADERMASK_VIS |
								 MGT_RHEADERMASK_SEL |
								 MGT_CHEADERMASK_RESIZE |
								 MGT_RHEADERMASK_WIDTH |
								 MGT_RHEADERMASK_FONT |
								 MGT_RHEADERMASK_ROWMASK;
	rheader.visible = TRUE;
	rheader.selectable = FALSE;
	rheader.resizeable = FALSE;
	rheader.width = 60;
	rheader.font.tflag = MGC_FONT_BOLD;
	rheader.rowmask = MGT_ROWMASK_LABEL;
	SendMessage(hga, MGT_TAM_SETROWHEADER, 0, (LPARAM)&rheader);
	//set row names
	for (r=0; r<inputs; r++) {
		rhead.mask = MGT_RHEADMASK_LABEL | MGT_RHEADMASK_HEIGHT;
		rhead.height = D_PA_H;
		if (names)
			__sprintf(rhead.label, TEXT("%.*s %s"), MGT_RHEADLABEL_LEN-10, D_("Input"), names[r]);
		else
			__sprintf(rhead.label, TEXT("%.*s %d"), MGT_RHEADLABEL_LEN-10, D_("Input"), r+1);
		SendMessage(hga, MGT_TAM_SETROWHEAD, r, (LPARAM)&rhead);		//
	}

	//1) trigger edge column
	memset(&chead, 0, sizeof(MGTCHEADPARAMS));
	chead.mask = MGT_CHEADMASK_LABEL |
							 MGT_CHEADMASK_WIDTH |
							 MGT_CHEADMASK_COLOR |
							 MGT_CHEADMASK_SPEC;
	lstrcpyn(chead.label, D_("Trigger edge"), MGT_CHEADLABEL_LEN);
	chead.width = 100;
	chead.color = MGC_COLOR_BLACK;
	chead.spec.type = MGC_TYPE_UINT;
	chead.spec.align = MGC_VALALIGN_CT;
	chead.spec.prec = 0;
	chead.spec.mask = MGC_VALMASK_NONE;
	chead.spec.ctr = MGC_CTR_COMBOICONBOX;
	chead.spec.data = &d_cbt_data;
	SendMessage(hga, MGT_TAM_SETCOLHEAD, 0, (LPARAM)&chead);		//

	//2) action column
	memset(&chead, 0, sizeof(MGTCHEADPARAMS));
	chead.mask = MGT_CHEADMASK_LABEL |
							 MGT_CHEADMASK_WIDTH |
							 MGT_CHEADMASK_COLOR |
							 MGT_CHEADMASK_SPEC;
	lstrcpyn(chead.label, D_("Action"), MGT_CHEADLABEL_LEN);
	chead.width = 100;
	chead.color = MGC_COLOR_BLACK;
	chead.spec.type = MGC_TYPE_UINT;
	chead.spec.align = MGC_VALALIGN_CT;
	chead.spec.prec = 0;
	chead.spec.mask = MGC_VALMASK_NONE;
	chead.spec.ctr = MGC_CTR_COMBOBOX;
	chead.spec.data = &d_cba_data;
	SendMessage(hga, MGT_TAM_SETCOLHEAD, 1, (LPARAM)&chead);		//

	//3) filter column
	memset(&chead, 0, sizeof(MGTCHEADPARAMS));
	chead.mask = MGT_CHEADMASK_LABEL |
							 MGT_CHEADMASK_WIDTH |
							 MGT_CHEADMASK_COLOR |
							 MGT_CHEADMASK_SPEC |
							 MGT_CHEADMASK_DIS;
	lstrcpyn(chead.label, D_("Filter"), MGT_CHEADLABEL_LEN-7);
	lstrcat(chead.label, TEXT(" [ms]"));
	chead.width = 100;
	chead.color = MGC_COLOR_BLACK;
	chead.spec.type = MGC_TYPE_UINT;
	chead.spec.align = MGC_VALALIGN_RT;
	chead.spec.prec = 0;
	chead.spec.mask = MGC_VALMASK_MINMAX;
	chead.spec.min.u = 0;
	chead.spec.max.u = 255;
	chead.disable = FALSE;
	SendMessage(hga, MGT_TAM_SETCOLHEAD, 2, (LPARAM)&chead);		//

	return(hga);
}

//*** function writes SP-table
BOOL D_WriteSPTable(HWND hga, D_DEV_PROP *prop, int inputs)
{
	//table
	DWORD r;
	DPOINT p;
	MGTCELL cell;

	if (hga == NULL || prop == NULL)
		return(FALSE);

	for (r=0; r<inputs; r++) {
		memset(&cell, 0, sizeof(MGTCELL));
		//set trigger edge cell
		p.x = 0;
		p.y = r;
		cell.val.u = D_CFLAG_TGET(prop[r].val.u);
		if (cell.val.u)
			cell.val.u--;
		SendMessage(hga, MGT_TAM_SETCELL, (WPARAM)&p, (LPARAM)&cell);

		//set action
		p.x = 1;
		p.y = r;
		cell.val.u = D_CFLAG_AGET(prop[r].val.u);
		SendMessage(hga, MGT_TAM_SETCELL, (WPARAM)&p, (LPARAM)&cell);

		//set filter
		p.x = 2;
		p.y = r;
		cell.val.u = D_CFLAG_FGET(prop[r].val.u);
		SendMessage(hga, MGT_TAM_SETCELL, (WPARAM)&p, (LPARAM)&cell);
	}
	return(TRUE);
}

//*** function reads SP-table
BOOL D_ReadSPTable(HWND hga, D_DEV_PROP *prop, int inputs)
{
	//table
	DWORD r;
	DPOINT p;
	MGTCELL cell;

	if (hga == NULL || prop == NULL)
		return(FALSE);

	for (r=0; r<inputs; r++) {
		prop[r].val.u = 0;

		memset(&cell, 0, sizeof(MGTCELL));
		//get trigger edge cell
		p.x = 0;
		p.y = r;
		SendMessage(hga, MGT_TAM_GETCELL, (WPARAM)&p, (LPARAM)&cell);
		prop[r].val.u |= D_CFLAG_TSET(cell.val.u+1);

		//get action
		p.x = 1;
		p.y = r;
		SendMessage(hga, MGT_TAM_GETCELL, (WPARAM)&p, (LPARAM)&cell);
		prop[r].val.u |= D_CFLAG_ASET(cell.val.u);

		//get filter
		p.x = 2;
		p.y = r;
		SendMessage(hga, MGT_TAM_GETCELL, (WPARAM)&p, (LPARAM)&cell);
		prop[r].val.u |= D_CFLAG_FSET(cell.val.u);
	}
	return(TRUE);
}

//----
typedef struct {
	TCHAR str[D_MAX_TEXT_LEN];
} D_SPSTRING;

/** @brief Create SP-table (v2)
 *
 * @param hparent HWND
 * @param id int
 * @param inputs int
 * @param names const TCHAR**
 * @param col int
 * @param row int
 * @return HWND
 *
 */
HWND D_CreateSPTable2(HWND hparent, int id, unsigned int prop_rows, unsigned int inputs, const TCHAR **names, int col, int row)
{
	HWND hta;
	//table
	MGTCHEADERPARAMS cheader;
	MGTRHEADERPARAMS rheader;
	MGTCHEADPARAMS chead;
	int i;
	D_SPSTRING *cbidata = NULL;
	TCHAR *pstr;

	hta = CreateWindow(MGC_TABLECLASSNAME, TEXT("SPTable"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
											D_PA_COLST(col), D_PA_ROWST(row), D_PA_GWIN(2), D_PA_HOF*(prop_rows+1),
											hparent, (HMENU)id, NULL, NULL);
	if (hta == NULL)
		return(hta);

	SendMessage(hta, MGT_TAM_SETPARAM, MGT_TPAR_BORDER_BGCOLOR, (LPARAM)MGC_COLOR_BNTFACE);		//bg. color
	//SendMessage(hta, MGT_TAM_SETPARAM, MGT_TPAR_BORDER_VISIBLE, (LPARAM)FALSE);		//bg. off
	SendMessage(hta, MGT_TAM_SETPARAM, MGT_TPAR_BORDER_SPACE, (LPARAM)0);		//bg. space=0
	SendMessage(hta, MGT_TAM_SETPARAM, MGT_TPAR_TITLE_VISIBLE, (LPARAM)FALSE);		//title off

	//triger combobox names
	memset(&d_cbt_item, 0, sizeof(d_cbt_item));
	for (i=0; i<D_CB_TRIGER_MAX; i++) {
		d_cbt_item[i].val.u = i;
		d_cbt_item[i].name = D_ram(c_strSMTriggers[i]);
		d_cbt_item[i].hicon = ImageList_GetIcon(g_hImgList, i, ILD_TRANSPARENT);
	}
	//c_data.item[0].hicon = LoadImage(g_hInst, MAKEINTRESOURCE(APPL_ICONS), IMAGE_ICON, 0, 0,	LR_DEFAULTCOLOR);


	//action combobox names
	memset(&d_cba_item, 0, sizeof(d_cba_item));
	for (i=0; i<D_CB_ACTION_MAX; i++) {
		d_cba_item[i].val.u = i;
		d_cba_item[i].name = D_ram(c_strSMActions[i]);
		d_cba_item[i].hicon = NULL;
	}

	//inputs
	if (inputs >= D_CB_INPUT_MAX)
		inputs = D_CB_INPUT_MAX-1;

	//allocate input data
	cbidata = malloc((inputs+1)*sizeof(D_SPSTRING));
	if (cbidata == NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
		return(NULL);
	}
	for (i=0; i<=inputs; i++) {
		pstr = cbidata[i].str;
		if (i == 0) {
			__sprintf(pstr, TEXT("%.*s"), D_MAX_TEXT_LEN, D_("none"));
		}
		else {
			if (names)
				__sprintf(pstr, TEXT("%.*s %s"), D_MAX_TEXT_LEN-10, D_("Input"), names[i-1]);
			else
				__sprintf(pstr, TEXT("%.*s %d"), D_MAX_TEXT_LEN-10, D_("Input"), i);
		}
		d_cbi_item[i].val.u = i;
		d_cbi_item[i].name = pstr;
		d_cbi_item[i].hicon = NULL;
	}
	d_cbi_data.count = i;

	//set string array to table
	SetWindowLong(hta, GWLP_USERDATA, (LONG)cbidata);

	//init table
	SendMessage(hta, MGT_TAM_RESETCONTENT, 0, 0);

	SendMessage(hta, MGT_TAM_INSERTCOLS, 0, 4);		//number of columns
	SendMessage(hta, MGT_TAM_INSERTROWS, 0, prop_rows);		//number of rows

	//column header
	memset(&cheader, 0, sizeof(MGTCHEADERPARAMS));
	cheader.mask = MGT_CHEADERMASK_VIS |
								 MGT_CHEADERMASK_RESIZE |
								 MGT_CHEADERMASK_SEL |
								 MGT_CHEADERMASK_COLMASK;
	cheader.visible = TRUE;
	cheader.selectable = FALSE;
	cheader.resizeable = FALSE;
	cheader.colmask = MGT_COLMASK_LABEL | MGT_COLMASK_WIDTH;
	SendMessage(hta, MGT_TAM_SETCOLHEADER, 0, (LPARAM)&cheader);

	//row header
	SendMessage(hta, MGT_TAM_GETROWHEADER, 0, (LPARAM)&rheader);
	rheader.mask = MGT_RHEADERMASK_VIS |
								 MGT_RHEADERMASK_SEL |
								 MGT_CHEADERMASK_RESIZE |
								 MGT_RHEADERMASK_ROWMASK;
	rheader.visible = TRUE;
	rheader.selectable = FALSE;
	rheader.resizeable = FALSE;
	rheader.rowmask = MGT_ROWMASK_NONE;
	SendMessage(hta, MGT_TAM_SETROWHEADER, 0, (LPARAM)&rheader);

	//1) inputs
	memset(&chead, 0, sizeof(MGTCHEADPARAMS));
	chead.mask = MGT_CHEADMASK_LABEL |
							 MGT_CHEADMASK_WIDTH |
							 MGT_CHEADMASK_COLOR |
							 MGT_CHEADMASK_SPEC;
	lstrcpyn(chead.label, D_("Input"), MGT_CHEADLABEL_LEN);
	chead.width = 100;
	chead.color = MGC_COLOR_BLACK;
	chead.spec.type = MGC_TYPE_UINT;
	chead.spec.align = MGC_VALALIGN_CT;
	chead.spec.prec = 0;
	chead.spec.mask = MGC_VALMASK_NONE;
	chead.spec.ctr = MGC_CTR_COMBOBOX;
	chead.spec.data = &d_cbi_data;
	SendMessage(hta, MGT_TAM_SETCOLHEAD, 0, (LPARAM)&chead);		//

	//2) trigger edge column
	memset(&chead, 0, sizeof(MGTCHEADPARAMS));
	chead.mask = MGT_CHEADMASK_LABEL |
							 MGT_CHEADMASK_WIDTH |
							 MGT_CHEADMASK_COLOR |
							 MGT_CHEADMASK_SPEC;
	lstrcpyn(chead.label, D_("Trigger edge"), MGT_CHEADLABEL_LEN);
	chead.width = 100;
	chead.color = MGC_COLOR_BLACK;
	chead.spec.type = MGC_TYPE_UINT;
	chead.spec.align = MGC_VALALIGN_CT;
	chead.spec.prec = 0;
	chead.spec.mask = MGC_VALMASK_NONE;
	chead.spec.ctr = MGC_CTR_COMBOICONBOX;
	chead.spec.data = &d_cbt_data;
	SendMessage(hta, MGT_TAM_SETCOLHEAD, 1, (LPARAM)&chead);		//

	//3) action column
	memset(&chead, 0, sizeof(MGTCHEADPARAMS));
	chead.mask = MGT_CHEADMASK_LABEL |
							 MGT_CHEADMASK_WIDTH |
							 MGT_CHEADMASK_COLOR |
							 MGT_CHEADMASK_SPEC;
	lstrcpyn(chead.label, D_("Action"), MGT_CHEADLABEL_LEN);
	chead.width = 100;
	chead.color = MGC_COLOR_BLACK;
	chead.spec.type = MGC_TYPE_UINT;
	chead.spec.align = MGC_VALALIGN_CT;
	chead.spec.prec = 0;
	chead.spec.mask = MGC_VALMASK_NONE;
	chead.spec.ctr = MGC_CTR_COMBOBOX;
	chead.spec.data = &d_cba_data;
	SendMessage(hta, MGT_TAM_SETCOLHEAD, 2, (LPARAM)&chead);		//

	//4) filter column
	memset(&chead, 0, sizeof(MGTCHEADPARAMS));
	chead.mask = MGT_CHEADMASK_LABEL |
							 MGT_CHEADMASK_WIDTH |
							 MGT_CHEADMASK_COLOR |
							 MGT_CHEADMASK_SPEC |
							 MGT_CHEADMASK_DIS;
	lstrcpyn(chead.label, D_("Filter"), MGT_CHEADLABEL_LEN-7);
	lstrcat(chead.label, TEXT(" [ms]"));
	chead.width = 100;
	chead.color = MGC_COLOR_BLACK;
	chead.spec.type = MGC_TYPE_UINT;
	chead.spec.align = MGC_VALALIGN_RT;
	chead.spec.prec = 0;
	chead.spec.mask = MGC_VALMASK_MINMAX;
	chead.spec.min.u = 0;
	chead.spec.max.u = 255;
	chead.disable = FALSE;
	SendMessage(hta, MGT_TAM_SETCOLHEAD, 3, (LPARAM)&chead);		//

	return(hta);
}

/** @brief
 *
 * @param hta HWND
 * @return BOOL
 *
 */
BOOL D_DestroySPTable2(HWND hta)
{
	void *mem;

	if (hta) {

		//get string array to table
		mem = (void *)GetWindowLong(hta, GWLP_USERDATA);
		if (mem) {
			free(mem);
		}

		return(DestroyWindow(hta));
	}

	return(FALSE);
}

/** @brief
 *
 * @param hta HWND
 * @param prop D_DEV_PROP*
 * @param prop_rows int
 * @param inputs int
 * @return BOOL
 *
 */
BOOL D_WriteSPTable2(HWND hta, D_DEV_PROP *prop, unsigned int prop_rows, unsigned int inputs)
{
	//table
	DWORD r;
	DPOINT p;
	MGTCELL cell;

	if (hta == NULL || prop == NULL)
		return(FALSE);

	for (r=0; r<prop_rows; r++) {
		memset(&cell, 0, sizeof(MGTCELL));
		//set input
		p.x = 0;
		p.y = r;
		cell.val.u = D_CFLAG_IGET(prop[r].val.u);
		if (cell.val.u > inputs)
			cell.val.u = 0;
		SendMessage(hta, MGT_TAM_SETCELL, (WPARAM)&p, (LPARAM)&cell);

		//set trigger edge cell
		p.x = 1;
		p.y = r;
		cell.val.u = D_CFLAG_TGET(prop[r].val.u);
		if (cell.val.u)
			cell.val.u--;
		SendMessage(hta, MGT_TAM_SETCELL, (WPARAM)&p, (LPARAM)&cell);

		//set action
		p.x = 2;
		p.y = r;
		cell.val.u = D_CFLAG_AGET(prop[r].val.u);
		SendMessage(hta, MGT_TAM_SETCELL, (WPARAM)&p, (LPARAM)&cell);

		//set filter
		p.x = 3;
		p.y = r;
		cell.val.u = D_CFLAG_FGET(prop[r].val.u);
		SendMessage(hta, MGT_TAM_SETCELL, (WPARAM)&p, (LPARAM)&cell);
	}
	return(TRUE);
}

/** @brief
 *
 * @param hga HWND
 * @param prop D_DEV_PROP*
 * @param prop_rows int
 * @param inputs int
 * @return BOOL
 *
 */
BOOL D_ReadSPTable2(HWND hga, D_DEV_PROP *prop, unsigned int prop_rows, unsigned int inputs)
{
	//table
	DWORD r;
	DPOINT p;
	MGTCELL cell;

	if (hga == NULL || prop == NULL)
		return(FALSE);

	for (r=0; r<prop_rows; r++) {
		prop[r].val.u = 0;

		memset(&cell, 0, sizeof(MGTCELL));
		//get input cell
		p.x = 0;
		p.y = r;
		SendMessage(hga, MGT_TAM_GETCELL, (WPARAM)&p, (LPARAM)&cell);
		if (cell.val.u > inputs)
			cell.val.u = inputs;
		prop[r].val.u |= D_CFLAG_ISET(cell.val.u);

		//get trigger edge cell
		p.x = 1;
		p.y = r;
		SendMessage(hga, MGT_TAM_GETCELL, (WPARAM)&p, (LPARAM)&cell);
		prop[r].val.u |= D_CFLAG_TSET(cell.val.u+1);

		//get action
		p.x = 2;
		p.y = r;
		SendMessage(hga, MGT_TAM_GETCELL, (WPARAM)&p, (LPARAM)&cell);
		prop[r].val.u |= D_CFLAG_ASET(cell.val.u);

		//get filter
		p.x = 3;
		p.y = r;
		SendMessage(hga, MGT_TAM_GETCELL, (WPARAM)&p, (LPARAM)&cell);
		prop[r].val.u |= D_CFLAG_FSET(cell.val.u);
	}
	return(TRUE);
}

//---------
//*** function tests for SM value (according prev. state and cfg. property) through inputs
D_SMVAL D_TestInputSMEvents(unsigned int prev_val, unsigned int val, D_DEV_PROP *cprop, unsigned int inputs, D_SMVAL *smdir)
{
	D_SMVAL sm = 0, dir = 0;
	unsigned int p, tchange, cval, cmask, act;

	//check
	if (cprop == NULL || prev_val == val)
		return(sm);

	//get changes only
	tchange = prev_val ^ val;		//get changes only

	//through inputs
	for (p=0; p<inputs; p++) {
		cval = (cprop+p)->val.u;
		cmask = (1<<p);
		if ((tchange & cmask) && 		//signal changed
				(cval & D_CFLAG_TMASK))	{		//signal is enabled
			if (((cval & D_CFLAG_TRIGDOWN) && (val & cmask) == 0) ||		//downward signal
					((cval & D_CFLAG_TRIGUP) && (val & cmask))) {		//upward signal

				//action
				switch(cval & D_CFLAG_AMASK) {
					case D_CFLAG_ACTMARK: 	act = D_SMMASK_ACT_MARK; break;		//mark
					case D_CFLAG_ACTSTART: 	act = D_SMMASK_ACT_START; break;	//start
					case D_CFLAG_ACTSTOP: 	act = D_SMMASK_ACT_STOP; break;		//stop
					case D_CFLAG_ACTZERO: 	act = D_SMMASK_ACT_ZERO; break;		//zero
					case D_CFLAG_ACTFCW: 		act = D_SMMASK_ACT_FCW; break;		//C/W
					case D_CFLAG_ACTSPEC: 	act = D_SMMASK_ACT_SPEC; break;		//spec
					default: act = 0; break;
				}
				sm |= act;

				//direction
				if (act && (val & cmask)) {
					dir |= act;
				}
			}
		}
	}

	if (smdir)
		*smdir = dir;

	return(sm);
}

/** @brief
 *
 * @param prev_val unsigned int
 * @param val unsigned int
 * @param cprop D_DEV_PROP*
 * @param prop_rows unsigned int
 * @param inputs unsigned int
 * @param smdir D_SMVAL*
 * @return D_SMVAL
 *
 */
D_SMVAL D_TestInputSMEvents2(unsigned int prev_val, unsigned int val, D_DEV_PROP *cprop, unsigned int prop_rows, unsigned int inputs, D_SMVAL *smdir)
{
	D_SMVAL sm = 0, dir = 0;
	unsigned int p, tchange, cval, inp, cmask, act;

	//check
	if (cprop == NULL || prev_val == val)
		return(sm);

	//get changes only
	tchange = prev_val ^ val;		//get changes only

	//through inputs
	for (p=0; p<prop_rows; p++) {
		cval = (cprop+p)->val.u;
		//get selected input
		inp = D_CFLAG_IGET(cval);
		if (inp > 0 && inp <= inputs) {
			inp--;

			cmask = (1<<inp);
			if ((tchange & cmask) && 		//signal changed
					(cval & D_CFLAG_TMASK))	{		//signal is enabled
				if (((cval & D_CFLAG_TRIGDOWN) && (val & cmask) == 0) ||		//downward signal
						((cval & D_CFLAG_TRIGUP) && (val & cmask))) {		//upward signal

					//action
					switch(cval & D_CFLAG_AMASK) {
						case D_CFLAG_ACTMARK: 	act = D_SMMASK_ACT_MARK; break;		//mark
						case D_CFLAG_ACTSTART: 	act = D_SMMASK_ACT_START; break;	//start
						case D_CFLAG_ACTSTOP: 	act = D_SMMASK_ACT_STOP; break;		//stop
						case D_CFLAG_ACTZERO: 	act = D_SMMASK_ACT_ZERO; break;		//zero
						case D_CFLAG_ACTFCW: 		act = D_SMMASK_ACT_FCW; break;		//C/W
						case D_CFLAG_ACTSPEC: 	act = D_SMMASK_ACT_SPEC; break;		//spec
						default: act = 0; break;
					}
					sm |= act;

					//direction
					if (act && (val & cmask)) {
						dir |= act;
					}
				}
			}
		}
	}

	if (smdir)
		*smdir = dir;

	return(sm);
}

//*** function sends SM-signal
BOOL D_SendSMEvents(HDEVICE hD, D_SMVAL sm, double smtime)
{
	if (sm == 0)
		return(FALSE);

	if ((sm & D_SMMASK_ACT_START))		//start
		D_SendExStartMsg(hD, smtime);	//send start message (top priority)
	if ((sm &D_SMMASK_ACT_MARK))		//mark
		D_SendExMarkMsg(hD, smtime, NULL);		//send mark message
	if ((sm & D_SMMASK_ACT_STOP))		//stop
		D_SendExStopMsg(hD, smtime);		//send stop message
	if ((sm & D_SMMASK_ACT_ZERO))		//zero
		D_SendExZeroMsg(hD);		//send zero message
	if ((sm & D_SMMASK_ACT_FCW))		//C/W
		D_SendExFracCWMsg(hD);	//send C/W message

	return(TRUE);
}

//---------------------

//*** generate data for ComboBox (int * mul -> string), string has to be enough long
DWORD D_GenerateCBIntData2(TCHAR *str, int count, const int *values, double mul)
{
	if (str == NULL || (count && values == NULL))
		return(ERROR_INVALID_HANDLE);

	//search by zero
	if (count < 0) {
		count = 0;
		while (values[count] != 0)
			count++;
	}

	//generate names
	while (count--) {
		__sprintf(str, TEXT("%g"), (double)*values * mul);
		str += __strlen(str) + 1;
		values++;
	}
	*str = '\0';

	return(NO_ERROR);
}

//*** generate data for ComboBox (dbl*mul -> string), string has to be enough long
DWORD D_GenerateCBDblData2(TCHAR *str, int count, const double *values, double mul)
{
	if (str == NULL || (count && values == NULL))
		return(ERROR_INVALID_HANDLE);

	//search by zero
	if (count < 0) {
		count = 0;
		while (values[count] != 0.0)
			count++;
	}

	//generate names
	while (count--) {
		__sprintf(str, TEXT("%g"), *values * mul);
		str += __strlen(str) + 1;
		values++;
	}
	*str = '\0';

	return(NO_ERROR);
}

//------------- temp dir -----------

static TCHAR cstr_tempdir[MAX_PATH] = TEXT("");

BOOL D_TempDirSet(const TCHAR *path)
{
	if (path) {
		__strncpy(cstr_tempdir, path, MAX_PATH);
		return(TRUE);
	}
	return(FALSE);
}

TCHAR *D_TempDirGet()
{
	return(cstr_tempdir);
}

/** @brief Create device temporary file (must be freed by user!)
 *
 * @param hD HDEVICE
 * @return TCHAR*
 *
 */
TCHAR *D_TempDirAllocFilename(HDEVICE hD, const TCHAR *name)
{
	TCHAR *ptext = NULL, *psn;

	if (hD) {
		//allocation
		ptext = (TCHAR *)malloc((2*MAX_PATH+1) * sizeof(TCHAR));
		if (ptext) {
			psn = D_GetDevSerialNumber(hD);
			__sprintf(ptext, TEXT("%s%s_%s_%s"),
								D_TempDirGet(),
								D_GetDeviceName(hD),
								psn ? psn : TEXT(""),
								name ? name : TEXT("dev.log"));

		}
	}
	return(ptext);
}

//-------------

/** @brief Evoke device error to application (reaction according severity)
 *
 * @param hD HDEVICE
 * @param severity D_SEVERITY
 * @param error int
 * @param descr TCHAR*
 * @return int
 *
 */
DWORD D_EvokeError(HDEVICE hD, D_SEVERITY severity, int error, TCHAR *descr)
{
	D_DEVICE *pD;
	D_DEVERROR derror;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE *)hD;

	//process dev. error message
	derror.hD = hD;
	GetLocalTime(&derror.st);
	derror.severity = severity;
	derror.error = error;
	derror.descr = descr;
	return(SendMessage(pD->_hwnd, WMD_DEVERROR, 0, (LPARAM)&derror));
}


//-------------

/** @brief Write value into property
 *
 * @param hD HDEVICE
 * @param pidx WORD
 * @param val D_VAL
 * @return DWORD
 *
 */
DWORD D_WritePropValue(HDEVICE hD, WORD pidx, D_VAL val)
{
	D_DEVICE *pD;
	DWORD ret;

	if (hD == NULL)
		return(ERROR_INVALID_HANDLE);
	pD = (D_DEVICE*)hD;

	if (pD->p == NULL || pidx >= pD->n_p)
		return(ERROR_INVALID_PARAMETER);

	ret = NO_ERROR;
	pD->p[pidx].val = val;
	pD->p[pidx].time = timer_GetTime();
	pD->p[pidx].lerr = ret;

	return(ret);
}

//----- SYSDEV -----------------

//*** function processes IPROC function for working with strings
//IPTYPE_CREATE: iparam -> pointer to lparam of item, lparam -> LPARAM
//IPTYPE_DELETE: iparam -> pointer to lparam of item, lparam -> 0
//IPTYPE_PRINT: iparam -> pointer to lparam of item, lparam -> FILE *
//IPTYPE_COMPARE: iparam -> pointer to lparam of item, lparam of other item -> LPARAM
BOOL D_LIST_IProcSignal(LPARAM *iparam, IPROCTYPE type, LPARAM lparam)
{
	return(LIST_IProcMemoryParam(sizeof(D_DEV_SIGNALINFO), iparam, type, lparam));
}

/** @brief List all signals to all sys-devices
 *
 * @param pDs HDEVICES*
 * @return DWORD
 *
 */
DWORD D_SYSD_ListSignalsTo(HDEVICES *pDs)
{
	if (pDs == NULL || pDs->hd == NULL)
		return(ERROR_INVALID_HANDLE);

	//list signals + sys devices
	HLIST hlist;
	unsigned n_sys = 0;

	hlist = LIST_Create(D_LIST_IProcSignal);

	//through devices
	WORD d, s;
	D_DEVICE *pD;
	D_DEV_SIGNALINFO sinfo;
	for (d=0; d<pDs->dcount; d++) {
		pD = (D_DEVICE*)pDs->hd[d];
		//count sys devices
		if (pD->model & D_MODEL_SYSDEVICE) {
			n_sys++;
		}
		//through signals
		for (s=0; s<pD->n_s; s++) {
			sinfo.sid = D_GetSignalID(d, s);
			sinfo.signal = pD->s[s];
			//sinfo.unit =
			LIST_AppendItem(hlist, (LPARAM)&sinfo);
		}
	}

	//through device again (process signals to sys-devices)
	unsigned n = LIST_GetSize(hlist);
	for (d=0; d<pDs->dcount; d++) {
		pD = (D_DEVICE*)pDs->hd[d];
		//through sys devices
		if (pD->model & D_MODEL_SYSDEVICE) {
			//send count
			if (pD->sysfce.pSetSignalCount == NULL || !pD->sysfce.pSetSignalCount((HDEVICE)pD, n)) {
				continue;	//FALSE -> skip listing
			}

			//send signals
			if (n && pD->sysfce.pSetSignalInfo) {
				//through list
				D_DEV_SIGNALINFO *psinfo;
				HITEM hitem = LIST_GetItem(hlist, LPOS_FIRST, 0);
				while (hitem) {
					if (LIST_GetItemValue(hitem, (LPARAM*)&psinfo) && psinfo) {
						if (!pD->sysfce.pSetSignalInfo((HDEVICE)pD, psinfo->sid, &psinfo->signal))
							break;	//do not continue
					}
					hitem = LIST_GetItem(hitem, LPOS_NEXT, 0);
				}

			}
		}
	}

	LIST_Discard(hlist);

	return(NO_ERROR);
}

/** @brief Test if device select this signal (by sid)
 *
 * @param hD HDEVICE
 * @param sid DWORD
 * @return BOOL
 *
 */
BOOL D_SYSD_IsSignalSelected(HDEVICE hD, DWORD sid)
{
	if (hD == NULL)
		return(FALSE);	//invalid handle

	D_DEVICE *pD = (D_DEVICE*)hD;
	if ((pD->model & D_MODEL_SYSDEVICE) == 0)
		return(FALSE);	//no sysdev

	if (pD->sysfce.pSelectSignal == NULL || pD->sysfce.pNotifySignalData == NULL || !pD->sysfce.pSelectSignal(hD, sid))
		return(FALSE);	//not selected or not supported

	//selected
	return(TRUE);
}

/** @brief Generate device array which selected the signal (by sid)
 *
 * @param pDs HDEVICES*
 * @param sid DWORD
 * @return HDEVICE* - null, or handle to buffer (need to be freed by user)
 *
 */
HDEVICE *D_SYSD_GenerateDevices(HDEVICES *pDs, DWORD sid)
{
	//test handles
	if (pDs == NULL || pDs->hd == NULL || pDs->dcount == 0)
		return(NULL);	//invalid handles, no devices

	//allocate max. device array (last item has to be null!)
	HDEVICE *hbuf = malloc((pDs->dcount+1)*sizeof(HDEVICE));
	if (hbuf == NULL)
		return(NULL);	//memory error
	memset(hbuf, 0, (pDs->dcount+1)*sizeof(HDEVICE));

	//fill array
	int n = 0, i;
	for (i=0; i<pDs->dcount; i++) {
		if (D_SYSD_IsSignalSelected(pDs->hd[i], sid)) {
			hbuf[n] = pDs->hd[i];
			n++;
		}
	}

	//nothing selected
	if (n == 0) {
		free((void*)hbuf);
		hbuf = NULL;
	}

	//return array
	return(hbuf);
}

/** @brief Notify sys-device with measured data for selected signal
 *
 * @param hDs HDEVICE*
 * @param sid DWORD
 * @param count DWORD
 * @param dtime double*
 * @param value double*
 * @return BOOL
 *
 */
BOOL D_SYSD_NotifyDataTo(HDEVICE *hDs, DWORD sid, DWORD count, double *dtime, double *value)
{
	assert(hDs);

	D_DEVICE *pD;

	//through sys-devices
	while (*hDs) {
		pD = (D_DEVICE*)*hDs;

		assert(pD->sysfce.pNotifySignalData);
		pD->sysfce.pNotifySignalData(*hDs, sid, count, dtime, value);

		//next sys-device
		hDs++;
	}

	return(TRUE);
}
