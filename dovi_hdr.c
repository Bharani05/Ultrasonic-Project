#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <errno.h>
#include <fcntl.h>


#include <string.h>
#include <signal.h>

#include "dovi_hdr.h"
#include "dovi_dump.h"
#include "dovi_ip_interface.h"
#include "dovi_backend_cp.h"
#include "dovi_defconfig.h"
#include "dtv_config_lib.h"
#include "dtv_config_lib_flt.h"
#include "osa_kadp.h"
#include <sys/types.h>
#include <sys/stat.h>


//#include "Dm3dLut.h"

/*-----------------------------------------------------------------------------
	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Macro Definitions
------------------------------------------------------------------------------*/
#define DCI_4K_WIDTH	4096
#define DCI_4K_HEIGHT	2160

#define THREAD_SHARED	0
#define PROC_SHARED	1
#define MAX_NUM_VIEWINGMODES 10
//#define DBG_TEST_CONFIG
#define DEF_CONFIG_FILE_1_6_2  "/mnt/lg/pqldb/dolby/TEMP.txt" /* mnt/lg/pqldb/dolby/TEMP.txt */

/*Offset to low latency mode in configuration file */
#define LL_CONFIG_OFFSET	10
#define LUT_START_CNT	10
/*-----------------------------------------------------------------------------
	Extern Variables & Function Prototype Declarations
------------------------------------------------------------------------------*/
extern int g_hdr_kap_logm_fd;
extern sDoviDbgInfo gsDolbyDbgContext[];

/*-----------------------------------------------------------------------------
	Local Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Local Type Definitions
------------------------------------------------------------------------------*/
#define AMBIENT_UPD_FRONT     (unsigned)(1<<0)
#define AMBIENT_UPD_REAR      (unsigned)(1<<1)
#define AMBIENT_UPD_WHITEXY   (unsigned)(1<<2)
#define AMBIENT_UPD_MODE      (unsigned)(1<<3)

#define CHECK_PARSE(expr) do { \
            if ((!expr)) { \
            printf("Parsing error at line %d: %s\nSource code line %d\n", line, ptr, __LINE__); \
            return -1; \
            } \
        } while (0)

/*-----------------------------------------------------------------------------
	Global Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Static Variables & Function Prototypes Declarations
------------------------------------------------------------------------------*/
//static char sw_ver[128];
//static char line_buffer[1024];

/*-----------------------------------------------------------------------------
	Global Variables & Function Prototypes Declarations
------------------------------------------------------------------------------*/

sDolbyControlPathContext g_sDolbyWndContext[MAX_WNDS]= {{.bInitialized=false},{.bInitialized=false},{.bInitialized=false},{.bInitialized=false}};

UI_CMD_T g_sUICommands[MAX_WNDS] = {{0,},};
PQ_CONFIG_INFO gsDolbyPQConfig = {NULL,NULL,0};
async_md_context g_sAsyncMDContext[MAX_WNDS]={{.semAsyncMD=NULL,.bInitialized=false},\
	{.semAsyncMD=NULL,.bInitialized=false}};
S_DOLBY_GD_DELAY_LUT_T g_sDolbySTDGddelaydb;
S_DOLBY_GD_DELAY_LUT_T g_sDolbyLLGddelaydb;
oled_boost_params g_oledBoostRefData;

/*-----------------------------------------------------------------------------
	Local Variables & Function Prototypes Declarations
------------------------------------------------------------------------------*/
static bool isHDRModeUpdated(hdr_input_pq_param_t *io_sptrInputParams,unsigned char i_ucWinId);
static int read_ambient_cfg(ambient_cfg_t* ambient_cfg,int fr_num);
int g_fr_num = 0;
extern int g_dolby_dev_fd;
/*-----------------------------------------------------------------------------
	Function Definitions
------------------------------------------------------------------------------*/

int DOVI_Init(UINT32 i_unWinID,dovi_frame_size i_sFrameSize,LX_HDR_MODE_T i_eHDRMode,E_HDR_CNTL_PATH i_eCntlPath)
{
	int retVal = RET_ERROR;
	int unSemInitVal = 1;
	LX_DOLBY_MODE_T eDolbyRunMode = LX_DOLBY_DEFAULT_MODE;
	int savedMask;
	HDR_DEBUG("ENTER %s\n",__func__);

	do
	{
		if(g_sDolbyWndContext[i_unWinID].bInitialized == false){
			memset(&g_sDolbyWndContext[i_unWinID],0,sizeof(sDolbyControlPathContext));

			//Initialize the mutex between backend control thread and main thread
			if(sem_init(&(g_sDolbyWndContext[i_unWinID].pqMutex),THREAD_SHARED,1) == -1){
				HDR_ERROR("Mutex Init Failed :: %s\n",strerror(errno));
				break;
			}
			else {
				HDR_NOTI("NOTI:: PQ Mutex initialized for Win [%d]\n",i_unWinID);
			}

			//this mutex will be used for synchronization in OTT mode also [for O18 case]
			if((lx_chip_rev() >= LX_CHIP_REV(M17,C0)) && (i_eHDRMode == LX_HDR_TYPE_DOLBY)){
				unSemInitVal = 0;
			}

			savedMask = umask(0);
			g_sDolbyWndContext[i_unWinID].mdSyncSem = sem_open( MD_SYNC_SEM, O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO, unSemInitVal);
			umask(savedMask);
			if(SEM_FAILED == g_sDolbyWndContext[i_unWinID].mdSyncSem){
				HDR_ERROR("OTT/LL Mutex Init Failed :: %s\n",strerror(errno));
				g_sDolbyWndContext[i_unWinID].mdSyncSem = NULL;
				sem_destroy(&g_sDolbyWndContext[i_unWinID].pqMutex);
				retVal = RET_ERROR;
				break;
			}
			else {
				HDR_PRINT("NOTI:: OTT/LL Mutex initialized for Win [%d]\n",i_unWinID);
			}

			if(i_eCntlPath == E_HDMI_CNTRL_PATH)
				eDolbyRunMode = LX_DOLBY_HDMI_MODE;
			else if(i_eCntlPath == E_RF_CNTRL_PATH)
				eDolbyRunMode = LX_DOLBY_RF_MODE;
			else if(i_eCntlPath == E_OTT_CNTRL_PATH)
				eDolbyRunMode = LX_DOLBY_OTT_MODE;
			else
				eDolbyRunMode = LX_DOLBY_HDMI_LL_MODE;

			if( DOVI_IP_INF_Init(i_unWinID,eDolbyRunMode) != 0){
				HDR_ERROR("Error:: IP Init Failed !!Function [%s] Line [%d] \n",__func__,__LINE__);
				sem_destroy(&g_sDolbyWndContext[i_unWinID].pqMutex);
				sem_close(g_sDolbyWndContext[i_unWinID].mdSyncSem);
				retVal = RET_ERROR;
				break;
			}

			g_sDolbyWndContext[i_unWinID].ucWinId = i_unWinID;
			g_sDolbyWndContext[i_unWinID].unFrameWidth = i_sFrameSize.width;
			g_sDolbyWndContext[i_unWinID].unFrameHeight= i_sFrameSize.height;

			g_sDolbyWndContext[i_unWinID].bInitialized = true;
			g_sDolbyWndContext[i_unWinID].bRunning = false;

			//Reset UI_CMD structure
			memset(&g_sUICommands[i_unWinID],-1,sizeof(UI_CMD_T));
			g_fr_num = 0;
		}
		else {
			HDR_PRINT("Dolby HDR is already initialized for [%d] Window",i_unWinID);
			retVal = RET_OK;
		}

		retVal = RET_OK;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);

	return retVal;
}


int DOVI_UnInit(UINT32 i_unWinID,LX_HDR_MODE_T i_eHDRMode)
{
	int retVal = RET_ERROR;

	HDR_DEBUG("ENTER %s\n",__func__);

	do
	{
		if(g_sDolbyWndContext[i_unWinID].bInitialized == false){
			HDR_ERROR("Dolby HDR is Not initialized for [%d] Window",i_unWinID);
			retVal = RET_OK;
			break;
		}
		else {
			//Lock the mutex
			HDR_DEBUG("Lock Mutex %s\n",__func__);
			sem_wait(&g_sDolbyWndContext[i_unWinID].pqMutex);

			//Un-Initialize IP
			if( DOVI_IP_INF_UnInit(i_unWinID) != RET_OK){
				HDR_ERROR("Error:: Dolby IP Uninit !! Module [%s], Function [%s] Line [%d] \n",__FILE__,__func__,__LINE__);
			}
			else {
				HDR_DEBUG("DBG:: Dolby driver un-initialization Done !!!\n");
				retVal = RET_OK;
			}

			g_sDolbyWndContext[i_unWinID].bInitialized = false;

			/* First post for any call waiting */
			sem_post(&g_sDolbyWndContext[i_unWinID].pqMutex);
			/* Wait again:: As already uninitialized flag is set so no further wait sud be there*/
			usleep(10000);
			sem_wait(&g_sDolbyWndContext[i_unWinID].pqMutex);
			sem_post(&g_sDolbyWndContext[i_unWinID].pqMutex);
			/* No body sould be waiting now */
			sem_destroy(&g_sDolbyWndContext[i_unWinID].pqMutex);
			HDR_PRINT("Destroying OTT/LL Mutex\n");

			if(sem_close(g_sDolbyWndContext[i_unWinID].mdSyncSem) != 0){
				HDR_ERROR("Error:: Name semaphore close failed %s",strerror(errno));
			}
			else
				g_sDolbyWndContext[i_unWinID].mdSyncSem = NULL;

			memset(&g_sDolbyWndContext[i_unWinID],0,sizeof(sDolbyControlPathContext));
			//Reset UI_CMD structure
			memset(&g_sUICommands[i_unWinID],-1,sizeof(UI_CMD_T));
			if(sem_unlink(MD_SYNC_SEM) != RET_OK) {
				HDR_ERROR("ERROR: Semaphore unlink failed %s\n",strerror(errno));
				retVal = RET_ERROR;
				break;
			}
		}
		g_fr_num = 0;
		retVal = RET_OK;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);
	return retVal;
}


int DOVI_Start(UINT32 i_unWinID,E_HDR_CNTL_PATH i_eCntlPath)
{
	int nRetVal = RET_ERROR;
	struct v4l2_ext_dolby_picture_mode st_PqMode = {1,HDR_MOVIE_BRIGHT};
	HDR_DEBUG("ENTER %s\n",__func__);

	do
	{
		if(g_sDolbyWndContext[i_unWinID].bInitialized == false){
			HDR_ERROR("Dolby HDR is not initialize for [%d] Window",i_unWinID);
			break;
		}

		//Lock the mutex
		HDR_DEBUG("Lock Mutex %s\n",__func__);
		sem_wait(&g_sDolbyWndContext[i_unWinID].pqMutex);

		g_sDolbyWndContext[i_unWinID].eCntlPath = i_eCntlPath;
		g_sDolbyWndContext[i_unWinID].hdrModeUpdateFn = isHDRModeUpdated;
		g_sDolbyWndContext[i_unWinID].bThreadExit = false;

		//Default initialization of PQ submodes
		g_sUICommands[i_unWinID].pqsubmode[HDR_PQ_BACKLIGHT] = 100;
		g_sUICommands[i_unWinID].pqsubmode[HDR_PQ_BRIGHTNESS] = 50;
		g_sUICommands[i_unWinID].pqsubmode[HDR_PQ_CONTRAST] = 50;
		// Initialization of PQ Mode from Dolby driver
		g_sUICommands[i_unWinID].pqmode = HDR_MOVIE_BRIGHT;
		if(DOVI_V4L2_GetPQMode((UINT8)i_unWinID,&st_PqMode) == RET_OK) {
			g_sUICommands[i_unWinID].pqmode = st_PqMode.uPictureMode;
			HDR_PRINT("Updated PQ value from driver is %d %d BRIGHT mode\n",st_PqMode.uPictureMode,g_sUICommands[i_unWinID].pqmode);
		}

		if(gsDolbyDbgContext[i_unWinID].unDebugFlag == IDK_DEBUG_DARK_MODE) {
			g_sUICommands[i_unWinID].pqmode = HDR_MOVIE_DARK;
			HDR_PRINT("thread started with %d DARK mode\n",g_sUICommands[i_unWinID].pqmode);
		}
		if(gsDolbyDbgContext[i_unWinID].unDebugFlag == IDK_DEBUG_VIVID_MODE) {
			g_sUICommands[i_unWinID].pqmode = HDR_MOVIE_VIVID;
			HDR_PRINT("thread started with %d VIVID mode\n",g_sUICommands[i_unWinID].pqmode);
		}

		//Default ambient setting
		memset(&g_sUICommands[i_unWinID].ambientParam,0,sizeof(ambient_cfg_t));


		//Default trigger configuration [first time]
		g_sUICommands[i_unWinID].pqUpdateFlag = (PQ_MODE_CHANGE_FLAG | PQ_UIPARAM_CHANGE_FLAG | PQ_SRCPARAM_CHANGE_FLAG );
		g_sUICommands[i_unWinID].updated = 1;

		g_sDolbyWndContext[i_unWinID].eHDRRunState = E_HDR_RUN_UNDEFINED;
		if(startBackEndControl(&g_sDolbyWndContext[i_unWinID]) != E_BEC_ERR_OK){
			HDR_ERROR("Error:: Back-end control failed for window [%d] \n",i_unWinID);

			sem_post(&g_sDolbyWndContext[i_unWinID].pqMutex);
			HDR_DEBUG("UnLocked PQ Mutex %s\n",__func__);
			break;
		}

		sem_post(&g_sDolbyWndContext[i_unWinID].pqMutex);
		HDR_DEBUG("UnLocked PQ Mutex %s\n",__func__);

		nRetVal = RET_OK;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);

    return nRetVal;
}

int DOVI_Stop(UINT32 i_unWinID)
{
	int nRetVal = RET_ERROR;
	dump_info_t sDoviDumpInfo = {0,};

	HDR_DEBUG("ENTER %s\n",__func__);

	do
	{
		if(g_sDolbyWndContext[i_unWinID].bInitialized == false ){
			HDR_ERROR("ERROR::Dolby HDR is not initialize for [%d] Window",i_unWinID);
			break;
		}

		if(g_sDolbyWndContext[i_unWinID].hdrModeUpdateFn == NULL){
			HDR_ERROR("ERROR::Dolby HDR is not running for [%d] Window",i_unWinID);
			break;
		}

		if(stopBackEndControl(&g_sDolbyWndContext[i_unWinID]) != E_BEC_ERR_OK){
			HDR_ERROR("Error:: Back-end control stop failed for window [%d]\n",i_unWinID);
			break;
		}

		/*Disable dump flags: START */
		gsDolbyDbgContext[i_unWinID].unLUTDumpEnable = 0;
		gsDolbyDbgContext[i_unWinID].unCompMetadataDumpEnable = 0;
		gsDolbyDbgContext[i_unWinID].unDmMetadataDumpEnable = 0;
		gsDolbyDbgContext[i_unWinID].unRegDumpEnable = 0;
		sDoviDumpInfo.ucWinId = i_unWinID;
		dump_dovi_dm_metadata(&sDoviDumpInfo,NULL,NULL,NULL);
		dump_dovi_comp_metadata(&sDoviDumpInfo,NULL,NULL);
		dovi_dump_lut(&sDoviDumpInfo,NULL,NULL);
		dovi_dump_reg(&sDoviDumpInfo,NULL,NULL);

		/*Disable dump flags: END */
		nRetVal = RET_OK;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);
    return nRetVal;
}

int DOVI_SetPQMode(UINT32 i_unWinID,E_HDR_MOVIE_MODES i_eDoviMovieMode)
{
	int nRetVal = RET_ERROR;
	UI_CMD_T *sptrDoviHDRInfo = &g_sUICommands[i_unWinID];

	HDR_DEBUG("ENTER %s\n",__func__);

	do
	{
		if(g_sDolbyWndContext[i_unWinID].bInitialized == false ){
			HDR_ERROR("ERROR::Dolby HDR is not initialize for [%d] Window\n",i_unWinID);
			break;
		}

		if(i_eDoviMovieMode >= HDR_MOVIE_NONE) {
			HDR_ERROR("ERROR::Invalid PQMODE for [%d] Window\n",i_unWinID);
			break;
		}

		//Lock the mutex
		HDR_DEBUG("Lock Mutex %s\n",__func__);
		sem_wait(&g_sDolbyWndContext[i_unWinID].pqMutex);

		if(sptrDoviHDRInfo->pqmode != i_eDoviMovieMode){
			sptrDoviHDRInfo->pqmode = (int)i_eDoviMovieMode;
			sptrDoviHDRInfo->pqUpdateFlag |= PQ_MODE_CHANGE_PENDING;
		//	sptrDoviHDRInfo->updated = 1;
		}else {
			HDR_PRINT("Same PQ Mode as before. Nothing to do\n");
		}

		sem_post(&g_sDolbyWndContext[i_unWinID].pqMutex);
		HDR_DEBUG("UnLocked PQ Mutex %s\n",__func__);
		nRetVal = RET_OK;
	}while(0);

	return nRetVal;
}

int DOVI_SetPQSubMode(UINT32 i_unWinID,E_HDR_PQ_SUB_PARAM i_unPQSubMode,int i_nVal)
{
	int nRetVal = RET_ERROR;

	UI_CMD_T *sptrDoviHDRInfo = &g_sUICommands[i_unWinID];

	HDR_DEBUG("ENTER %s\n",__func__);

	do
	{
		if(g_sDolbyWndContext[i_unWinID].bInitialized == false ){
			HDR_ERROR("ERROR::Dolby HDR is not initialize for [%d] Window",i_unWinID);
			break;
		}

		if(i_unPQSubMode >= HDR_PQ_MAX)
			break;

		HDR_DEBUG("Lock Mutex %s\n",__func__);
		sem_wait(&g_sDolbyWndContext[i_unWinID].pqMutex);

		if(sptrDoviHDRInfo->pqsubmode[i_unPQSubMode] != i_nVal){
			sptrDoviHDRInfo->pqsubmode[i_unPQSubMode] = i_nVal;
			sptrDoviHDRInfo->pqUpdateFlag |= PQ_UIPARAM_CHANGE_FLAG;

			if(sptrDoviHDRInfo->pqUpdateFlag & PQ_MODE_CHANGE_PENDING){
				sptrDoviHDRInfo->pqUpdateFlag |= PQ_MODE_CHANGE_FLAG;
				sptrDoviHDRInfo->pqUpdateFlag &=  ~PQ_MODE_CHANGE_PENDING;
			}
			sptrDoviHDRInfo->updated = 1;
		}else if (sptrDoviHDRInfo->pqUpdateFlag & PQ_MODE_CHANGE_PENDING){
			sptrDoviHDRInfo->pqUpdateFlag |= PQ_MODE_CHANGE_FLAG;
			sptrDoviHDRInfo->pqUpdateFlag |= PQ_UIPARAM_CHANGE_FLAG;
			sptrDoviHDRInfo->pqUpdateFlag &=  ~PQ_MODE_CHANGE_PENDING;
			sptrDoviHDRInfo->updated = 1;
		}
		else{
			HDR_PRINT("Same backlight value and PQ Mode as before. Nothing to do\n");
			nRetVal = RET_OK;

			sem_post(&g_sDolbyWndContext[i_unWinID].pqMutex);
			HDR_DEBUG("UnLocked PQ Mutex %s\n",__func__);
			break;
		}

		/*Note: PQmode change will always be accompanied by PQSubmode change. so trigger lut gen only after both happens.*/
		if(g_sDolbyWndContext[i_unWinID].eCntlPath == E_HDMI_LL_CNTRL_PATH){
			sem_post(g_sDolbyWndContext[i_unWinID].mdSyncSem);
			HDR_PRINT("UnLocked OTT/LL Mutex %s\n",__func__);
		}

		if((lx_chip_rev() >= LX_CHIP_REV(M17,C0)) && (g_sDolbyWndContext[i_unWinID].eCntlPath == E_OTT_CNTRL_PATH)){
			sem_post(g_sDolbyWndContext[i_unWinID].mdSyncSem);
			HDR_PRINT("UnLocked OTT/LL Mutex %s\n",__func__);
		}

		sem_post(&g_sDolbyWndContext[i_unWinID].pqMutex);
		HDR_DEBUG("UnLocked PQ Mutex %s\n",__func__);

		nRetVal = RET_OK;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);
	return nRetVal;
}

#define USE_BINARY_CFG 0
#define CHECK_CONFIG_SAME 0
#define TXT_CFG_MAX_SIZE 30720

/**
 * KADP_HDR_DOVI_SetPQConfig
 *
 * @param i_unWinID[in] VIDEO_WID_T.
 * @param i_unConfigCnt[in] Configuration Files count.
 * @param i_cptrConfigFilePaths[in] Configuration file full path
 * @return  int
 * Note: PQ configurations will be irrespective of window. The configuration file will contain all the configuration
 * modes parameters. To support preloaded configuratoin we make it separate
 */
int DOVI_SetPQConfig(char* i_cptrConfigFilePaths[],UINT32 i_unConfigCnt)
{
	int nRetVal = RET_ERROR;
	//int cnt = 0;
	FILE *fp = NULL;
	size_t frsize = 0;
	int nBinGenRet = 0;
	UINT32 unDefConfigSize = 0;
	int wid;
	/*for txt to bin conversion float cfg also needs to be maintained*/

	HDR_DEBUG("ENTER %s\n",__func__);

#ifdef DBG_TEST_CONFIG
	if(i_cptrConfigFilePaths != NULL){
		strcpy(i_cptrConfigFilePaths[0],DEF_CONFIG_FILE_1_6_2);
	}
#endif
	do
	{
		gsDolbyPQConfig.unConfigCnt = 0;
		if(gsDolbyPQConfig.sptrPQConfigurations != NULL){
			memset(gsDolbyPQConfig.sptrPQConfigurations,0,sizeof(pq_config_t)*2*MAX_NUM_VIEWINGMODES);
		}
		if((i_cptrConfigFilePaths == NULL) || (i_cptrConfigFilePaths[CFG_MAIN] == NULL)||(strcmp(i_cptrConfigFilePaths[CFG_MAIN],"")==0) || ((fp = fopen(i_cptrConfigFilePaths[CFG_MAIN],"rb")) == NULL)){
			unDefConfigSize = 0;
			HDR_ERROR("ERROR::Config file read error. Using Default configuration parameters\n");
			if(gsDolbyPQConfig.sptrPQConfigurations == NULL){
				gsDolbyPQConfig.sptrPQConfigurations = (pq_config_t *)malloc(sizeof(pq_config_t)*2*MAX_NUM_VIEWINGMODES);
				if(gsDolbyPQConfig.sptrPQConfigurations == NULL){
					HDR_ERROR("ERROR:: Malloc Failed Module [%s], Function [%s] \n",__FILE__,__func__);
					break;
				}
			}
			while(gsDolbyPQConfig.unConfigCnt < 2*MAX_NUM_VIEWINGMODES) {
				memcpy((void *)((UINT8 *)gsDolbyPQConfig.sptrPQConfigurations+(gsDolbyPQConfig.unConfigCnt*sizeof(pq_config_t))),(void *)((UINT8 *)gucDefConfig+(gsDolbyPQConfig.unConfigCnt*sizeof(pq_config_t))),sizeof(pq_config_t));
				HDR_DEBUG("Read %d PQ Configurations\n",(gsDolbyPQConfig.unConfigCnt+1));
				unDefConfigSize += sizeof(pq_config_t);
				gsDolbyPQConfig.unConfigCnt++;
				if(unDefConfigSize >= sizeof(gucDefConfig)) {
					HDR_ERROR("Default Configuration structure Setting done!!\n");
					break;
				}
			}
		}
		else{
			if(strlen(i_cptrConfigFilePaths[CFG_MAIN])> MAX_CONFIG_STRING){
				HDR_ERROR("File Path Name Too Long. \n");
				break;
			}

			if(gsDolbyPQConfig.cptrDolbyConfigFile == NULL){
				gsDolbyPQConfig.cptrDolbyConfigFile = (char *)malloc(MAX_CONFIG_STRING);
				if(gsDolbyPQConfig.cptrDolbyConfigFile != NULL){
					strncpy(gsDolbyPQConfig.cptrDolbyConfigFile,i_cptrConfigFilePaths[CFG_MAIN],(size_t)MAX_CONFIG_STRING-1);
				}
				else{
					HDR_ERROR("ERROR:: Malloc Failed Module [%s], Function [%s] \n",__FILE__,__func__);
					break;
				}
			}
			else //This should not call as the configuration file need to set only once
				strncpy(gsDolbyPQConfig.cptrDolbyConfigFile,i_cptrConfigFilePaths[CFG_MAIN],(size_t)MAX_CONFIG_STRING-1);

			if(gsDolbyPQConfig.sptrPQConfigurations == NULL){
				gsDolbyPQConfig.sptrPQConfigurations = (pq_config_t *)malloc(sizeof(pq_config_t)*2*MAX_NUM_VIEWINGMODES);
				if(gsDolbyPQConfig.sptrPQConfigurations == NULL){
					HDR_ERROR("ERROR:: Malloc Failed Module [%s], Function [%s] \n",__FILE__,__func__);
					fclose(fp);
					fp = NULL;
					break;
				}
			}

	#if USE_BINARY_CFG == 0
	#if 0
		/*test open to check file exist*/
			if(fp){
				fclose(fp);
				fp = NULL;
			}
			if((fp = fopen("/var/LG_Y17_C6_65UJ7500.cfg","r")==NULL)){
				HDR_ERROR("test cfg txt file not present in var Using default config\n");
				while(gsDolbyPQConfig.unConfigCnt < MAX_NUM_VIEWINGMODES) {
					memcpy((void *)((UINT8 *)gsDolbyPQConfig.sptrPQConfigurations+(gsDolbyPQConfig.unConfigCnt*sizeof(pq_config_t))),(void *)((UINT8 *)gucDefConfig+(gsDolbyPQConfig.unConfigCnt*sizeof(pq_config_t))),sizeof(pq_config_t));
					HDR_DEBUG("Read %d PQ Configurations\n",(gsDolbyPQConfig.unConfigCnt+1));
					unDefConfigSize += sizeof(pq_config_t);
					gsDolbyPQConfig.unConfigCnt++;
					if(unDefConfigSize >= sizeof(gucDefConfig)) {
						HDR_ERROR("Default Configuration structure Setting done!!\n");
						break;
					}
				}
				nRetVal = RET_OK;
				break;
			}else{
				if(fp)
					fclose(fp);
				fp = NULL;
			}
	#endif
			/*for txt to bin*/
			/*check file size to check if binary file is passed*/
			if(fp){
				fseek(fp,0L,SEEK_END);
				frsize = ftell(fp);
				fseek(fp,0L,SEEK_SET);
				HDR_PRINT("file size %u\n",(UINT32)frsize);
			}
			/*read and parse cfg file to get fix point configuration*/
			if(frsize<TXT_CFG_MAX_SIZE) {/*if file is not bin file*/
				if((i_cptrConfigFilePaths[CFG_USB_BESTPQ] != NULL) && (strlen(i_cptrConfigFilePaths[CFG_USB_BESTPQ]) > 0)){/*support USB BestPQ file*/
					HDR_PRINT("Using %s BESTPQ config file  for dolby bin generation\n",i_cptrConfigFilePaths[CFG_USB_BESTPQ]);
					nBinGenRet = generate_dolbyVision_pqconfig(gsDolbyPQConfig.cptrDolbyConfigFile ,i_cptrConfigFilePaths[CFG_USB_BESTPQ], (pq_config_t *)&gsDolbyPQConfig.sptrPQConfigurations[0]);
					strncpy((void *)gsDolbyPQConfig.cptrDolbyConfigFile,i_cptrConfigFilePaths[CFG_USB_BESTPQ],(size_t)MAX_CONFIG_STRING);
				}else{
					HDR_PRINT("Using %s MAIN config for dolby bin generation\n",gsDolbyPQConfig.cptrDolbyConfigFile);
					nBinGenRet = generate_dolbyVision_pqconfig(gsDolbyPQConfig.cptrDolbyConfigFile ,NULL, (pq_config_t *)&gsDolbyPQConfig.sptrPQConfigurations[0]);
				}
			}
			else {
				nBinGenRet = -1;
			}
			/*Use default if invalid config is passed*/
			if(nBinGenRet == -1){
				unDefConfigSize = 0;
				HDR_ERROR("Error: Config file invalid. Setting default configuration\n");
				while(gsDolbyPQConfig.unConfigCnt < 2*MAX_NUM_VIEWINGMODES) {
					memcpy((void *)((UINT8 *)gsDolbyPQConfig.sptrPQConfigurations+(gsDolbyPQConfig.unConfigCnt*sizeof(pq_config_t))),(void *)((UINT8 *)gucDefConfig+(gsDolbyPQConfig.unConfigCnt*sizeof(pq_config_t))),sizeof(pq_config_t));
					HDR_DEBUG("Read %d PQ Configurations\n",(gsDolbyPQConfig.unConfigCnt+1));
					unDefConfigSize += sizeof(pq_config_t);
					gsDolbyPQConfig.unConfigCnt++;
					if(unDefConfigSize >= sizeof(gucDefConfig)) {
						HDR_ERROR("Default Configuration structure Setting done!!\n");
						break;
					}
				}
			}
			gsDolbyPQConfig.unConfigCnt = 2*MAX_NUM_VIEWINGMODES;
			/* generate binary file to verify the conversion*/
			//nBinGenRet |= write_config_binary(gsDolbyPQConfig.sptrPQConfigurations ,"/tmp/pq_bin_out.bin", MAX_NUM_VIEWINGMODES );
			//HDR_PRINT("bin file /tmp/pq_bin_out.bin is generated\n");
	#else

			while(gsDolbyPQConfig.unConfigCnt < 2*MAX_NUM_VIEWINGMODES) {
				frsize = fread((UINT8 *)gsDolbyPQConfig.sptrPQConfigurations+(gsDolbyPQConfig.unConfigCnt*sizeof(pq_config_t)),sizeof(pq_config_t),1,fp);

				if(frsize < 1) {
					HDR_ERROR("Error: File Read Error!!\n");
					break;
				}
				HDR_DEBUG("Read %d PQ Configurations\n",(gsDolbyPQConfig.unConfigCnt+1));
				gsDolbyPQConfig.unConfigCnt++;
				if (feof(fp))
					break;
			}
	#endif
		}

		nRetVal = RET_OK;
	}while(0);

	if(NULL != fp)
		fclose(fp);
	for(wid = 0;wid< MAX_WNDS;wid++){
		g_sUICommands[wid].pqUpdateFlag |= PQ_CFG_FILE_CHANGED;
		g_sUICommands[wid].updated = 1;
	}

	HDR_DEBUG("EXIT %s\n",__func__);

	return nRetVal;
}


//#define PROFILE_SIMULATE
static bool isHDRModeUpdated(hdr_input_pq_param_t *io_sptrInputParams,unsigned char i_ucWinId)
{
	bool bModeUpdated = false;

	HDR_DEBUG("ENTER %s\n",__func__);

	if((NULL == io_sptrInputParams)||(NULL==io_sptrInputParams->sPQConfigParam) || (NULL == io_sptrInputParams->sptrUIParams) || (NULL == io_sptrInputParams->nptrPQModeFlag) || (NULL == io_sptrInputParams->sptrSrcParam) || (NULL == io_sptrInputParams->sptrAmbientParam)){
		HDR_ERROR("Error [APP]:: NULL Input parameters !! Module [%s], Function [%s] \n",__FILE__,__func__);
		return false;
	}

	if(i_ucWinId >= MAX_WNDS){
		HDR_ERROR("Error [APP]:: Invalid Window ID !! Module [%s], Function [%s] \n",__FILE__,__func__);
		return false;
	}

	HDR_DEBUG("Lock Mutex %s\n",__func__);
	sem_wait(&g_sDolbyWndContext[i_ucWinId].pqMutex);
	/*as per Boyeon request on [9-01-2020] work around for DOlby HDMI_LL issue(QEVENTTWEN-22876) this is Dolby Issue so doing work around for 8k model*/
	if(g_sDolbyWndContext[i_ucWinId].eCntlPath == E_HDMI_LL_CNTRL_PATH)
		io_sptrInputParams->sPQConfigParam->target_display_config.gdConfig.gdEnable = 0;/*setting to gdEnable to 0 to fix  QEVENTTWEN-22876 issue for 8K model */
	if(gsDolbyPQConfig.unConfigCnt > 0){
		// configuration found [configuration file set + parsed and config loaded]
		g_sUICommands[i_ucWinId].pqUpdateFlag |= PQ_CONFIG_FILE_SET_FLAG;
	}

	//Update based on PQ/PQSub mode change
	*(io_sptrInputParams->nptrPQModeFlag) = g_sUICommands[i_ucWinId].pqUpdateFlag;

	if((g_sUICommands[i_ucWinId].updated == 1 && gsDolbyPQConfig.unConfigCnt > 0) || gsDolbyDbgContext[i_ucWinId].unEnAmbFile)
    {
		if((g_sUICommands[i_ucWinId].pqUpdateFlag & (PQ_MODE_CHANGE_FLAG | PQ_CFG_FILE_CHANGED)) ){
			if(gsDolbyPQConfig.sptrPQConfigurations){
				HDR_PRINT("Get Configuration for [%d] PQ Mode\n",g_sUICommands[i_ucWinId].pqmode);
				memcpy(io_sptrInputParams->sPQConfigParam,(UINT8 *)gsDolbyPQConfig.sptrPQConfigurations+(g_sUICommands[i_ucWinId].pqmode*sizeof(pq_config_t)),sizeof(pq_config_t));

#ifdef DUMP_DISPLAY_CONFIG
				displayTargetConfig(io_sptrInputParams->sPQConfigParam);
#endif
				g_oledBoostRefData.abenable = io_sptrInputParams->sPQConfigParam->target_display_config.abConfig.abEnable;
				g_oledBoostRefData.gdenable = io_sptrInputParams->sPQConfigParam->target_display_config.gdConfig.gdEnable;
				g_sUICommands[i_ucWinId].pqUpdateFlag &= ~PQ_MODE_CHANGE_FLAG;
				g_sUICommands[i_ucWinId].pqUpdateFlag &= ~PQ_CFG_FILE_CHANGED;
				g_oledBoostRefData.bPQModeUpdate = true;
			}
			else {
				HDR_ERROR("ERROR:: PQ Config file is not set yet\n");
			}
		}

		if(g_sUICommands[i_ucWinId].pqUpdateFlag & PQ_UIPARAM_CHANGE_FLAG){
			io_sptrInputParams->sptrUIParams->u16BackLightUIVal = g_sUICommands[i_ucWinId].pqsubmode[HDR_PQ_BACKLIGHT];
			io_sptrInputParams->sptrUIParams->u16BrightnessUIVal= g_sUICommands[i_ucWinId].pqsubmode[HDR_PQ_BRIGHTNESS];
			io_sptrInputParams->sptrUIParams->u16ContrastUIVal  = g_sUICommands[i_ucWinId].pqsubmode[HDR_PQ_CONTRAST];
			HDR_PRINT("Backlight [%d]\n",io_sptrInputParams->sptrUIParams->u16BackLightUIVal);
			g_sUICommands[i_ucWinId].pqUpdateFlag &= ~PQ_UIPARAM_CHANGE_FLAG;
		}

		//Update source param also [NTBD]
		if(g_sUICommands[i_ucWinId].pqUpdateFlag & PQ_SRCPARAM_CHANGE_FLAG){
			if(g_sDolbyWndContext[i_ucWinId].eCntlPath != E_HDMI_LL_CNTRL_PATH){
				io_sptrInputParams->sptrSrcParam->src_bit_depth = 10;
				io_sptrInputParams->sptrSrcParam->src_chroma_format = 0; //0:420 and 1:422
				io_sptrInputParams->sptrSrcParam->src_yuv_range = 1; //0:Narrow and 1:Full
				io_sptrInputParams->sptrSrcParam->src_color_format = SIG_COLOR_YUV;
			}
			else {
				//memcpy(io_sptrInputParams->sptrSrcParam,&(g_sUICommands[i_ucWinId].srcParam),sizeof(src_param_t));
				io_sptrInputParams->sptrSrcParam->src_bit_depth = 12;//As per hubert's mail about vsvdb1 supporting only 12 bit
				io_sptrInputParams->sptrSrcParam->src_chroma_format = 1; //0:420 and 1:422 as per sheng's information
				io_sptrInputParams->sptrSrcParam->src_color_format = SIG_COLOR_YUV;
			}

			g_sUICommands[i_ucWinId].pqUpdateFlag &= ~PQ_SRCPARAM_CHANGE_FLAG;
		}
		/*Arjun:use ambient cfg values if file is present - only test purpose. if /var/ambient.txt file not present,  normal sequence will occur*/
		if(gsDolbyDbgContext[i_ucWinId].unEnAmbFile && (read_ambient_cfg(io_sptrInputParams->sptrAmbientParam,g_fr_num) >= 0)){
			HDR_PRINT("FrameNo: %d ambientOn %d tRearLum %d tFrontLux %d tWhiteXY[0] %d tWhiteXY[1] %d\n",g_fr_num,io_sptrInputParams->sptrAmbientParam->ambient,\
				io_sptrInputParams->sptrAmbientParam->tRearLum,io_sptrInputParams->sptrAmbientParam->tFrontLux,\
				io_sptrInputParams->sptrAmbientParam->tWhiteXY[0],io_sptrInputParams->sptrAmbientParam->tWhiteXY[1]);
			*(io_sptrInputParams->nptrPQModeFlag) |= PQ_AMBIENT_CHANGE_FLAG;
			g_fr_num++;
		}else{
			if(g_sUICommands[i_ucWinId].pqUpdateFlag & PQ_AMBIENT_CHANGE_FLAG){
				/* real TV environment only ambient flag and tFrontLux is changing */
				io_sptrInputParams->sptrAmbientParam->nUpdateFlags = AMBIENT_UPD_MODE | AMBIENT_UPD_FRONT | AMBIENT_UPD_REAR | AMBIENT_UPD_WHITEXY;
				io_sptrInputParams->sptrAmbientParam->ambient = g_sUICommands[i_ucWinId].ambientParam.ambient;
				io_sptrInputParams->sptrAmbientParam->tFrontLux = g_sUICommands[i_ucWinId].ambientParam.tFrontLux;
				io_sptrInputParams->sptrAmbientParam->tRearLum = io_sptrInputParams->sPQConfigParam->target_display_config.ambientConfig.tRearLum;
				io_sptrInputParams->sptrAmbientParam->tWhiteXY[0] = io_sptrInputParams->sPQConfigParam->target_display_config.ambientConfig.tWhitexy[0];
				io_sptrInputParams->sptrAmbientParam->tWhiteXY[1] = io_sptrInputParams->sPQConfigParam->target_display_config.ambientConfig.tWhitexy[1];
				g_sUICommands[i_ucWinId].pqUpdateFlag &= ~PQ_AMBIENT_CHANGE_FLAG;
			}
		}

		//Reset the flags
		g_sUICommands[i_ucWinId].updated = 0;
        bModeUpdated = true;
    }

	sem_post(&g_sDolbyWndContext[i_ucWinId].pqMutex);
	HDR_DEBUG("UnLocked Mutex %s\n",__func__);

	HDR_DEBUG("EXIT %s\n",__func__);
	return bModeUpdated;
}

int DOVI_Get_SWVersion(char *pstVersion)
{
	int nRetVal = RET_OK;
	HDR_DEBUG("ENTER %s\n",__func__);


	do{
		if(NULL == pstVersion){
			HDR_ERROR("ERROR: Null Input params\n");
			break;
		}

		snprintf(pstVersion,20,"%02d.%02d",K_DOVI_SW_MAJOR_NUM,K_DOVI_SW_MINOR_NUM);
	}while(0);


	HDR_DEBUG("EXIT %s\n",__func__);
	return nRetVal;
}

int DOVI_Get_DMVersion(char *pstVersion,char *pVerText)
{
	int nRetVal = RET_ERROR;
	HDR_DEBUG("ENTER %s\n",__func__);
	DLB_VERSION_INFO cDmVersion;

	do{
		if(NULL == pstVersion){
			HDR_ERROR("ERROR: Null Input params for version\n");
			break;
		}
		if(NULL == pVerText){
			HDR_ERROR("ERROR: Null Input params for Version Text\n");
			break;
		}

		nRetVal = get_DM_Version(&cDmVersion);
		snprintf(pstVersion,128,"%d.%d.%d.%02d",cDmVersion.vApi, cDmVersion.vFct, cDmVersion.vMtnc, cDmVersion.vBuild);
		if(cDmVersion.text)
		{
		  snprintf(pVerText,128,"-%s", cDmVersion.text);
		}else{
			snprintf(pVerText,128," ");
		}
	}while(0);


	HDR_DEBUG("EXIT %s\n",__func__);
	return nRetVal;

}



void displayTargetConfig(pq_config_t *i_sPQConfigParam)
{
	int i,j;
	HDR_PRINT("gamma 					:: %d\n",i_sPQConfigParam->target_display_config.gamma);
	HDR_PRINT("eotf 					:: %d\n",i_sPQConfigParam->target_display_config.eotf);
	HDR_PRINT("rangeSpec				:: %d\n",i_sPQConfigParam->target_display_config.rangeSpec);
	HDR_PRINT("maxPq 					:: %d\n",i_sPQConfigParam->target_display_config.maxPq);
	HDR_PRINT("minPq 					:: %d\n",i_sPQConfigParam->target_display_config.minPq);
	HDR_PRINT("maxPq_dm3 				:: %d\n",i_sPQConfigParam->target_display_config.maxPq_dm3);
	HDR_PRINT("min_lin					:: %d\n",i_sPQConfigParam->target_display_config.min_lin);
	HDR_PRINT("max_lin					:: %d\n",i_sPQConfigParam->target_display_config.max_lin);
	HDR_PRINT("max_lin_dm3				:: %d\n",i_sPQConfigParam->target_display_config.max_lin_dm3);
	HDR_PRINT("tPrimaries 	\n");
	for(j=0;j<8;j++){
		HDR_PRINT("%d ",i_sPQConfigParam->target_display_config.tPrimaries[j]);
	}
	HDR_PRINT("mSWeight					:: %d\n",i_sPQConfigParam->target_display_config.mSWeight);
	HDR_PRINT("trimSlopeBias 				:: %d\n",i_sPQConfigParam->target_display_config.trimSlopeBias);
	HDR_PRINT("trimOffsetBias 				:: %d\n",i_sPQConfigParam->target_display_config.trimOffsetBias);
	HDR_PRINT("trimPowerBias 				:: %d\n",i_sPQConfigParam->target_display_config.trimPowerBias);
	HDR_PRINT("msWeightBias 				:: %d\n",i_sPQConfigParam->target_display_config.msWeightBias);
	HDR_PRINT("chromaWeightBias 			:: %d\n",i_sPQConfigParam->target_display_config.chromaWeightBias);
	HDR_PRINT("saturationGainBias 			:: %d\n",i_sPQConfigParam->target_display_config.saturationGainBias);
	HDR_PRINT("tuningMode 					:: %d\n",i_sPQConfigParam->target_display_config.tuningMode);
	HDR_PRINT("brightness 					:: %d\n",i_sPQConfigParam->target_display_config.brightness);
	HDR_PRINT("contrast 					:: %d\n",i_sPQConfigParam->target_display_config.contrast);
	HDR_PRINT("dColorShift 				:: %d\n",i_sPQConfigParam->target_display_config.dColorShift);
	HDR_PRINT("dSaturation 				:: %d\n",i_sPQConfigParam->target_display_config.dSaturation);
	HDR_PRINT("dBacklight 					:: %d\n",i_sPQConfigParam->target_display_config.dBacklight);
	HDR_PRINT("dbgExecParamsPrintPeriod	:: %d\n",i_sPQConfigParam->target_display_config.dbgExecParamsPrintPeriod);
	HDR_PRINT("dbgDmMdPrintPeriod 			:: %d\n",i_sPQConfigParam->target_display_config.dbgDmMdPrintPeriod);
	HDR_PRINT("dbgDmCfgPrintPeriod 		:: %d\n",i_sPQConfigParam->target_display_config.dbgDmCfgPrintPeriod);
	HDR_PRINT("gdConfig.gdEnable 			:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdEnable);
	HDR_PRINT("gdConfig.gdWMin 			:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdWMin);
	HDR_PRINT("gdConfig.gdWMax 			:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdWMax);
	HDR_PRINT("gdConfig.gdWMm 				:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdWMm);
	HDR_PRINT("gdConfig.gdWDynRngSqrt 		:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdWDynRngSqrt);
	HDR_PRINT("gdConfig.gdWeightMean 		:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdWeightMean);
	HDR_PRINT("gdConfig.gdWeightStd 		:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdWeightStd);
	HDR_PRINT("gdConfig.gdDelayMilliSec_hdmi:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdDelayMilliSec_hdmi);
	HDR_PRINT("gdConfig.gdRgb2YuvExt 		:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdRgb2YuvExt);
	HDR_PRINT("gdConfig.gdM33Rgb2Yuv 	    ::\n");
	for(i=0;i<3;i++){
		HDR_PRINT("\t\t\t");
		for(j=0;j<3;j++){
			HDR_PRINT("%d ",i_sPQConfigParam->target_display_config.gdConfig.gdM33Rgb2Yuv[i][j]);
		}
		HDR_PRINT("\n");
	}
	HDR_PRINT("gdConfig.gdM33Rgb2YuvScale2P:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdM33Rgb2YuvScale2P);
	HDR_PRINT("gdConfig.gdRgb2YuvOffExt 	:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdRgb2YuvOffExt);
	HDR_PRINT("\t\t\t");
	for(i=0;i<3;i++){
			HDR_PRINT("%d ",i_sPQConfigParam->target_display_config.gdConfig.gdV3Rgb2YuvOff[i]);
	}
	HDR_PRINT("\n");
	HDR_PRINT("gdConfig.gdUpBound 				:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdUpBound);
	HDR_PRINT("gdConfig.gdLowBound 			:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdLowBound);
	HDR_PRINT("gdConfig.lastMaxPq 				:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.lastMaxPq);
	HDR_PRINT("gdConfig.gdWMinPq 				:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdWMinPq);
	HDR_PRINT("gdConfig.gdWMaxPq 				:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdWMaxPq);
	HDR_PRINT("gdConfig.gdWMmPq 				:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdWMmPq);
	HDR_PRINT("gdConfig.gdTriggerPeriod 		:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdTriggerPeriod);
	HDR_PRINT("gdConfig.gdTriggerLinThresh 	:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdTriggerLinThresh);
	HDR_PRINT("gdConfig.gdDelayMilliSec_ott 	:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdDelayMilliSec_ott);
	HDR_PRINT("gdConfig.gdRiseWeight 			:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdRiseWeight);
	HDR_PRINT("gdConfig.gdFallWeight 			:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdFallWeight);
	HDR_PRINT("gdConfig.gdDelayMilliSec_ll 	:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdDelayMilliSec_ll);
	HDR_PRINT("gdConfig.gdContrast 			:: %d\n",i_sPQConfigParam->target_display_config.gdConfig.gdContrast);
	HDR_PRINT("abConfig.abEnable 				:: %d\n",i_sPQConfigParam->target_display_config.abConfig.abEnable);
	HDR_PRINT("abConfig.abHighestTmax 			:: %d\n",i_sPQConfigParam->target_display_config.abConfig.abHighestTmax);
	HDR_PRINT("abConfig.abLowestTmax 			:: %d\n",i_sPQConfigParam->target_display_config.abConfig.abLowestTmax);
	HDR_PRINT("abConfig.abRiseWeight 			:: %d\n",i_sPQConfigParam->target_display_config.abConfig.abRiseWeight);
	HDR_PRINT("abConfig.abFallWeight 			:: %d\n",i_sPQConfigParam->target_display_config.abConfig.abFallWeight);
	HDR_PRINT("abConfig.abDelayMilliSec_hdmi 	:: %d\n",i_sPQConfigParam->target_display_config.abConfig.abDelayMilliSec_hdmi);
	HDR_PRINT("abConfig.abDelayMilliSec_ott 	:: %d\n",i_sPQConfigParam->target_display_config.abConfig.abDelayMilliSec_ott);
	HDR_PRINT("abConfig.abDelayMilliSec_ll 	:: %d\n",i_sPQConfigParam->target_display_config.abConfig.abDelayMilliSec_ll);
	HDR_PRINT("ambientConfig.ambient			:: %d\n",i_sPQConfigParam->target_display_config.ambientConfig.ambient);
	HDR_PRINT("ambientConfig.tFrontLux			:: %d\n",i_sPQConfigParam->target_display_config.ambientConfig.tFrontLux);
	HDR_PRINT("ambientConfig.tFrontLuxScale	:: %d\n",i_sPQConfigParam->target_display_config.ambientConfig.tFrontLuxScale);
	HDR_PRINT("ambientConfig.tRearLum			:: %d\n",i_sPQConfigParam->target_display_config.ambientConfig.tRearLum);
	HDR_PRINT("ambientConfig.tRearLumScale		:: %d\n",i_sPQConfigParam->target_display_config.ambientConfig.tRearLumScale);
	HDR_PRINT("ambientConfig.tWhitexy[0]		:: %d\n",i_sPQConfigParam->target_display_config.ambientConfig.tWhitexy[0]);
	HDR_PRINT("ambientConfig.tWhitexy[1]		:: %d\n",i_sPQConfigParam->target_display_config.ambientConfig.tWhitexy[1]);
	HDR_PRINT("ambientConfig.tSurroundReflection:: %d\n",i_sPQConfigParam->target_display_config.ambientConfig.tSurroundReflection);
	HDR_PRINT("ambientConfig.tScreenReflection	:: %d\n",i_sPQConfigParam->target_display_config.ambientConfig.tScreenReflection);
	HDR_PRINT("\nvsvdb	\n");
	for(j=0;j<7;j++){
		HDR_PRINT("%d ",i_sPQConfigParam->target_display_config.vsvdb[j]);
	}
	HDR_PRINT("reference_mode_dark_id			:: %d\n",i_sPQConfigParam->target_display_config.reference_mode_dark_id);
//	HDR_PRINT("reference_mode_bright_id		:: %d\n",i_sPQConfigParam->target_display_config.reference_mode_bright_id);
	HDR_PRINT("backlight_scaler 				:: %d\n",i_sPQConfigParam->target_display_config.backlight_scaler);
	HDR_PRINT("ocscConfig.lms2RgbMat 	\n");
	for(i=0;i<3;i++){
		HDR_PRINT("\t\t\t");
		for(j=0;j<3;j++){
			HDR_PRINT("%d ",i_sPQConfigParam->target_display_config.ocscConfig.lms2RgbMat[i][j]);
		}
		HDR_PRINT("\n");
	}
	HDR_PRINT("ocscConfig.lms2RgbMatScale 			:: %d\n",i_sPQConfigParam->target_display_config.ocscConfig.lms2RgbMatScale);
	HDR_PRINT("brightnessPreservation 				:: %d\n",i_sPQConfigParam->target_display_config.brightnessPreservation);
}

/**
 * @brief start F/W communication thread
 *
 * @param i_unWinID  [IN] UINT32
 * @param i_eCntlPath [IN] E_HDR_CNTL_PATH
 *
 * @return RET_OK(0) if success, none zero for otherwise
 */
int DOVI_FWCommStart(UINT32 i_unWinID,E_HDR_CNTL_PATH i_eCntlPath)
{
	int nRetVal = RET_ERROR;

	HDR_DEBUG("ENTER %s\n",__func__);

	do
	{
		HDR_PRINT("%s API is not required for this ARCHICTECTUER\n",__func__);

		nRetVal = RET_OK;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);

    return nRetVal;

}

/**
 * @brief stop F/W communication thread
 *
 * @param i_unWinID  [IN] UINT32
 *
 * @return RET_OK(0) if success, none zero for otherwise
 */
int DOVI_FWCommStop(UINT32 i_unWinID)
{
	int nRetVal = RET_ERROR;

	HDR_DEBUG("ENTER %s\n",__func__);

	do
	{
		HDR_PRINT("%s API is not required for this ARCHICTECTUER\n",__func__);

		nRetVal = RET_OK;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);

    return nRetVal;
}

/**
 * @brief metadata mapping initialization [one at a time]
 * @param i_unWinID  			[IN] UINT32
 * @param i_eCntlPath  	       [IN] E_HDR_CNTL_PATH
 * @return RET_OK(0) if success, none zero for otherwise
 */
int Dovi_MDAsyncMap_Init(UINT32 i_unWinID,E_HDR_CNTL_PATH i_eCntlPath)
{
	int nRetVal = RET_ERROR;
	LX_DOLBY_MODE_T eDolbyRunMode;
	UINT32 unSemInitVal = 0;
	int savedMask;
	HDR_DEBUG("ENTER %s\n",__func__);

	if(	g_sAsyncMDContext[i_unWinID].bInitialized == true){
		HDR_NOTI("NOTI:: MDSync MAP init success\n");
		return RET_OK;
	}

	do
	{
		if(i_eCntlPath == E_HDMI_CNTRL_PATH)
			eDolbyRunMode = LX_DOLBY_HDMI_MODE;
		else if(i_eCntlPath == E_OTT_CNTRL_PATH)
			eDolbyRunMode = LX_DOLBY_OTT_MODE;
		else
			eDolbyRunMode = LX_DOLBY_HDMI_LL_MODE;

		if(g_sAsyncMDContext[i_unWinID].semAsyncMD == NULL){
			savedMask = umask(0);
			g_sAsyncMDContext[i_unWinID].semAsyncMD = sem_open( MD_SYNC_SEM, O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO, unSemInitVal );
			umask(savedMask);
			if(SEM_FAILED == g_sAsyncMDContext[i_unWinID].semAsyncMD){
				HDR_ERROR("OTT/LL Mutex Init Failed :: %s\n",strerror(errno));
				g_sAsyncMDContext[i_unWinID].semAsyncMD = NULL;
				nRetVal = RET_ERROR;
				break;
			}
			else {
				HDR_PRINT("NOTI:: OTT/LL Mutex initialized for Win [%d]\n",i_unWinID);
			}
		}

		HDR_PRINT("%s API is not required for this ARCHICTECTUER Dolby Mode is %d\n",__func__,eDolbyRunMode);

		g_sAsyncMDContext[i_unWinID].bInitialized = true;
		g_sAsyncMDContext[i_unWinID].ucWinId = i_unWinID;
		nRetVal = RET_OK;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);
	return nRetVal;
}

/**
 * @brief Uninitialize metadata mapping
 * Format of metadata buffer is:
 * @param i_unWinID  			[IN] UINT32
 * @return RET_OK(0) if success, none zero for otherwise
 */
int Dovi_MDAsyncMap_UnInit(UINT32 i_unWinID)
{
	int nRetVal = RET_ERROR;

	HDR_DEBUG("ENTER %s\n",__func__);

	if(	g_sAsyncMDContext[i_unWinID].bInitialized == false){
		HDR_NOTI("NOTI:: MDSync MAP not initialized\n");
		return RET_OK;
	}

	do
	{
		HDR_PRINT("%s API is not required for this ARCHICTECTUER\n",__func__);

		if(g_sAsyncMDContext[i_unWinID].semAsyncMD != NULL){
			sem_close(g_sAsyncMDContext[i_unWinID].semAsyncMD);
		//	sem_unlink(MD_SYNC_SEM);
		}

		g_sAsyncMDContext[i_unWinID].semAsyncMD = NULL;

		g_sAsyncMDContext[i_unWinID].bInitialized = false;
		g_sAsyncMDContext[i_unWinID].ucWinId = MAX_WNDS;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);
	return nRetVal;
}

/**
 * @brief write metadata [one at a time]
 * Format of metadata buffer is:
 * @param i_unWinID  			[IN] UINT32
 * @param i_unHDRSubMode  	[IN] E_HDR_CNTL_PATH
 * @param i_cptrMDBuf  		[IN] void *
 * @return RET_OK(0) if success, none zero for otherwise
 */
int DOVI_MDAsyncWrite(UINT32 i_unWinID,dovi_md_inf *i_cptrMDBuf)
{
	int nRetVal = RET_ERROR;
	HDR_DEBUG("ENTER %s\n",__func__);


	if(	g_sAsyncMDContext[i_unWinID].bInitialized == false){
		HDR_NOTI("NOTI:: MDSync MAP not initialized\n");
		return RET_OK;
	}

	do
	{
		HDR_PRINT("%s API is not required for this ARCHICTECTUER\n",__func__);
		nRetVal = RET_OK;
	}while(0);

	HDR_DEBUG("INFO:: Written metadata [%llu]\n",i_cptrMDBuf->ulTimeStamp);

	sem_post(g_sAsyncMDContext[i_unWinID].semAsyncMD);

	HDR_DEBUG("EXIT %s\n",__func__);
    return nRetVal;
}

int DOVI_SetGDDelayDB(S_DOLBY_GD_DELAY_LUT_T *i_pstdGddelaydb,S_DOLBY_GD_DELAY_LUT_T *i_pllGddelaydb)
{
	int nRetVal = RET_OK;

	do {
		memcpy((void *)&g_sDolbySTDGddelaydb,(void *)i_pstdGddelaydb,sizeof(S_DOLBY_GD_DELAY_LUT_T));
		memcpy((void *)&g_sDolbyLLGddelaydb,(void *)i_pllGddelaydb,sizeof(S_DOLBY_GD_DELAY_LUT_T));
		HDR_NOTI("INFO:: The GD table has bee updated\n");
	}while(0);

	return nRetVal;
}


int DOVI_SetAmbientLight(UINT32 i_unWinID,UINT32 uOnOff,UINT32 uLux,UINT32 *uRawdata)
{
	int nRetVal = RET_ERROR;
	UI_CMD_T *sptrDoviHDRInfo = &g_sUICommands[i_unWinID];

	//HDR_PRINT("ENTER %s\n",__func__);

	do
	{
		if(g_sDolbyWndContext[i_unWinID].bInitialized == false ){
			HDR_ERROR("ERROR::Dolby HDR is not initialize for [%d] Window\n",i_unWinID);
			break;
		}

		HDR_DEBUG("Lock Mutex %s\n",__func__);
		sem_wait(&g_sDolbyWndContext[i_unWinID].pqMutex);

		sptrDoviHDRInfo->ambientParam.ambient = (int)uOnOff;
		/*as per sunny song comment UNIT32 *uRawdata is array. (or the uRawdata[4], [0],[3] doesn't have any meaning, [1] is green wavelength channel, [2] is white wavelength channel.
		Assigning [1] until more clarity about it is available*/
		sptrDoviHDRInfo->ambientParam.tFrontLux= uRawdata[1];

		/* Setting tFrontLux to 0 in-case ambient is 0 to resolve sudden effect of ambient in-case of UI on/off
		     as suggested by Dolby on 20 September. Forcing ambient to 1 to make ambient algorithm execute
		     with tFrontLux=0 values
		 */
		if(sptrDoviHDRInfo->ambientParam.ambient == 0){
			sptrDoviHDRInfo->ambientParam.tFrontLux = 0;
		}

		sptrDoviHDRInfo->pqUpdateFlag |= PQ_AMBIENT_CHANGE_FLAG;
		sptrDoviHDRInfo->updated = 1;

		if(g_sDolbyWndContext[i_unWinID].eCntlPath == E_HDMI_LL_CNTRL_PATH){
			sem_post(g_sDolbyWndContext[i_unWinID].mdSyncSem);
		}

		if((lx_chip_rev() >= LX_CHIP_REV(M17,C0)) && (g_sDolbyWndContext[i_unWinID].eCntlPath == E_OTT_CNTRL_PATH)) {
			sem_post(g_sDolbyWndContext[i_unWinID].mdSyncSem);
		}

		sem_post(&g_sDolbyWndContext[i_unWinID].pqMutex);
		HDR_DEBUG("UnLocked PQ Mutex %s\n",__func__);

		nRetVal = RET_OK;
	}while(0);

	HDR_DEBUG("EXIT %s\n",__func__);

	return nRetVal;
}

int DOVI_SetPDMode(UINT32 i_unWinID,UINT32 uOnOff)
{
	HDR_PRINT("Nothing to do [PD Mode not supported]");
	return 0;
}

int DOVI_V4L2_GetPQMode(UINT8 i_unWinID,struct v4l2_ext_dolby_picture_mode *st_PictureMode )
{
	//if(!bOnOff) return OK;
	int nRetval = RET_ERROR;
	int fd = g_dolby_dev_fd;
	if(!st_PictureMode) {
		HDR_ERROR("Null input\n");
		return nRetval;
	}
	//struct v4l2_ext_dolby_picture_mode st_PictureMode;
	struct v4l2_ext_controls s_ext_controls;
	struct v4l2_ext_control s_ext_control;
	struct v4l2_ext_vpq_cmn_data pqData;

	memset(&s_ext_controls, 0, sizeof(struct v4l2_ext_controls));
	memset(&s_ext_control, 0, sizeof(struct v4l2_ext_control));
	memset(&pqData, 0, sizeof(struct v4l2_ext_vpq_cmn_data));


	s_ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	s_ext_controls.count = 1;
	s_ext_controls.controls = &s_ext_control;

	pqData.length = sizeof(struct v4l2_ext_dolby_picture_mode);
	pqData.wid=i_unWinID;
	pqData.p_data=(UINT8 *)st_PictureMode;
	s_ext_controls.controls->id = V4L2_CID_EXT_DOLBY_PICTURE_MODE;
	s_ext_controls.controls->size = sizeof(pqData);
	s_ext_controls.controls->string = (char *)&pqData;
	nRetval = ioctl(fd, VIDIOC_G_EXT_CTRLS, &s_ext_controls);
	if (nRetval < 0)
	{
		HDR_ERROR("ioctl(%d)(VIDIOC_S_EXT_CTRLS),err:%d\n", fd, nRetval);
		return nRetval;
	}
	HDR_NOTI("picture Mode from driver is %d %d \n",st_PictureMode->uPictureMode,st_PictureMode->bOnOff);
	return nRetval;
}
#if 0 /*2018 Implementation */
static int str_to_int(char* str, int* val)
{
    char* endptr;
    long tmp = -1, len;

    if (str == NULL) {
		HDR_PRINT("NULL string\n");
		return -1;
    }
	else {
		len = strlen(str);
		/* Remove \n */
		if(str[len-1] == '\n')
			str[len-1] = '\0';
	}
    tmp = strtol(str, &endptr, 10);
	#if 0
    if ((errno == ERANGE && (tmp == LONG_MAX || tmp == LONG_MIN)) || (errno != 0 && tmp == 0)) {
		HDR_PRINT("ERANGE etc [%d] \n",tmp);
        return -1;
    }
    if (endptr == str) {
		HDR_PRINT("endptr==str etc\n");
        return -1;
    }
    if ((*endptr != '\0') && (*endptr != '\n')){
		HDR_PRINT("endptr != eos or new line etc\n");
        return -1;
    }
	#endif
    *val = (int)tmp;
    return 0;
}

static int read_ambient_cfg(ambient_cfg_t* ambient_cfg, int frNum)
{
	char *tok;
	int tmp, rt = 0;
	static int current_frame = 0;
	int line = 0;
	static FILE *fp = NULL;
	float fVal;

	if(frNum == 0 && fp){
		fclose(fp);
		fp = NULL;
	}

	if(!fp){
		fp = fopen("/var/ambient.cfg","r");
		if(!fp)return -1;
	}
	while (1)
	{
		if (feof(fp)){
			return 0;
		}
		if(fgets(line_buffer, 1024, fp) == NULL){
			if (feof(fp)) return rt;
            else return -1;
		}
		line++;

		CHECK_PARSE((tok = strtok(line_buffer, " \t")) == NULL);
//		HDR_PRINT("TOKEN [%s]\n",tok);
		if (tok[0] == '%')
			continue;
		if (tok[0] == '-') tok++; /* all parameters can start with a '-', so we remove it here */
		if ((strcmp(tok, "cfgFrmStartNum") == 0) || (strcmp(tok, "CfgFrmStartNum") == 0)) {
			CHECK_PARSE((tok = strtok(NULL, " \t")) == NULL);
			CHECK_PARSE(str_to_int(tok, &tmp));
			current_frame = tmp;
			if(frNum < current_frame){
				ambient_cfg->nUpdateFlags = rt;
				return rt;
			}
		} else if ((strcmp(tok, "tWhitexy") == 0) || (strcmp(tok, "TWhitexy") == 0)) {
			CHECK_PARSE((tok = strtok(NULL, " \t")) == NULL);
			//CHECK_PARSE(str_to_int(tok, &tmp));
			str_to_float(tok,&fVal);
			//ambient_cfg->tWhiteXY[0] = tmp;
			ambient_cfg->tWhiteXY[0] = (int)(fVal * (1<<15));
			tok = strtok(NULL, " \t");
			//CHECK_PARSE(str_to_int(tok, &tmp));
			//ambient_cfg->tWhiteXY[1] = tmp;
			str_to_float(tok,&fVal);
			ambient_cfg->tWhiteXY[1] = (int)(fVal * (1<<15));
			rt = rt | AMBIENT_UPD_WHITEXY;
		} else if ((strcmp(tok, "tRearLum") == 0) || (strcmp(tok, "TRearLum") == 0)) {
			CHECK_PARSE((tok = strtok(NULL, " \t")) == NULL);
			CHECK_PARSE(str_to_int(tok, &tmp));
			ambient_cfg->tRearLum = tmp;
			rt = rt | AMBIENT_UPD_REAR;
		} else if ((strcmp(tok, "tFrontLux") == 0) || (strcmp(tok, "TFrontLux") == 0)) {
			CHECK_PARSE((tok = strtok(NULL, " \t")) == NULL);
			CHECK_PARSE(str_to_int(tok, &tmp));
			ambient_cfg->tFrontLux = tmp;
			rt = rt | AMBIENT_UPD_FRONT;
		} else if ((strcmp(tok, "ambient") == 0) || (strcmp(tok, "Ambient") == 0)) {
			CHECK_PARSE((tok = strtok(NULL, " \t")) == NULL);
			CHECK_PARSE(str_to_int(tok, &tmp));
			ambient_cfg->ambient = tmp;
			rt = rt | AMBIENT_UPD_MODE;
		}
	}
	return 0;
}

#else

static int starts_with(const char *full_str, const char *test_str)
{
    size_t len_test = strlen(test_str);
    size_t len_full = strlen(full_str);
    if (len_full < len_test)
        return 0;
    else {
        if (strncmp(test_str, full_str, len_test) == 0)
            return len_test;
        else
            return 0;
    }
}

static int read_ambient_cfg(ambient_cfg_t* ambient_cfg,int frame_nr)
{
    char line_buffer[4096];
    char *ptr;
    int len, rt = 0;
    static int current_frame = 0;
    int line = 0;
	float fVal1, fVal2, fAmb;
	static FILE *fp = NULL;

	if(!fp){
		fp = fopen("/var/ambient.cfg","r");
		if(!fp) return -1;
	}

    if (frame_nr < current_frame){
        if (feof(fp)){
			current_frame = 0;
			fclose(fp);
			fp = NULL;
            return 0;
        }
    }
    while (1)
    {
        if (feof(fp)){
			current_frame = 0;
			fclose(fp);
			fp = NULL;
            return 0;
        }
        if(fgets(line_buffer, 4096, (fp)) == NULL){
            if (feof((fp))){
				current_frame = 0;
				fclose(fp);
				fp = NULL;
				return rt;
            }
            else return -1;
        }
        line++;
        ptr = line_buffer;
        if (ptr[0] == '%')
            continue;
        if (ptr[0] == '-')
            ptr++;

        /* all parameters can start with a '-', so we remove it here */
        if (((len = starts_with(ptr, "cfgFrmStartNum")) > 0) || ((len = starts_with(ptr, "CfgFrmStartNum")) > 0)) {
            ptr += len;
            CHECK_PARSE(sscanf(ptr, "%d", &current_frame));
            if (frame_nr < current_frame){
				ambient_cfg->nUpdateFlags = rt;
                return rt;
            }
        } else if (((len = starts_with(ptr, "tWhitexy")) > 0) || ((len = starts_with(ptr, "TWhitexy")) > 0)) {
            ptr += len;
            CHECK_PARSE(sscanf(ptr, "%f %f", &fVal1, &fVal2));
			ambient_cfg->tWhiteXY[0] = (int)(fVal1 * (1<<15));
			ambient_cfg->tWhiteXY[1] = (int)(fVal2 * (1<<15));
            rt = rt | AMBIENT_UPD_WHITEXY;
        } else if (((len = starts_with(ptr, "tRearLum")) > 0) || ((len = starts_with(ptr, "TRearLum")) > 0)) {
            ptr += len;
            CHECK_PARSE(sscanf(ptr, "%d", &ambient_cfg->tRearLum));
            rt = rt | AMBIENT_UPD_REAR;
        } else if (((len = starts_with(ptr, "tFrontLux")) > 0) || ((len = starts_with(ptr, "TFrontLux")) > 0)) {
            ptr += len;
            CHECK_PARSE(sscanf(ptr, "%d", &ambient_cfg->tFrontLux));
            rt = rt | AMBIENT_UPD_FRONT;
        } else if (((len = starts_with(ptr, "ambient")) > 0) || ((len = starts_with(ptr, "Ambient")) > 0)) {
            ptr += len;
            CHECK_PARSE(sscanf(ptr, "%f", &fAmb));
			ambient_cfg->ambient = (int)fAmb /**(1<<16) */;
            rt = rt | AMBIENT_UPD_MODE;
        }
    }
    return 0;
}
#endif
