#ifndef DOVI_HDR_H
#define DOVI_HDR_H

/*-----------------------------------------------------------------------------
	File Inclusions
------------------------------------------------------------------------------*/
#include <stdbool.h>
#include "be_kadp.h"
#include "hdr_kadp_defs.h"
#include "hdr_v4l2_cmn.h"
#include "dovi_algorithm.h"


/*----------------------------------------------------------------------------------------
 *   Constant Definitions
 *---------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------
 *   Macro Definitions
 *---------------------------------------------------------------------------------------*/

//#define IP_SIMULATION
#define MAX_CONFIG_FILES	1
#define KADP_DOVI_SW_MAJOR_NUM 28
#define KADP_DOVI_SW_MINOR_NUM 7
/*----------------------------------------------------------------------------------------
 *   Type Definitions
 *---------------------------------------------------------------------------------------*/

typedef struct UI_COMMANDS{
	E_HDR_MOVIE_MODES pqmode;
	E_HDR_PQ_SUB_PARAM pqsubmode[HDR_PQ_MAX];
	src_param_t srcParam;
	ambient_cfg_t ambientParam;
	int pqUpdateFlag; //notify the all changes
	int updated;
}UI_CMD_T;

typedef enum E_ACCESS_FLAG
{
	E_GET = 0,
	E_SET
} ACCESS_FLAG_T;

typedef struct S_DOLBY_PQ_CONFIGURATIONS
{
	pq_config_t *sptrPQConfigurations;
	char *cptrDolbyConfigFile;
	unsigned int unConfigCnt;
} PQ_CONFIG_INFO;

typedef struct _DOLBY_ASYNC_MD_CONTEXT
{
	sem_t  *semAsyncMD;//Mutex for LL mode single run [also re-used in OTT case for new design]
	bool	bInitialized;
	unsigned char ucWinId;
} async_md_context;

typedef struct _OLED_BOOST_PARAMS_T
{
	int abenable;
	int gdenable;
	bool bPQModeUpdate;
	bool bSetBoost;
}oled_boost_params;



/*-----------------------------------------------------------------------------
	Extern Variables & Function Prototype Declarations
------------------------------------------------------------------------------*/
int DOVI_Init(UINT32 i_unWinID,dovi_frame_size i_dovi_size,LX_HDR_MODE_T i_eHDRMode,E_HDR_CNTL_PATH i_eCntlPath);
int DOVI_UnInit(UINT32 i_unWinID,LX_HDR_MODE_T i_eHDRMode);
int DOVI_Start(UINT32 i_unWinID,E_HDR_CNTL_PATH i_eCntlPath);
int DOVI_Stop(UINT32 i_unWinID);
int DOVI_SetPQMode(UINT32 i_unWinID,E_HDR_MOVIE_MODES i_eDoviMovieMode);
int DOVI_SetPQSubMode(UINT32 i_unWinID,E_HDR_PQ_SUB_PARAM i_unPQSubMode,int i_nVal);
int DOVI_SetPQConfig(char* i_cptrConfigFilePaths[],UINT32 i_unConfigCnt);
int DOVI_Get_SWVersion(char *pstVersion);
int DOVI_Get_DMVersion(char *pstVersion,char *pVerText);
int DOVI_FWCommStart(UINT32 i_unWinID,E_HDR_CNTL_PATH i_eCntlPath);
int DOVI_FWCommStop(UINT32 i_unWinID);
int Dovi_MDAsyncMap_Init(UINT32 i_unWinID,E_HDR_CNTL_PATH i_eCntlPath);
int Dovi_MDAsyncMap_UnInit(UINT32 i_unWinID);
int DOVI_MDAsyncWrite(UINT32 i_unWinID,dovi_md_inf *i_cptrMDBuf);
void displayTargetConfig(pq_config_t *i_sPQConfigParam);
int DOVI_SetGDDelayDB(S_DOLBY_GD_DELAY_LUT_T *i_pstdGddelaydb,S_DOLBY_GD_DELAY_LUT_T *i_pllGddelaydb);
int DOVI_SetAmbientLight(UINT32 i_unWinID,UINT32 uOnOff,UINT32 uLux,UINT32 *uRawdata);
int DOVI_V4L2_GetPQMode(UINT8 i_unWinID,struct v4l2_ext_dolby_picture_mode *st_PictureMode );
int DOVI_SetPDMode(UINT32 i_unWinID,UINT32 uOnOff);
#endif //DOVI_HDR_H
