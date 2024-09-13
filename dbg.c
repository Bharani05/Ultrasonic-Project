#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include "de_drv.h"
#include "de_def.h"
#include "de_proc_cmn.h"
#include "proc_util.h"
#include "debug_util.h"
#include <linux/string.h>
#include "de_io.h"
#include "resource_mgr.h"
#include "debug_util.h"
#include "os_util.h"
#include <linux/kallsyms.h>
#include <linux/vmalloc.h>
#include <linux/seq_file.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include "linux/v4l2-ext/v4l2-ext-renderer.h"

#include "v4l2_dolby.h"
#include "v4l2_vsc_event.h"

/*----------------------------------------------------------------------------------------
	Constant Definitions
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
	Macro Definitions
----------------------------------------------------------------------------------------*/
#define DOLBY_MODULE_NAME "dolby"
/*----------------------------------------------------------------------------------------
	Type Definitions
----------------------------------------------------------------------------------------*/
enum {
	PROC_ID_AUTHOR	= 0,
	PROC_ID_COMMAND,
	PROC_ID_DBG_FUNC,
	PROC_ID_LOGM,
	PROC_ID_ARGS,
	PROC_ID_HELP,
	PROC_ID_DOLBY_DEBUG,
	PROC_ID_MAX,
};
enum
{
	PROC_ID_WIN_STATUS,
	PROC_ID_WIN_MAX,
};

/*----------------------------------------------------------------------------------------
	External Function Prototype Declarations
----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
	External Variables
----------------------------------------------------------------------------------------*/
extern struct dolby_priv *gp_prvData;
extern char _g_strMainPQPath[];
extern char _g_strBestPQPath[];
extern enum v4l2_ext_dolby_config_type g_eConfigType;

/*----------------------------------------------------------------------------------------
	global Variables
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
	Static Function Prototypes Declarations
----------------------------------------------------------------------------------------*/
static void __V4l2_DOLBY_PROC_Help(void);
static int _v4l2_dolby_debug_test(DOLBY_DBG_DATA_T *sDbgInfo);
static int _v4l2_dolby_debug_event(void *pData);
/*----------------------------------------------------------------------------------------
	Static Variables
----------------------------------------------------------------------------------------*/

static struct proc_dir_entry* _g_dolby_proc_dir;

static OS_PROC_DESC_TABLE_T	_g_dolby_device_proc_table[] =
{
	{ "author",				PROC_ID_AUTHOR,			OS_PROC_FLAG_READ },
	{ "help",				PROC_ID_HELP, 			OS_PROC_FLAG_READ },
	{ "command",			PROC_ID_COMMAND , 		OS_PROC_FLAG_WRITE },
	{ "dolby_debug",		PROC_ID_DOLBY_DEBUG,	OS_PROC_FLAG_WRITE },

	{ NULL, 				PROC_ID_MAX		, 0 }
};

//static void _DE_WIN_PrintStatus	(struct seq_file *m, void* data);

/*========================================================================================
	Implementation Group
========================================================================================*/

/*
 * read_proc implementation of de device
 *
*/
static int	_DOLBY_ReadProcFunction(	UINT32 procId, char* buffer )
{
	int		ret = RET_OK;
	//int listsize,i;
	DOLBY_DBG_DATA_T sDbgInfo;
	/* TODO: add your proc_write implementation */
	//listsize = sizeof(gDE_FuncList)/sizeof(gDE_FuncList[0]);
	switch( procId )
	{
		case PROC_ID_AUTHOR:
		{
			ret = snprintf( buffer, 48, "%s\n", "Manjunath C Goudar (manjunath.goudar@lge.com)" );
		}
		break;
		case PROC_ID_HELP:
		{
			__V4l2_DOLBY_PROC_Help();
		}break;
		
		default:
		{
			ret = snprintf( buffer, 48, "%s(%d)\n", "unimplemented read proc", procId );
		}
	}

	(void ) sDbgInfo;
	return ret;
}

/*
 * write_proc implementation of de device
 *
*/
static int _DOLBY_WriteProcFunction( UINT32 procId, char* command )
{
	/* TODO: add your proc_write implementation */
	int ret;
	DOLBY_DBG_DATA_T sDbgInfo = {0,};
	switch( procId )
	{
		case PROC_ID_DOLBY_DEBUG:
		{
			int i_winid;
			int funcnum = 0;
			int profileenable;
			int nMode;
		      int npqmode;
			ret = sscanf( command, " %d %d %d %d %d %s", &funcnum, &i_winid,&profileenable ,&nMode,&npqmode,&sDbgInfo.ucconfigpath[0]);
			sDbgInfo.ucwinId = (UINT8)i_winid;
			sDbgInfo.funcnum = (UINT8)funcnum;
			sDbgInfo.ucDbgValue = (UINT8)profileenable;
			sDbgInfo.ucMode = (UINT8)nMode;
			sDbgInfo.nMDDumpEnd = nMode;
			sDbgInfo.ucDolbyEnable = (UINT8)profileenable;
			sDbgInfo.ucEnbestpq = (UINT8)npqmode;
			if(RET_OK != _v4l2_dolby_debug_test(&sDbgInfo))
			{
				DE_NOTI("DOLBY DEBUG INVOKED FAILED\n");
			}
			
		}break;
		case PROC_ID_COMMAND:
		{
			DE_NOTI("command string : %s\n", command );
		}break;
		default:
		{
			/* do nothing */
		}
		break;
	}

	return strlen(command);
}


/**
 * initialize proc utility for de device
 *
 * @see DOLBY_REGISTER
*/

void	DOLBY_PROC_Init (void)
{
	_g_dolby_proc_dir = OS_PROC_CreateEntryEx ( DOLBY_MODULE_NAME, _g_dolby_device_proc_table,_DOLBY_ReadProcFunction,_DOLBY_WriteProcFunction );
	DE_NOTI("INFO[%s]: DOLBY PROC Init DOne.\n",__func__);
	return;
}

/**
 * cleanup proc utility for de device
 *
 * @see DE_Cleanup
*/
void	DOLBY_PROC_Cleanup (void)
{
	OS_PROC_RemoveEntry( "dolby" );
}

static int _v4l2_dolby_debug_test(DOLBY_DBG_DATA_T *sDbgInfo)
{
	DOLBY_DBG_DATA_T s_DbgInfo = {0,};
	int nRetVal;
	memcpy((void *)&s_DbgInfo,(void *)sDbgInfo,sizeof(DOLBY_DBG_DATA_T));
	if(RET_OK != _v4l2_dolby_debug_event((void *)&s_DbgInfo)) {
		DE_ERROR("Debug EVENT FAILED\n");
		nRetVal =  RET_ERROR;
	}
	else
		nRetVal = RET_OK;
	return nRetVal;
}
static int _v4l2_dolby_debug_event(void *pData)
{
	struct v4l2_event dolby_dbg_event;
	DOLBY_DBG_DATA_T *s_DbgInfo = (DOLBY_DBG_DATA_T *)pData;
	int nRetVal = RET_OK;
	LX_DOLBY_IOCTL_IPC_FEATURE_T param;

	dolby_dbg_event.type = V4L2_EVENT_CTRL;
	dolby_dbg_event.id = V4L2_CID_EXT_DOLBY_SUBSCRIBE_DHDR_DEBUG;
	dolby_dbg_event.u.data[0] = (UINT8)s_DbgInfo->ucwinId;
	dolby_dbg_event.u.data[1] = (UINT8)s_DbgInfo->funcnum;

	switch(s_DbgInfo->funcnum){
		case 1:
		{
			dolby_dbg_event.u.data[2] = (UINT8)s_DbgInfo->ucDolbyEnable;
			dolby_dbg_event.u.data[3] = (UINT8)s_DbgInfo->ucMode;
			dolby_dbg_event.u.data[4] = (UINT8)s_DbgInfo->ucEnbestpq;
			//DE_NOTI("Posting DOlby Mode  Enable event %d %d\n",s_DbgInfo->ucDbgValue,dolby_dbg_event.u.data[2]);
				if(dolby_dbg_event.u.data[4] == 1){
			if(strlen(s_DbgInfo->ucconfigpath) > 0) {
				g_eConfigType = V4L2_EXT_DOLBY_CONFIG_BEST;
				DE_NOTI("Setting Best  Dolby congif path to %s of length = %d\n",s_DbgInfo->ucconfigpath,(int)strlen(s_DbgInfo->ucconfigpath));
				memcpy((void *)&_g_strBestPQPath[0],(void *)s_DbgInfo->ucconfigpath,MAX_NAME_LENGTH);
				}
			}
			else{
				if(strlen(s_DbgInfo->ucconfigpath) > 0) {
				g_eConfigType = V4L2_EXT_DOLBY_CONFIG_MAIN;
				DE_NOTI("Setting Main Dolby congif path to %s of length = %d\n",s_DbgInfo->ucconfigpath,(int)strlen(s_DbgInfo->ucconfigpath));
				memcpy((void *)&_g_strMainPQPath[0],(void *)s_DbgInfo->ucconfigpath,MAX_NAME_LENGTH);
				}
			}
			//For RF sending mannualy to IPC
			if(dolby_dbg_event.u.data[3] == 2) {
				vsc_set_hdr_ipc_type(s_DbgInfo->ucwinId,LX_HDR_TYPE_DOLBY_RF,LX_HDR_MODE_FROM_SCL);
			}else if(dolby_dbg_event.u.data[3] == 1 || dolby_dbg_event.u.data[3] == 3) {
				vsc_set_hdr_ipc_type(s_DbgInfo->ucwinId,LX_HDR_TYPE_DOLBY,LX_HDR_MODE_FROM_SCL);
			}
			else if(dolby_dbg_event.u.data[3] == 4){
				vsc_set_hdr_ipc_type(s_DbgInfo->ucwinId,LX_HDR_TYPE_DOLBY_LL,LX_HDR_MODE_FROM_SCL);
			}
			else {
				vsc_set_hdr_ipc_type(s_DbgInfo->ucwinId,LX_HDR_TYPE_OFF,LX_HDR_MODE_FROM_SCL);
			}
		}break;
		case 2:
		case 6:
		{
			dolby_dbg_event.u.data[2] = (UINT8)s_DbgInfo->ucDbgValue;
			//DE_NOTI("Posting 2|6 event %d %d\n",s_DbgInfo->ucDbgValue,dolby_dbg_event.u.data[2]);
		}break;
		case 3:
		case 4:
		case 5:
		case 14:
		case 21:
		case 24:
		case 26:
		case 28:
		case 31: /* TV5 LUT dump */
		case 32: /* TV5 Register dump */
		{
			dolby_dbg_event.u.data[2] = (UINT8)s_DbgInfo->ucDbgValue;
			dolby_dbg_event.u.data[3] = (s_DbgInfo->nMDDumpEnd & 0xFF);
			dolby_dbg_event.u.data[4] = (UINT8)((s_DbgInfo->nMDDumpEnd >> 8) & 0xFF);
			dolby_dbg_event.u.data[5] = (UINT8)((s_DbgInfo->nMDDumpEnd >> 16) & 0xFF);
			dolby_dbg_event.u.data[6] = (UINT8)((s_DbgInfo->nMDDumpEnd >> 24) & 0xFF);
			//DE_NOTI("Posting 3|4|5|14|2124|26|28 event %d s_DbgInfo->nMDDumpEnd = %d\n",s_DbgInfo->ucDbgValue,s_DbgInfo->nMDDumpEnd);
		}break;
		case 9:
		{
			DE_NOTI("Posting dolby debug event 9 [status check]\n");
		}break;
		case 10:
		{
			DE_NOTI("Posting dolby debug event 10 [DisplayTarget Configuration]\n");
		}break;
		case 11:
		{
			DE_NOTI("Posting dolby debug event 11 [Main configuration file status]\n");
		}break;
		case 12:
		{
			DE_NOTI("Posting dolby debug event 12 [Precision Detail Enable/Disable]\n");
			DE_NOTI("Setting PD IPC --> START");
			param.ucWinID = 0;
			param.ucFlag = (UINT8)s_DbgInfo->ucMode;
			param.eDoviFeature = LX_DOVI_FEATURE_PD;
			if(s_DbgInfo->ucDbgValue){
				DE_NOTI("Enabling PD debug with PD value [%d]",param.ucFlag);
				if(DE_IO_SetIPCPDMode((unsigned long)&param,1) != 0)
						DE_ERROR("Setting Precision Detail IPC failed");
				else
					DE_NOTI("[Success]Setting PD IPC --> END");
			}
			else {
				DE_NOTI("Disabling PD debug and resetting PD to off");
				param.ucFlag = (UINT8)0;
				if(DE_IO_SetIPCPDMode((unsigned long)&param,1) != 0)
						DE_ERROR("Setting Precision Detail IPC failed");
				else
					DE_NOTI("[Success]Setting PD IPC --> END");
			}

			dolby_dbg_event.u.data[3] = (UINT8)s_DbgInfo->ucMode;
			dolby_dbg_event.u.data[2] = (UINT8)s_DbgInfo->ucDbgValue;
		}break;
		case 17:
		case 18:
		case 19:
		case 20:
		case 29:
		{
			;//DE_NOTI("POSTING 9|10|11|12|17|18|19|20 option debug EVENT\n");
		}break;
		case 7:
		case 8:
		case 13:
		case 15:
		case 16:
		case 22:
		case 23:
		case 25:
		case 27:
		case 30:
		{
			dolby_dbg_event.u.data[2] = (UINT8)s_DbgInfo->ucDbgValue;
			dolby_dbg_event.u.data[3] = (UINT8)s_DbgInfo->ucMode;
			//DE_NOTI("POSTING 7|13|15|16|22|23|25|27 DEbug event %d\n",*(UINT8 *)pData);
		}break;
		case 40:
		{
			dolby_dbg_event.u.data[2] = (UINT8)s_DbgInfo->ucDbgValue;
			dolby_dbg_event.u.data[3] = (UINT8)(s_DbgInfo->nMDDumpEnd & 0xFF);
            dolby_dbg_event.u.data[4] = (UINT8)((s_DbgInfo->nMDDumpEnd >> 8) & 0xFF);
            dolby_dbg_event.u.data[5] = (UINT8)((s_DbgInfo->nMDDumpEnd >> 16) & 0xFF);
            dolby_dbg_event.u.data[6] = (UINT8)((s_DbgInfo->nMDDumpEnd >> 24) & 0xFF);
			DE_NOTI("DlocalContrast Debug value :  %d\n",s_DbgInfo->nMDDumpEnd);
		}break;
		default:
		{
			nRetVal = RET_ERROR;
			DE_ERROR("Unknown funcnum = %d",s_DbgInfo->funcnum);
		}break;
	}

	if(gp_prvData)
		v4l2_event_queue(gp_prvData->dolby_vdev, &dolby_dbg_event);
	else {
		DE_ERROR("DOlby device is NULL\n");
		nRetVal = RET_ERROR;
	}
	//DE_NOTI("#######EVENTS are successfully posted\n");
	return nRetVal;
}

void __V4l2_DOLBY_PROC_Help(void)
{
	DE_NOTI("------------------------------------------\n");
	DE_NOTI("		 DEBUG FUNCTION MENU OPTION			   \n");
	DE_NOTI("[1] Enable/Disable Dolby\n");
	DE_NOTI("[2] Profiling Support\n");
	DE_NOTI("[3] Enable Composer Metadata Dump\n");
	DE_NOTI("[4] Enable DM Metadata Dump\n");
	DE_NOTI("[5] Enable output LUT Dump\n");
	DE_NOTI("[6] Change PQ Mode\n");
	DE_NOTI("[7] Change PQ Sub Mode\n");
	DE_NOTI("[8] Display Current PQ Config Data\n");
	DE_NOTI("[9] Display Current Dolby Status\n");
	DE_NOTI("[10] Display Current Target Config Params\n");
	DE_NOTI("[11] Display Current PQ Config setup\n");
	DE_NOTI("[12] Display Current Dm3 PQ Config\n");
	DE_NOTI("[13] Enable/Disable Global Dimming\n");
	DE_NOTI("[14] Enable DM and Comp Register Dump\n");
	DE_NOTI("[15] Create Config Binary with current PQ config\n");
	DE_NOTI("[16] Test All 0 Metadata (div0 verify)\n");
	DE_NOTI("[17] Print SW Ver and DM Ver\n");
	DE_NOTI("[18] Get Current GD delay Value\n");
	DE_NOTI("[19] Print GD delay value DB\n");
	DE_NOTI("[20] Set Debug GD Table\n");
	DE_NOTI("[21] Set Bl Scaler Value\n");
//	DE_NOTI("[0x17] Set Debug OLED Boost Table\n");
	DE_NOTI("[22] Enable/Disable Adaptive Boost\n");
	DE_NOTI("[23] Enable Ambient data from File [/var/ambient.cfg]\n");
	DE_NOTI("[24] MAnual Ambient settings\n");
	DE_NOTI("[25] Block Ambient API call\n");
	DE_NOTI("[26] User ambient enable call\n");
	DE_NOTI("[27] KADP_HDR_Debug Print Control\n");
	DE_NOTI("[28] Enable specific prints\n");
	DE_NOTI("[29] Level 11 MD print\n");
	DE_NOTI("[30] Ambient Testing by changing  tfrontlux value from 0 to 65536 in steps of 100\n");
	DE_NOTI("------------------------------------------\n");
	DE_NOTI("DEBUG COMMAND --> echo function_num window_id fuction_options > /proc/lg/dolby/dolby_debug > \n");
	DE_NOTI("=================PROC DOLBY DEBUG COMMAND EXAMPLES=================\n");
	DE_NOTI("echo 1 0 Ena[1]/Dis[0] OTT[1]/RF[2]/HDMI[3]]/LL[4]/NONE[0] (cfgpath)\n");
	DE_NOTI("echo 2 0 Ena[1]/Dis[0]\n");
	DE_NOTI("echo 3 0 Ena[1]/Dis[0] (Num of Comp Frames to dump)\n");
	DE_NOTI("echo 4 0 Ena[1]/Dis[0] (Num of DM Frames to dump)\n");
	DE_NOTI("echo 5 0 Ena[1]/Dis[0] (Num of LUT to dump)\n");
	DE_NOTI("echo 6 0 Vivid[0]/Bright[1]/Dark[2]/Std[3]/Game[4]\n");
	DE_NOTI("echo 7 0 Backlight[0]/Brightness/[1]/Contrast[2] (value)\n");
	DE_NOTI("echo 8 0 Vivid[0]/Bright[1]/Dark[2]/Std[3]/Game[4]\n");
	DE_NOTI("echo 9 0 \n");
	DE_NOTI("echo 10 0 \n");
	DE_NOTI("echo 11 0 \n");
	DE_NOTI("echo 12 0 \n");
	DE_NOTI("echo 13 0 Ena[1]/Dis[0] Vivid[0]/Bright[1]/Dark[2]/Std[3]/Game[4]/ALL[5]\n");
	DE_NOTI("echo 14 0 Ena[1]/Dis[0] (Num of Regs to dump)\n");
	DE_NOTI("echo 15 0\n");
	DE_NOTI("echo 16 0 Ena[1]/Dis[0] Ena[1]/Dis[0]\n");
	DE_NOTI("echo 17 0 \n");
	DE_NOTI("echo 18 0 \n");
	DE_NOTI("echo 19 0 \n");
	DE_NOTI("echo 20 0 \n");
	DE_NOTI("echo 21 0 1 (backlight scalar Value)\n");
	DE_NOTI("echo 22 0 Ena[1]/Dis[0] Vivid[0]/Bright[1]/Dark[2]/Std[3]/Game[4]/ALL[5] \n");
	DE_NOTI("echo 23 0 Ena[1]/Dis[0]\n");
	DE_NOTI("echo 24 0 Ena[1]/Dis[0] (TfronLUtx value)\n");
	DE_NOTI("echo 25 0 Ena[1]/Dis[0]\n");
	DE_NOTI("echo 26 0 Ena[1]/Dis[0] (ambientParam.ambient value)\n");
	DE_NOTI("echo 27 0 (DEBUG OFF[0]/ON[1])(HDR NOTI OFF[0]/ON[1]) (HDR WARN OFF[0]/ON[1])\n");
	DE_NOTI("echo 28 0 1 (Debug Flag Value[9999-Gd enable,8888-only PWM enable,7777- Crop Area,6666-PTS REceive,4444-PTS Error,3333-OLED BOOST ENABLE,2222- Ambient Debug])\n");
	DE_NOTI("echo 29 0 \n");
	DE_NOTI("echo 30 0 \n");
	DE_NOTI("==============DOOLBY DEBUG MENU HELP END==============\n");
}
/** @} */
