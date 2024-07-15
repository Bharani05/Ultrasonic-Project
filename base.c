/****************************************************************************************
 *   DTV LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *   COPYRIGHT(c) 1998-2010 by LG Electronics Inc.
 *
 *   All rights reserved. No part of this work covered by this copyright hereon
 *   may be reproduced, stored in a retrieval system, in any form
 *   or by any means, electronic, mechanical, photocopying, recording
 *   or otherwise, without the prior written  permission of LG Electronics.
 ***************************************************************************************/

/** @file
 *
 *  Brief description.
 *  Detailed description starts here.
 *
 *  @author		raxis
 *  @version	1.0
 *  @date		2010-04-14
 *  @note		Additional information.
 */

/*----------------------------------------------------------------------------------------
	Control Constants
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
	File Inclusions
----------------------------------------------------------------------------------------*/
#include "osa_kadp.h"
#include "debug_kadp.h"
#include <string.h>
#include <sys/types.h>
#ifndef __BIONIC__
#include <semaphore.h>
#endif

/*----------------------------------------------------------------------------------------
	Constant Definitions
----------------------------------------------------------------------------------------*/
#define KADP_INTRO_MSG

/*----------------------------------------------------------------------------------------
	Macro Definitions
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
	Type Definitions
----------------------------------------------------------------------------------------*/
typedef	void (*INIT_MOD_FUNC_T)(void);
typedef void (*EXIT_MOD_FUNC_T)(void);

typedef struct
{
	BOOLEAN	b_active;
	int		ref_cnt;
}
KADP_BASE_CTX_T;

/*----------------------------------------------------------------------------------------
	External Function Prototype Declarations
----------------------------------------------------------------------------------------*/
extern void	KADP_GFX_Initialize		(void);
extern void	KADP_GFX_Shutdown		(void);
extern void	KADP_GAL_Initialize		(void);
extern void	KADP_GAL_Shutdown		(void);
extern void	KADP_FBDEV_Initialize	(void);
extern void	KADP_FBDEV_Shutdown		(void);
extern void KADP_SYS_Initialize		(void);
extern void KADP_SYS_Shutdown		(void);
extern void KADP_SE_Shutdown		(void);	// Note : SE only needs Shutdown
extern void KADP_LOGM_Init 			(void);
extern void KADP_LOGM_Final 		(void);
extern void KADP_VENC_Initialize 	(void);
extern void KADP_VENC_Shutdown 		(void);
extern void KADP_MMC_Initialize 	(void);
extern void KADP_PMS_Initialize 	(void);
extern void KADP_PMS_Shutdown 		(void);
extern void KADP_PNG_Initialize 	(void);
extern void KADP_PNG_Shutdown 		(void);
extern void KADP_VP_DBG_Initialize 	(void);
extern void KADP_VP_DBG_Shutdown 	(void);
extern void KADP_DE_DBG_Initialize 	(void);
extern void KADP_DE_DBG_Shutdown 	(void);
extern void KADP_HDMI20_DBG_Initialize(void);
extern void KADP_AFE_DBG_Initialize (void);
extern void KADP_HDMI20_DBG_Shutdown 	(void);
extern void KADP_AFE_DBG_Shutdown 	(void);
extern void KADP_TE_Initialize		(void);
extern void KADP_CI_Initialize 		(void);
extern void KADP_CI_Shutdown 		(void);
extern void KADP_SCI_Initialize 	(void);
extern void KADP_SCI_Shutdown 		(void);
extern void KADP_PVR_Initialize 	(void);
extern void KADP_PVR_Shutdown 		(void);
extern void KADP_SDEC_Initialize 	(void);
extern void KADP_SDEC_Shutdown 		(void);
extern void KADP_ATSC3_Initialize	(void);
extern void KADP_ATSC3_Shutdown		(void);
extern void KADP_CIPLUS_Initialize	(void);
extern void KADP_CIPLUS_Shutdown	(void);
extern void KADP_ARIB2_Initialize	(void);
extern void KADP_ARIB2_Shutdown		(void);
extern void KADP_HDR_DBG_Initialize(void);
extern void KADP_HDR_DBG_Shutdown(void);
extern void KADP_SE_DBG_Initialize 	(void);
extern void KADP_SE_DBG_Shutdown 	(void);
extern void KADP_APR_DBG_Initialize (void);
extern void KADP_APR_DBG_Shutdown 	(void);
extern void KADP_PE_DBG_Initialize 	(void);
extern void KADP_PE_DBG_Shutdown 	(void);
extern void KADP_BE_DBG_Initialize(void);
extern void KADP_BE_DBG_Shutdown(void);
extern void KADP_DEMOD_DBG_Initialize(void);
extern void KADP_DEMOD_DBG_Shutdown (void);

//extern void KADP_SSM_Initialize		(void);
//extern void KADP_SSM_Shutdown		(void);
extern void KADP_VTERM_Initialize	(void);
extern void KADP_VTERM_Shutdown		(void);

extern void KADP_VBI_DBG_Initialize (void);
extern void KADP_VBI_DBG_Shutdown 	(void);
extern void	KADP_UCOM_Initialize	(void);
extern void	KADP_UCOM_Shutdown		(void);
/*----------------------------------------------------------------------------------------
	External Variables
----------------------------------------------------------------------------------------*/
extern char *__progname;

/*----------------------------------------------------------------------------------------
	global Variables
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
	Static Function Prototypes Declarations
----------------------------------------------------------------------------------------*/
static INIT_MOD_FUNC_T	g_kadp_mod_init_func_list[] =
{
	KADP_SYS_Initialize,
	//KADP_SSM_Initialize,
	KADP_GFX_Initialize,
	KADP_GAL_Initialize,
	KADP_FBDEV_Initialize,
	KADP_VTERM_Initialize,
	KADP_VENC_Initialize,
	KADP_PNG_Initialize,
	KADP_VP_DBG_Initialize,
	KADP_DE_DBG_Initialize,
	KADP_HDMI20_DBG_Initialize,
	KADP_AFE_DBG_Initialize,
	KADP_TE_Initialize,
	KADP_CI_Initialize,
	KADP_SCI_Initialize,
	KADP_PVR_Initialize,
	KADP_SDEC_Initialize,
	KADP_ATSC3_Initialize,
	KADP_CIPLUS_Initialize,
	KADP_ARIB2_Initialize,
#ifndef PLATFORM_FPGA
	KADP_SE_DBG_Initialize,
#endif
	KADP_APR_DBG_Initialize,
	KADP_PE_DBG_Initialize,
	KADP_BE_DBG_Initialize,
	KADP_DEMOD_DBG_Initialize,
	KADP_HDR_DBG_Initialize,
#ifndef ANDROID
	KADP_PMS_Initialize,
#endif
	KADP_MMC_Initialize,
	KADP_VBI_DBG_Initialize,
	KADP_UCOM_Initialize,
	NULL,
};

static INIT_MOD_FUNC_T	g_kadp_mod_exit_func_list[] =
{
#ifndef PLATFORM_FPGA
	KADP_SE_Shutdown,
#endif
	KADP_VTERM_Shutdown,
	KADP_FBDEV_Shutdown,
	KADP_GFX_Shutdown,
	KADP_GAL_Shutdown,
	KADP_SYS_Shutdown,
	KADP_VENC_Shutdown,
	KADP_PNG_Shutdown,
#ifndef ANDROID
	KADP_PMS_Shutdown,
#endif
	KADP_VP_DBG_Shutdown,
	KADP_DE_DBG_Shutdown,
	KADP_HDMI20_DBG_Shutdown,
	KADP_AFE_DBG_Shutdown,
	KADP_CI_Shutdown,
	KADP_SCI_Shutdown,
	KADP_PVR_Shutdown,
	KADP_SDEC_Shutdown,
	KADP_ATSC3_Shutdown,
	KADP_CIPLUS_Shutdown,
	KADP_ARIB2_Shutdown,
#ifndef PLATFORM_FPGA
	KADP_SE_DBG_Shutdown,
#endif
	KADP_APR_DBG_Shutdown,
	KADP_PE_DBG_Shutdown,
	KADP_BE_DBG_Shutdown,
	KADP_DEMOD_DBG_Shutdown,
	KADP_HDR_DBG_Shutdown,

	//KADP_SSM_Shutdown,
	KADP_SYS_Shutdown,
	KADP_VBI_DBG_Shutdown,
	KADP_UCOM_Shutdown,
	NULL,
};

/*----------------------------------------------------------------------------------------
	Static Variables
----------------------------------------------------------------------------------------*/
static  BOOLEAN 		b_logm_inited = FALSE;
static	sem_t*			g_kadp_init_mtx = NULL;
static	KADP_BASE_CTX_T	g_kadp_ctx	= { .b_active = FALSE, .ref_cnt = 0 };

/*========================================================================================
	Implementation Group
========================================================================================*/
void	KADP_InitSystem	( void )
{
	UINT32	osa_init_status = 0;
	UINT32	dbg_init_status = 0;
	int		max_retry_num = 2;
	BOOLEAN	b_init_locked = FALSE;

	if (!b_logm_inited)
	{
		KADP_LOGM_Init();
		b_logm_inited = TRUE;
	}

	/* serialize kadaptor initialization
	 * DO NOT block the initializaiton even though the current thread can't get init_mtx
	 */
	if (g_kadp_init_mtx)
	{
		for (int i=0;i<=max_retry_num; i++)
		{
			if(0==sem_trywait(g_kadp_init_mtx)) { b_init_locked=TRUE; break; }

			usleep(10*1000);	/* 10 msec */
			KADP_PRINT("waiting for init_mtx (%d/%d)\n", i, max_retry_num);
		}

		if(!b_init_locked) KADP_PRINT("continue init process\n");
	}

	/* ignore the further initialization if other thread already initialized kadaptor */
	if ( ++g_kadp_ctx.ref_cnt > 1 || g_kadp_ctx.b_active == TRUE ) goto func_exit;

#ifdef KADP_INTRO_MSG
    KADP_PRINT("KAdaptor Init [%d] PS %s, TID %05d (Kdriver Adaptation Layer for lg1k)\n",
		g_kadp_ctx.ref_cnt, __progname, KADP_OSA_GetCurrThreadId());
#endif

	/* initialize kadaptor core */
	osa_init_status = KADP_OSA_Initialize( );
	dbg_init_status = KADP_DBG_Initialize( );

	int	i;
	for (i=0; ;i++)
	{
		if ( NULL==g_kadp_mod_init_func_list[i] ) break;

		g_kadp_mod_init_func_list[i]();
	}

	g_kadp_ctx.b_active = TRUE;
#ifdef KADP_INTRO_MSG
    KADP_PRINT("KAdaptor Init Done. status=0x%08x,0x%08x\n", osa_init_status, dbg_init_status);
#endif

func_exit:
	/* always release init_mtx even though the current thread can't get init_mtx. simple recovery mode. */
	if (g_kadp_init_mtx) sem_post(g_kadp_init_mtx);
}

void	KADP_ShutdownSystem	( void )
{
	/* ignore if other process/thread opened kadaptor */
	if ( --g_kadp_ctx.ref_cnt > 0 || g_kadp_ctx.b_active == FALSE ) return;

#ifdef KADP_INTRO_MSG
    KADP_PRINT("KAdaptor Exit [%d] PS %s\n", g_kadp_ctx.ref_cnt, __progname );
#endif

	int	i;
	for (i=0; ;i++)
	{
		if ( !g_kadp_mod_exit_func_list[i] ) break;

		g_kadp_mod_exit_func_list[i]();
	}

	/* shutdown kadaptor core */
	KADP_DBG_Shutdown( );
	KADP_OSA_Shutdown( );

	g_kadp_ctx.ref_cnt = 0;
	g_kadp_ctx.b_active = FALSE;

#ifdef KADP_INTRO_MSG
    KADP_PRINT("KAdaptor Exit Done\n");
#endif
	KADP_LOGM_Final();
}

/*========================================================================================
	Implementation Group
========================================================================================*/
static void LX_OSA_ON_INIT __KADP_Initialize( void )
{
	if (!g_kadp_init_mtx) g_kadp_init_mtx = sem_open( "kadp-init-mtx", O_CREAT, 0777, 1 );
	//printf("+++ g_kadp_init_mtx = %x\n", (int)g_kadp_init_mtx);
}

#if 0
static void LX_OSA_ON_EXIT	__KADP_Shutdown	( void )
{
	KADP_ShutdownSystem( );
}
#endif

