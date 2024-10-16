{
		switch(g_sDbgData.funcnum)
		{
			case 0x01:
			{
				unEnable = g_sDbgData.ucDolbyEnable;
				i_unHDRSubMode = g_sDbgData.ucMode;
				ucEnbestpq = g_sDbgData.ucEnbestpq;
				dovi_frame_size sFrameSize;
				HDR_PRINT("unEnable = %d i_unHDRSubMode = %d ucEnbestpq = %d g_sDbgData.ucEnbestpq  %d\n",unEnable,i_unHDRSubMode,ucEnbestpq,g_sDbgData.ucEnbestpq);
				if(unEnable){
						if(i_unHDRSubMode > 4){
						HDR_PRINT("Unsupported Mode %d\n",i_unHDRSubMode);
						break;
					}
						if(lx_chip_rev() >= LX_CHIP_REV(M17,A0)){
						UINT32 unWidth = 3840;
						UINT32 unHeight= 2160;
						LX_HDR_MODE_T eHDRMode; ;
						KADP_VP_TIMING_INFO_T sSrcTimingInfo;
						E_HDR_CNTL_PATH eHDRCntlPath = E_UNKNOWN_CNTRL_PATH;
						DOLBY_PQ_CONFIG_INFO i_sPQInfo;
						if(i_unHDRSubMode == 4){
							eHDRMode = LX_HDR_TYPE_DOLBY_LL;
						}
						else if(i_unHDRSubMode == 2) {
							eHDRMode = LX_HDR_TYPE_DOLBY_RF;
						}
						else{
							eHDRMode = LX_HDR_TYPE_DOLBY;
						}

						/*Get timing info of source */
						if(DOVI_IP_INF_GetTimingInfo(g_sDbgData.ucwinId,&sSrcTimingInfo) == RET_OK){
							unWidth 	= sSrcTimingInfo.hActive;
							unHeight	= sSrcTimingInfo.vActive;
						}
						else {
							HDR_PRINT("ERROR: Failed to get src information\n");
						}
						if(ucEnbestpq == 1)
						{
							i_sPQInfo.dev_fd = gs_dolby_event.dolby_dev_fd;
							i_sPQInfo.eConfigType = V4L2_EXT_DOLBY_CONFIG_BEST;
							i_sPQInfo.eHDRMode = eHDRMode;
							i_sPQInfo.unWinID = g_sDbgData.ucwinId;
							if(KADP_HDR_DOLBY_V4L2_SetPQConfigFile(i_sPQInfo) != RET_OK){
							HDR_PRINT("HDR Set PQ Config failed for Window [%d]\n",g_sDbgData.ucwinId);
							}
						}
						else{
							i_sPQInfo.dev_fd = gs_dolby_event.dolby_dev_fd;
							i_sPQInfo.eConfigType = V4L2_EXT_DOLBY_CONFIG_MAIN;
							i_sPQInfo.eHDRMode = eHDRMode;
							i_sPQInfo.unWinID = g_sDbgData.ucwinId;
							if(KADP_HDR_DOLBY_V4L2_SetPQConfigFile(i_sPQInfo) != RET_OK){
								HDR_PRINT("HDR Set PQ Config failed for Window [%d]\n",g_sDbgData.ucwinId);
							}
						}
						/*Map to HDR */
						if(i_unHDRSubMode == OVER_THE_TOP){
							eHDRCntlPath = E_OTT_CNTRL_PATH;
						}
						else if(i_unHDRSubMode == RF) {
								eHDRCntlPath = E_RF_CNTRL_PATH;
						}
						else if(i_unHDRSubMode == HDMI){
							eHDRCntlPath = E_HDMI_CNTRL_PATH;
						}
						else {
							eHDRCntlPath = E_HDMI_LL_CNTRL_PATH;
						}
						sFrameSize.width = unWidth;
						sFrameSize.height = unHeight;
						HDR_PRINT("width and heigt are %d %d\n",sFrameSize.width,sFrameSize.height);
						if(RET_OK != KADP_HDR_DOLBY_Backend_Start(g_sDbgData.ucwinId,eHDRCntlPath,eHDRMode)) {
							HDR_ERROR("Dolby Backend Thread START FAILED\n");
						}
						else
							HDR_PRINT("Dolby Backend Thread START SUCCESS\n");
				}

			}
			else{
				if(RET_OK != KADP_HDR_DOLBY_Backend_Stop(g_sDbgData.ucwinId)) {
						HDR_ERROR("Dolby Backend Thread stop FAILED\n");
				}
				else
					HDR_PRINT("Dolby Backend Thread stop SUCCESS\n");
			}
			}break;
			case 0x02:
			{
				gsDolbyDbgContext[unWinId].unProfileEnable = g_sDbgData.ucDbgValue;
				HDR_PRINT("g_sDbgData.ucDbgValue = %d gsDolbyDbgContext[unWinId].unProfileEnable= %d\n",g_sDbgData.ucDbgValue,gsDolbyDbgContext[unWinId].unProfileEnable);
			}break;
			case 0x03:
			{
				unEnable = g_sDbgData.ucDbgValue;
				if(unEnable){
					gsDolbyDbgContext[unWinId].unDumpRange_Start = 0;
					gsDolbyDbgContext[unWinId].unDumpRange_end= g_sDbgData.nMDDumpEnd;
					gsDolbyDbgContext[unWinId].unDumpDone = 0;
					gsDolbyDbgContext[unWinId].unCompMetadataDumpEnable = unEnable;
				}
				else{
					gsDolbyDbgContext[unWinId].unCompMetadataDumpEnable = unEnable;
					gsDolbyDbgContext[unWinId].unDumpRange_Start = 0;
					gsDolbyDbgContext[unWinId].unDumpRange_end = 0;
				}
				HDR_PRINT("\ng_sDbgData.ucwinId = %d g_sDbgData.ucDbgValue = %d g_sDbgData.nMDDumpEnd = %d\n",g_sDbgData.ucwinId,g_sDbgData.ucDbgValue,g_sDbgData.nMDDumpEnd);
				HDR_PRINT("gsDolbyDbgContext[unWinId].unDumpRange_end = %d\n",gsDolbyDbgContext[unWinId].unDumpRange_end);
			}break;
			case 0x04:
			{
				unEnable = g_sDbgData.ucDbgValue;

				if(unEnable){
					gsDolbyDbgContext[unWinId].unDumpRange_Start = 0;
					gsDolbyDbgContext[unWinId].unDumpRange_end= g_sDbgData.nMDDumpEnd;
					gsDolbyDbgContext[unWinId].unDumpDone = 0;
					gsDolbyDbgContext[unWinId].unDmMetadataDumpEnable= unEnable;
					gsDolbyDbgContext[unWinId].unDumpRange_Start = 0;
					gsDolbyDbgContext[unWinId].unDumpRange_end = g_sDbgData.nMDDumpEnd;
				}else{
					gsDolbyDbgContext[unWinId].unDumpRange_Start = 0;
					gsDolbyDbgContext[unWinId].unDumpRange_end = 0;
					gsDolbyDbgContext[unWinId].unDmMetadataDumpEnable= unEnable;
				}
				HDR_PRINT("g_sDbgData.ucwinId = %d g_sDbgData.ucDbgValue = %d g_sDbgData.nMDDumpEnd = %d\n",g_sDbgData.ucwinId,g_sDbgData.ucDbgValue,g_sDbgData.nMDDumpEnd);
				HDR_PRINT("gsDolbyDbgContext[unWinId].unDumpRange_end = %d\n",gsDolbyDbgContext[unWinId].unDumpRange_end);
			}
			break;
			case 0x05:
			{
				unEnable= g_sDbgData.ucDbgValue;
				if(unEnable){
					gsDolbyDbgContext[unWinId].unDumpRange_Start = 0;
					gsDolbyDbgContext[unWinId].unDumpRange_end = g_sDbgData.nMDDumpEnd;
				}
				gsDolbyDbgContext[unWinId].unLUTDumpEnable= unEnable;
				if(g_sDolbyWndContext[unWinId].eCntlPath == E_HDMI_LL_CNTRL_PATH){
					sem_post(g_sDolbyWndContext[unWinId].mdSyncSem);
					HDR_PRINT("UnLocked OTT/LL Mutex %s\n",__func__);
				}
				HDR_PRINT("g_sDbgData.ucwinId = %d g_sDbgData.ucDbgValue = %d g_sDbgData.nMDDumpEnd = %d\n",g_sDbgData.ucwinId,g_sDbgData.ucDbgValue,g_sDbgData.nMDDumpEnd);
				HDR_PRINT("gsDolbyDbgContext[unWinId].unDumpRange_end = %d gsDolbyDbgContext[unWinId].unDumpRange_Start=%d\n",\
					gsDolbyDbgContext[unWinId].unDumpRange_end,gsDolbyDbgContext[unWinId].unDumpRange_Start);
				HDR_PRINT("ProcessID:: %x ThreadID:: %lx",getpid(),syscall(SYS_gettid));
			}
			break;
			case 0x06:
			{
				pqMode = g_sDbgData.ucDbgValue;
				HDR_PRINT("PQ Mode for Debug Event is pqMode = %d\n",pqMode);
				HDR_PRINT("PQ Mode for Debug Event is pqMode = %d\n",pqMode);
				if(pqMode == 0){
					pqMode = HDR_MOVIE_VIVID;
				}
				else if(pqMode == 1){
					pqMode = HDR_MOVIE_BRIGHT;
				}
				else if(pqMode == 2){
					pqMode = HDR_MOVIE_DARK;
				}
				else if(pqMode == 3){
					pqMode = HDR_MOVIE_STANDARD;
				}
				else if(pqMode == 4){
					pqMode = HDR_MOVIE_GAME;
				}
				else {
					pqMode = HDR_MOVIE_VIVID;
					HDR_PRINT("pq AFter updating pqMode = %d\n",pqMode);
				}

				HDR_PRINT("Mode Change from %d to %d\n",g_sUICommands[unWinId].pqmode,pqMode);
				KADP_HDR_SetPQMode(unWinId,(E_HDR_MOVIE_MODES)pqMode);
				/*call submode change as they will be called in pair always*/
				KADP_HDR_SetPQSubMode(unWinId,HDR_PQ_BACKLIGHT,g_sUICommands[unWinId].pqsubmode[HDR_PQ_BACKLIGHT]);
			}
			break;
			case 0x07:
			{
				pqSubMode = g_sDbgData.ucDbgValue;
				unVal = g_sDbgData.ucMode;
				if(pqSubMode == 0){
					pqSubMode = HDR_PQ_BACKLIGHT;
					HDR_PRINT("\nBacklight value Change from %d to %d\n",g_sUICommands[unWinId].pqsubmode[HDR_PQ_BACKLIGHT],unVal);
				}
				else if(pqSubMode == 1){
					pqSubMode = HDR_PQ_BRIGHTNESS;
					HDR_PRINT("Brightness value Change from %d to %d\n",g_sUICommands[unWinId].pqsubmode[HDR_PQ_BRIGHTNESS],unVal);
				}
				else if(pqSubMode == 2){
					pqSubMode = HDR_PQ_CONTRAST;
					HDR_PRINT("Contrast value Change from %d to %d\n",g_sUICommands[unWinId].pqsubmode[HDR_PQ_CONTRAST],unVal);
				}

				KADP_HDR_SetPQSubMode(unWinId,(E_HDR_PQ_SUB_PARAM)pqSubMode,unVal);
			}
			break;
			case 0x08:
			{
				int i_data_index;
				pqMode =  g_sDbgData.ucDbgValue;
				if(pqMode == 0){
					pqMode = HDR_MOVIE_DARK;
					HDR_PRINT("\nDumping data of dark mode\n");
				}
				else if(pqMode == 1){
					pqMode = HDR_MOVIE_BRIGHT;
					HDR_PRINT("Dumping data of bright mode\n");
				}
				else if(pqMode == 2){
					pqMode = HDR_MOVIE_VIVID;
					HDR_PRINT("Dumping data of vivid mode\n");
				}
				else if(pqMode == 3){
					pqMode = HDR_MOVIE_STANDARD;
					HDR_PRINT("Dumping data of standard mode\n");
				}
				else if(pqMode == 4){
					pqMode = HDR_MOVIE_GAME;
					HDR_PRINT("Dumping data of game mode\n");
				}else{
					HDR_PRINT("Invalid mode. \n");
					break;
				}
				if(gsDolbyPQConfig.sptrPQConfigurations == NULL){
					HDR_PRINT("Config file not set yet\n");
					break;
				}
				for(i_data_index = 0;i_data_index<sizeof(pq_config_t);i_data_index++){
					if(((i_data_index%10)==0)&&(i_data_index!=0)){
						HDR_PRINT("\n");
					}
					HDR_PRINT("0x%02x,",*((UINT8 *)gsDolbyPQConfig.sptrPQConfigurations+(pqMode*sizeof(pq_config_t))+i_data_index));
				}
			}break;
			case 0x09:
			{
				HDR_PRINT("Thread Running Status [%d]\n",g_sDolbyWndContext[unWinId].bRunning);
				if(g_sDolbyWndContext[unWinId].bRunning){
					HDR_PRINT("Current Running Mode [0-OTT,1-RF, 2-HDMI, 3-HDMI-LL] %d\n",g_sDolbyWndContext[unWinId].eCntlPath);
				}
				if(g_sDolbyWndContext[unWinId].eCntlPath == E_OTT_CNTRL_PATH){
					HDR_PRINT("OTT Profile ID = %d\n",gsDolbyDbgContext[unWinId].unDolbyProfId);
				}
				if(gsDolbyPQConfig.sptrPQConfigurations){
					if(gsDolbyPQConfig.cptrDolbyConfigFile){
						HDR_PRINT("PQ Configurations is set and loaded from the file %s\n",gsDolbyPQConfig.cptrDolbyConfigFile);
					}else{
						HDR_PRINT("Default PQ Configuration is loaded.\n");
					}
				}else{
					HDR_PRINT("PQ Configuration is not loaded.\n");
				}
				switch(g_sUICommands[unWinId].pqmode)
				{
					case HDR_MOVIE_DARK:
					{
						HDR_PRINT("Current PQ mode is DARK\n");
					}break;
					case HDR_MOVIE_BRIGHT:
					{
						HDR_PRINT("Current PQ mode is BRIGHT\n");
					}break;
					case HDR_MOVIE_VIVID:
					{
						HDR_PRINT("Current PQ mode is VIVID\n");
					}break;
					case HDR_MOVIE_STANDARD:
					{
						HDR_PRINT("Current PQ mode is STANDARD\n");
					}break;
					case HDR_MOVIE_GAME:
					{
						HDR_PRINT("Current PQ mode is GAME\n");
					}break;
					default:
					{
						HDR_PRINT("Current PQ mode is INVALID\n");
					}break;
				}

				HDR_PRINT("Backlight = %d, Brightness = %d, Contrast = %d\n",g_sUICommands[unWinId].pqsubmode[HDR_PQ_BACKLIGHT],\
					g_sUICommands[unWinId].pqsubmode[HDR_PQ_BRIGHTNESS],g_sUICommands[unWinId].pqsubmode[HDR_PQ_CONTRAST]);
				HDR_PRINT("Resolution = %dx%d\n",g_sDolbyWndContext[unWinId].unFrameWidth,g_sDolbyWndContext[unWinId].unFrameHeight);
				HDR_PRINT("Algorithm thread  ID [%x]\n",gsDolbyDbgContext[unWinId].unThreadID);
				HDR_PRINT("Algorithm Process ID [%x]\n",gsDolbyDbgContext[unWinId].unProcessID);
				HDR_PRINT("HDMI descrmabler method [0-SW/1-HW/2-None] [0%x]\n",gsDolbyDbgContext[unWinId].eHdmiDescMethod);
#ifdef HDR_HW2_SUPPORT
				HDR_PRINT("Current DM algorithm is %d[-1=DM_ALGO_INVALID,0=DM_ALGO_DM29,1=DM_ALGO_DM31,2=DM_ALGO_DM4]\n",egDMAlgo);
#endif
				//HDR_PRINT("Shared MAX METADATA Count is %d\n",MAX_METADATA_COUNT);
				HDR_PRINT("Dolby Event Monitor Thread Status [%d]\n",gs_dolby_event.threadenable);
				HDR_PRINT("MAin CONFIG FILE path  is %s\n",g_mainCfg);
				if(g_bestCfg[0] == '\0'){
					HDR_PRINT("Best CONFIG FILE is not available\n");
				}else{
					HDR_PRINT("Best CONFIG FILE path is %s\n",g_bestCfg);
				}
				HDR_PRINT("Dolby KADP Software Major Number: %d\n",KADP_DOVI_SW_MAJOR_NUM);
				HDR_PRINT("Dolby KADP Software Minor  Number : %d\n",KADP_DOVI_SW_MINOR_NUM);
			}break;
			case 10:
			{
				if(gsDolbyPQConfig.sptrPQConfigurations == NULL){
					HDR_PRINT("Config file not set yet\n");
					break;
				}
				displayTargetConfig(&gsDolbyPQConfig.sptrPQConfigurations[g_sUICommands[unWinId].pqmode]);
			}break;
			case 11:
			{
				if(gsDolbyPQConfig.cptrDolbyConfigFile == NULL){
					HDR_PRINT("Config file not set yet\n");
					if(gsDolbyPQConfig.unConfigCnt>0){
						HDR_PRINT("Default config is loaded. Total Configurations: %d\n",gsDolbyPQConfig.unConfigCnt);
					}
					break;
				}
				else {
					HDR_PRINT("\nPQ Configuration setup:\n");
					HDR_PRINT("Configuration File  : %s\n",gsDolbyPQConfig.cptrDolbyConfigFile);
					HDR_PRINT("Total Configurations: %d\n",gsDolbyPQConfig.unConfigCnt);
				}
			}break;
			case 12:
			{
				
				gsDolbyDbgContext[unWinId].unPDControlFlag = g_sDbgData.ucDbgValue;
				gsDolbyDbgContext[unWinId].unPDValue = g_sDbgData.ucMode;
				HDR_PRINT("Dolby:: PD Enable [%d] and Value [%d]\n",g_sDbgData.ucDbgValue,g_sDbgData.ucMode);
			}
			break;
			case 13:
			{
				UINT32 gdEnable;
				gdEnable = g_sDbgData.ucDbgValue;
				pqMode = g_sDbgData.ucMode;
#ifdef HDR_HW5_SUPPORT
				if(pqMode < 5) {
					gsDolbyPQConfig.sptrPQConfigurations[pqMode].target_display_config.gdConfig.globalDimming = gdEnable;
					HDR_PRINT("Global Dimming Enable = %d\n",gsDolbyPQConfig.sptrPQConfigurations[pqMode].target_display_config.gdConfig.globalDimming);
				}
				else {
					int i;
					for(i = 0 ; i < 5 ; i++) {
						gsDolbyPQConfig.sptrPQConfigurations[i].target_display_config.gdConfig.globalDimming = gdEnable;
						HDR_PRINT("Global Dimming Enable = %d\n",gsDolbyPQConfig.sptrPQConfigurations[i].target_display_config.gdConfig.globalDimming);
					}
				}
#else

				if(pqMode < 5) {
					gsDolbyPQConfig.sptrPQConfigurations[pqMode].target_display_config.gdConfig.gdEnable = gdEnable;
					HDR_PRINT("Global Dimming Enable = %d\n",gsDolbyPQConfig.sptrPQConfigurations[pqMode].target_display_config.gdConfig.gdEnable);
				}
				else {
					int i;
					for(i = 0 ; i < 5 ; i++) {
						gsDolbyPQConfig.sptrPQConfigurations[i].target_display_config.gdConfig.gdEnable = gdEnable;
						HDR_PRINT("Global Dimming Enable = %d\n",gsDolbyPQConfig.sptrPQConfigurations[i].target_display_config.gdConfig.gdEnable);
					}
				}
#endif
			}
			break;
			case 14:
			{
				unEnable = g_sDbgData.ucDbgValue;
				if(unEnable){
					gsDolbyDbgContext[unWinId].unDumpRange_Start = 0;
					gsDolbyDbgContext[unWinId].unDumpRange_end= g_sDbgData.nMDDumpEnd;
					gsDolbyDbgContext[unWinId].unDumpDone = 0;
				}
				gsDolbyDbgContext[0].unRegDumpEnable = unEnable;
				if(g_sDolbyWndContext[unWinId].eCntlPath == E_HDMI_LL_CNTRL_PATH){
					sem_post(g_sDolbyWndContext[unWinId].mdSyncSem);
					HDR_PRINT("UnLocked OTT/LL Mutex %s\n",__func__);
				}
				HDR_PRINT("g_sDbgData.ucwinId = %d g_sDbgData.ucDbgValue = %d g_sDbgData.nMDDumpEnd = %d\n",g_sDbgData.ucwinId,g_sDbgData.ucDbgValue,g_sDbgData.nMDDumpEnd);
				HDR_PRINT("gsDolbyDbgContext[unWinId].unDumpRange_end = %d gsDolbyDbgContext[unWinId].unDumpRange_Start=%d\n",\
					gsDolbyDbgContext[unWinId].unDumpRange_end,gsDolbyDbgContext[unWinId].unDumpRange_Start);
			}break;
			case 15:
			{
				if(gsDolbyPQConfig.sptrPQConfigurations == NULL){
					HDR_PRINT("Config file not set yet\n");
					break;
				}
				strcpy((void *)cDumpFilePath,"/tmp/pq_bin_out.bin");
				if(strcmp(cDumpFilePath,"") == 0){
					HDR_PRINT("No Config File Specified!!! \nDumping to default path /tmp/pq_bin_out.bin\n");
					strcpy(cDumpFilePath,"/tmp/pq_bin_out.bin");
				}
#ifdef HDR_HW5_SUPPORT
				write_config_binary_dvp(gsDolbyPQConfig.sptrPQConfigurations ,cDumpFilePath, gsDolbyPQConfig.unConfigCnt );
#else
				write_config_binary(gsDolbyPQConfig.sptrPQConfigurations ,cDumpFilePath, gsDolbyPQConfig.unConfigCnt );
#endif
				HDR_PRINT("bin file %s is generated with %d PQ Modes\n",cDumpFilePath,gsDolbyPQConfig.unConfigCnt);

			}
			break;
			case 16:
			{
				UINT32 unDMzeroEnable = g_sDbgData.ucDbgValue;
				UINT32 unCompzeroEnable = g_sDbgData.ucMode;
				gsDolbyDbgContext[unWinId].unTestAllZeroMd_DM = unDMzeroEnable;
				gsDolbyDbgContext[unWinId].unTestAllZeroMd_Comp = unCompzeroEnable;
				HDR_PRINT("gsDolbyDbgContext[unWinId].unTestAllZeroMd_DM = %d gsDolbyDbgContext[unWinId].unTestAllZeroMd_Comp = %d",gsDolbyDbgContext[unWinId].unTestAllZeroMd_DM,\
					gsDolbyDbgContext[unWinId].unTestAllZeroMd_Comp);
			}break;
			case 17:
			{
				char sVersion[128],sVerText[128],sAlgoversion[128];
				int verRet;
				verRet = KADP_HDR_Get_SWVersion(LX_HDR_TYPE_DOLBY,&sVersion[0]);
				HDR_PRINT("ret:%d, SWVersion:%s\n", verRet,  sVersion);
				verRet = KADP_HDR_Get_DMVersion(LX_HDR_TYPE_DOLBY,&sVersion[0],&sVerText[0]);
				HDR_PRINT("ret:%d, DMVersion:%s\n", verRet, sVersion);
				HDR_PRINT("DMVer Text:%s\n",  sVerText);
				verRet = getDoviAlgoVersion(&sAlgoversion[0]);
				HDR_PRINT("Dolby Algorithm Version:%s\n",sAlgoversion);
			}break;
			case 18:
			{
				UINT16 gddelaval;
				HDR_PRINT("DOVI_SWSync_GetGDdelayValue = %d\n",DOVI_FWSync_GetGDdelayValue(unWinId,&gddelaval));
				HDR_PRINT("Current GD Delay value is %hu\n",gddelaval);

			}break;
			case 19:
			{
				HDR_PRINT("%d\n",KADP_VPQ_PRINT_GDDELAYTABLE(&g_sDolbySTDGddelaydb,&g_sDolbyLLGddelaydb));
			}break;
			case 20:
			{
				HDR_PRINT("Setting GD table for OTT and HDMI STD\n");
				HDR_PRINT(" DOVI_SetGDDelayDB() = %d\n", DOVI_SetGDDelayDB(&g_sDolbyDbgSTDGddelaydb,&g_sDolbyDbgLLGddelaydb));

			}break;
			case 21:
			{

				gsDolbyDbgContext[unWinId].gd_rf_adjust = g_sDbgData.nMDDumpEnd;
				HDR_PRINT("The backlight scaler is in signed Q3.12 format. I.e. 16 bit value, 1 sign bit, 3 integer bits, 12 decimal bits.\n\
					For example if the scaler is 2.5 the value is 2.5 * 4096 = 10240. Default 4096 (= 1.0).\n");
			}break;
			case 22:
			{
				UINT32 abEnable;
				abEnable = g_sDbgData.ucDbgValue;
				pqMode = g_sDbgData.ucMode;
#ifdef HDR_HW5_SUPPORT
				if(pqMode < 5) {
					gsDolbyPQConfig.sptrPQConfigurations[pqMode].target_display_config.ambientConfig.ambient = abEnable;
					HDR_PRINT("Adaptive Boost Enable = %d\n",gsDolbyPQConfig.sptrPQConfigurations[pqMode].target_display_config.ambientConfig.ambient);
				}
				else {
					int i;
					for(i = 0 ; i < 5 ; i++) {
						gsDolbyPQConfig.sptrPQConfigurations[i].target_display_config.ambientConfig.ambient = abEnable;
						HDR_PRINT("Adaptive Boost Enable = %d\n",gsDolbyPQConfig.sptrPQConfigurations[i].target_display_config.ambientConfig.ambient);
					}
				}

#else
				if(pqMode < 5) {
					gsDolbyPQConfig.sptrPQConfigurations[pqMode].target_display_config.abConfig.abEnable = abEnable;
					HDR_PRINT("Adaptive Boost Enable = %d\n",gsDolbyPQConfig.sptrPQConfigurations[pqMode].target_display_config.abConfig.abEnable);
				}
				else {
					int i;
					for(i = 0 ; i < 5 ; i++) {
						gsDolbyPQConfig.sptrPQConfigurations[i].target_display_config.abConfig.abEnable = abEnable;
						HDR_PRINT("Adaptive Boost Enable = %d\n",gsDolbyPQConfig.sptrPQConfigurations[i].target_display_config.abConfig.abEnable);
					}
				}
#endif
			}break;
			case 23:
			{
				gsDolbyDbgContext[unWinId].unEnAmbFile = g_sDbgData.ucDbgValue;
				HDR_PRINT("gsDolbyDbgContext[unWinId].unEnAmbFile = %d\n",gsDolbyDbgContext[unWinId].unEnAmbFile);
			}break;
			case 24:
			{
					UINT32 UOnOff;
					UINT32 uRawdata[4] = {0,};
					UINT32 uLux = 0;
					UOnOff = g_sDbgData.ucDbgValue;
					uRawdata[1] = g_sDbgData.nMDDumpEnd;
					if( g_sDbgData.ucDbgValue == 2)
						gtmctlrtfrontlux = g_sDbgData.nMDDumpEnd;
					else
						HDR_PRINT("%d\n",DOVI_SetAmbientLight(unWinId,UOnOff,uLux,&uRawdata[0]));
			}
			break;
			case 25:
			{
					gsDolbyDbgContext[unWinId].unHalAmbDisable =  g_sDbgData.ucDbgValue;
					HDR_PRINT("gsDolbyDbgContext[unWinId].unHalAmbDisable = %d\n",gsDolbyDbgContext[unWinId].unHalAmbDisable);
			}
			break;
			case 26:
			{
				gsDolbyDbgContext[unWinId].unUserAmbientdataEnable = g_sDbgData.ucDbgValue;
				gambientuserval = g_sDbgData.nMDDumpEnd;
				HDR_PRINT("gsDolbyDbgContext[unWinId].unUserAmbientdataEnable = %d,gambientuserval = %d\n",gsDolbyDbgContext[unWinId].unUserAmbientdataEnable,gambientuserval);
				if(gsDolbyDbgContext[unWinId].unUserAmbientdataEnable) {
					HDR_PRINT("%d user value will be used inside algorithm\n",gambientuserval);
				}
				else
					HDR_PRINT("ambinet value will be used form config file\n");
			}
			break;
			case 27:
			{
				KADP_HDR_BitMaskEnableTest();
			}break;
			case 28:
			{
				gsDolbyDbgContext[unWinId].unDebugFlag = g_sDbgData.nMDDumpEnd;
				HDR_PRINT("Global Dimming Debug = %d\n",GLOBAL_DIMMING_DBG_ENABLE);
				HDR_PRINT("Only Pwm value setting debug = %d\n",GLOBAL_DIMMING_PWM_DBG_ENABLE);
				HDR_PRINT("Crop Area Value setting debug = %d\n",CROP_AREA_DBG_ENABLE);
				HDR_PRINT("PTS Checking Debug = %d\n",PTS_RECEIVE_DEBUG);
				HDR_PRINT("FW Thread PTS Receive Err print = %d\n",4444);
				HDR_PRINT("MD Underflow Err print = %d\n",5555);
				HDR_PRINT("OLED Boost Dbg Print = %d\n",OLED_BOOST_DBG_ENABLE);
				HDR_PRINT("Ambient Light Dbg Print = %d\n",AMBIENT_DBG_ENABLE);
				HDR_PRINT("Apply LUT from Files = %d\n",LUT_APPLY_DBG_FROM_FILE);
				HDR_PRINT("Actuall Level 11 MD print enable = %d\n",LEVEL_11_ACMD_PRINT);
				HDR_PRINT("HDMI LUT Index Print Data = %d\n",HDMI_LL_DDR_LUT_INDEX_PRINT);
				HDR_PRINT("Invalid Resultion Debug = %d\n",ERROR_RESOLUTION_PRINT);
				HDR_PRINT("Mute on Debug Print = %d\n",MUTE_ON_DEBUG_PRINT);
				HDR_PRINT("Enable MD parser Event handler PRINT = %d\n",MD_APRSER_EVENT_DEBUG_PRINT);
				HDR_PRINT("RF mode Debug PRINT = %d\n",RF_MD_DEBUG_PRINT);
				HDR_PRINT("RF mode 2086 static MD PRINT = %d\n",RF_2086_MD_DEBUG_PRINT);
				HDR_PRINT("Enable Err RPU packet dump = %d\n",DUMP_RAW_RPU_ERR);
			}break;
			case 29:
			{
				//gsDolbyDbgContext[unWinId].unDebugFlag = g_sDbgData.nMDDumpEnd;
				HDR_PRINT("Level 11 Byte 0 = 0x%x(content_type)\n",g_slevele11_md.content_type);
				HDR_PRINT("Level 11 Byte 1 = 0x%x(desired_white_point)\n",g_slevele11_md.desired_white_point);
				HDR_PRINT("Level 11 Byte 2 = 0x%x(L11_byte2)\n",g_slevele11_md.L11_byte2);
				HDR_PRINT("Level 11 Byte 3 = 0x%x(L11_byte3)\n",g_slevele11_md.L11_byte3);
			}break;
			case 30:
			{
				UINT32 i = 0;
				UINT32 uRawdata[3] = {0,};
				HDR_PRINT("AMBIENT settings for 100 values\n");
				while(i <= 65535) {
					uRawdata[1] = i;
					HDR_PRINT("ambient raw data %d\n",i);
					KADP_HDR_SetDolbyAmbientLight(0,1,10,uRawdata);
					i += 100;
				}
				HDR_PRINT("AMBIENT TEST is Done\n");
			}break;
			case 31:
			{
				unEnable = g_sDbgData.ucDbgValue;
				if(unEnable){
					gsDolbyDbgContext[unWinId].unDumpRange_Start = 0;
					gsDolbyDbgContext[unWinId].unDumpRange_end= g_sDbgData.nMDDumpEnd;
					gsDolbyDbgContext[unWinId].unDumpDone = 0;
				}
				gsDolbyDbgContext[0].unTV5RegDumpEnable = unEnable;
				if(g_sDolbyWndContext[unWinId].eCntlPath == E_HDMI_LL_CNTRL_PATH){
					sem_post(g_sDolbyWndContext[unWinId].mdSyncSem);
					HDR_PRINT("UnLocked OTT/LL Mutex %s\n",__func__);
				}
				HDR_PRINT("g_sDbgData.ucwinId = %d g_sDbgData.ucDbgValue = %d g_sDbgData.nMDDumpEnd = %d\n",g_sDbgData.ucwinId,g_sDbgData.ucDbgValue,g_sDbgData.nMDDumpEnd);
				HDR_PRINT("gsDolbyDbgContext[unWinId].unDumpRange_end = %d gsDolbyDbgContext[unWinId].unDumpRange_Start=%d\n",\
					gsDolbyDbgContext[unWinId].unDumpRange_end,gsDolbyDbgContext[unWinId].unDumpRange_Start);
			}break;
			case 32:
			{
				unEnable = g_sDbgData.ucDbgValue;
				if(unEnable){
					gsDolbyDbgContext[unWinId].unDumpRange_Start = 0;
					gsDolbyDbgContext[unWinId].unDumpRange_end= g_sDbgData.nMDDumpEnd;
					gsDolbyDbgContext[unWinId].unDumpDone = 0;
				}
				gsDolbyDbgContext[0].unTV5LUTDumpEnable = unEnable;
				if(g_sDolbyWndContext[unWinId].eCntlPath == E_HDMI_LL_CNTRL_PATH){
					sem_post(g_sDolbyWndContext[unWinId].mdSyncSem);
					HDR_PRINT("UnLocked OTT/LL Mutex %s\n",__func__);
				}
				HDR_PRINT("g_sDbgData.ucwinId = %d g_sDbgData.ucDbgValue = %d g_sDbgData.nMDDumpEnd = %d\n",g_sDbgData.ucwinId,g_sDbgData.ucDbgValue,g_sDbgData.nMDDumpEnd);
				HDR_PRINT("gsDolbyDbgContext[unWinId].unDumpRange_end = %d gsDolbyDbgContext[unWinId].unDumpRange_Start=%d\n",\
					gsDolbyDbgContext[unWinId].unDumpRange_end,gsDolbyDbgContext[unWinId].unDumpRange_Start);
			}break;
			case 33:
			{
				unEnable = g_sDbgData.ucDbgValue;
				unVal = g_sDbgData.ucMode;
				gsDolbyDbgContext[unWinId].unDDFlag = unEnable;
				if(unEnable){
					gsDolbyDbgContext[unWinId].unDDValue = unVal;
				}
				HDR_PRINT("Dolby:: Dark Detail Enable [%d] \n",gsDolbyDbgContext[unWinId].unDDValue);
			}
			break;
			case 34:
			{
				unVal = g_sDbgData.nMDDumpEnd;
				unEnable = g_sDbgData.ucDbgValue;
				gsDolbyDbgContext[unWinId].unDarkDetailCompLumEnable = unEnable;
				if(unEnable)
					gsDolbyDbgContext[unWinId].unDarkDetailCompLum = unVal;
				else
					gsDolbyDbgContext[unWinId].unDarkDetailCompLum = 0;
				HDR_PRINT("gsDolbyDbgContext[unWinID].unDarkDetailCompLum = %d\n",gsDolbyDbgContext[unWinId].unDarkDetailCompLum);
			}break;
			case 40 :
			{
				unVal = g_sDbgData.nMDDumpEnd; 
				unEnable = g_sDbgData.ucDbgValue;
				gsDolbyDbgContext[unWinId].unAlgoCntEnable = unEnable;
				if(unEnable){
					gsDolbyDbgContext[unWinId].unAlgoCnt = unVal;
				}
				else
				{
					gsDolbyDbgContext[unWinId].unAlgoCnt = 0;
				}
				HDR_PRINT("gsDolbyDbgContext[unWinID].unAlgoCnt = %d\n",gsDolbyDbgContext[unWinId].unAlgoCnt);
			}break;
			default:
			{
				HDR_PRINT("Unsupported!!\n");
			}
		}
	}
