/******************************************************************************
 *   DTV LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *   COPYRIGHT(c) 1998-2010 by LG Electronics Inc.
 *
 *   All rights reserved. No part of this work covered by this copyright hereon
 *   may be reproduced, stored in a retrieval system, in any form
 *   or by any means, electronic, mechanical, photocopying, recording
 *   or otherwise, without the prior written  permission of LG Electronics.
 ******************************************************************************/

/** @file
 *
 *  Brief description.
 *  Detailed description starts here.
 *
 *  @author		root
 *  @version	1.0
 *  @date		2010-01-03
 *  @note		Additional information.
 */

/*------------------------------------------------------------------------------
	Control Constants
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
	File Inclusions
------------------------------------------------------------------------------*/
#include <sys/poll.h>
#include "osa_kadp.h"
#include "debug_kadp.h"
#include "afe_kadp.h"

#include "adc_resolution_m17ax.h"
#include "adc_resolution_m17cx.h"
#include "adc_resolution_o18ax.h"
/*------------------------------------------------------------------------------
	Constant Definitions
------------------------------------------------------------------------------*/
#define	AFE_DEVICE	"/dev/lg/afe0"
#define KADP_AFE	"kad-afe"

/*------------------------------------------------------------------------------
	Macro Definitions
------------------------------------------------------------------------------*/
#define AFE_NOTI(fmt, args...) 		KADP_LOGM_PRINT(g_afe_logm_fd, LX_LOGM_LEVEL_NOTI, fmt, ##args)
#define AFE_PRINT(fmt, args...) 	KADP_LOGM_PRINT(g_afe_logm_fd, LX_LOGM_LEVEL_INFO, fmt, ##args)
#define AFE_WARN(fmt, args...) 		KADP_LOGM_PRINT(g_afe_logm_fd, LX_LOGM_LEVEL_WARNING, fmt, ##args)
#define AFE_ERROR(fmt, args...) 	KADP_LOGM_PRINT(g_afe_logm_fd, LX_LOGM_LEVEL_ERROR, fmt, ##args)
#define AFE_DEBUG(fmt, args...) 	KADP_LOGM_PRINT(g_afe_logm_fd, LX_LOGM_LEVEL_DEBUG, fmt, ##args)
#define AFE_TRACE(fmt, args...) 	KADP_LOGM_PRINT(g_afe_logm_fd, LX_LOGM_LEVEL_TRACE, fmt, ##args)

#define AFE_TRACE_BEGIN()		AFE_TRACE("[de] %s:%d -- BEGIN\n", __F__, __L__)
#define AFE_TRACE_END()			AFE_TRACE("[de] %s:%d -- END\n", __F__, __L__ )
#define AFE_TRACE_MARK()		AFE_TRACE("[de] %s:%d -- TRACE !!!\n", __F__, __L__ )

#define AFE_INIT_LOCK()			g_afe_ctx.dev_mtx = KADP_OSA_OpenSema("kadp-afe-mtx", LX_OSA_GLOBAL_SEMA, 1);
#define AFE_LOCK()				(void)KADP_OSA_LockSema( g_afe_ctx.dev_mtx, LX_OSA_INF_WAIT)
#define AFE_UNLOCK()			(void)KADP_OSA_UnlockSema( g_afe_ctx.dev_mtx )

#define AFE_CHECK_CODE(__checker,__if_action,fmt,args...)   \
						__CHECK_IF_ERROR(__checker, AFE_ERROR, __if_action , fmt, ##args )
#define L8_HSTART_SHIFT_DUE_TO_DE_CROP	1

#define	VS_WIDTH_MSPG_209	0x141c
#define	VS_WIDTH_MSPG_227	0x2838

#define ADC_OTP_REG_SIZE	12
/*------------------------------------------------------------------------------
	Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	int 					dev_fd;
	LX_OSA_OBJECT_T 		dev_mtx;	/* mutex(semaphore) for AFE  device */

	int 					dev_init_cvd;
	int 					dev_init_adc;
	int						ref_cnt;	/* reference count */
}
LX_AFE_CXT_T;

/*------------------------------------------------------------------------------
	External Function Prototype Declarations
------------------------------------------------------------------------------*/
extern int KADP_SE_GetADC(int index, UINT8 *pData);
extern int KADP_SE_SetADC(int index, UINT8 *pData);

/*------------------------------------------------------------------------------
	External Variables
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
	global Variables
------------------------------------------------------------------------------*/
int g_afe_kadp_revision = 0x22060800;

BOOLEAN	_gAFE_ADC_CALIB_MODE			= FALSE;
int 	_gAFE_Enable_SCART_RGB_WA = 0;
int g_afe_logm_fd = -1;
LX_AFE_CVD_SOURCE_ATTRIBUTE_T _gSource_attribute = LX_CVD_INPUT_SOURCE_ATTRIBUTE_RF;

LX_AFE_CVD_BYPASS_CONTROL_T	_g_cvd_bypass_control_t;

LX_AFE_CVD_SUPPORT_COLOR_SYSTEM_T	_g_support_color_system = LX_COLOR_SYSTEM_MULTI;
static pthread_t _gADCOTPThread  = 0;
static pthread_attr_t _gADCOTPThreadAttr;
static BOOLEAN gIsADCOTPThreadAlive = FALSE;
/*------------------------------------------------------------------------------
	Static Function Prototypes Declarations
------------------------------------------------------------------------------*/
static void KADP_AFE_OTP_Thread(void);
/*------------------------------------------------------------------------------
	Static Variables
------------------------------------------------------------------------------*/
typedef enum {
	CVD_NO_SIGNAL,
	CVD_HLOCK,
	CVD_VLOCK,
	CVD_HLOCK_OR_VLOCK,
	CVD_HLOCK_AND_VLOCK,
	CVD_HLOCK_OR_VLOCK_AND_SIGNAL,
	CVD_HLOCK_AND_VLOCK_AND_SIGNAL,
	CVD_HLOCK_AND_VLOCK_AND_REPEAT,
} LX_AFE_CVD_SYNC_STATUS_TO_CHECK;

//111122 wonsik.do
//static UINT32 	g_CVD_status_to_check = CVD_HLOCK_OR_VLOCK;
static UINT32 	g_CVD_status_to_check = CVD_VLOCK;
//UINT32 	g_CVD_status_to_check_ATV_Search = CVD_HLOCK_AND_VLOCK;
UINT32 	g_CVD_status_to_check_ATV_Search = CVD_VLOCK;	//141210 : To Fix channel skip(Philippine NTSC C30)
//static LX_AFE_CVD_SUPPORT_COLOR_SYSTEM_T	g_support_color_system = LX_COLOR_SYSTEM_MULTI;
static LX_AFE_CXT_T 	g_afe_ctx =
{
	.dev_fd 	= -1,		/* dev_fd should be initialized to -1 */
	.dev_mtx	= NULL,
	.dev_init_cvd	= 0,
	.dev_init_adc	= 0,
	.ref_cnt	= 0,
};

LX_AFE_PCMODE_MODETABLE_T *pLX_Default_ModeTable = NULL;

// Resolution of PC Mode (VGA / YPbPr)
static KADP_AFE_RESOLUTION_T LX_Default_Resolution[LX_RES_MAXIMUM] =
{
    { 640,  350}, // 00: LX_RES_640X350
    { 640,  400}, // 01: LX_RES_640X400
    { 720,  400}, // 02: LX_RES_720X400
    { 640,  480}, // 03: LX_RES_640X480
    { 800,  600}, // 04: LX_RES_800X600
    { 832,  624}, // 05: LX_RES_832X624
    {1024,  768}, // 06: LX_RES_1024X768
    {1280, 1024}, // 07: LX_RES_1280X1024
    {1600, 1200}, // 08: LX_RES_1600X1200
    {1152,  864}, // 09: LX_RES_1152X864
    {1152,  870}, // 10: LX_RES_1152X870
    {1280,  768}, // 11: LX_RES_1280x768
    {1280,  960}, // 12: LX_RES_1280X960
    { 720,  480}, // 13: LX_RES_720X480
    {1920, 1080}, // 14: LX_RES_1920X1080

    {1280,  720}, // 15: LX_RES_1280X720
    { 720,  576}, // 16: LX_RES_720X576


    {1920, 1200}, // 17: LX_RES_1920X1200

    {1400, 1050}, // 18: LX_RES_1400X1050
    {1440,  900}, // 19: LX_RES_1440X900
    {1680, 1050}, // 20: LX_RES_1680X1050

    {1280,  800}, // 21: LX_RES_1280X800
    {1600, 1024}, // 22: LX_RES_1600X1024
    {1600,  900}, // 23: LX_RES_1600X900
    {1360,  768}, // 24: LX_RES_1360X768
    { 848,  480}, // 25: LX_RES_848X480
    {1920, 1080}, // 26: LX_RES_1920X1080P

    {1366,  768}, // 27: LX_RES_1366X768,
    { 864,  648}, // 28: LX_RES_864X648,
	{1152, 900},	// 29: LX_RES_1152X900,
	{1152, 720},	// 30: LX_RES_1152X720,
	{1728, 1080},	// 31: LX_RES_1728X1080,
};

#define LX_DEFAULT_MODE_TABLE_COUNT (sizeof(M17Ax_Default_ModeTable)/sizeof(LX_AFE_PCMODE_MODETABLE_T))

static LX_AFE_CVD_TIMING_INFO_T LX_Default_CVD_Table[] =
{  /* 0	system			hfreq	vfreq	htotal	hstart	vstart	hsize	vsize */
	{0,	LX_DEFAULT,		157,	599,	858,	246-150,		18,		704,	480},
	{0,	LX_NTSC_M,		157,	599,	858,	246-150,		18,		704,	480},
	{0,	LX_NTSC_J,		157,	599,	858,	246-150,		18,		704,	480},
	{0,	LX_NTSC_443,	157,	599,	858,	246-150,		18,		704,	480},
	{0,	LX_PAL_BG,		156,	500,	864,	243-150,		22,		704,	576},
	{0,	LX_PAL_N,		156,	500,	864,	243-150,		22,		704,	576},
	{0,	LX_PAL_M,		157,	599,	858,	246-150,		18,		704,	480},
	{0,	LX_PAL_CN,		156,	500,	864,	243-150,		22,		704,	576},
	{0,	LX_PAL_60,		157,	599,	858,	246-150,		18,		704,	480},
	{0,	LX_SECAM,		156,	500,	864,	243-150,		22,		704,	576},
//	{0,	LX_NON_STANDARD,	0,	0,	0,	0,	0,			0,		0},
//	{0,	LX_NON_STANDARD,		157,	599,	858,	246-150,		18,		704,	480},
	{0,	LX_NON_STANDARD,		156,	500,	864,	238-150,		22,		704,	576},
};
/*========================================================================================
	Implementation Group
========================================================================================*/
int KADP_AFE_Open(void)
{
	int 	dev_fd;
	int 	ret = RET_ERROR;

	KADP_InitSystem(); /* initialize kadp system if necessary */

	//AFE_CHECK_CODE( g_afe_ctx.dev_fd >= 0, return RET_OK, "[afe] %s : ignore multiple initialization\n", __F__ );
	/* if device is already opened, just increase ref_cnt and exit */
	if ( g_afe_ctx.dev_fd >= 0)
	{
		AFE_LOCK();
		g_afe_ctx.ref_cnt++;
		AFE_ERROR("[kadp_afe] %s : ignore multiple initialization\n", __F__ );
		AFE_UNLOCK();
		return RET_OK;
	}

	AFE_INIT_LOCK();

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	dev_fd = open( AFE_DEVICE, O_RDWR );
	AFE_CHECK_CODE( dev_fd < 0, goto func_exit, "[afe] %s : can't open AFE device (%s)\n", __F__, AFE_DEVICE );
	g_afe_ctx.dev_fd = dev_fd;
	g_afe_ctx.ref_cnt = 1;

	AFE_NOTI("[afe] device is opened\n" );
	ret = RET_OK; /* all work done */


//	KADP_AFE_Init_OTP_Thread();

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;

}

int KADP_AFE_IsOpen(void)
{
	if(g_afe_ctx.dev_fd < 0)
		return	RET_ERROR;
	else
		return	RET_OK;
}

/**
 *	AFE device clode
 *
 */
int KADP_AFE_Close(void)
{
	int	ret = RET_ERROR;

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	/* check AFE ref_cnt before closing the real device */
	if ( --g_afe_ctx.ref_cnt > 0 )
	{
		ret = RET_OK; goto func_exit;
	}

	ret = close( g_afe_ctx.dev_fd );
	AFE_CHECK_CODE( ret<0, goto func_exit, "[afe] %s : can't close AFE device (%s)\n", __F__, AFE_DEVICE );

	/* [TODO] any post clean up ? */
	g_afe_ctx.dev_fd = -1;
	g_afe_ctx.dev_init_cvd = 0;
	g_afe_ctx.dev_init_adc = 0;
	g_afe_ctx.ref_cnt = 0;

	AFE_NOTI("[afe] device is closed\n" );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	/* osa resource destroyed */
	if ( g_afe_ctx.ref_cnt <= 0 )
	{
		KADP_OSA_CloseSema( g_afe_ctx.dev_mtx );

		g_afe_ctx.dev_mtx 	= NULL;
	}

	return ret;
}

int KADP_AFE_ADC_InitializeModule(void)
{
	int	ret = RET_ERROR;

	if(RET_ERROR == KADP_AFE_IsOpen())
		KADP_AFE_Open();

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_ADC_Init();
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in KADP_AFE_ADC_Init() call\n", __F__ );

	ret = KADP_AFE_ADC_Set_Default_ModeTable();
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in KADP_AFE_ADC_Set_Default_ModeTable() call\n", __F__ );

func_exit:
	return ret;
}

int KADP_AFE_CVD_InitializeModule(void)
{
	int	ret = RET_ERROR;

	if(RET_ERROR == KADP_AFE_IsOpen())
		KADP_AFE_Open();

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_CVD_Init(LX_CVD_MAIN);
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in KADP_AFE_CVD_Init() call\n", __F__ );

#ifdef PIONEER_PLATFORM
	// moved to pdl_vbe_ave.c
//	KADP_AFE_CVD_VDAC_Power_Control(TRUE);
#endif
func_exit:
	return ret;
}

int KADP_AFE_ADC_UninitializeModule(void)
{
	int	ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_IsOpen();
	return ret;
}

int KADP_AFE_CVD_UninitializeModule(void)
{
	int	ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_IsOpen();
	return ret;
}
int KADP_AFE_ADC_EnableModule(void)
{
	int	ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_IsOpen();
	//ret = KADP_AFE_ADC_POWER_CONTROL(1);
	return ret;
}

int KADP_AFE_CVD_EnableModule(void)
{
	int	ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_IsOpen();
	//ret = KADP_AFE_CVD_POWER_CONTROL(1);
	return ret;
}

int KADP_AFE_ADC_DisableModule(void)
{
	int	ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_IsOpen();
	//ret = KADP_AFE_ADC_POWER_CONTROL(0);
	return ret;
}

int KADP_AFE_CVD_DisableModule(void)
{
	int	ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_IsOpen();
	//ret = KADP_AFE_CVD_POWER_CONTROL(0);
	return ret;
}

int KADP_AFE_ADC_ConnectModule(LX_AFE_ADC_INPUT_SOURCE_TYPE_T select_rgb_ypbpr, LX_AFE_ADC_INPUT_PORT_T select_adc_input_port)
{
	int	ret = RET_ERROR;

	LX_AFE_LVDS_SELECT_T lvds_sel = LX_AFE_LVDS0_SEL;
	LX_AFE_LVDS_PDB_T lvds_power = LX_AFE_LVDS_POWER_ON;
	LX_AFE_LVDS_DATA_TYPE_T lvds_type = LX_AFE_LVDS_TYPE_VESA;
	LX_AFE_LVDS_SOURCE_T lvds_source = LX_AFE_LVDS_SOURCE_CVD;
	LX_AFE_LVDS_MODE_T lvds_mode = LX_AFE_LVDS_MODE_NORMAL;

	AFE_NOTI("[%s] entered type[%d],port[%d]\n", __func__, select_rgb_ypbpr, select_adc_input_port);

	ret = KADP_AFE_IsOpen();

	//160614 : ADC power control is controlled in kdrv
//	KADP_AFE_ADC_POWER_CONTROL(1);

	if(select_rgb_ypbpr == LX_ADC_INPUT_SOURCE_RGB_SCART)
	{
		lvds_source = LX_AFE_LVDS_SOURCE_CVD;
		KADP_AFE_LVDS_Src_Control(lvds_sel, lvds_power, lvds_type, lvds_source, LX_AFE_LVDS_MODE_SCART_MIX);
	}
	else
	{
		lvds_source = LX_AFE_LVDS_SOURCE_3CH;
		KADP_AFE_LVDS_Src_Control(lvds_sel, lvds_power, lvds_type, lvds_source, lvds_mode);
	}

	KADP_AFE_ADC_Set_Source_Type(select_rgb_ypbpr, select_adc_input_port);
	KADP_AFE_ADC_Enable_Periodic_Signal_Info_Read(1);

	return ret;
}

int KADP_AFE_CVD_ConnectModule(LX_AFE_CVD_SOURCE_ATTRIBUTE_T select_source_attribute, LX_AFE_CVD_INPUT_PORT_T select_cvbs_input_port)
{
	int	ret = RET_ERROR;

	LX_AFE_LVDS_SELECT_T lvds_sel = LX_AFE_LVDS0_SEL;
	LX_AFE_LVDS_PDB_T lvds_power = LX_AFE_LVDS_POWER_ON;
	LX_AFE_LVDS_DATA_TYPE_T lvds_type = LX_AFE_LVDS_TYPE_VESA;
	LX_AFE_LVDS_SOURCE_T lvds_source = LX_AFE_LVDS_SOURCE_CVD;
	LX_AFE_LVDS_MODE_T lvds_mode = LX_AFE_LVDS_MODE_NORMAL;

	AFE_NOTI("[%s] entered attr[%d],port[%d]\n", __func__, select_source_attribute, select_cvbs_input_port);

	ret = KADP_AFE_IsOpen();

	if(select_source_attribute == LX_CVD_INPUT_SOURCE_ATTRIBUTE_SCART)
		KADP_AFE_LVDS_Src_Control(lvds_sel, lvds_power, lvds_type, lvds_source, LX_AFE_LVDS_MODE_SCART_MIX);
	else
		KADP_AFE_LVDS_Src_Control(lvds_sel, lvds_power, lvds_type, lvds_source, lvds_mode);

	ret |= KADP_AFE_CVD_POWER_CONTROL(1);

	if(ret < 0)
	{
		AFE_ERROR("[%s] CVD Connect Error [%d] !!!\n", __func__, ret);
		return ret;
	}

	KADP_AFE_CVD_Set_Source_Type(LX_CVD_MAIN, LX_CVD_INPUT_SOURCE_CVBS, select_cvbs_input_port, LX_AFE_CVBS_NONE, select_source_attribute);

	if(select_source_attribute == LX_CVD_INPUT_SOURCE_ATTRIBUTE_RF)
		KADP_AFE_Set_Scart_Overlay(FALSE);
	else if(select_source_attribute == LX_CVD_INPUT_SOURCE_ATTRIBUTE_AV)
	{
		KADP_AFE_Set_Scart_Overlay(FALSE);
		KADP_AFE_CVD_SetSyncDetectionForTuning(FALSE);	//130206 : DTV only tunning
	}
	else 
	{
		KADP_AFE_Set_Scart_Overlay(TRUE);
		KADP_AFE_CVD_SetSyncDetectionForTuning(FALSE);	//130206 : DTV only tunning
	}

	KADP_AFE_CVD_Start_Timer(LX_CVD_MAIN, 0);

	return ret;
}

int KADP_AFE_ADC_DisconnectModule(void)
{
	int	ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_IsOpen();

	KADP_AFE_ADC_POWER_CONTROL(0);
	KADP_AFE_ADC_Enable_Periodic_Signal_Info_Read(0);

	return ret;
}

int KADP_AFE_CVD_DisconnectModule(void)
{
	int	ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	ret = KADP_AFE_IsOpen();

	KADP_AFE_CVD_POWER_CONTROL(0);
	KADP_AFE_CVD_Stop_Timer(LX_CVD_MAIN);

	return ret;
}


int KADP_AFE_ADC_Init(void)
{
	int ret = RET_ERROR;

	AFE_NOTI("[%s] entered : ver[%x] \n", __func__, g_afe_kadp_revision);

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	if(pLX_Default_ModeTable == NULL)
	{
		if (lx_chip_rev() >= LX_CHIP_REV(O18, A0))
			pLX_Default_ModeTable = &O18Ax_Default_ModeTable[0];
		if (lx_chip_rev() >= LX_CHIP_REV(M17, C0))
			pLX_Default_ModeTable = &M17Cx_Default_ModeTable[0];
		else	//default M17
			pLX_Default_ModeTable = &M17Ax_Default_ModeTable[0];
	}

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IO_ADC_INIT);
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

	g_afe_ctx.dev_init_adc = 1;

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Fake_Init(void)
{
	int ret = RET_OK;

	AFE_NOTI("[%s] entered : ver[%x] \n", __func__, g_afe_kadp_revision);

	if(RET_ERROR == KADP_AFE_IsOpen())
		KADP_AFE_Open();

	if(pLX_Default_ModeTable == NULL)
	{
		if (lx_chip_rev() >= LX_CHIP_REV(O18, A0))
			pLX_Default_ModeTable = &O18Ax_Default_ModeTable[0];
		if (lx_chip_rev() >= LX_CHIP_REV(M17, C0))
			pLX_Default_ModeTable = &M17Cx_Default_ModeTable[0];
		else	//default M17
			pLX_Default_ModeTable = &M17Ax_Default_ModeTable[0];
	}

	g_afe_ctx.dev_init_adc = 1;

	return ret;
}

int KADP_AFE_CVD_Init(LX_AFE_CVD_SELECT_T select_main_sub)
{
	int ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_INIT, select_main_sub);
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

	g_afe_ctx.dev_init_cvd = 1;
func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Fake_Init(void)
{
	int ret = RET_OK;

	AFE_NOTI("[%s] entered \n", __func__);

	if(RET_ERROR == KADP_AFE_IsOpen())
		KADP_AFE_Open();

	g_afe_ctx.dev_init_cvd = 1;

	return ret;
}

int KADP_AFE_CVD_Set_Source_Type(LX_AFE_CVD_SELECT_T select_main_sub, LX_AFE_CVD_SOURCE_TYPE_T select_cvbs_svideo, LX_AFE_CVD_INPUT_PORT_T select_cvbs_input_port, LX_AFE_CVD_INPUT_PORT_T select_chroma_input_port, LX_AFE_CVD_SOURCE_ATTRIBUTE_T select_source_attribute)
{
	LX_AFE_CVD_SET_INPUT_T	CVD_Input_Info;
	int ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);
	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	CVD_Input_Info.cvd_main_sub = select_main_sub;
	CVD_Input_Info.cvd_input_source_type = select_cvbs_svideo;
	CVD_Input_Info.cvbs_input_port	= select_cvbs_input_port;
	CVD_Input_Info.chroma_input_port = select_chroma_input_port;
	CVD_Input_Info.cvd_input_source_attribute = select_source_attribute;
	_gSource_attribute = CVD_Input_Info.cvd_input_source_attribute;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_SET_SOURCE_TYPE, &CVD_Input_Info);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}
int KADP_AFE_ADC_Set_Source_Type(LX_AFE_ADC_INPUT_SOURCE_TYPE_T select_rgb_ypbpr, LX_AFE_ADC_INPUT_PORT_T select_adc_input_port)
{
	LX_AFE_ADC_SET_INPUT_T AFE_Input_Info;
	int ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);
	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	AFE_Input_Info.adc_input_source_type = select_rgb_ypbpr;
	AFE_Input_Info.adc_input_port = select_adc_input_port;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_SET_SOURCE_TYPE, &AFE_Input_Info);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}
int KADP_AFE_Set_Analog_Color_System(LX_AFE_CVD_SELECT_T select_main_sub, LX_AFE_CVD_SUPPORT_COLOR_SYSTEM_T 	color_system)
{
	LX_AFE_VIDEO_SYSTEM_INFO_T	CVD_Video_System_Info;
	int ret = RET_ERROR;

	AFE_PRINT("[%s] entered color_system [%d]\n", __func__, color_system);
	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	CVD_Video_System_Info.cvd_main_sub = select_main_sub;
	CVD_Video_System_Info.cvd_video_system = color_system;

	_g_support_color_system = color_system;

	//Temp for Multi System Test on ATSC System
	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_ANALOG_COLOR_SYSTEM, &CVD_Video_System_Info);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;

}

int KADP_AFE_CVD_SetSyncDetectionForTuning(BOOLEAN bEnable)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	AFE_PRINT("%s Entered width value %d \n",__func__, bEnable);

	if(bEnable == TRUE)	// For ATV Tunning
	{
		g_CVD_status_to_check = g_CVD_status_to_check_ATV_Search;
		AFE_PRINT("status to check in ATV searching [0x%x] \n", g_CVD_status_to_check);
//		g_CVD_status_to_check = CVD_HLOCK_AND_VLOCK_AND_REPEAT;
	}
	else
		g_CVD_status_to_check = CVD_VLOCK;
		//g_CVD_status_to_check = CVD_HLOCK_OR_VLOCK;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_HSYNC_DETECTION_FOR_TUNING, &bEnable);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Sync_Exist(LX_AFE_CVD_SELECT_T select_main_sub, BOOLEAN *cvd_sync)
{
	LX_AFE_CVD_STATES_INFO_T	CVD_States_Info;
	UINT32						status_to_check;
	UINT32						cvd_lock_status;
	UINT8						check_count;
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	if(cvd_sync == NULL)
		AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : Null Pointer in parameters\n", __F__ );

	CVD_States_Info.cvd_main_sub = select_main_sub;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_CVD_GET_STATES, &CVD_States_Info);

	cvd_lock_status = CVD_States_Info.cvd_status;

	if(select_main_sub != LX_CVD_MAIN)
	{
		*cvd_sync = FALSE;
		AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : No Sub CVD support\n", __F__ );
	}
	//if(CVD_States_Info.cvd_status & LX_NO_SIGNAL)
	//dbgprint("g_CVD_status_to_check = %d\n", g_CVD_status_to_check);
	switch (g_CVD_status_to_check)
	{
		case CVD_HLOCK_AND_VLOCK_AND_REPEAT:

			for (check_count = 0; check_count < 5; check_count++)
			{
				ret |= ioctl(g_afe_ctx.dev_fd, AFE_IOR_CVD_GET_STATES, &CVD_States_Info);
				cvd_lock_status &= CVD_States_Info.cvd_status;
			}

			if ( (cvd_lock_status & (LX_HLOCK | LX_VLOCK)) == (LX_HLOCK | LX_VLOCK) ) // If HLock AND VLock occures
				*cvd_sync = TRUE;
			else
				*cvd_sync = FALSE;
			break;

		case CVD_HLOCK_AND_VLOCK:

			if ( (cvd_lock_status & (LX_HLOCK | LX_VLOCK)) == (LX_HLOCK | LX_VLOCK) ) // If HLock AND VLock occures
			{
				*cvd_sync = TRUE;
				//AFE_PRINT("$ CVD H/V Locked : status[0x%x] $\n", cvd_lock_status);
			}
			else if ( ( (cvd_lock_status & LX_VLOCK) == LX_VLOCK ) \
					&& ( (cvd_lock_status & LX_NO_SIGNAL) != LX_NO_SIGNAL) \
					&& ( (cvd_lock_status & LX_VNON_STANDARD) == LX_VNON_STANDARD ) )
			{
				*cvd_sync = TRUE;
				AFE_PRINT("$$$$$ CVD [%s] in HDCT workaround ? : status[0x%x] $$$$$\n", __func__, cvd_lock_status);
			}
			else
				*cvd_sync = FALSE;
			break;

		case CVD_VLOCK:

			if ( (cvd_lock_status & LX_VLOCK) == LX_VLOCK ) // If VLock occures
				*cvd_sync = TRUE;
			else
				*cvd_sync = FALSE;
			break;


		case CVD_HLOCK_OR_VLOCK:
		default:
			if ( (cvd_lock_status & (LX_HLOCK | LX_VLOCK)) > 0)  // If HLock OR VLock occures
				*cvd_sync = TRUE;
			else
				*cvd_sync = FALSE;
			break;
	}

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Get_Lock_States(LX_AFE_CVD_SELECT_T select_main_sub, UINT32 *Cvd_Lock_States)
{
	LX_AFE_CVD_STATES_INFO_T	CVD_States_Info;
	int ret = RET_ERROR;

	if(Cvd_Lock_States == NULL)
		return RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	CVD_States_Info.cvd_main_sub = select_main_sub;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_CVD_GET_STATES, &CVD_States_Info);

	*Cvd_Lock_States = CVD_States_Info.cvd_status;

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}


/* Read reg_line625_detected, and determine 50Hz or 60Hz
   * return 1(reg_line625 set) : 50Hz
   * return 0(reg_line625 unset) : 60Hz
   */
int	KADP_AFE_CVD_Read_VFreq(LX_AFE_CVD_SELECT_T select_main_sub, UINT8 *is_line625)
{
	LX_AFE_CVD_STATES_INFO_T	CVD_States_Info;
	int ret = RET_ERROR;

	if(is_line625 == NULL)
		return RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	CVD_States_Info.cvd_main_sub = select_main_sub;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_CVD_GET_STATES, &CVD_States_Info);

	if(CVD_States_Info.cvd_status & LX_625LINES_DETECTED)
		*is_line625 = 1;
	else
		*is_line625 = 0;

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Get_Timing_Info(LX_AFE_CVD_SELECT_T select_main_sub, LX_AFE_CVD_TIMING_INFO_T *cvd_timing_info)
{
	int ret = RET_ERROR;
	static LX_AFE_VIDEO_SYSTEM_T	prev_cvd_standard = LX_DEFAULT;	///< cvd video standard
	static LX_AFE_VIDEO_SYSTEM_T	locked_cvd_standard = LX_DEFAULT;	///< cvd video standard
	static int	prev_vfreq = 0;
	LX_AFE_CVD_STATES_INFO_T	CVD_States_Info;

	int cvd_chromalock;
	int cvd_pal_detected;
	int cvd_secam_detected;
	int cs0_chromalock;
	int cs0_pal_detected;
	int cs0_secam_detected;
	int cvd_color_system_lock;
	int stable_count = 3;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	if(cvd_timing_info == NULL)
		AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : Null Pointer in parameters\n", __F__ );

	cvd_timing_info->cvd_main_sub = select_main_sub;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_CVD_GET_TIMING_INFO, cvd_timing_info);

	CVD_States_Info.cvd_main_sub = select_main_sub;

	ret |= ioctl(g_afe_ctx.dev_fd, AFE_IOR_CVD_GET_STATES, &CVD_States_Info);

	cvd_chromalock = CVD_States_Info.cvd_status & LX_CHROMALOCK;
	cvd_pal_detected = CVD_States_Info.cvd_status & LX_PAL_DETECTED;
	cvd_secam_detected = CVD_States_Info.cvd_status & LX_SECAM_DETECTED;
	cs0_chromalock = CVD_States_Info.cvd_status & LX_CS0_CHROMALOCK;
	cs0_pal_detected = CVD_States_Info.cvd_status & LX_CS0_PAL_DETECTED;
	cs0_secam_detected = CVD_States_Info.cvd_status & LX_CS0_SECAM_DETECTED;

	// enlarge stable_count when PAL/SECAM both detected !!!
	if( (cvd_timing_info->cvd_standard == LX_PAL_BG) && cs0_chromalock && cs0_secam_detected)
	{
		AFE_PRINT("### CVD GetTiming : PAL/SECAM both lock !!! : lock_count[%d],chroma[%d],pal[%d],secam[%d] ,cs0_chroma[%d],cs0_pal[%d],cs0_secam[%d]###\n",\
				cvd_timing_info->cvd_lock_stable_count, cvd_chromalock, cvd_pal_detected, cvd_secam_detected, cs0_chromalock, cs0_pal_detected, cs0_secam_detected);
		stable_count = 8;
	}
	else if ( _g_support_color_system == LX_COLOR_SYSTEM_NTSC_M )
		stable_count = 1;

	cvd_color_system_lock = ( (cvd_timing_info->cvd_standard >= LX_NTSC_M) && (cvd_timing_info->cvd_standard <= LX_NTSC_443) && cvd_chromalock ) \
							|| ( (cvd_timing_info->cvd_standard >= LX_PAL_BG) && (cvd_timing_info->cvd_standard <= LX_PAL_60) && cvd_chromalock && cvd_pal_detected ) \
							|| ( (cvd_timing_info->cvd_standard == LX_SECAM) && cvd_secam_detected );

	if(cvd_color_system_lock && (cvd_timing_info->cvd_lock_stable_count >= stable_count) )
	{
	}
	else if ( (cvd_timing_info->cvd_standard >= LX_NON_STANDARD) || (cvd_timing_info->cvd_standard < LX_NTSC_M) )
	{
		cvd_timing_info->cvd_standard = LX_NON_STANDARD;
	}
	else if ( ( (cvd_timing_info->cvd_lock_stable_count < stable_count) && (_gSource_attribute != LX_CVD_INPUT_SOURCE_ATTRIBUTE_RF) ) 	// added to remove transition color system
			|| ( (cvd_timing_info->cvd_lock_stable_count < stable_count) && (_gSource_attribute == LX_CVD_INPUT_SOURCE_ATTRIBUTE_RF) ) )	// added to remove transition color system
		cvd_timing_info->cvd_standard = prev_cvd_standard;

	if(prev_cvd_standard != cvd_timing_info->cvd_standard)
	{
		AFE_PRINT("### CVD GetTiming : Standard Changed [%d] => [%d] : lock_count[%d],chroma[%d],pal[%d],secam[%d] ,cs0_chroma[%d],cs0_pal[%d],cs0_secam[%d]###\n",\
				prev_cvd_standard ,cvd_timing_info->cvd_standard, cvd_timing_info->cvd_lock_stable_count, cvd_chromalock, cvd_pal_detected, cvd_secam_detected, cs0_chromalock, cs0_pal_detected, cs0_secam_detected);
		prev_cvd_standard = cvd_timing_info->cvd_standard;
		if(prev_cvd_standard < LX_NON_STANDARD)
			locked_cvd_standard = prev_cvd_standard;
	}

	// for 576i detection in NTSC only system
	/*
	if(cvd_timing_info->cvd_standard < LX_NON_STANDARD)
	{
		cvd_timing_info->u16_HFreq = LX_Default_CVD_Table[cvd_timing_info->cvd_standard].u16_HFreq;
		//cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[cvd_timing_info->cvd_standard].u16_VFreq;
		cvd_timing_info->u16_VFreq = cvd_timing_info->u16_VFreq_Stable;
		cvd_timing_info->u16_HTotal = LX_Default_CVD_Table[cvd_timing_info->cvd_standard].u16_HTotal;
		cvd_timing_info->u16_HStart = LX_Default_CVD_Table[cvd_timing_info->cvd_standard].u16_HStart;
		cvd_timing_info->u16_VStart = LX_Default_CVD_Table[cvd_timing_info->cvd_standard].u16_VStart;
		cvd_timing_info->u16_HSize = LX_Default_CVD_Table[cvd_timing_info->cvd_standard].u16_HSize;
		cvd_timing_info->u16_VSize = LX_Default_CVD_Table[cvd_timing_info->cvd_standard].u16_VSize;
	}
	// For 50Hz Only System
	else if (( g_support_color_system & (LX_COLOR_SYSTEM_NTSC_M |LX_COLOR_SYSTEM_PAL_M | LX_COLOR_SYSTEM_NTSC_443 | LX_COLOR_SYSTEM_PAL_60)) == 0 )
	{
		cvd_timing_info->u16_HFreq = LX_Default_CVD_Table[LX_PAL_BG].u16_HFreq;
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_PAL_BG].u16_VFreq;
		cvd_timing_info->u16_HTotal = LX_Default_CVD_Table[LX_PAL_BG].u16_HTotal;
		cvd_timing_info->u16_HStart = LX_Default_CVD_Table[LX_PAL_BG].u16_HStart;
		cvd_timing_info->u16_VStart = LX_Default_CVD_Table[LX_PAL_BG].u16_VStart;
		cvd_timing_info->u16_HSize = LX_Default_CVD_Table[LX_PAL_BG].u16_HSize;
		cvd_timing_info->u16_VSize = LX_Default_CVD_Table[LX_PAL_BG].u16_VSize;
	}
	// For 60Hz Only System
	else if ((  g_support_color_system & (LX_COLOR_SYSTEM_PAL_G |LX_COLOR_SYSTEM_PAL_NC | LX_COLOR_SYSTEM_SECAM)) == 0 )
	{
		cvd_timing_info->u16_HFreq = LX_Default_CVD_Table[LX_NTSC_M].u16_HFreq;
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_NTSC_M].u16_VFreq;
		cvd_timing_info->u16_HTotal = LX_Default_CVD_Table[LX_NTSC_M].u16_HTotal;
		cvd_timing_info->u16_HStart = LX_Default_CVD_Table[LX_NTSC_M].u16_HStart;
		cvd_timing_info->u16_VStart = LX_Default_CVD_Table[LX_NTSC_M].u16_VStart;
		cvd_timing_info->u16_HSize = LX_Default_CVD_Table[LX_NTSC_M].u16_HSize;
		cvd_timing_info->u16_VSize = LX_Default_CVD_Table[LX_NTSC_M].u16_VSize;
	}
	*/

	// Use stable VFreq to reduce 60Hz transition when channel change
	/*
	if(cvd_timing_info->u16_VFreq_Stable > 550)
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_NTSC_M].u16_VFreq;
	else
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_PAL_BG].u16_VFreq;
		*/
#if 1

	if(cvd_timing_info->u16_VFreq > 550)
	{
		cvd_timing_info->u16_HFreq = LX_Default_CVD_Table[LX_NTSC_M].u16_HFreq;
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_NTSC_M].u16_VFreq;
		cvd_timing_info->u16_HTotal = LX_Default_CVD_Table[LX_NTSC_M].u16_HTotal;
		cvd_timing_info->u16_HStart = LX_Default_CVD_Table[LX_NTSC_M].u16_HStart;
		cvd_timing_info->u16_VStart = LX_Default_CVD_Table[LX_NTSC_M].u16_VStart;
		cvd_timing_info->u16_HSize = LX_Default_CVD_Table[LX_NTSC_M].u16_HSize;
		cvd_timing_info->u16_VSize = LX_Default_CVD_Table[LX_NTSC_M].u16_VSize;
	}
	else
	{
		cvd_timing_info->u16_HFreq = LX_Default_CVD_Table[LX_PAL_BG].u16_HFreq;
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_PAL_BG].u16_VFreq;
		cvd_timing_info->u16_HTotal = LX_Default_CVD_Table[LX_PAL_BG].u16_HTotal;
		cvd_timing_info->u16_HStart = LX_Default_CVD_Table[LX_PAL_BG].u16_HStart;
		cvd_timing_info->u16_VStart = LX_Default_CVD_Table[LX_PAL_BG].u16_VStart;
		cvd_timing_info->u16_HSize = LX_Default_CVD_Table[LX_PAL_BG].u16_HSize;
		cvd_timing_info->u16_VSize = LX_Default_CVD_Table[LX_PAL_BG].u16_VSize;
	}

	if( (locked_cvd_standard == LX_PAL_BG) || (locked_cvd_standard == LX_PAL_N)\
			|| (locked_cvd_standard == LX_PAL_CN) || (locked_cvd_standard == LX_SECAM) )
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_PAL_BG].u16_VFreq;
	else
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_NTSC_M].u16_VFreq;
#else
	if( (locked_cvd_standard == LX_PAL_BG) || (locked_cvd_standard == LX_PAL_N)\
			|| (locked_cvd_standard == LX_PAL_CN) || (locked_cvd_standard == LX_SECAM) )
	{
		cvd_timing_info->u16_HFreq = LX_Default_CVD_Table[LX_PAL_BG].u16_HFreq;
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_PAL_BG].u16_VFreq;
		cvd_timing_info->u16_HTotal = LX_Default_CVD_Table[LX_PAL_BG].u16_HTotal;
		cvd_timing_info->u16_HStart = LX_Default_CVD_Table[LX_PAL_BG].u16_HStart;
		cvd_timing_info->u16_VStart = LX_Default_CVD_Table[LX_PAL_BG].u16_VStart;
		cvd_timing_info->u16_HSize = LX_Default_CVD_Table[LX_PAL_BG].u16_HSize;
		cvd_timing_info->u16_VSize = LX_Default_CVD_Table[LX_PAL_BG].u16_VSize;
	}
	else
	{
		cvd_timing_info->u16_HFreq = LX_Default_CVD_Table[LX_NTSC_M].u16_HFreq;
		cvd_timing_info->u16_VFreq = LX_Default_CVD_Table[LX_NTSC_M].u16_VFreq;
		cvd_timing_info->u16_HTotal = LX_Default_CVD_Table[LX_NTSC_M].u16_HTotal;
		cvd_timing_info->u16_HStart = LX_Default_CVD_Table[LX_NTSC_M].u16_HStart;
		cvd_timing_info->u16_VStart = LX_Default_CVD_Table[LX_NTSC_M].u16_VStart;
		cvd_timing_info->u16_HSize = LX_Default_CVD_Table[LX_NTSC_M].u16_HSize;
		cvd_timing_info->u16_VSize = LX_Default_CVD_Table[LX_NTSC_M].u16_VSize;
	}
#endif

	if( prev_vfreq != cvd_timing_info->u16_VFreq )
	{
		AFE_PRINT("### CVD GetTiming : VFreq Changed [%d] => [%d] : color_system[%d] lock_count[%d] ###\n",\
				prev_vfreq, cvd_timing_info->u16_VFreq ,cvd_timing_info->cvd_standard, cvd_timing_info->cvd_lock_stable_count);
		prev_vfreq = cvd_timing_info->u16_VFreq ;
	}

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Get_FB_Status(LX_AFE_SCART_MODE_T *mode)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	if ( _gAFE_Enable_SCART_RGB_WA == 1) 
	{
		ret = KADP_AFE_Get_ACE_FB_Status(mode);
		return ret;
	}

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	if(mode == NULL)
		AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : Null Pointer in parameters\n", __F__ );

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_GET_FB_STATUS, mode);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}
int KADP_AFE_Get_Scart_AR(LX_AFE_SCART_ID_T Scart_Id_select, LX_AFE_SCART_AR_T *Scart_AR)
{
	LX_AFE_SCART_AR_INFO_T Scart_AR_info;
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	if(Scart_AR == NULL)
		AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : Null Pointer in parameters\n", __F__ );

	Scart_AR_info.Scart_Id = Scart_Id_select;
	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_GET_SCART_AR, &Scart_AR_info);

	*Scart_AR = Scart_AR_info.Scart_AR;
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}


int KADP_AFE_TTX_SET_Video_Standard(LX_AFE_VIDEO_SYSTEM_T cvd_standard)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_TTX_VIDEO_STANDARD, cvd_standard);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}
int KADP_AFE_Set_ADC_Gain(UINT16 adc_red_value, UINT16 adc_green_value, UINT16 adc_blue_value)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	LX_AFE_ADC_GAIN_VALUE_T	ADC_Gain_Data;

	ADC_Gain_Data.R_Gain_Value = adc_red_value;
	ADC_Gain_Data.G_Gain_Value = adc_green_value;
	ADC_Gain_Data.B_Gain_Value = adc_blue_value;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_SET_GAIN, &ADC_Gain_Data);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Set_ADC_Offset(UINT16 adc_red_value, UINT16 adc_green_value, UINT16 adc_blue_value)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	LX_AFE_ADC_OFFSET_VALUE_T ADC_Offset_Data;
	ADC_Offset_Data.R_Offset_Value = adc_red_value;
	ADC_Offset_Data.G_Offset_Value = adc_green_value;
	ADC_Offset_Data.B_Offset_Value = adc_blue_value;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_SET_OFFSET, &ADC_Offset_Data);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Get_ADC_Gain(LX_AFE_ADC_GAIN_VALUE_T *gain_param)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_ADC_GET_GAIN, gain_param) ;

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Get_ADC_Offset(LX_AFE_ADC_OFFSET_VALUE_T *offset_param)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_ADC_GET_OFFSET, offset_param);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Get_Timing_Info(LX_AFE_ADC_TIMING_INFO_T *pAdc_timing_info)
{
	UINT8 default_table_index;
	LX_AFE_ADC_TIMING_INFO_T Driver_adc_timing_info = {0,};
	int ret = RET_ERROR;
//	static UINT8	adc_sync_hys = 0;
	static LX_AFE_ADC_TIMING_INFO_T Prev_Adc_timing_info = {0,};

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	if(pAdc_timing_info == NULL)
		return RET_ERROR;

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_ADC_GET_TIMING_INFO, &Driver_adc_timing_info);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

	default_table_index = Driver_adc_timing_info.u8_Table_Idx;

	memcpy(pAdc_timing_info , &Driver_adc_timing_info , sizeof(LX_AFE_ADC_TIMING_INFO_T));

	/*
	pAdc_timing_info->adc_type = Driver_adc_timing_info.adc_type;
	pAdc_timing_info->selmux = Driver_adc_timing_info.selmux;

	pAdc_timing_info->Unstable = Driver_adc_timing_info.Unstable;
	pAdc_timing_info->llpll_status = Driver_adc_timing_info.llpll_status;

	pAdc_timing_info->u16_Cur_HFreq = Driver_adc_timing_info.u16_Cur_HFreq;
	pAdc_timing_info->u16_Cur_VFreq = Driver_adc_timing_info.u16_Cur_VFreq;
	pAdc_timing_info->u16_Cur_VTotal = Driver_adc_timing_info.u16_Cur_VTotal;

	pAdc_timing_info->u16_Prev_HFreq = Driver_adc_timing_info.u16_Prev_HFreq;
	pAdc_timing_info->u16_Prev_VFreq = Driver_adc_timing_info.u16_Prev_VFreq;
	pAdc_timing_info->u16_Prev_VTotal = Driver_adc_timing_info.u16_Prev_VTotal;

	pAdc_timing_info->u8_Table_Idx = default_table_index;

	pAdc_timing_info->vs_width = Driver_adc_timing_info.vs_width;
	pAdc_timing_info->hs_width = Driver_adc_timing_info.hs_width;
	pAdc_timing_info->Sync_Exist = Driver_adc_timing_info.Sync_Exist;
	pAdc_timing_info->comp_sync_level = Driver_adc_timing_info.comp_sync_level;
	pAdc_timing_info->comp_green_level = Driver_adc_timing_info.comp_green_level;

	pAdc_timing_info->llpll_phase_shift = Driver_adc_timing_info.llpll_phase_shift;
	pAdc_timing_info->g_gain_value = Driver_adc_timing_info.g_gain_value;
	pAdc_timing_info->sync_level = Driver_adc_timing_info.sync_level;
	*/
/*
	if(Driver_adc_timing_info.Sync_Exist == 0)		// No Sync
	{
		adc_sync_hys = 0;
		pAdc_timing_info->Sync_Exist = 0;
	}
	else if(adc_sync_hys > 3)
		pAdc_timing_info->Sync_Exist = 1;
	else
	{
		pAdc_timing_info->Sync_Exist = 0;
		adc_sync_hys++;
	}
	*/

	// Use Caculated value for H/V Frequency.
	//if ((Driver_adc_timing_info.Sync_Exist == 0) || (Driver_adc_timing_info.Unstable)/* ||(Driver_adc_timing_info.u16_VTotal < 250) */) // No Signal???
	if ((pAdc_timing_info->Sync_Exist == 0) || (Driver_adc_timing_info.Unstable)/* ||(Driver_adc_timing_info.u16_VTotal < 250) */) // No Signal???
	{
		pAdc_timing_info->u16_HFreq = 0;
		pAdc_timing_info->u16_VFreq = 0;

		pAdc_timing_info->u16_HTotal = 0;
		pAdc_timing_info->u16_VTotal = 0;

		pAdc_timing_info->u16_HStart = 0;
		pAdc_timing_info->u16_VStart = 0;

		pAdc_timing_info->u16_HActive = 0;
		pAdc_timing_info->u16_VActive = 0;

		pAdc_timing_info->u8_ScanType = 0x0;

		pAdc_timing_info->u16_Phase = 0;

		goto func_exit;
	}
	else
	{
		pAdc_timing_info->u16_HFreq = Driver_adc_timing_info.u16_HFreq;
		pAdc_timing_info->u16_VFreq = Driver_adc_timing_info.u16_VFreq;
	}

	if(default_table_index == 0xff) // No matching Table
	{
		pAdc_timing_info->u16_HTotal = 0;
		pAdc_timing_info->u16_VTotal = 0;

		pAdc_timing_info->u16_HStart = 0;
		pAdc_timing_info->u16_VStart = 0;

		pAdc_timing_info->u16_HActive = 0xffe;
		pAdc_timing_info->u16_VActive = 0xffe;

		pAdc_timing_info->u8_ScanType = 0x0;

		pAdc_timing_info->u16_Phase = 0;

		goto func_exit;
	}

	//pAdc_timing_info->u16_HFreq = LX_Default_ModeTable[default_table_index].u16_HFreq;
	//pAdc_timing_info->u16_VFreq = LX_Default_ModeTable[default_table_index].u16_VFreq;
	pAdc_timing_info->u16_HTotal = (pLX_Default_ModeTable+default_table_index)->u16_HTotal;
	// Use Table Value for VTotal
	pAdc_timing_info->u16_VTotal = (pLX_Default_ModeTable+default_table_index)->u16_VTotal;

	//pAdc_timing_info->u16_HStart = 0;
	//pAdc_timing_info->u16_VStart = 0;
	pAdc_timing_info->u16_HStart = (pLX_Default_ModeTable+default_table_index)->u16_HStart;

	pAdc_timing_info->u16_HActive = LX_Default_Resolution[(pLX_Default_ModeTable+default_table_index)->u8_Res_Idx].u16_Width;
	pAdc_timing_info->u16_VActive = LX_Default_Resolution[(pLX_Default_ModeTable+default_table_index)->u8_Res_Idx].u16_Height;

	// for Component 480i (MSPG timing 209 and 227 have different vstart values)
	if(default_table_index == 59)
	{
		if ( abs(Driver_adc_timing_info.vs_width - VS_WIDTH_MSPG_209) < abs(Driver_adc_timing_info.vs_width - VS_WIDTH_MSPG_227))
			pAdc_timing_info->u16_VStart = (pLX_Default_ModeTable+default_table_index)->u16_VStart;		// for MSPG 209 timing
		else
		{
			pAdc_timing_info->u16_VStart = (pLX_Default_ModeTable+default_table_index)->u16_VStart - 5;		// for MSPG 227 timing
			/* 483i display issue */
//			pAdc_timing_info->u16_VActive += 3;			// add for two 480i format with other vPorch value(window size issue) 20111125
		}
	}
	else
		pAdc_timing_info->u16_VStart = (pLX_Default_ModeTable+default_table_index)->u16_VStart;

	// Use Table value for scan_type(Interlace/Progressive) determination.
	//pAdc_timing_info->u8_ScanType = Driver_adc_timing_info.u8_ScanType;
	if ((pLX_Default_ModeTable+default_table_index)->u8_Status_Flag & AFE_PCMODE_FLAG_INTERLACE)
	   pAdc_timing_info->u8_ScanType = 0x0;
	else
	   pAdc_timing_info->u8_ScanType = 0x1;

	pAdc_timing_info->u16_Phase = (pLX_Default_ModeTable+default_table_index)->u16_ADC_Phase;


func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

//	if( memcmp(&Prev_Adc_timing_info, pAdc_timing_info, sizeof(LX_AFE_ADC_TIMING_INFO_T)) )
	if( ( ret == RET_OK ) && ( ( Prev_Adc_timing_info.Unstable != pAdc_timing_info->Unstable ) \
			|| (Prev_Adc_timing_info.llpll_status != pAdc_timing_info->llpll_status ) \
			|| (Prev_Adc_timing_info.u16_HTotal != pAdc_timing_info->u16_HTotal ) \
			|| (Prev_Adc_timing_info.u16_VTotal != pAdc_timing_info->u16_VTotal ) \
			|| (Prev_Adc_timing_info.u16_HStart != pAdc_timing_info->u16_HStart ) \
			|| (Prev_Adc_timing_info.u16_VStart != pAdc_timing_info->u16_VStart ) \
			|| (Prev_Adc_timing_info.u16_HActive != pAdc_timing_info->u16_HActive ) \
			|| (Prev_Adc_timing_info.u16_VActive != pAdc_timing_info->u16_VActive ) \
			|| (Prev_Adc_timing_info.Sync_Exist != pAdc_timing_info->Sync_Exist ) ) )
	{
		AFE_PRINT("!!!!!!!! ADC Timing Changed !!!!!!!! \n");
		AFE_PRINT("! index [%d]=>[%d] !!!\n", Prev_Adc_timing_info.u8_Table_Idx, pAdc_timing_info->u8_Table_Idx);
		AFE_PRINT("! adc_type [%d]=>[%d] !!!\n", Prev_Adc_timing_info.adc_type, pAdc_timing_info->adc_type);
		AFE_PRINT("! selmux [%d]=>[%d] !!!\n", Prev_Adc_timing_info.selmux, pAdc_timing_info->selmux);
		AFE_PRINT("! Unstable [%d]=>[%d] !!!\n", Prev_Adc_timing_info.Unstable, pAdc_timing_info->Unstable);
		AFE_PRINT("! llpll_status [%d]=>[%d] !!!\n", Prev_Adc_timing_info.llpll_status, pAdc_timing_info->llpll_status);
		AFE_PRINT("! u16_HTotal [%d]=>[%d] !!!\n", Prev_Adc_timing_info.u16_HTotal, pAdc_timing_info->u16_HTotal);
		AFE_PRINT("! u16_VTotal [%d]=>[%d] !!!\n", Prev_Adc_timing_info.u16_VTotal, pAdc_timing_info->u16_VTotal);
		AFE_PRINT("! u16_HStart [%d]=>[%d] !!!\n", Prev_Adc_timing_info.u16_HStart, pAdc_timing_info->u16_HStart);
		AFE_PRINT("! u16_VStart [%d]=>[%d] !!!\n", Prev_Adc_timing_info.u16_VStart, pAdc_timing_info->u16_VStart);
		AFE_PRINT("! u16_HActive [%d]=>[%d] !!!\n", Prev_Adc_timing_info.u16_HActive, pAdc_timing_info->u16_HActive);
		AFE_PRINT("! u16_VActive [%d]=>[%d] !!!\n", Prev_Adc_timing_info.u16_VActive, pAdc_timing_info->u16_VActive);
		AFE_PRINT("! u8_ScanType [%d]=>[%d] !!!\n", Prev_Adc_timing_info.u8_ScanType, pAdc_timing_info->u8_ScanType);
		AFE_PRINT("! u16_Phase [%d]=>[%d] !!!\n", Prev_Adc_timing_info.u16_Phase, pAdc_timing_info->u16_Phase);
		AFE_PRINT("! vs_width [%d]=>[%d] !!!\n", Prev_Adc_timing_info.vs_width, pAdc_timing_info->vs_width);
		AFE_PRINT("! hs_width [%d]=>[%d] !!!\n", Prev_Adc_timing_info.hs_width, pAdc_timing_info->hs_width);
		AFE_PRINT("! Sync_Exist [%d]=>[%d] !!!\n", Prev_Adc_timing_info.Sync_Exist, pAdc_timing_info->Sync_Exist);

		memcpy(&Prev_Adc_timing_info , pAdc_timing_info , sizeof(LX_AFE_ADC_TIMING_INFO_T));
	}

	return ret;
}

int KADP_AFE_Get_PC_Mode_Info(LX_AFE_PCMODE_INFO_T *pPCmode_info)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

 	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_GET_PCMODE_INFO, pPCmode_info);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}
int KADP_AFE_PC_Adjust(LX_AFE_ADJ_PCMODE_T PC_adjust_mode, SINT16 PC_adjust_value)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	LX_AFE_ADJ_PCMODE_INFO_T PC_adjust_mode_info;
	PC_adjust_mode_info.mode = PC_adjust_mode;
	PC_adjust_mode_info.value = PC_adjust_value;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_PC_ADJUST, &PC_adjust_mode_info);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}
int KADP_AFE_PC_Auto_Adjust(LX_AFE_ADJ_PC_T *pPC_adjust)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_PC_AUTO_ADJUST, pPC_adjust);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}
int KADP_AFE_Get_Default_PC_Mode(UINT8 index, KADP_AFE_PCMODE_INFO_T *pPC_Mode_Info)
{
	if(pPC_Mode_Info == NULL)
		return RET_ERROR;

	if (index == 0xff)
	{
		pPC_Mode_Info->u16_HStart=0;
		pPC_Mode_Info->u16_VStart=0;
		pPC_Mode_Info->u16_Width=0;
		pPC_Mode_Info->u16_Height=0;
		pPC_Mode_Info->u16_HTotal=0;
	}
	else
	{
		pPC_Mode_Info->u16_HStart = (pLX_Default_ModeTable+index)->u16_HStart;
		pPC_Mode_Info->u16_VStart = (pLX_Default_ModeTable+index)->u16_VStart;
		pPC_Mode_Info->u16_Width = LX_Default_Resolution[(pLX_Default_ModeTable+index)->u8_Res_Idx].u16_Width;
		pPC_Mode_Info->u16_Height = LX_Default_Resolution[(pLX_Default_ModeTable+index)->u8_Res_Idx].u16_Height;
		pPC_Mode_Info->u16_HTotal = (pLX_Default_ModeTable+index)->u16_HTotal;
	}
	return 0;
}
int KADP_AFE_Set_Scart_Overlay(BOOLEAN bOnOff)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_SCART_OVERLAY, &bOnOff);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Calibration(BOOLEAN bInternal, BOOLEAN bRGB, UINT16 TargetForRGain, UINT16 TargetForGGain, UINT16 TargetForBGain, BOOLEAN bEnableCompParams)
{
	int ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);
	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	LX_AFE_ADC_CALIB_INFO_T	Calib_Param;
	Calib_Param.bInternal = bInternal;
	Calib_Param.bRGB = bRGB;
	Calib_Param.TargetForRGain = TargetForRGain;
	Calib_Param.TargetForGGain = TargetForGGain;
	Calib_Param.TargetForBGain = TargetForBGain;
	Calib_Param.bEnableCompParams = bEnableCompParams;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_AUTO_CALIBRATION, &Calib_Param);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

	if (Calib_Param.bResult == FALSE)
		ret = RET_ERROR;
	else
		ret = RET_OK;

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Set_Default_ModeTable(void)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	LX_AFE_SET_PCMODE_TABLE_T	Default_Table_info;
	Default_Table_info.pPCMode_Table = (UINT32 *)pLX_Default_ModeTable;
	Default_Table_info.Table_Size = sizeof(M17Ax_Default_ModeTable);
	Default_Table_info.Table_Count = LX_DEFAULT_MODE_TABLE_COUNT;
	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_PCMODE_TABLE, &Default_Table_info);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Set_User_PCMode_Resolution(LX_AFE_RESOLUTION_TYPE_T user_resolution)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_USER_PCMODE_RESOLUTION, user_resolution);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Set_Scart_Mode(BOOLEAN scart_enable)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_SET_SCART_MODE, scart_enable);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Select_Vport_Output(LX_AFE_VPORT_OUT_SRC_T out_src, LX_AFE_VPORT_OUT_CH_T out_ch)
{
	LX_AFE_VPORT_OUT_INFO_T vport_out;
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	vport_out.sel_out_src = out_src;
	vport_out.sel_out_ch = out_ch;
	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_VPORT_OUTPUT, &vport_out);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Vport_Reg_Read(UINT32 addr, UINT32 *data)
{
	LX_AFE_REG_RW_T	reg_addr_data_t;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	if(data == NULL)
		return RET_ERROR;

	reg_addr_data_t.addr = addr;
	reg_addr_data_t.data = 0;

	if(ioctl(g_afe_ctx.dev_fd, AFE_IOR_VPORT_REG_READ, &reg_addr_data_t) < 0)
		return -1;

	*data = reg_addr_data_t.data;
	return 0;
}

int KADP_AFE_Vport_Reg_Write(UINT32 addr, UINT32 data)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	LX_AFE_REG_RW_T	reg_addr_data_t;

	reg_addr_data_t.addr = addr;
	reg_addr_data_t.data = data;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_VPORT_REG_WRITE, &reg_addr_data_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Stop_Timer(LX_AFE_CVD_SELECT_T select_main_sub)
{
	LX_AFE_CVD_TIMER_T cvd_timer_t;
	int ret = RET_ERROR;

	AFE_PRINT("[%s] entered \n", __func__);
	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();


	cvd_timer_t.cvd_main_sub = select_main_sub;
	cvd_timer_t.timeout = 0;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_STOP_TIMER, &cvd_timer_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Start_Timer(LX_AFE_CVD_SELECT_T select_main_sub, UINT32 timeout)
{
	LX_AFE_CVD_TIMER_T cvd_timer_t;
	int ret = RET_ERROR;

	AFE_PRINT("[%s] entered \n", __func__);
	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	cvd_timer_t.cvd_main_sub = select_main_sub;
	cvd_timer_t.timeout = timeout;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_START_TIMER, &cvd_timer_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Program_Color_System(LX_AFE_CVD_SELECT_T select_main_sub, UINT8 color_system)
{
	LX_AFE_CVD_PROGRAM_COLOR_SYSTEM_T program_color_system_t;
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	program_color_system_t.cvd_main_sub = select_main_sub;
	program_color_system_t.color_system = color_system;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_PROGRAM_COLOR_SYSTEM, &program_color_system_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_SET_LLPLL(UINT32 index)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_SET_LLPLL, index);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Get_Sync_Status(BOOLEAN *sync_status)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_ADC_SYNC_EXIST, sync_status);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Test(LX_AFE_ADC_TEST_PARAM_T *pADC_Test_Param_t)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_TEST, pADC_Test_Param_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Test(LX_AFE_CVD_TEST_PARAM_T *pCVD_Test_Param_t)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_TEST, pCVD_Test_Param_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Reset_Digital(void)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_RESET_DIGITAL);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Reset_Digital_24MHZ(void)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_RESET_DIGITAL_24MHZ);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Reset_LLPLL(void)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_RESET_LLPLL);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_SET_SW_3DCOMB_CONTROL(UINT8 enable, UINT32 hf_thr, UINT32 lf_thr, UINT32 diff_thr, UINT32 region_30_mode)
{
	LX_AFE_CVD_SW_3DCOMB_CONTROL_T	control_value_t;
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	control_value_t.Enable = enable;
	control_value_t.Hf_thr = hf_thr;
	control_value_t.Lf_thr = lf_thr;
	control_value_t.Diff_thr = diff_thr;
	control_value_t.Region_30_mode = region_30_mode;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_SET_SW_3DCOMB_CONTROL, &control_value_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_POWER_CONTROL(UINT32 enable)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_POWER_CONTROL, &enable);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_POWER_CONTROL(UINT32 enable)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_POWER_CONTROL, &enable);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Read_Pixel_Value(UINT32 x_pos, UINT32 y_pos, UINT32 x_size, UINT32 y_size, UINT32 *sum_r_value, UINT32 *sum_g_value, UINT32 *sum_b_value)
{
	LX_AFE_ADC_PIXEL_VALUE_T	adc_pixel_value_t;
	UINT32	number_of_pixel;
	int ret = RET_ERROR;

	if ( (sum_r_value == NULL) || (sum_g_value == NULL) || (sum_b_value == NULL))
		return RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

//	adc_pixel_value_t.X_Pos	=	(h_active / 2 ) - x_size;
//	adc_pixel_value_t.Y_Pos	=	(v_active / 2 ) - y_size;
	adc_pixel_value_t.X_Pos	=	x_pos;
	adc_pixel_value_t.Y_Pos	=	y_pos;

	AFE_PRINT("X pos [%d], Y pos [%d]\n", adc_pixel_value_t.X_Pos, adc_pixel_value_t.Y_Pos);

	adc_pixel_value_t.X_Size	=	x_size;
	adc_pixel_value_t.Y_Size	=	y_size;

	number_of_pixel = x_size * y_size;

	AFE_CHECK_CODE( ( number_of_pixel==0 ), goto func_exit, "[afe] %s : parameter wrong\n", __F__ );

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_ADC_READ_PIXEL_VALUE, &adc_pixel_value_t);

	*sum_r_value	= adc_pixel_value_t.Sum_R_Value / number_of_pixel;
	*sum_g_value	= adc_pixel_value_t.Sum_G_Value / number_of_pixel;
	*sum_b_value	= adc_pixel_value_t.Sum_B_Value / number_of_pixel;

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Enable_Periodic_Signal_Info_Read(UINT32 enable)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	if ((enable == 1) && (_gAFE_ADC_CALIB_MODE == TRUE)) {
		AFE_PRINT("In ADC Calibration, do not enable ADC Thread\n");
		ret = RET_OK;
	}
	else
		ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_ENABLE_PERIODIC_SIGNAL_INFO_READ, &enable);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Execute_Format_Detection(void)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_EXECUTE_FORMAT_DETECTION);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Enable_Component_Auto_Phase(UINT32 enable)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_ENABLE_COMPONENT_AUTO_PHASE, &enable);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Enable_Detection_Interrupt(UINT32 enable)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_ENABLE_DETECTION_INTERRUPT, &enable);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Get_States_Detail(LX_AFE_CVD_STATES_DETAIL_T *pcvd_states_detail_t)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_CVD_GET_STATES_DETAIL, pcvd_states_detail_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_SET_PQ_VALUE(LX_AFE_CVD_PQ_MODE_T	cvd_pq_mode)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_3DCOMB_VALUE, &cvd_pq_mode);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Bypass_Control(LX_AFE_CVD_BYPASS_SOURCE_T buf_out_1_sel,  LX_AFE_CVD_BYPASS_SOURCE_T buf_out_2_sel,  LX_AFE_CVD_BYPASS_CVBS_SOURCE_T	cvbs_source_sel)
{
	int ret = RET_ERROR;
	LX_AFE_CVD_BYPASS_CONTROL_T	cvd_bypass_control_t;

	AFE_NOTI("[%s] entered buf1[%d], buf2[%d], source[%d]\n", __func__, buf_out_1_sel, buf_out_2_sel, cvbs_source_sel);
	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	cvd_bypass_control_t.buf_out_1_sel = buf_out_1_sel;
	cvd_bypass_control_t.buf_out_2_sel = buf_out_2_sel;
	cvd_bypass_control_t.cvbs_source_sel = cvbs_source_sel;

	memcpy(&_g_cvd_bypass_control_t, &cvd_bypass_control_t, sizeof(LX_AFE_CVD_BYPASS_CONTROL_T) );

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_BYPASS_CONTROL, &cvd_bypass_control_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_VDAC_Power_Control(BOOLEAN bOnOff)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_VDAC_POWER_CONTROL, &bOnOff);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_VDAC_Mute_Control(int Enable)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_VDAC_MUTE_CONTROL, &Enable);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s [%d] : error in ioctl call\n", __F__ ,ret);

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_LVDS_Src_Control(LX_AFE_LVDS_SELECT_T lvds_sel, LX_AFE_LVDS_PDB_T lvds_power, LX_AFE_LVDS_DATA_TYPE_T lvds_type, LX_AFE_LVDS_SOURCE_T lvds_source, LX_AFE_LVDS_MODE_T lvds_mode)
{
	int ret = RET_ERROR;
	LX_AFE_LVDS_SRC_CONTROL_T	LVDS_Control_t;

	AFE_PRINT("[%s] entered lvds_sel[%d], lvds_power[%d], lvds_type[%d], lvds_power[%d], lvds_mode[%d]\n",\
	__func__, lvds_sel, lvds_power, lvds_type, lvds_source, lvds_mode);
	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	LVDS_Control_t.lvds_sel = lvds_sel;
	LVDS_Control_t.lvds_power = lvds_power;
	LVDS_Control_t.lvds_type = lvds_type;
	LVDS_Control_t.lvds_source = lvds_source;
	LVDS_Control_t.lvds_mode = lvds_mode;

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_LVDS_SRC_CONTROL, &LVDS_Control_t);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_SET_SOG_Slicer_Level(UINT32 sog_lvl)
{
	int ret = RET_ERROR;

	AFE_PRINT("[%s] entered sog_lvl[%d] \n", __func__, sog_lvl);
	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_COMP_SYNC_LEVEL, &sog_lvl);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_SET_Hstate_Max(UINT32 hstate_max_value)
{
	int ret = RET_ERROR;
	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_SET_HSTATE_MAX, &hstate_max_value);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

/**
 * @brief Read kernel memory configuration of CVD driver(Test only)
 *
 * @param mem_base_comb_buffer
 * @param mem_size_comb_buffer
 * @param mem_base_cvd_reg
 * @param mem_size_cvd_reg
 *
 * @return 
 */
int KADP_AFE_CVD_Get_Mem_Cfg(UINT32* mem_base_comb_buffer, UINT32* mem_size_comb_buffer, UINT32* mem_base_cvd_reg, UINT32* mem_size_cvd_reg)
{
	int ret = RET_ERROR;
	LX_AFE_CVD_MEM_CFG_T	cvd_mem_cfg_t;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	if ( (mem_base_comb_buffer == NULL) || (mem_size_comb_buffer == NULL) || (mem_base_cvd_reg == NULL) \
		|| (mem_size_cvd_reg == NULL) )
		return RET_ERROR;

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_GET_MEM_CFG, &cvd_mem_cfg_t);

	*mem_base_comb_buffer = cvd_mem_cfg_t.mem_base_comb_buffer;
	*mem_size_comb_buffer = cvd_mem_cfg_t.mem_size_comb_buffer;

	*mem_base_cvd_reg = cvd_mem_cfg_t.mem_base_cvd_reg;
	*mem_size_cvd_reg = cvd_mem_cfg_t.mem_size_cvd_reg;

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

/**
* @brief Read A-Die Register
*
* @param addr
* @param data
*
* @return 
*/
int KADP_AFE_Read_ACE_Reg(UINT32 addr, UINT32 *data)
{
	int ret = RET_OK;
	LX_AFE_REG_RW_T read_param;

	if(data == NULL)
		return RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	read_param.addr = addr;

	AFE_PRINT("[kadp_afe]Reg read addr 0x%x. \n", read_param.addr);

	if ( read_param.addr > 0xfffffffe ) // ÀÓÀÇ °ª --> ¼öÁ¤¿ä
		AFE_PRINT("[kadp_afe]Register address is out of range.\n");

	/* ioctl function call */
	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_READ_ACE_REG, &read_param);

	if ( ret < 0 )
	{
		*data = 0xFFFFFFFF;
		AFE_PRINT("[kadp_afe] %s : error in ioctl call.\n", __F__ );
		goto func_exit;
	}

	*data = read_param.data;

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

/**
* @brief Write A-Die Register Value
*
* @param addr
* @param data
*
* @return 
*/
int KADP_AFE_Write_ACE_Reg(UINT32 addr, UINT32 data)
{
	int ret = RET_OK;
	LX_AFE_REG_RW_T write_param;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	if ( addr > 0xfffffffe ) // ÀÓÀÇ °ª --> ¼öÁ¤¿ä
		AFE_PRINT("[kadp_afe]Register address is out of range.\n");

	write_param.addr = addr;
	write_param.data = data;

	AFE_PRINT("[kadp_afe]Reg write addr 0x%x. \n", write_param.addr);
	AFE_PRINT("[kadp_afe]Reg write data %d. \n", write_param.data);

	/* ioctl function call */
	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_WRITE_ACE_REG, &write_param);
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[AFE] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

/**
 * @brief Read CVD Crunky Detection Status
 *
 * @param ck_vbi_detected
 * @param number_of_cs
 *
 * @return 
 */
int KADP_AFE_CVD_Get_Crunky_Status(UINT32 *ck_vbi_detected, UINT32 *number_of_cs)
{
	int ret = RET_OK;

	LX_AFE_CVD_CK_T ck_detection_t;

	if ( (ck_vbi_detected == NULL) || (number_of_cs == NULL) )
		return RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	/* ioctl function call */
	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_GET_CK_DETCTION, &ck_detection_t);
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[AFE] %s : error in ioctl call\n", __F__ );

	*ck_vbi_detected = ck_detection_t.ck_vbi_detected;
	*number_of_cs 	 = ck_detection_t.ck_colorstrip_detected;

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Set_ADC_Calibration_Mode(BOOLEAN mode)
{
	int ret = RET_OK;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	_gAFE_ADC_CALIB_MODE = mode;

	AFE_PRINT("[%s] entered mode[%d] \n", __func__, mode);

	if(mode == TRUE)
		KADP_AFE_ADC_Enable_Periodic_Signal_Info_Read(0);
	else
		KADP_AFE_ADC_Enable_Periodic_Signal_Info_Read(1);

	return ret;
}

/**
 * Control Kdrv Debug Print of AFE
 *
 * @param 	printType			[in]  kdrvier afe debug type
 * @param 	printColor		[in]  kdriver afe debug color
 * @return 	if succeeded - OK, else - RET_ERROR.
 * @see	 .
 */
 int KADP_AFE_DebugPrintCtrl(unsigned int printType, unsigned int printColor, unsigned int printEnable)
{
	int ret = RET_ERROR;

	LX_AFE_DEBUG_CTRL_T debugCtrl;

	debugCtrl.printType  = printType;
	debugCtrl.printColor = printColor;
	debugCtrl.printEnable = printEnable;

	AFE_LOCK();
	AFE_TRACE_BEGIN();

	AFE_PRINT( "[AFE] %s is started.\n", __FUNCTION__ );

	/* ioctl function call */
	ret = ioctl ( g_afe_ctx.dev_fd, AFE_IOW_DEBUG_PRINT_CTRL, &debugCtrl );
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[adec] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}


/**
 * @brief Read Component Pseudo Pulse State.
 *
 * @param ck_vbi_detected
 * @param number_of_cs
 *
 * @return 
 */
int KADP_AFE_ADC_Get_Comp_PSP_Status(UINT32 *pPSP_Detected, UINT32 *pVline_Normal, UINT32 *pVline_Measured, UINT32 *pValid_Signal )
{
	int ret = RET_OK;

	LX_AFE_ADC_COMP_PSEUDO_PULSE_T comp_psp_t;

	if ( (pPSP_Detected == NULL) || (pVline_Normal == NULL) || (pVline_Measured == NULL) \
		|| (pValid_Signal == NULL) )
		return RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	/* ioctl function call */
	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_DETECT_COMPONENT_PSEUDO_PULSE, &comp_psp_t);
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[AFE] %s : error in ioctl call\n", __F__ );

	*pPSP_Detected = comp_psp_t.psp_detected;
	*pVline_Normal = comp_psp_t.vline_normal;
	*pVline_Measured = comp_psp_t.vline_measured;
	*pValid_Signal = comp_psp_t.valid_signal;

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}


/**
 * @brief Set Current Black Level to CVD Kernel Driver 
 *
 * @param black_level (0:low, 1:high, 2:auto)
 *
 * @return 
 */
int KADP_AFE_CVD_Set_BlackLevel(UINT32 black_level)
{
	int ret = RET_OK;

	LX_AFE_CVD_BLACK_LEVEL_T cvd_black_level_t;

	AFE_PRINT("[%s] entered black_level[%d] \n", __func__, black_level);

	if(black_level == 0)
		cvd_black_level_t = LX_AFE_CVD_BLACK_LEVEL_LOW;
	else if(black_level == 1)
		cvd_black_level_t = LX_AFE_CVD_BLACK_LEVEL_HIGH;
	else if(black_level == 2)
		cvd_black_level_t = LX_AFE_CVD_BLACK_LEVEL_AUTO;
	else
	{
		ret = RET_INVALID_PARAMS;
		return ret;
	}

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	/* ioctl function call */
	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_CVD_BLACK_LEVEL, &cvd_black_level_t);
	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[AFE] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Get_ACE_FB_Status(LX_AFE_SCART_MODE_T *mode)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_GET_ACE_FB_STATUS, mode);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_Set_Scart_RGB_operation_mode(LX_AFE_SCART_BYPASS_MODE_T scart_rgb_mode)
{
	int ret = RET_ERROR;

	if(scart_rgb_mode == LX_SCART_RGB_MODE_NORMAL)
		_gAFE_Enable_SCART_RGB_WA = 0;
	else if(scart_rgb_mode == LX_SCART_RGB_MODE_BYPASS)
		_gAFE_Enable_SCART_RGB_WA = 1;
	else
		_gAFE_Enable_SCART_RGB_WA = 0;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();


	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_SCART_BYPASS_MODE, &scart_rgb_mode);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );


func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_ATV_Channel_Change(BOOLEAN bChannel_Change)
{
	int ret = RET_ERROR;

	AFE_PRINT("[%s] entered bChannel_Change[%d] \n", __func__, bChannel_Change);

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();


	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_CVD_CHANNEL_CHANGE, &bChannel_Change);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );


func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_CVD_Get_Noise_Status(UINT32 *pCVD_Noise_Status)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();


	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_CVD_GET_NOISE_STATUS, pCVD_Noise_Status);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );


func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Get_Comp_CP_Data(LX_AFE_ADC_COMP_VBI_CP_T *stComp_VBI_CP_Data)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_GET_COMPONENT_VBI_CP, stComp_VBI_CP_Data);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

int KADP_AFE_ADC_Flush_Comp_CP_Data(void)
{
	int ret = RET_ERROR;

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IO_FLUSH_COMPONENT_VBI_CP);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

/**
* @brief Read Stored ADC Calibration Gain DATA in OTP Register
*
* @param index(0:Component, 1:SCART RGB)
* @param gain_param
*
* @return 
*/
int KADP_AFE_Get_OTP_ADC_Gain(int index, LX_AFE_ADC_GAIN_VALUE_T *gain_param)
{
	int ret = RET_ERROR;
	UINT8 OTP_Data_Array[ADC_OTP_REG_SIZE] = {0,};

	if ( (lx_chip_rev() >= LX_CHIP_REV(E60, A0)) && (index > 0) )
		AFE_ERROR("RGB not supported !!!\n");
	else 
		ret = KADP_SE_GetADC(index, OTP_Data_Array);

	gain_param->R_Gain_Value = (UINT16)OTP_Data_Array[0] << 8 | OTP_Data_Array[1] ;
	gain_param->G_Gain_Value = (UINT16)OTP_Data_Array[2] << 8 | OTP_Data_Array[3] ;
	gain_param->B_Gain_Value = (UINT16)OTP_Data_Array[4] << 8 | OTP_Data_Array[5] ;

	return ret;
}

/**
* @brief Read Stored ADC Calibration Offset DATA in OTP Register
*
* @param index(0:Component, 1:SCART RGB)
* @param gain_param
*
* @return 
*/
int KADP_AFE_Get_OTP_ADC_Offset(int index, LX_AFE_ADC_OFFSET_VALUE_T *offset_param)
{
	int ret = RET_ERROR;
	UINT8 OTP_Data_Array[ADC_OTP_REG_SIZE] = {0,};

	if ( (lx_chip_rev() >= LX_CHIP_REV(E60, A0)) && (index > 0) )
		AFE_ERROR("RGB not supported !!!\n");
	else 
		ret = KADP_SE_GetADC(index, OTP_Data_Array);

	offset_param->R_Offset_Value = (UINT16)OTP_Data_Array[6] << 8 | OTP_Data_Array[7] ;
	offset_param->G_Offset_Value = (UINT16)OTP_Data_Array[8] << 8 | OTP_Data_Array[9] ;
	offset_param->B_Offset_Value = (UINT16)OTP_Data_Array[10] << 8 | OTP_Data_Array[11] ;

	return ret;
}

/**
* @brief Write ADC Calibration Gain/Offset DATA into OTP Register
*
* @param index(0:Component, 1:SCART RGB)
* @param write_enable(0:just test not real writing, 1:REAL write to OTP Register)
* @param gain_param
* @param offset_param
*
* @return 
*/
int KADP_AFE_Set_OTP_ADC_GainOffset(int index, int write_enable, LX_AFE_ADC_GAIN_VALUE_T *gain_param, LX_AFE_ADC_OFFSET_VALUE_T *offset_param)
{
	int ret = RET_ERROR;
	int loop;
	UINT8 OTP_Data_Array[ADC_OTP_REG_SIZE];
	UINT8 OTP_Data_Array_Check[ADC_OTP_REG_SIZE];
	UINT8 All_Zero_Array[ADC_OTP_REG_SIZE] = {0,};

	if ( (lx_chip_rev() >= LX_CHIP_REV(E60, A0)) && (index > 0) )
	{
		AFE_ERROR("RGB not supported !!!\n");
		return ret;
	}
	ret = KADP_SE_GetADC(index, OTP_Data_Array);

	if(write_enable && memcmp(OTP_Data_Array, All_Zero_Array ,ADC_OTP_REG_SIZE))
	{
		AFE_WARN(" OTP ADC index[%d] is NOT empty \n", index);	
		return RET_ERROR;
	}

	OTP_Data_Array[0] = (gain_param->R_Gain_Value >> 8) & 0xff;
	OTP_Data_Array[1] = (gain_param->R_Gain_Value ) & 0xff;
	OTP_Data_Array[2] = (gain_param->G_Gain_Value >> 8) & 0xff;
	OTP_Data_Array[3] = (gain_param->G_Gain_Value ) & 0xff;
	OTP_Data_Array[4] = (gain_param->B_Gain_Value >> 8) & 0xff;
	OTP_Data_Array[5] = (gain_param->B_Gain_Value ) & 0xff;

	OTP_Data_Array[6] = (offset_param->R_Offset_Value >> 8) & 0xff;
	OTP_Data_Array[7] = (offset_param->R_Offset_Value ) & 0xff;
	OTP_Data_Array[8] = (offset_param->G_Offset_Value >> 8) & 0xff;
	OTP_Data_Array[9] = (offset_param->G_Offset_Value ) & 0xff;
	OTP_Data_Array[10] = (offset_param->B_Offset_Value >> 8) & 0xff;
	OTP_Data_Array[11] = (offset_param->B_Offset_Value ) & 0xff;

	AFE_NOTI("OTP_Data to write in index[%d]\n", index);
	for(loop=0;loop<ADC_OTP_REG_SIZE;loop++)
	{
		AFE_NOTI("DATA to Write [%02d]=[0x%02x]\n", loop, OTP_Data_Array[loop]);
	}

	/* ############################################# */
	/* Following Code REALLY Write ADC OPT Registers */
	/* ############################################# */
	if(write_enable)
		ret |= KADP_SE_SetADC(index, OTP_Data_Array);
	else
		AFE_NOTI("Not Writting to OTP\n");


	ret |= KADP_SE_GetADC(index, OTP_Data_Array_Check);

	if(memcmp(OTP_Data_Array, OTP_Data_Array_Check, ADC_OTP_REG_SIZE))
	{
		AFE_ERROR(" Written OTP ADC Data index[%d] is WRONG \n", index);	
		for(loop=0;loop<ADC_OTP_REG_SIZE;loop++)
		{
			AFE_PRINT("DATA written[%02d]=[0x%02x]\n", loop, OTP_Data_Array_Check[loop]);
		}
		return RET_ERROR;
	}

	return ret;
}

/**
* @brief Set Internal/External Demod Mode to CVD
*
* @param internal_mode
*
* @return 
*/
int KADP_AFE_Set_Internal_Demod_Mode(BOOLEAN internal_mode)
{
	int ret = RET_ERROR;

	AFE_NOTI("[%s] entered internal_mode[%d] \n", __func__, internal_mode);

	AFE_CHECK_CODE( g_afe_ctx.dev_init_cvd == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
	AFE_LOCK();
	AFE_TRACE_BEGIN();

	ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_SET_INTERNAL_DEMOD_MODE, &internal_mode);

	AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

func_exit:
	AFE_TRACE_END();
	AFE_UNLOCK();

	return ret;
}

/**
 * @brief Reset ADC Gain/Offset value in ADC register
 *
 * @return 
 */
int KADP_AFE_Reset_ADC_GainOffset(void)
{
	int ret = RET_ERROR;

	AFE_NOTI("[%s] entered \n", __func__);

	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );
    ret = KADP_AFE_Set_ADC_Gain(0x1000, 0x1000, 0x1000);
    ret |= KADP_AFE_Set_ADC_Offset(0x800, 0x800, 0x800);

	return ret;
}

void KADP_AFE_DBG_Initialize(void)
{
	g_afe_logm_fd = KADP_LOGM_ObjRegister(KADP_AFE);
	if(g_afe_logm_fd < 0)
	{
		printf("[%s:%u] fail register logm\n", __F__, __L__);
		return;
	}

	KADP_LOGM_BitMaskEnable(g_afe_logm_fd, LX_LOGM_LEVEL_NOTI);
	KADP_LOGM_BitMaskEnable(g_afe_logm_fd, LX_LOGM_LEVEL_ERROR);
	KADP_LOGM_BitMaskEnable(g_afe_logm_fd, LX_LOGM_LEVEL_WARNING);

//	KADP_AFE_ADC_Fake_Init();
//	KADP_AFE_CVD_Fake_Init();
}

void KADP_AFE_DBG_Shutdown(void)
{
	int ret;

	ret = KADP_LOGM_ObjDeregister(KADP_AFE);
	if(ret < 0)
	{
		printf("[%s:%u] fail deregister logm\n", __F__, __L__);
		return;
	}
	g_afe_logm_fd = -1; 
}

int KADP_AFE_Init_OTP_Thread(void)
{
	int ret = 0;
	int thread_create_result;

	if (lx_chip_rev() >= LX_CHIP_REV(O22, A0))
		return ret;

	if(RET_ERROR == KADP_AFE_IsOpen())
		KADP_AFE_Open();

	if(!gIsADCOTPThreadAlive) {
		thread_create_result = 0;

		do{
			if(pthread_attr_init(&_gADCOTPThreadAttr) != 0){
				break;
			}

			if(pthread_attr_setdetachstate(&_gADCOTPThreadAttr, PTHREAD_CREATE_DETACHED) != 0){
				if(pthread_attr_destroy(&_gADCOTPThreadAttr) != 0){
					printf("Warning! Thread Attribute release failed!\n");
				}

				break;
			}

			if(pthread_create(&_gADCOTPThread, &_gADCOTPThreadAttr, (void*)&KADP_AFE_OTP_Thread, (void *) NULL) != 0){
				break;	
			}

			if(pthread_attr_destroy(&_gADCOTPThreadAttr) != 0){
				printf("Warning! Thread Attribute release failed!\n");
			}

			thread_create_result = 1;
		} while(0);

		if(thread_create_result >0)
		{
			printf("_gADCOTPThread pthread_create Success!\n");
		}
		else
		{
			printf("ERROR!!! _gADCOTPThread pthread_create Fail!\n");
			ret = -1;
		}
	}
	else {
		printf("Skip ADC OTP Thread Creation because it is already alive!\n");
	}
	return ret;
}

static void KADP_AFE_OTP_Thread(void)
{
	struct pollfd Events;
	int ret = RET_ERROR;

	printf("\n_______ ADC OTP Thread ______\n");

	while(1)
	{
		memset(&Events, 0, sizeof(Events));
		Events.fd = g_afe_ctx.dev_fd;
		Events.events = POLLIN;
		Events.revents = 0;

		ret = poll(&Events, 1, -1);

		AFE_PRINT("AFE POLL ret [%d]\n", ret);

		if (ret < 0)
		{
			printf("\n POLL ERROR [%d]\n", ret);
			exit(EXIT_FAILURE);
		}
		else if (ret == 0)
		{
			printf("\n POLL ret [%d]\n", ret);
		}
		else 
		{
			LX_AFE_EVENT_T event_type;
			LX_AFE_ADC_GAIN_VALUE_T	adc_gain_comp = {0,}, adc_gain_rgb = {0,};
			LX_AFE_ADC_OFFSET_VALUE_T adc_offset_comp = {0,}, adc_offset_rgb = {0,};
			LX_AFE_ADC_OTP_CAL_VALUE_T adc_otp_gain_offset = {0,};
			// event from kernel
			// at first, find which event occured

		//	AFE_CHECK_CODE( g_afe_ctx.dev_init_adc == 0, return RET_ERROR, "[afe] %s :ignore before initialization\n", __F__ );

			AFE_LOCK();
			AFE_TRACE_BEGIN();

			ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_GET_EVENT, &event_type) ;

			AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

			AFE_PRINT("event type [%d]\n", event_type);

			if(event_type == LX_AFE_EVENT_OTP_CAL_READ_FROM_SE)
			{
				ret |= KADP_AFE_Get_OTP_ADC_Gain(0, &adc_gain_comp);
				if (lx_chip_rev() < LX_CHIP_REV(E60, A0))
					ret |= KADP_AFE_Get_OTP_ADC_Gain(1, &adc_gain_rgb);
				ret |= KADP_AFE_Get_OTP_ADC_Offset(0, &adc_offset_comp);
				if (lx_chip_rev() < LX_CHIP_REV(E60, A0))
					ret |= KADP_AFE_Get_OTP_ADC_Offset(1, &adc_offset_rgb);

				AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error OTP ADC get \n", __F__ );

				adc_otp_gain_offset.R_Gain_Comp = adc_gain_comp.R_Gain_Value;
				adc_otp_gain_offset.G_Gain_Comp = adc_gain_comp.G_Gain_Value;
				adc_otp_gain_offset.B_Gain_Comp = adc_gain_comp.B_Gain_Value;
				adc_otp_gain_offset.R_Offset_Comp = adc_offset_comp.R_Offset_Value;
				adc_otp_gain_offset.G_Offset_Comp = adc_offset_comp.G_Offset_Value;
				adc_otp_gain_offset.B_Offset_Comp = adc_offset_comp.B_Offset_Value;
				if (lx_chip_rev() < LX_CHIP_REV(E60, A0))
				{
					adc_otp_gain_offset.R_Gain_RGB = adc_gain_rgb.R_Gain_Value;
					adc_otp_gain_offset.G_Gain_RGB = adc_gain_rgb.G_Gain_Value;
					adc_otp_gain_offset.B_Gain_RGB = adc_gain_rgb.B_Gain_Value;
					adc_otp_gain_offset.R_Offset_RGB = adc_offset_rgb.R_Offset_Value;
					adc_otp_gain_offset.G_Offset_RGB = adc_offset_rgb.G_Offset_Value;
					adc_otp_gain_offset.B_Offset_RGB = adc_offset_rgb.B_Offset_Value;
				}

				ret = ioctl(g_afe_ctx.dev_fd, AFE_IOW_ADC_SET_OTP_DATA, &adc_otp_gain_offset) ;

				AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );
			}
			else if( (event_type == LX_AFE_EVENT_OTP_CAL_TEST) || (event_type == LX_AFE_EVENT_OTP_CAL_WRITE_TO_SE) )
			{
				int write_enable;

				ret = ioctl(g_afe_ctx.dev_fd, AFE_IOR_ADC_GET_OTP_DATA, &adc_otp_gain_offset) ;

				AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );

				adc_gain_comp.R_Gain_Value = adc_otp_gain_offset.R_Gain_Comp;
				adc_gain_comp.G_Gain_Value = adc_otp_gain_offset.G_Gain_Comp;
				adc_gain_comp.B_Gain_Value = adc_otp_gain_offset.B_Gain_Comp;
				adc_offset_comp.R_Offset_Value = adc_otp_gain_offset.R_Offset_Comp;
				adc_offset_comp.G_Offset_Value = adc_otp_gain_offset.G_Offset_Comp;
				adc_offset_comp.B_Offset_Value = adc_otp_gain_offset.B_Offset_Comp;
				if (lx_chip_rev() < LX_CHIP_REV(E60, A0))
				{
					adc_gain_rgb.R_Gain_Value = adc_otp_gain_offset.R_Gain_RGB;
					adc_gain_rgb.G_Gain_Value = adc_otp_gain_offset.G_Gain_RGB;
					adc_gain_rgb.B_Gain_Value = adc_otp_gain_offset.B_Gain_RGB;
					adc_offset_rgb.R_Offset_Value = adc_otp_gain_offset.R_Offset_RGB;
					adc_offset_rgb.G_Offset_Value = adc_otp_gain_offset.G_Offset_RGB;
					adc_offset_rgb.B_Offset_Value = adc_otp_gain_offset.B_Offset_RGB;
				}

				if(event_type == LX_AFE_EVENT_OTP_CAL_TEST)
					write_enable = 0;
				else
					write_enable = 1;

				ret = KADP_AFE_Set_OTP_ADC_GainOffset(0, write_enable, &adc_gain_comp, &adc_offset_comp);

				if (lx_chip_rev() < LX_CHIP_REV(E60, A0))
					ret |= KADP_AFE_Set_OTP_ADC_GainOffset(1, write_enable, &adc_gain_rgb, &adc_offset_rgb);

				AFE_CHECK_CODE( ret != RET_OK, goto func_exit, "[afe] %s : error in ioctl call\n", __F__ );
			}
func_exit:
			AFE_TRACE_END();
			AFE_UNLOCK();

			AFE_PRINT("\n AFE Event Type [%d]\n", event_type);
		}
		usleep(10*1000);
	}
	return ;
}
