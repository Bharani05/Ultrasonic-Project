/*******************************************************************************
 * SIC R&D LAB, LG ELECTRONICS INC., SEOUL, KOREA 
 * COPYRIGHT(c) 2011,2012 by LG Electronics Inc.
 * 
 * All rights reserved. No part of this work covered by this copyright hereon
 * may be reproduced, stored in a retrieval system, in any form
 * or by any means, electronic, mechanical, photocopying, recording 
 * or otherwise, without the prior written  permission of LG Electronics.
 ******************************************************************************/

/** @file 
 *
 *  Brief description. 
 *  Detailed description starts here. 
 *
 *  @author		wonsik.do
 *  @version	1.0 
 *  @date		2012-05-07
 *  @note		Additional information. 
 */

/*------------------------------------------------------------------------------
  Control Constants
  ------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  File Inclusions
  ------------------------------------------------------------------------------*/
#include "osa_kadp.h"
#include "debug_kadp.h"
#include "afe_kapi.h"
#include "afe_kadp.h"
#include "demod_kadp_analog.h"
/*------------------------------------------------------------------------------
  Constant Definitions
  ------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  Macro Definitions
  ------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  Type Definitions
  ------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  External Function Prototype Declarations
  ------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  External Variables
  ------------------------------------------------------------------------------*/
extern int g_afe_logm_fd;

extern LX_AFE_CVD_BYPASS_CONTROL_T	_g_cvd_bypass_control_t;

extern UINT32 	g_CVD_status_to_check_ATV_Search;

extern int g_afe_kadp_revision;
/*------------------------------------------------------------------------------
  global Variables
  ------------------------------------------------------------------------------*/
BOOLEAN _gEnablePeriodicCVDPrint = FALSE;
BOOLEAN _gEnableCVDPESetting = TRUE;

/*------------------------------------------------------------------------------
  Static Function Prototypes Declarations
  ------------------------------------------------------------------------------*/
static void AFE_DebugPrintCtrl(void);

static void KADP_AFE_ADC_A_Die_Read(int param_valid, UINT32 param_addr);
static void KADP_AFE_ADC_A_Die_Write(int param_valid, UINT32 param_addr);
static int KADP_AFE_ADC_Input_Address(UINT32 * input_addr);
static int KADP_AFE_CVD_Input_Address(UINT32 * input_addr);
static void KADP_AFE_CVD_Read(int param_valid, UINT32 param_addr);
static void KADP_AFE_CVD_Register_Dump(int param_valid, UINT32 param_addr);
static void KADP_AFE_CVD_Write(int param_valid, UINT32 param_addr);
static void	KADP_AFE_ADC_A_Die_PDB_Test(void);
static void KADP_AFE_ADC_A_Die_Register_Dump(int param_valid, UINT32 param_addr);
static void KADP_AFE_ADC_A_Die_Register_Dump_Number(int param_valid, UINT32 param_addr, int byte_number);
/*------------------------------------------------------------------------------
  Static Variables
  ------------------------------------------------------------------------------*/

/*========================================================================================
  Implementation Group
  ========================================================================================*/
static	char* KADP_AFE_DBG_GetString( char *buf, int n, FILE *fp)
{
	int		len;
	char* 	ret;

    if ((buf == NULL) || (n < 1)) return NULL;

	ret = fgets( buf, n-1, fp );

	if ( ret && (len=strlen(buf) ) )
	{
		if ( buf[len-1] == '\n' ) buf[len-1] = 0x0;
	}

	return ret;
}

int KADP_AFE_GetHexInput(const char* comment, UINT32* pValue)
{
	/* User Hex Input check */
	char   inputStr[20]= "0x";

    KADP_PRINTF("%s: 0x", comment);

	(void)KADP_AFE_DBG_GetString(&inputStr[2], 17, stdin);

	if ( (inputStr[2] < '0') || ( (inputStr[2] > '9') &&  (inputStr[2] < 'a')) || (inputStr[2] > 'f') )
	{
		KADP_PRINTF("!!! First char is Not valid hex data\n");
		return -1;
	}

	*pValue = strtoul( inputStr, (char **) NULL, 16 );
	errno = 0;

	if(errno)
	{
		KADP_PRINTF("[AFE Debug] Error in Hexa data [%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	return 0;
}

int KADP_AFE_GetDecimalInput(const char* comment, UINT32* pValue)
{
	/* User Hex Input check */
	char   inputStr[20];

    KADP_PRINTF("%s: ", comment);

	(void)KADP_AFE_DBG_GetString(&inputStr[0], 17, stdin);

	if ( (inputStr[0] < '0') || (inputStr[0] > '9') )
	{
		KADP_PRINTF("!!! First char is Not valid decimal data\n");
		return -1;
	}

	*pValue = strtoul( inputStr, (char **) NULL, 10 );
	errno = 0;

	if(errno)
	{
		KADP_PRINTF("[AFE Debug] Error in decimal data [%d][%s]\n", errno, strerror(errno));
		return -1;
	}

	return 0;
}
/**
 * ADC TestMenu
 *
 * @param void
 * @return void
 */
UINT32 KADP_AFE_ADC_TestMenu(void)
{
	UINT32 nItem;

	KADP_PRINTF("\n	-------------------------\n");
	KADP_PRINTF("	|        ADC    MENU             |\n");
	KADP_PRINTF("	-------------------------\n");
	KADP_PRINTF("	[0x01] ADC_Init \n");
	KADP_PRINTF("	[0x02] ADC Select Source \n");
	KADP_PRINTF("	[0x03] Print ADC Timing Info \n");
	//	KADP_PRINTF("	[0x04] PC Auto Adjust \n");
	KADP_PRINTF("	[0x05] Auto Calibration \n");
	KADP_PRINTF("	[0x06] Print ADC Gain \n");
	KADP_PRINTF("	[0x07] Print ADC Offset \n");
	KADP_PRINTF("	[0x08] Set ADC Gain \n");
	KADP_PRINTF("	[0x09] Set ADC Offset \n");
	KADP_PRINTF("	[0x0A] Set ADC LLPLL \n");
	KADP_PRINTF("	[0x0B] Reset ADC digital\n");
	KADP_PRINTF("	[0x0C] Reset ADC LLPLL \n");
	KADP_PRINTF("	[0x0D] ADC Channel Power Control \n");
	KADP_PRINTF("	[0x0E] Adjust ADC Phase(Up/Down Control) \n");
	KADP_PRINTF("	[0x0F] Set ADC Phase Value\n");
	{
		//	KADP_PRINTF("	[0x10] CVD/DE Register Read \n");
		//	KADP_PRINTF("	[0x11] CVD/DE Register Write \n");
		KADP_PRINTF("	[0x12] Mixed IP Register Read \n");
		KADP_PRINTF("	[0x13] Mixed IP Register Write \n");
	}
	KADP_PRINTF("	[0x1F] Reset Gain/Offset Register \n");
	KADP_PRINTF("	[0x20] ADC R/G/B Center Sum \n");
	KADP_PRINTF("	[0x21] ADC R/G/B Vertical 8 Colorbar Sum \n");
	KADP_PRINTF("	[0x22] ADC R/G/B Horizontal 8 Colorbar Sum \n");
	KADP_PRINTF("	[0x23] ADC R/G/B Vertical 10 Colorbar Sum \n");
	KADP_PRINTF("	[0x24] ADC R/G/B Pixel Value \n");
	KADP_PRINTF("	[0x30] ADC Enable/Disable Periodic Signal Info Read \n");
	KADP_PRINTF("	[0x31] ADC Execute Format Detection \n");
	KADP_PRINTF("	[0x32] ADC Enable/Disable Component Auto Phase \n");
	//	KADP_PRINTF("	[0x33] ADC Set Calibration Target Value \n");
	KADP_PRINTF("	[0x34] Check Component PSP Status \n");
	KADP_PRINTF("	[0x35] Check Component VBI CP Status \n");
	KADP_PRINTF("	[0x40] A-Die LVDS Source Control \n");
	KADP_PRINTF("	[0x41] SYS Register Read \n");
	KADP_PRINTF("	[0x42] SYS Register Write \n");
	KADP_PRINTF("	[0x43] SYS Register Re-Read \n");
	KADP_PRINTF("	[0x44] SYS Register Re-Write \n");
	KADP_PRINTF("	[0x45] CTOP BMC Register Read (M16P3/O18)\n");
	KADP_PRINTF("	[0x46] CTOP BMC Register Write (M16P3/O18)\n");
	KADP_PRINTF("	[0x50] Component Auto Phase Adjust Test \n");
	KADP_PRINTF("	[0x51] Execute Auto Phase Adjust \n");
	KADP_PRINTF("	[0x52] Enable/Disable ADC HTotal Diff Workaround \n");
	KADP_PRINTF("	[0x53] Enable/Disable Sync Low Level Workaround \n");
	KADP_PRINTF("	[0x60] Reset ADC digital 24MHZ \n");
	KADP_PRINTF("	[0x61] A-Die Register Dump \n");
	KADP_PRINTF("	[0x62] ADC Driver Revision \n");
	KADP_PRINTF("	[0x63] ADC Test Debug \n");
	KADP_PRINTF("	[0x70] ADC Noise Quality Check \n");
	KADP_PRINTF("	[0x71] ADC Noise Quality Check(repeat mode)\n");
	KADP_PRINTF("	[0xa0] A-DIE PDB Test \n");
	KADP_PRINTF("	[0xec] ADC IRE Test\n");
	KADP_PRINTF("	[0xed] ADC OTP Calibration Test\n");
	KADP_PRINTF("	[0xee] AFE KDRV DebugPrintCtrl\n");
	KADP_PRINTF("	[0xef] AFE KADP DebugPrintCtrl\n");
	KADP_PRINTF("	[0xfb] ADC Fake Init\n");
	KADP_PRINTF("	[0xff] Exit...\n");
	KADP_PRINTF("	-------------------------\n");
//	KADP_DBG_GetHexInput("	Select Menu", &nItem);

	return nItem;
}
/**
 * ADC DEBUG TEST
 *
 * @param void
 * @return void
 */
void	KADP_AFE_ADC_DEBUG_Test (void)
{
	UINT32				nTest;
	//VIDEO_DDI_ADJ_PC_T adjresult;
	LX_AFE_ADC_TIMING_INFO_T adc_timing_info = {0,};
	LX_AFE_ADC_GAIN_VALUE_T	adc_gain_data;
	LX_AFE_ADC_OFFSET_VALUE_T adc_offset_data;
	LX_AFE_ADC_TEST_PARAM_T adc_test_param_t;
	UINT32 				r_addr = 0;
	UINT32				w_addr = 0;

	KADP_AFE_ADC_Fake_Init();

	KADP_AFE_ADC_TestMenu();
	do {
		KADP_DBG_GetHexInput("\n### Select ADC Debug Menu", &nTest);

		if ((nTest == 0) || (nTest > 0xff) )
		{
			KADP_AFE_ADC_TestMenu();
			continue;
		}

		UINT32	input;
		UINT32	select;
		UINT32 input1, input2, input3, input4;
		UINT32 				rData, wData;
		int x_pos_pixel, y_pos_pixel, ret_value;
		char		arrow[255] = {0,};

		switch (nTest)
		{
			case 0x01:
				KADP_AFE_ADC_Init();
				KADP_PRINTF("[ADC Debug] ADC Initialized \n");
				break;

			case 0x02:
				KADP_DBG_GetDecimalInput("[ADC Debug]Select RGB_PC, RGB_SCART or YPbPr[0:RGB_PC, 1:RGB_SCART 2:YPbPr] = ", &input);

				if(input == 0)
					input = LX_ADC_INPUT_SOURCE_RGB_PC;
				else if(input == 1)
					input = LX_ADC_INPUT_SOURCE_RGB_SCART;
				else if(input == 2)
					input = LX_ADC_INPUT_SOURCE_YPBPR;
				else
					break;

				KADP_DBG_GetDecimalInput("[ADC Debug]Select Port Number[0:RGB, 1:YPbPr1, 2:YPbPr2] = ", &select);

				if(select == 0)
					select = LX_ADC_RGB_IN;
				else if(select == 1)
					select = LX_ADC_YPbPr_IN1;
				else if(select == 2)
					select = LX_ADC_YPbPr_IN2;
				else
					break;

				KADP_AFE_ADC_Set_Source_Type(input, select);
				break;

			case 0x03 :
				KADP_AFE_ADC_Get_Timing_Info(&adc_timing_info);
				KADP_PRINTF("[ADC Debug] ADC Timing Info \n");

				KADP_PRINTF("[ADC Debug]  Table Index= %d \n", adc_timing_info.u8_Table_Idx);
				KADP_PRINTF("[ADC Debug]  hFreq= %d (* 100Hz) \n", adc_timing_info.u16_HFreq);
				KADP_PRINTF("[ADC Debug]  vFreq= %d (* 0.1Hz) \n", adc_timing_info.u16_VFreq);
				KADP_PRINTF("[ADC Debug]  hTotal= %d \n", adc_timing_info.u16_HTotal);
				KADP_PRINTF("[ADC Debug]  vTotal= %d \n", adc_timing_info.u16_VTotal);
				KADP_PRINTF("[ADC Debug]  hStart= %d \n", adc_timing_info.u16_HStart);
				KADP_PRINTF("[ADC Debug]  vStart= %d \n", adc_timing_info.u16_VStart);
				KADP_PRINTF("[ADC Debug]  hActive= %d \n", adc_timing_info.u16_HActive);
				KADP_PRINTF("[ADC Debug]  vActive= %d \n", adc_timing_info.u16_VActive);
				KADP_PRINTF("[ADC Debug]  scanType= %d (0:interlace, 1: progressive)\n", adc_timing_info.u8_ScanType);
				KADP_PRINTF("[ADC Debug]  LLPLL phase shift = %d \n", adc_timing_info.llpll_phase_shift);
				KADP_PRINTF("[ADC Debug]  phase= %d \n", adc_timing_info.u16_Phase);
				KADP_PRINTF("[ADC Debug]  Sync Exist= %d \n", adc_timing_info.Sync_Exist);
				KADP_PRINTF("[ADC Debug]  Current HFreq = %d \n", adc_timing_info.u16_Cur_HFreq);
				KADP_PRINTF("[ADC Debug]  Current VFreq = %d \n", adc_timing_info.u16_Cur_VFreq);
				KADP_PRINTF("[ADC Debug]  Current VTotal = %d \n", adc_timing_info.u16_Cur_VTotal);
				KADP_PRINTF("[ADC Debug]  Prev HFreq = %d \n", adc_timing_info.u16_Prev_HFreq);
				KADP_PRINTF("[ADC Debug]  Prev VFreq = %d \n", adc_timing_info.u16_Prev_VFreq);
				KADP_PRINTF("[ADC Debug]  Prev VTotal = %d \n", adc_timing_info.u16_Prev_VTotal);
				KADP_PRINTF("[ADC Debug]  LLPLL Status = %d \n", adc_timing_info.llpll_status);
				KADP_PRINTF("[ADC Debug]  vs_width = %d \n", adc_timing_info.vs_width);
				KADP_PRINTF("[ADC Debug]  hs_width = %d \n", adc_timing_info.hs_width);
				KADP_PRINTF("[ADC Debug]  selmux = %d \n", adc_timing_info.selmux);
				KADP_PRINTF("[ADC Debug]  ADC_type(0:RGB, 1:SCART_RGB, 2:YPbPr) = %d \n", adc_timing_info.adc_type);
				KADP_PRINTF("[ADC Debug]  Unstable = %d \n", adc_timing_info.Unstable);
				KADP_PRINTF("[ADC Debug]  comp sync level/green level = 0x%x/%d \n", adc_timing_info.comp_sync_level, adc_timing_info.comp_green_level);
				KADP_PRINTF("[ADC Debug]  post g_gain/comp sync level = 0x%x/%d \n", adc_timing_info.g_gain_value, adc_timing_info.sync_level);

				KADP_PRINTF("[ADC Debug] End of ADC Timing Info \n");
				break;

			case 0x04 :
				/*
				VIDEO_SOC_AdjustPCAuto(&adjresult);
				KADP_PRINTF("[ADC Debug] PC Auto Adjust Result... \n");
				KADP_PRINTF("[ADC Debug] HDelay = %d \n", adjresult.pcHDelay);
				KADP_PRINTF("[ADC Debug] VDelay = %d \n", adjresult.pcVDelay);
				KADP_PRINTF("[ADC Debug] Clock = %d \n", adjresult.pcClock);
				KADP_PRINTF("[ADC Debug] Phase = %d \n", adjresult.pcPhase);
				KADP_PRINTF("[ADC Debug] End of PC Auto Adjust \n");
				*/
				break;

			case 0x05 :
				KADP_PRINTF("[ADC Debug] ADC AUTO Calibration ... \n");
			//	KADP_DBG_GetDecimalInput("[ADC Debug]Select Internal/External  [1:Internal, 0:External] = ", &binput1);
			//	KADP_DBG_GetDecimalInput("[ADC Debug]Select RGB/YPbPr  [1:RGB, 0:YPbPr] = ", &binput2);
				KADP_DBG_GetDecimalInput("[ADC Debug]Input Target Red Value = ", &input1);
				KADP_DBG_GetDecimalInput("[ADC Debug]Input Target Green Value = ", &input2);
				KADP_DBG_GetDecimalInput("[ADC Debug]Input Target Blue Value = ", &input3);

				ret_value = KADP_AFE_ADC_Calibration(TRUE, TRUE, input1, input2, input3, 0);

				if(ret_value < 0)
					KADP_PRINTF("[ADC Debug] ADC AUTO Calibration FAILED!!!!!!!\n");
				else
					KADP_PRINTF("[ADC Debug] ADC AUTO Calibration Succeded\n");

				break;

			case 0x06 :
				KADP_PRINTF("[ADC Debug] Print ADC Gain \n");
				KADP_AFE_Get_ADC_Gain(&adc_gain_data);
				KADP_PRINTF("[ADC Debug] Red Gain Value = [%d] [0x%x]\n", adc_gain_data.R_Gain_Value, adc_gain_data.R_Gain_Value);
				KADP_PRINTF("[ADC Debug] Green Gain Value = [%d] [0x%x]\n", adc_gain_data.G_Gain_Value, adc_gain_data.G_Gain_Value);
				KADP_PRINTF("[ADC Debug] Blue Gain Value = [%d] [0x%x]\n", adc_gain_data.B_Gain_Value, adc_gain_data.B_Gain_Value);

				break;

			case 0x07 :
				KADP_PRINTF("[ADC Debug] Print ADC Offset \n");
				KADP_AFE_Get_ADC_Offset(&adc_offset_data);
				KADP_PRINTF("[ADC Debug] Red Offset Value = [%d] [0x%x]\n", adc_offset_data.R_Offset_Value, adc_offset_data.R_Offset_Value);
				KADP_PRINTF("[ADC Debug] Green Offset Value = [%d] [0x%x]\n", adc_offset_data.G_Offset_Value, adc_offset_data.G_Offset_Value);
				KADP_PRINTF("[ADC Debug] Blue Offset Value = [%d] [0x%x]\n", adc_offset_data.B_Offset_Value, adc_offset_data.B_Offset_Value);

				break;

			case 0x08 :
				KADP_PRINTF("[ADC Debug] Set ADC Gain \n");
				KADP_DBG_GetHexInput("[ADC Debug]Input Red ADC Gain Value = ", &input1);
				KADP_DBG_GetHexInput("[ADC Debug]Input Green ADC Gain Value = ", &input2);
				KADP_DBG_GetHexInput("[ADC Debug]Input Blue ADC Gain Value = ", &input3);
				KADP_AFE_Set_ADC_Gain((UINT16)input1, (UINT16)input2, (UINT16)input3);
				KADP_AFE_Get_ADC_Gain(&adc_gain_data);
				KADP_PRINTF("[ADC Debug] Red Gain Value = [%d] [0x%x]\n", adc_gain_data.R_Gain_Value, adc_gain_data.R_Gain_Value);
				KADP_PRINTF("[ADC Debug] Green Gain Value = [%d] [0x%x]\n", adc_gain_data.G_Gain_Value, adc_gain_data.G_Gain_Value);
				KADP_PRINTF("[ADC Debug] Blue Gain Value = [%d] [0x%x]\n", adc_gain_data.B_Gain_Value, adc_gain_data.B_Gain_Value);

				break;

			case 0x09 :
				KADP_PRINTF("[ADC Debug] Set ADC Offset \n");
				KADP_DBG_GetHexInput("[ADC Debug]Input Red ADC Offset Value = ", &input1);
				KADP_DBG_GetHexInput("[ADC Debug]Input Green ADC Offset Value = ", &input2);
				KADP_DBG_GetHexInput("[ADC Debug]Input Blue ADC Offset Value = ", &input3);
				KADP_AFE_Set_ADC_Offset((UINT16)input1, (UINT16)input2, (UINT16)input3);
				KADP_AFE_Get_ADC_Offset(&adc_offset_data);
				KADP_PRINTF("[ADC Debug] Red Offset Value = [%d] [0x%x]\n", adc_offset_data.R_Offset_Value, adc_offset_data.R_Offset_Value);
				KADP_PRINTF("[ADC Debug] Green Offset Value = [%d] [0x%x]\n", adc_offset_data.G_Offset_Value, adc_offset_data.G_Offset_Value);
				KADP_PRINTF("[ADC Debug] Blue Offset Value = [%d] [0x%x]\n", adc_offset_data.B_Offset_Value, adc_offset_data.B_Offset_Value);

				break;

			case 0x0a :
				KADP_PRINTF("[ADC Debug] Set ADC LLPLL \n");
				KADP_DBG_GetDecimalInput("[ADC Debug]Input ADC Index Value = ", &input1);
				KADP_AFE_ADC_SET_LLPLL(input1);
				break;

			case 0xb :
				KADP_AFE_ADC_Reset_Digital();
				break;

			case 0xc :
				KADP_AFE_ADC_Reset_LLPLL();
				break;

			case 0x0d :
				KADP_PRINTF("[ADC Debug] ADC Channel Power Control \n");
				KADP_DBG_GetDecimalInput("[ADC Debug]Input On/Off Value(0:off, 1:on) = ", &input1);
				KADP_AFE_ADC_POWER_CONTROL(input1);

				break;

			case 0x0e :
				KADP_PRINTF("[ADC Debug] Adjust ADC Phase\n");

				adc_test_param_t.item_to_test = ADC_TEST_PHASE_READ;
				KADP_AFE_ADC_Test(&adc_test_param_t);

				KADP_PRINTF("[ADC Debug] Current Phase Value = [0x%x] \n", adc_test_param_t.ADC_Phase_Value);
				KADP_PRINTF(" Phase Down Key : a/s , Phase Up Key = d/w\n");
				KADP_PRINTF(" Exit Key Control : q \n");

//				OSA_SetTermOneShot(SINGLE_KEY_MODE);
				do
				{
					KADP_DBG_ReadCmdString( "Adjust ADC Phase - Input Command :", arrow, 5);

					switch(arrow[0])
					{
						case 'a':
						case 'A':
						case 's':
						case 'S':
							if(adc_test_param_t.ADC_Phase_Value > 0)
								adc_test_param_t.ADC_Phase_Value--;

							adc_test_param_t.item_to_test = ADC_TEST_PHASE_WRITE;
							KADP_AFE_ADC_Test(&adc_test_param_t);

							KADP_PRINTF("[ADC Debug] Set Phase Value = [0x%x] \n", adc_test_param_t.ADC_Phase_Value);
							break;

						case 'd':
						case 'D':
						case 'e':
						case 'E':
							if(adc_test_param_t.ADC_Phase_Value < 1024)
								adc_test_param_t.ADC_Phase_Value++;

							adc_test_param_t.item_to_test = ADC_TEST_PHASE_WRITE;
							KADP_AFE_ADC_Test(&adc_test_param_t);

							KADP_PRINTF("[ADC Debug] Set Phase Value = [0x%x] \n", adc_test_param_t.ADC_Phase_Value);

							break;

						case 'q':
						case 'Q':
							KADP_PRINTF ("Exit Single Key Mode! %c\n", arrow[0]);
							KADP_PRINTF("[ADC Debug] Last Phase Value = [0x%x] \n", adc_test_param_t.ADC_Phase_Value);
							break;

						default :
							break;

					} 
				} while ( (arrow[0]!='q') && (arrow[0]!='Q') );

				break;


			case 0x0f :
				KADP_PRINTF("[ADC Debug] Set ADC Phase Value \n");

				adc_test_param_t.item_to_test = ADC_TEST_PHASE_READ;
				KADP_AFE_ADC_Test(&adc_test_param_t);
				KADP_PRINTF("[ADC Debug] Current Phase Value = [0x%x] \n", adc_test_param_t.ADC_Phase_Value);
				KADP_DBG_GetHexInput("[ADC Debug] Input Phase Value", &input1);
				adc_test_param_t.ADC_Phase_Value = input1;
				adc_test_param_t.item_to_test = ADC_TEST_PHASE_WRITE;
				KADP_AFE_ADC_Test(&adc_test_param_t);
				// tmp code debug only
				/*
				   KADP_AFE_Vport_Reg_Read(0x210, &input1);
				   KADP_PRINTF("[ADC Debug] Current Phase Value = [0x%x] \n", input1);
				   KADP_DBG_GetHexInput("[ADC Debug] Input Phase Value", &input2);
				   KADP_AFE_Vport_Reg_Write(0x210, input2);
				 */
				break;

			case 0x12:
					KADP_AFE_ADC_A_Die_Read(0, 0);
					break;

			case 0x13:
					KADP_AFE_ADC_A_Die_Write(0, 0);
					break;

			case 0x1F:
					KADP_AFE_Reset_ADC_GainOffset();
					break;

			case 0x20:
				KADP_AFE_ADC_Get_Timing_Info(&adc_timing_info);
				KADP_PRINTF("[ADC Debug]  hActive= %d \n", adc_timing_info.u16_HActive);
				KADP_PRINTF("[ADC Debug]  vActive= %d \n", adc_timing_info.u16_VActive);
				KADP_PRINTF("[ADC Debug]  scanType= %d (0:interlace, 1: progressive)\n", adc_timing_info.u8_ScanType);

				if(adc_timing_info.u8_ScanType == 0)
					adc_timing_info.u16_VActive = adc_timing_info.u16_VActive / 2;

				x_pos_pixel = adc_timing_info.u16_HActive /2;
				y_pos_pixel = adc_timing_info.u16_VActive /2;

				KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel, y_pos_pixel, 100, 100, &input1, &input2, &input3);
				KADP_PRINTF("[ADC Debug] Center (100*100) Result [RED:%d 0x%x], [GREEN:%d 0x%x], [BLUE:%d 0x%x]\n", input1, input1, input2, input2, input3, input3);

				break;

			case 0x21:

//				KADP_DBG_GetDecimalInput("[ADC Debug]Input H Offset Value = ", &input);

				KADP_AFE_ADC_Get_Timing_Info(&adc_timing_info);

				KADP_PRINTF("[ADC Debug]  hActive= %d \n", adc_timing_info.u16_HActive);
				KADP_PRINTF("[ADC Debug]  vActive= %d \n", adc_timing_info.u16_VActive);
				KADP_PRINTF("[ADC Debug]  scanType= %d (0:interlace, 1: progressive)\n", adc_timing_info.u8_ScanType);

				if(adc_timing_info.u8_ScanType == 0)
					adc_timing_info.u16_VActive = adc_timing_info.u16_VActive / 2;

				//x_pos_pixel = adc_timing_info.u16_HActive / 32 + adc_timing_info.u16_HStart;
				x_pos_pixel = adc_timing_info.u16_HActive / 32 + adc_timing_info.u16_HStart;
				y_pos_pixel = adc_timing_info.u16_VActive / 2;

				for (;x_pos_pixel < (adc_timing_info.u16_HActive + adc_timing_info.u16_HStart); x_pos_pixel += adc_timing_info.u16_HActive/8)
				{
					KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel, y_pos_pixel, adc_timing_info.u16_HActive/16, 100, &input1, &input2, &input3);
					KADP_PRINTF("[ADC Debug] ColorBar Pos(%d,%d) Result [RED:%d 0x%x], [GREEN:%d 0x%x], [BLUE:%d 0x%x]\n", x_pos_pixel, y_pos_pixel , input1, input1, input2, input2, input3, input3);
				}

				break;

			case 0x22:
				// Modified for MSPG 65 pattern(Horizontal Color Bar)

//				KADP_DBG_GetDecimalInput("[ADC Debug]Input Vertical Offset Value = ", &input);

				KADP_AFE_ADC_Get_Timing_Info(&adc_timing_info);

				KADP_PRINTF("[ADC Debug]  hActive= %d \n", adc_timing_info.u16_HActive);
				KADP_PRINTF("[ADC Debug]  vActive= %d \n", adc_timing_info.u16_VActive);
				KADP_PRINTF("[ADC Debug]  scanType= %d (0:interlace, 1: progressive)\n", adc_timing_info.u8_ScanType);

				if(adc_timing_info.u8_ScanType == 0)
					adc_timing_info.u16_VActive = adc_timing_info.u16_VActive / 2;

				//x_pos_pixel = adc_timing_info.u16_HActive / 32 + adc_timing_info.u16_HStart;
				x_pos_pixel = adc_timing_info.u16_HActive / 2;
				y_pos_pixel = adc_timing_info.u16_VActive / 32 + adc_timing_info.u16_VStart;

				for (;y_pos_pixel < (adc_timing_info.u16_VActive + adc_timing_info.u16_VStart); y_pos_pixel += adc_timing_info.u16_VActive/8)
				{
					KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel, y_pos_pixel, 100, adc_timing_info.u16_VActive/16, &input1, &input2, &input3);
					KADP_PRINTF("[ADC Debug] ColorBar Pos(%d,%d) Result [RED:%d 0x%x], [GREEN:%d 0x%x], [BLUE:%d 0x%x]\n", x_pos_pixel, y_pos_pixel , input1, input1, input2, input2, input3, input3);
				}

				break;

			case 0x23:

//				KADP_DBG_GetDecimalInput("[ADC Debug]Input H Offset Value = ", &input);

				KADP_AFE_ADC_Get_Timing_Info(&adc_timing_info);

				KADP_PRINTF("[ADC Debug]  hActive= %d \n", adc_timing_info.u16_HActive);
				KADP_PRINTF("[ADC Debug]  vActive= %d \n", adc_timing_info.u16_VActive);
				KADP_PRINTF("[ADC Debug]  scanType= %d (0:interlace, 1: progressive)\n", adc_timing_info.u8_ScanType);

				if(adc_timing_info.u8_ScanType == 0)
					adc_timing_info.u16_VActive = adc_timing_info.u16_VActive / 2;

				//x_pos_pixel = adc_timing_info.u16_HActive / 32 + adc_timing_info.u16_HStart;
				x_pos_pixel = adc_timing_info.u16_HActive / 40 + adc_timing_info.u16_HStart;
				y_pos_pixel = adc_timing_info.u16_VActive / 2;

				for (;x_pos_pixel < (adc_timing_info.u16_HActive + adc_timing_info.u16_HStart); x_pos_pixel += adc_timing_info.u16_HActive/10)
				{
					KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel, y_pos_pixel, adc_timing_info.u16_HActive/20, 100, &input1, &input2, &input3);
					KADP_PRINTF("[ADC Debug] ColorBar Pos(%d,%d) Result [RED:%d 0x%x], [GREEN:%d 0x%x], [BLUE:%d 0x%x]\n", x_pos_pixel, y_pos_pixel , input1, input1, input2, input2, input3, input3);
				}

				break;

			case 0x24:

				KADP_DBG_GetDecimalInput("[ADC Debug]Input X position = ", &x_pos_pixel);
				KADP_DBG_GetDecimalInput("[ADC Debug]Input Y position = ", &y_pos_pixel);

				KADP_AFE_ADC_Get_Timing_Info(&adc_timing_info);

				x_pos_pixel += adc_timing_info.u16_HStart;
				y_pos_pixel += adc_timing_info.u16_VStart;

				for(input4 = x_pos_pixel;x_pos_pixel < (input4 + 16) ; x_pos_pixel++)
				{
					KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel, y_pos_pixel, 1, 1, &input1, &input2, &input3);
					KADP_PRINTF("Y[ADC Debug] Pixel Pos(%d,%d) Result : ", x_pos_pixel, y_pos_pixel );
					KADP_PRINTF("R[%3d 0x%3x]", input1, input1);
					KADP_PRINTF("G[%3d 0x%3x]", input2, input2);
					KADP_PRINTF("B[%3d 0x%3x]\n", input3, input3);
				}

				break;

			case 0x30:
				KADP_PRINTF("	[ADC Debug] ADC Enable/Disable Periodic Signal Info Read \n");
				KADP_DBG_GetDecimalInput("[ADC Debug] Input On/Off Value(0:disable, 1:Enable , 2:Disable Fast IFM Mode, 3:Enable Fast IFM Mode) = ", &input1);
				KADP_AFE_ADC_Enable_Periodic_Signal_Info_Read(input1);
				break;

			case 0x31:
				KADP_PRINTF("	[ADC Debug] ADC Execute Format Detection \n");
				KADP_AFE_ADC_Execute_Format_Detection();
				break;

			case 0x32:
				KADP_PRINTF("	[ADC Debug] ADC Enable/Disable Component Auto Phase \n");
				KADP_DBG_GetDecimalInput("[ADC Debug] Input On/Off Value(0:disable, 1:Enable) = ", &input1);
				KADP_AFE_ADC_Enable_Component_Auto_Phase(input1);
				break;

			case 0x33:
				/*
				KADP_PRINTF("	[ADC Debug] ADC Set Calibration Target Value \n");
				KADP_DBG_GetDecimalInput("[ADC Debug] Target R = ", &target_R);
				KADP_DBG_GetDecimalInput("[ADC Debug] Target G = ", &target_G);
				KADP_DBG_GetDecimalInput("[ADC Debug] Target B = ", &target_B);
				KADP_DBG_GetDecimalInput("[ADC Debug] Target Y = ", &target_Y);
				KADP_DBG_GetDecimalInput("[ADC Debug] Target Cb = ", &target_Cb);
				KADP_DBG_GetDecimalInput("[ADC Debug] Target Cr = ", &target_Cr);
				KADP_PRINTF("	[ADC Debug] R[%d],G[%d],B[%d],Y[%d],Cb[%d],Cr[%d] \n", target_R, target_G, target_B, target_Y, target_Cb, target_Cr);
				*/
				break;

			case 0x34:
				{
					UINT32 comp_psp_detected, comp_vline_normal, comp_vline_measured, comp_valid_signal;

					KADP_PRINTF("[ADC Debug] Component PSP Check \n");

					KADP_AFE_ADC_Get_Comp_PSP_Status(&comp_psp_detected, &comp_vline_normal, &comp_vline_measured, &comp_valid_signal);

					KADP_PRINTF("[ADC Debug] PSP Detected ? [%d]\n", comp_psp_detected);
					KADP_PRINTF("[ADC Debug] Vline Normal [%d]\n", comp_vline_normal);
					KADP_PRINTF("[ADC Debug] Vline Measured [%d]\n", comp_vline_measured);
					KADP_PRINTF("[ADC Debug] Valid signal ? [%d]\n", comp_valid_signal);

				}
				break;

			case 0x35:
				{
					LX_AFE_ADC_COMP_VBI_CP_T stComp_VBI_CP_Data;

					KADP_PRINTF("[ADC Debug] Component VBI Copy Protection Data \n");

					KADP_AFE_ADC_Get_Comp_CP_Data(&stComp_VBI_CP_Data);

					KADP_PRINTF("[ADC Debug] CGMS A data (with APS) [CGMS_B0:CGMS_B1:APS_B2:APS_B3] [0x%x]\n", stComp_VBI_CP_Data.cgms_cp_data);
					KADP_PRINTF("[ADC Debug] WSS CGMS data [CGMS_B12:CGMS_B13] [0x%x]\n", stComp_VBI_CP_Data.wss_cp_data);
					KADP_PRINTF("[ADC Debug] CGMS data 0 (even) 		[0x%08x]\n", stComp_VBI_CP_Data.cgms_data0);
					KADP_PRINTF("[ADC Debug] CGMS data 1 (odd) 		[0x%08x]\n", stComp_VBI_CP_Data.cgms_data1);
					KADP_PRINTF("[ADC Debug] WSS data 0 (raw)		[0x%08x]\n", stComp_VBI_CP_Data.wss_data0);
					KADP_PRINTF("[ADC Debug] WSS data 1 (bi-phase) 	[0x%08x]\n", stComp_VBI_CP_Data.wss_data1);
				}
				break;

			case 0x40:
				KADP_PRINTF("	[ADC Debug] A-Die LVDS Source Control \n");
				KADP_DBG_GetDecimalInput("[ADC Debug] Select LVDS (LVDS0 = 0, LVDS1 = 1) = ", &input1);
				KADP_DBG_GetDecimalInput("[ADC Debug] Select PBD Control (Power Down = 0, Up = 1) = ", &input2);
				KADP_DBG_GetDecimalInput("[ADC Debug] Select Type (VESA = 0, JEIDA = 1) = ", &input3);
				KADP_DBG_GetDecimalInput("[ADC Debug] Select Source (HDMI Normal = 0, 3CH ADC= 1, CVD = 2, HDMI_LEFT = 4, HDMI_RIGHT = 5) = ", &input4);
				KADP_DBG_GetDecimalInput("[ADC Debug] Select Scart Mix mode (Normal Mode = 0, RGB Mix Mode= 1) = ", &input);
				KADP_AFE_LVDS_Src_Control(input1, input2, input3, input4, input);
				break;

			case 0x41:
				{
					int		binary[4], number, pow, count, byte;
					do{
						if ( KADP_DBG_GetHexInput("SYS Read Addr", &r_addr) < 0)
							break;
						if( KADP_SYS_RegReadSimple(r_addr, &rData) )
							break;

						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("                         [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", r_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");

					} while(0);
				}
				break;
			case 0x42:
				{
					int		binary[4], number, pow, count, byte;

					do{
						if ( KADP_DBG_GetHexInput("SYS Write Addr", &w_addr) < 0)
							break;

						KADP_SYS_RegReadSimple(w_addr, &rData);

						KADP_PRINTF("SYS Current Data [0x%x] 0x%x. \n",w_addr, rData);
						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("Current                  [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", w_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");

						if ( KADP_DBG_GetHexInput("INPUT SYS Write data ", &wData) < 0)
						{
							KADP_PRINTF("---- Data[0x%x] NOT Written -----\n", wData);
							break;
						}
						if (KADP_SYS_RegWriteSimple(w_addr, wData) )
							break;

						usleep(10000);

						if (KADP_SYS_RegReadSimple(w_addr, &rData) )
							break;

						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("Written !!!              [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", w_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");
					} while(0);
				}
				break;
			case 0x43:
				{
					int		binary[4], number, pow, count, byte;
					do{
						if ( r_addr== 0)
						{
							KADP_PRINTF("Addr is 0 !!!\n");
							break;
						}
						if( KADP_SYS_RegReadSimple(r_addr, &rData) )
							break;

						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("                         [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", r_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");

					} while(0);
				}
				break;
			case 0x44:
				{
					int		binary[4], number, pow, count, byte;

					do{
						if ( w_addr == 0)
						{
							KADP_PRINTF("Addr is 0 !!!\n");
							break;
						}

						KADP_SYS_RegReadSimple(w_addr, &rData);

						KADP_PRINTF("SYS Current Data [0x%x] 0x%x. \n",w_addr, rData);
						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("Current                  [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", w_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");

						if ( KADP_DBG_GetHexInput("INPUT SYS Write data ", &wData) < 0)
						{
							KADP_PRINTF("---- Data[0x%x] NOT Written -----\n", wData);
							break;
						}
						if (KADP_SYS_RegWriteSimple(w_addr, wData) )
							break;

						usleep(10000);

						if (KADP_SYS_RegReadSimple(w_addr, &rData) )
							break;

						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("Written !!!              [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", w_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");
					} while(0);
				}
				break;
			case 0x45:
				{
					int		binary[4], number, pow, count, byte;
					UINT32 bmc_base_addr;
					do{
						if (lx_chip_rev() >= LX_CHIP_REV(O20, A0))
							bmc_base_addr = 0xc34e0800;
						else if (lx_chip_rev() >= LX_CHIP_REV(M19, A0))
							bmc_base_addr = 0xc930e400;
						else if(lx_chip_rev() >= LX_CHIP_REV(O18, A0))
							bmc_base_addr = 0xc7fc0000;
						else
							break;

						KADP_PRINTF("CTOP BMC [base:0x%8x]\n",bmc_base_addr);
						if ( KADP_DBG_GetHexInput("CTOP BMC Read Addr offset :", &r_addr) < 0)
							break;
						if(r_addr > 0xfff)
							break;

						r_addr += bmc_base_addr;

						if( KADP_SYS_RegReadSimple(r_addr, &rData) )
							break;

						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("                         [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", r_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");

					} while(0);
				}
				break;
			case 0x46:
				{
					int		binary[4], number, pow, count, byte;

					UINT32 bmc_base_addr;
					do{
						if (lx_chip_rev() >= LX_CHIP_REV(O20, A0))
							bmc_base_addr = 0xc34e0800;
						else if (lx_chip_rev() >= LX_CHIP_REV(M19, A0))
							bmc_base_addr = 0xc930e400;
						else if(lx_chip_rev() >= LX_CHIP_REV(O18, A0))
							bmc_base_addr = 0xc7fc0000;
						else
							break;

						KADP_PRINTF("CTOP BMC [base:0x%8x]\n",bmc_base_addr);
						if ( KADP_DBG_GetHexInput("CTOP BMC Write Addr offset :", &w_addr) < 0)
							break;
						if(w_addr > 0xfff)
							break;

						w_addr += bmc_base_addr;

						KADP_SYS_RegReadSimple(w_addr, &rData);

						KADP_PRINTF("SYS Current Data [0x%x] 0x%x. \n",w_addr, rData);
						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("Current                  [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", w_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");

						if ( KADP_DBG_GetHexInput("INPUT SYS Write data ", &wData) < 0)
						{
							KADP_PRINTF("---- Data[0x%x] NOT Written -----\n", wData);
							break;
						}
						if (KADP_SYS_RegWriteSimple(w_addr, wData) )
							break;

						usleep(10000);

						if (KADP_SYS_RegReadSimple(w_addr, &rData) )
							break;

						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("Written !!!              [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", w_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");
					} while(0);
				}
				break;
			case 0x50:
				KADP_PRINTF("	[ADC Debug] Component Auto Phase Adjust \n");
				KADP_DBG_GetDecimalInput("[ADC Debug] Input Delay (in msec)(initial:0)  = ", &input1);
				KADP_DBG_GetDecimalInput("[ADC Debug] Input APA Enable/Disable (Disable = 0, Enable = 1)(initial:1) = ", &input2);
				KADP_DBG_GetDecimalInput("[ADC Debug] Input APA Same Count (initial:0) = ", &input3);
//				KADP_DBG_GetDecimalInput("[ADC Debug] Input APA Result Print Enable/Disable (Disable = 0, Enable = 1)(initial:0) = ", &input4);

				adc_test_param_t.item_to_test = ADC_TEST_APA;
				adc_test_param_t.ADC_APA_Delay = input1;
				adc_test_param_t.ADC_APA_Enable = input2;
				adc_test_param_t.ADC_APA_Same_Count = input3;
				adc_test_param_t.ADC_APA_Execute = 0;
//				adc_test_param_t.ADC_APA_Enable_Print = input4;
				adc_test_param_t.ADC_APA_Enable_Print = 1;
				KADP_AFE_ADC_Test(&adc_test_param_t);
				break;

			case 0x51:
				KADP_PRINTF("	[ADC Debug] Execute Component Auto Phase Adjust \n");
				adc_test_param_t.item_to_test = ADC_TEST_APA;
				adc_test_param_t.ADC_APA_Enable = 1;
				adc_test_param_t.ADC_APA_Execute = 1;
				KADP_AFE_ADC_Test(&adc_test_param_t);
				break;

			case 0x52:
				KADP_PRINTF("	[ADC Debug] ADC HTotal Diff Workaround (Default Enabled)\n");
				KADP_DBG_GetDecimalInput("[ADC Debug] Input Enable(1) /Disable(0) = ", &input1);
				adc_test_param_t.item_to_test = ADC_TEST_HTOTAL_DIFF_ENABLE;
				adc_test_param_t.ADC_HTotal_Diff_Enable = input1;
				KADP_AFE_ADC_Test(&adc_test_param_t);
				break;

			case 0x53:
				KADP_PRINTF("	[ADC Debug] Component Sync Low level read (Default Enabled)\n");
				KADP_DBG_GetDecimalInput("[ADC Debug] Input Enable(1) /Disable(0) = ", &input1);
				adc_test_param_t.item_to_test = ADC_TEST_SYNC_LOW_LEVEL_READ;
				adc_test_param_t.ADC_Sync_Low_Level_Read_Enable = input1;
				KADP_AFE_ADC_Test(&adc_test_param_t);
				break;

			case 0x60 :
				KADP_PRINTF("	[ADC Debug] Execute Reset Digital 24MHZ \n");
				KADP_AFE_ADC_Reset_Digital_24MHZ();
				break;

			case 0x61:
				{
					KADP_PRINTF("	[ADC Debug] Dump A-Die Registers \n");
					if (lx_chip_rev() >= LX_CHIP_REV(M16P, A0))
					{
						KADP_AFE_ADC_A_Die_Register_Dump_Number(1, 0x0,0x299);
					}
					else
					{
					}

				}
				break;

			case 0x62:
				{
					KADP_PRINTF("[ADC Debug] ADC Driver Revision \n");
					adc_test_param_t.item_to_test = ADC_TEST_GET_REVISION;
					KADP_AFE_ADC_Test(&adc_test_param_t);
					KADP_PRINTF("[ADC Debug] ADC HW Revision 		[0x%x] \n", adc_test_param_t.adc_hw_revision );
					KADP_PRINTF("[ADC Debug] ADC Module Revision 	[0x%x] \n", adc_test_param_t.adc_module_revision );
					KADP_PRINTF("[ADC Debug] ADC Control Revision 	[0x%x] \n", adc_test_param_t.adc_control_revision );
				}
				break;

			case 0x63:
				{
					KADP_PRINTF("	[ADC Debug] ADC test debug\n");
					adc_test_param_t.item_to_test = ADC_TEST_DEBUG;
					KADP_DBG_GetDecimalInput("[ADC Debug] Input offset = ", &adc_test_param_t.adc_test_debug_1);
					KADP_DBG_GetDecimalInput("[ADC Debug] Input size = ", &adc_test_param_t.adc_test_debug_2);
					KADP_AFE_ADC_Test(&adc_test_param_t);
				}
				break;
				// Pixel Capture for ADC noise quality check.
			case 0x70:
				{
					int width = 400;
					int height = 100;
					UINT32 average_y, value_y, average_cb, value_cb, average_cr, value_cr;
					int max_y = 0;
					int min_y = 1024;
					int max_cb = 0;
					int min_cb = 1024;
					int max_cr = 0;
					int min_cr = 1024;
					UINT32 square_diff_sum = 0;
					UINT32 square_diff_sum_cb = 0;
					UINT32 square_diff_sum_cr = 0;
					int i,j;
					int variance, variance_cr, variance_cb;

					KADP_AFE_ADC_Get_Timing_Info(&adc_timing_info);
					KADP_PRINTF("[ADC Debug]  hActive= %d \n", adc_timing_info.u16_HActive);
					KADP_PRINTF("[ADC Debug]  vActive= %d \n", adc_timing_info.u16_VActive);
					KADP_PRINTF("[ADC Debug]  scanType= %d (0:interlace, 1: progressive)\n", adc_timing_info.u8_ScanType);

					if(adc_timing_info.u8_ScanType == 0)
						adc_timing_info.u16_VActive = adc_timing_info.u16_VActive / 2;

					x_pos_pixel = 300;
					y_pos_pixel = 100;

					/*
					x_pos_pixel = adc_timing_info.u16_HActive /2;
					y_pos_pixel = adc_timing_info.u16_VActive /2;
					*/

					do{
						KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel, y_pos_pixel, width, height, &average_cr, &average_y, &average_cb);
						KADP_PRINTF("[ADC Debug] Center (%d*%d) Average [Y:%d][Cb:%d][Cr:%d]\n" , width, height , average_y, average_cb, average_cr);
					} while ( (average_y > 1024) || (average_cb > 1024) || (average_cr > 1024) );

					for(i= 0; i<height;i++)
					{
						for(j=0;j<width; j++)
						{
							do{
								KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel + j, y_pos_pixel + i, 1, 1, &value_cr, &value_y, &value_cb);

								if ( ( abs(value_cr - average_cr) > 200 ) \
										  || ( abs(value_y - average_y) > 200 ) \
										  || ( abs(value_cb - average_cb) > 200 ) ) 
								{
									KADP_PRINTF("[ADC Debug] Pixel value wrong ??? (%d*%d) value_y[%d] value_cb[%d] value_cr[%d] \n"\
											,j,i , value_y, value_cb, value_cr);
									usleep(1000*100);
									KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel + j, y_pos_pixel + i, 1, 1, &value_cr, &value_y, &value_cb);
									KADP_PRINTF("[ADC Debug] Re_read Value : (%d*%d) value_y[%d] value_cb[%d] value_cr[%d] \n"\
											,j,i , value_y, value_cb, value_cr);
								}
							} while ( (value_y > 1024) || (value_cb > 1024) || (value_cr > 1024) );

							if ( ( abs(value_cr - average_cr) < 200 ) \
									&& ( abs(value_y - average_y) < 200 ) \
									&& ( abs(value_cb - average_cb) < 200 ) )
							{
								if(value_y > max_y)
								{
									max_y = value_y;
									if( (i>0)||(j>0) )
										KADP_PRINTF("[ADC Debug] (%d*%d) min_y[%d],max_y[%d] \n" , j, i , min_y, max_y);
								}
								if(value_y < min_y)
								{
									min_y = value_y;
									if( (i>0)||(j>0) )
										KADP_PRINTF("[ADC Debug] (%d*%d) min_y[%d],max_y[%d] \n" , j, i , min_y, max_y);
								}
								square_diff_sum += ( abs(average_y - value_y) ) * (abs(average_y - value_y) );

								if(value_cb > max_cb)
								{
									max_cb = value_cb;
									if( (i>0)||(j>0) )
										KADP_PRINTF("[ADC Debug] (%d*%d) min_cb[%d],max_cb[%d] \n" , j, i , min_cb, max_cb);
								}
								if(value_cb < min_cb)
								{
									min_cb = value_cb;
									if( (i>0)||(j>0) )
										KADP_PRINTF("[ADC Debug] (%d*%d) min_cb[%d],max_cb[%d] \n" , j, i , min_cb, max_cb);
								}
								square_diff_sum_cb += ( abs(average_cb - value_cb) ) * (abs(average_cb - value_cb) );

								if(value_cr > max_cr)
								{
									max_cr = value_cr;
									if( (i>0)||(j>0) )
										KADP_PRINTF("[ADC Debug] (%d*%d) min_cr[%d],max_cr[%d] \n" , j, i , min_cr, max_cr);
								}
								if(value_cr < min_cr)
								{
									min_cr = value_cr;
									if( (i>0)||(j>0) )
										KADP_PRINTF("[ADC Debug] (%d*%d) min_cr[%d],max_cr[%d] \n" , j, i , min_cr, max_cr);
								}
								square_diff_sum_cr += ( abs(average_cr - value_cr) ) * (abs(average_cr - value_cr) );
							}
							else
								KADP_PRINTF("[ADC Debug] !!!!! NOT USE WRONG DATA !!!!! : (%d*%d) value_y[%d] value_cb[%d] value_cr[%d] \n"\
											,j,i , value_y, value_cb, value_cr);
						}
					}
					variance = square_diff_sum / (height * width);
					variance_cb = square_diff_sum_cb / (height * width);
					variance_cr = square_diff_sum_cr / (height * width);

					KADP_PRINTF("-----------------------------------------------------------------------------------------\n");
					KADP_PRINTF("[Y  Result] MinY[%d],MaxY[%d], Diff[%d],  Variance[%d]\n"\
							, min_y, max_y, max_y-min_y, variance );
					KADP_PRINTF("[Cb Result] MinCb[%d],MaxCr[%d], Diff[%d],  Variance[%d]\n"\
							, min_cb, max_cb, max_cb-min_cb, variance_cb );
					KADP_PRINTF("[Cr Result] MinCr[%d],MaxCr[%d], Diff[%d],  Variance[%d]\n"\
							, min_cr, max_cr, max_cr-min_cr, variance_cr );
					KADP_PRINTF("-----------------------------------------------------------------------------------------\n");

				}
				break;

			case 0x71:
				{
					int width = 400;
					int height = 100;
					UINT32 average_y, value_y, average_cb, value_cb, average_cr, value_cr;
					int max_y = 0;
					int min_y = 1024;
					int max_cb = 0;
					int min_cb = 1024;
					int max_cr = 0;
					int min_cr = 1024;
					UINT32 square_diff_sum = 0;
					UINT32 square_diff_sum_cb = 0;
					UINT32 square_diff_sum_cr = 0;
					int i,j;
					int variance, variance_cr, variance_cb;

					UINT32 y_sum = 0;
					UINT32 cb_sum = 0;
					UINT32 cr_sum = 0;

					int max_y_array = 0;
					int min_y_array = 1024;
					int max_cb_array = 0;
					int min_cb_array = 1024;
					int max_cr_array = 0;
					int min_cr_array = 1024;

					int max_y_sum = 0;
					int min_y_sum = 0;
					int max_cb_sum = 0;
					int min_cb_sum = 0;
					int max_cr_sum = 0;
					int min_cr_sum = 0;

					int repeat, repeat_count;

					KADP_DBG_GetDecimalInput("[ADC Debug] Input Repeat Time (1 ~ 100) = ", &repeat);

					if(repeat > 100 || repeat < 1)
					{
						KADP_PRINTF("[ADC Debug] Wrong Repeat Number [%d] \n", repeat);
						break;
					}

					KADP_AFE_ADC_Get_Timing_Info(&adc_timing_info);
					KADP_PRINTF("[ADC Debug]  hActive= %d \n", adc_timing_info.u16_HActive);
					KADP_PRINTF("[ADC Debug]  vActive= %d \n", adc_timing_info.u16_VActive);
					KADP_PRINTF("[ADC Debug]  scanType= %d (0:interlace, 1: progressive)\n", adc_timing_info.u8_ScanType);

					if(adc_timing_info.u8_ScanType == 0)
						adc_timing_info.u16_VActive = adc_timing_info.u16_VActive / 2;

					x_pos_pixel = 300;
					y_pos_pixel = 100;

					/*
					x_pos_pixel = adc_timing_info.u16_HActive /2;
					y_pos_pixel = adc_timing_info.u16_VActive /2;
					*/

					for(repeat_count = 0; repeat_count < repeat ; repeat_count++)
					{
						max_y = 0;
						min_y = 1024;
						max_cb = 0;
						min_cb = 1024;
						max_cr = 0;
						min_cr = 1024;
						square_diff_sum = 0;
						square_diff_sum_cb = 0;
						square_diff_sum_cr = 0;

						KADP_PRINTF("################# Repeat Number [%d/%d] ######################\n", repeat_count+1, repeat);

						do{
							KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel, y_pos_pixel, width, height, &average_cr, &average_y, &average_cb);
							KADP_PRINTF("[ADC Debug] Center (%d*%d) Average [Y:%d][Cb:%d][Cr:%d]\n" , width, height , average_y, average_cb, average_cr);
						} while ( (average_y > 1024) || (average_cb > 1024) || (average_cr > 1024) );

						for(i= 0; i<height;i++)
						{
							for(j=0;j<width; j++)
							{
								KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel + j, y_pos_pixel + i, 1, 1, &value_cr, &value_y, &value_cb);

								do{

									if ( ( abs(value_cr - average_cr) > 200 ) \
											|| ( abs(value_y - average_y) > 200 ) \
											|| ( abs(value_cb - average_cb) > 200 ) ) 
									{
										KADP_PRINTF("[ADC Debug] Pixel value wrong ??? (%d*%d) value_y[%d] value_cb[%d] value_cr[%d] \n"\
												,j,i , value_y, value_cb, value_cr);
										usleep(1000*100);
										KADP_AFE_ADC_Read_Pixel_Value(x_pos_pixel + j, y_pos_pixel + i, 1, 1, &value_cr, &value_y, &value_cb);
										KADP_PRINTF("[ADC Debug] Re_read Value : (%d*%d) value_y[%d] value_cb[%d] value_cr[%d] \n"\
												,j,i , value_y, value_cb, value_cr);
									}
								} while ( (value_y > 1024) || (value_cb > 1024) || (value_cr > 1024) );

								if ( ( abs(value_cr - average_cr) < 200 ) \
										&& ( abs(value_y - average_y) < 200 ) \
										&& ( abs(value_cb - average_cb) < 200 ) )
								{
									if(value_y > max_y)
									{
										max_y = value_y;
										if( (i>0)||(j>0) )
											KADP_PRINTF("[ADC Debug] (%d*%d) min_y[%d],max_y[%d] \n" , j, i , min_y, max_y);
									}
									if(value_y < min_y)
									{
										min_y = value_y;
										if( (i>0)||(j>0) )
											KADP_PRINTF("[ADC Debug] (%d*%d) min_y[%d],max_y[%d] \n" , j, i , min_y, max_y);
									}
									square_diff_sum += ( abs(average_y - value_y) ) * (abs(average_y - value_y) );

									if(value_cb > max_cb)
									{
										max_cb = value_cb;
										if( (i>0)||(j>0) )
											KADP_PRINTF("[ADC Debug] (%d*%d) min_cb[%d],max_cb[%d] \n" , j, i , min_cb, max_cb);
									}
									if(value_cb < min_cb)
									{
										min_cb = value_cb;
										if( (i>0)||(j>0) )
											KADP_PRINTF("[ADC Debug] (%d*%d) min_cb[%d],max_cb[%d] \n" , j, i , min_cb, max_cb);
									}
									square_diff_sum_cb += ( abs(average_cb - value_cb) ) * (abs(average_cb - value_cb) );

									if(value_cr > max_cr)
									{
										max_cr = value_cr;
										if( (i>0)||(j>0) )
											KADP_PRINTF("[ADC Debug] (%d*%d) min_cr[%d],max_cr[%d] \n" , j, i , min_cr, max_cr);
									}
									if(value_cr < min_cr)
									{
										min_cr = value_cr;
										if( (i>0)||(j>0) )
											KADP_PRINTF("[ADC Debug] (%d*%d) min_cr[%d],max_cr[%d] \n" , j, i , min_cr, max_cr);
									}
									square_diff_sum_cr += ( abs(average_cr - value_cr) ) * (abs(average_cr - value_cr) );
								}
								else
									KADP_PRINTF("[ADC Debug] !!!!! NOT USE WRONG DATA !!!!! : (%d*%d) value_y[%d] value_cb[%d] value_cr[%d] \n"\
											,j,i , value_y, value_cb, value_cr);
							}
						}
						variance = square_diff_sum / (height * width);
						variance_cb = square_diff_sum_cb / (height * width);
						variance_cr = square_diff_sum_cr / (height * width);

						KADP_PRINTF("-----------------------------------------------------------------------------------------\n");
						KADP_PRINTF("[Y  Result] MinY[%d],MaxY[%d], Diff[%d],  Variance[%d]\n"\
								, min_y, max_y, max_y-min_y, variance );
						KADP_PRINTF("[Cb Result] MinCb[%d],MaxCr[%d], Diff[%d],  Variance[%d]\n"\
								, min_cb, max_cb, max_cb-min_cb, variance_cb );
						KADP_PRINTF("[Cr Result] MinCr[%d],MaxCr[%d], Diff[%d],  Variance[%d]\n"\
								, min_cr, max_cr, max_cr-min_cr, variance_cr );
						KADP_PRINTF("-----------------------------------------------------------------------------------------\n");

						y_sum += average_y;
						cb_sum += average_cb;
						cr_sum += average_cr;

						max_y_sum += max_y;
						min_y_sum += min_y;
						max_cb_sum += max_cb;
						min_cb_sum += min_cb;
						max_cr_sum += max_cr;
						min_cr_sum += min_cr;

						if(max_y > max_y_array)
						{
							max_y_array = max_y;
						}
						if(min_y < min_y_array)
						{
							min_y_array = min_y;
						}

						if(max_cb > max_cb_array)
						{
							max_cb_array = max_cb;
						}
						if(min_cb < min_cb_array)
						{
							min_cb_array = min_cb;
						}

						if(max_cr > max_cr_array)
						{
							max_cr_array = max_cr;
						}
						if(min_cr < min_cr_array)
						{
							min_cr_array = min_cr;
						}

					}
					KADP_PRINTF("#########################################################################################\n");
					KADP_PRINTF("[RESULT] repeat time [%d] \n", repeat);
					KADP_PRINTF("Average [Y:%d][Cb:%d][Cr:%d]\n" , y_sum/repeat, cb_sum/repeat, cr_sum/repeat);
					KADP_PRINTF("-----------------------------------------------------------------------------------------\n");
					KADP_PRINTF("[Y Average Result] MinY[%d],MaxY[%d], Diff[%d]\n"\
							, min_y_sum/repeat, max_y_sum/repeat, (max_y_sum-min_y_sum)/repeat);
					KADP_PRINTF("[Cb Average Result] MinCb[%d],MaxCr[%d], Diff[%d]\n"\
							, min_cb_sum/repeat, max_cb_sum/repeat, (max_cb_sum-min_cb_sum)/repeat);
					KADP_PRINTF("[Cr Average Result] MinCr[%d],MaxCr[%d], Diff[%d]\n"\
							, min_cr_sum/repeat, max_cr_sum/repeat, (max_cr_sum-min_cr_sum)/repeat);
					KADP_PRINTF("-----------------------------------------------------------------------------------------\n");
					KADP_PRINTF("[Y Peak Result] MinY[%d],MaxY[%d], Diff[%d]\n"\
							, min_y_array, max_y_array, (max_y_array-min_y_array));
					KADP_PRINTF("[Cb Peak Result] MinCb[%d],MaxCr[%d], Diff[%d]\n"\
							, min_cb_array, max_cb_array, (max_cb_array-min_cb_array));
					KADP_PRINTF("[Cr Peak Result] MinCr[%d],MaxCr[%d], Diff[%d]\n"\
							, min_cr_array, max_cr_array, (max_cr_array-min_cr_array));
					KADP_PRINTF("#########################################################################################\n");

				}
				break;

			case 0xa0 :
				KADP_PRINTF("	[ADC Debug] Execute A-Die PDB Test \n");
				KADP_AFE_ADC_A_Die_PDB_Test();
				break;

			case 0xec :
				KADP_PRINTF("[ADC Debug] ADC IRE LEVEL TEST \n");
				KADP_DBG_GetDecimalInput("Input IRE Level (0:IRE0, 1:IRE73, 2:IRE950, 3:IRE1023 ", &input1);
				adc_test_param_t.item_to_test = ADC_TEST_IRE_LEVEL;
				adc_test_param_t.adc_ire_value = input1;
				KADP_AFE_ADC_Test(&adc_test_param_t);
				break;

			case 0xed :
				KADP_PRINTF("[ADC Debug] ADC Calibration in KDRV \n");
				adc_test_param_t.item_to_test = ADC_TEST_CALIBRATION;
				KADP_AFE_ADC_Test(&adc_test_param_t);
				break;

			case 0xee :
				KADP_PRINTF("	[ADC Debug] KDRV AFE_DebugPrintCtrl \n");
				AFE_DebugPrintCtrl();
				break;

			case 0xef :
				KADP_DBG_GetDecimalInput("[ADC Debug] KADP INFO DBG_PRINT Control [0:off, 1:ON] = ", &input);

				if(input)
					KADP_LOGM_BitMaskEnable(g_afe_logm_fd, LX_LOGM_LEVEL_INFO);
				else
					KADP_LOGM_BitMaskDisable(g_afe_logm_fd, LX_LOGM_LEVEL_INFO);
				break;

			case 0xfa:
				KADP_PRINTF("	[ADC Debug] ADC Workaround On/Off \n");
				KADP_DBG_GetDecimalInput("Input Workaround Number ", &input1);
				KADP_DBG_GetDecimalInput("Input On(1)/Off(0) ", &input2);

				adc_test_param_t.item_to_test = ADC_TEST_WORKAROUND_CONTROL;
				adc_test_param_t.workaround_number = input1;
				adc_test_param_t.workaround_on_off = input2;
				KADP_AFE_ADC_Test(&adc_test_param_t);
				break;

			case 0xfb:
				KADP_PRINTF("	[ADC Debug] ADC Fake Init \n");
				KADP_AFE_ADC_Fake_Init();
				break;

			case 0xFF:
				KADP_PRINTF("\n	Exit \n");
				break;
			default:
				KADP_PRINTF("\n	Invalid choice! \n");
				break;
		}
	} while (nTest != 0xFF);

	return;

}
/**
 * CVD_Testmenu
 *
 * @param void
 * @return UINT32
 */
UINT32 KADP_AFE_CVD_TestMenu(void)
{
//	UINT32 nItem;

	KADP_PRINTF("\n	-------------------------\n");
	KADP_PRINTF("	|        CVD    MENU             |\n");
	KADP_PRINTF("	-------------------------\n");
	KADP_PRINTF("	[0x01] CVD_Init \n");
	KADP_PRINTF("	[0x02] CVD Select Source \n");
	KADP_PRINTF("	[0x03] Print CVD Timing Info \n");
	KADP_PRINTF("	[0x05] ON/OFF CVD Timer \n");
	KADP_PRINTF("	[0x06] Program Color System \n");
	KADP_PRINTF("	[0x07] Enable/Disable Scart FB \n");
	KADP_PRINTF("	[0x08] Set B0 Color Enhancement Params\n");
	KADP_PRINTF("	[0x09] Print Detailed States of CVD\n");
	KADP_PRINTF("	[0x0A] On/Off Periodic Print States of CVD\n");
	KADP_PRINTF("	[0x0B] Get Crunky Detection Status\n");
	KADP_PRINTF("	[0x0d] CVD Channel Power Control\n");
	KADP_PRINTF(" 	[0x0e] KADP_AFE_Open \n");
	KADP_PRINTF(" 	[0x0f] KADP_AFE_Close \n");
		KADP_PRINTF("	[0x10] CVD/DE Register Read \n");
		KADP_PRINTF("	[0x11] CVD/DE Register Write \n");
		KADP_PRINTF("	[0x12] A-Die Register Read \n");
		KADP_PRINTF("	[0x13] A-Die Register Write \n");
	KADP_PRINTF("	[0x14] Enable/Disable CVD PE Setting\n");
	KADP_PRINTF("	[0x15] Force CVD PE Setting\n");
	KADP_PRINTF("	[0x16] Read SCART ID/FB\n");
	KADP_PRINTF("	[0x17] Set Sync Detection for Tuning\n");
	KADP_PRINTF("	[0x18] Black Level Control\n");
	KADP_PRINTF("	[0x19] CVD HSync Stable Function Test\n");
	KADP_PRINTF("	[0x1A] CVD Comb2d Only Test\n");
	KADP_PRINTF("	[0x1B] SCART out buffer control test\n");
	KADP_PRINTF("	[0x1C] Set Analog Color System \n");
	KADP_PRINTF("	[0x1D] Set Analog Internal/External Demod Mode\n");
	KADP_PRINTF("	[0x1E] Switch Analog Internal/External Demod Mux\n");
	KADP_PRINTF("	[0x20] Enable/Disable status_fld print\n");
	KADP_PRINTF("	[0x21] Set SW 3DCOMB Control Value\n");
	KADP_PRINTF("	[0x22] Enable/Disable 3Dcomb state print\n");
	KADP_PRINTF("	[0x23] Set Blue mode\n");
	KADP_PRINTF("	[0x24] ABB DMD DAC Out Debug\n");
	KADP_PRINTF("	[0x25] Scart RGB Debug\n");
	KADP_PRINTF("	[0x26] VDAC out mute \n");
	KADP_PRINTF("	[0x27] HSync Enhancement Test \n");
	KADP_PRINTF("	[0x28] VSync Enhancement Test \n");
	KADP_PRINTF("	[0x29] 3DCOMB(YC Seperation)Blend Control \n");
	KADP_PRINTF("	[0x2A] Clamp Up/Down Control \n");
	KADP_PRINTF("	[0x2B] DC Clamp Mode Control \n");
	KADP_PRINTF("	[0x2C] ABB Clamp Parameter Setting \n");
	KADP_PRINTF("	[0x30] Enable/Disable CVD Detection Interrupt\n");
	KADP_PRINTF("	[0x31] SCART out control print \n");
	KADP_PRINTF("	[0x32] CVD Sync Exist debug \n");
	KADP_PRINTF("	[0x40] AGC Peak Nominal Control \n");
	KADP_PRINTF("	[0x41] SYS Register Read \n");
	KADP_PRINTF("	[0x42] SYS Register Write \n");

	KADP_PRINTF("	[0xee] AFE DebugPrintCtrl\n");
	KADP_PRINTF("	[0xfa] CVD Workaround On/off\n");
	KADP_PRINTF("	[0xfb] CVD Fake Init\n");
	KADP_PRINTF("	[0xfe] kadp test \n");
	KADP_PRINTF("	[0xff] Exit...\n");
	KADP_PRINTF("	-------------------------\n");
	//KADP_DBG_GetHexInput("	Select Menu", &nItem);

	//return nItem;
	return 0;
}
/**
 * cvd_debug_test
 *
 * @param void
 * @return void
 */
void	KADP_AFE_CVD_DEBUG_Test (void)
{

	int					rc;
	UINT32				nTest;
	LX_AFE_CVD_TIMING_INFO_T cvd_timing_info = {0,};
	LX_AFE_CVD_STATES_DETAIL_T cvd_states_detail_t;
	LX_AFE_CVD_TEST_PARAM_T cvd_test_param_t;

	KADP_AFE_CVD_Fake_Init();

	KADP_AFE_CVD_TestMenu();
	do {
		KADP_DBG_GetHexInput("\n### Select CVD Debug Menu", &nTest);

		if ((nTest == 0) || (nTest > 0xff) )
		{
			KADP_AFE_CVD_TestMenu();
			continue;
		}

		UINT32	input1 = 0;
		UINT32	input2 = 0;
		UINT32	input3 = 0;
		UINT32	input4 = 0;
		UINT32	input5 = 0;
		UINT8	isLine625;
		BOOLEAN onoff1, onoff2;
		UINT32 				rData, wData;
		UINT32 				r_addr = 0;
		UINT32				w_addr = 0;

		switch (nTest)
		{
			case 0x01:
				//KADP_DBG_GetDecimalInput("[CVD Debug]Select Main or CHB CVD[0:Main, 1:CHB] = ", &input1);
				input1 = 0;
				if(input1 ==0)
					input1 = LX_CVD_MAIN;
				else if (input1 == 1)
					input1 = LX_CVD_SUB;
				else
					break;

				KADP_AFE_CVD_Init(input1);
				KADP_PRINTF("[CVD Debug] CVD Initialized \n");
				break;

			case 0x02:
		//		KADP_DBG_GetDecimalInput("[CVD Debug]Select CVBS or S-VIDEO[0:CVBS, 1:S-VIDEO, 2:RF] = ", &input1);
				//NO S-VIDEO Supported from L9
				input1 = 0;

				KADP_DBG_GetDecimalInput("[CVD Debug]Select CVBS(Y) input port [1~6,  1:ATV] = ", &input2);

				if ((input2<1 ) || (input2>6))
					break;
				
				input3 = 1; // to avoid static analysis alarm

				KADP_DBG_GetDecimalInput("[CVD Debug]Select Input Source attribute :[0:RF, 1:AV,] = ", &input4);

				KADP_PRINTF("[CVD Debug] OK! Calling Adap. functioin \n");
				KADP_AFE_CVD_Set_Source_Type(LX_CVD_MAIN, input1, input2, input3, input4);

				break;

			case 0x03 :
				input1 = LX_CVD_MAIN;

				KADP_AFE_CVD_Get_Timing_Info(input1,&cvd_timing_info);
				KADP_PRINTF("[CVD Debug] CVD Timing Info \n");
				KADP_AFE_CVD_Sync_Exist(input1, &onoff1);
				if(onoff1)
					KADP_PRINTF("[CVD Debug] CVD Sync Exist \n");
				else
					KADP_PRINTF("[CVD Debug] CVD NO SIGNAL !!!!! \n");

				KADP_PRINTF("[CVD Debug] Color System= %d (1:NTSC_M, 2:NTSC_J, 3:NTSC_443, 4:PAL_BG, 5:PAL_N, 6:PAL_M, 7:PAL_Cn, 8:PAL60, 9:SECAM, 10:Non Standard) \n", cvd_timing_info.cvd_standard);

				KADP_PRINTF("[CVD Debug]  hFreq= %d (* 100Hz) \n", cvd_timing_info.u16_HFreq);
				KADP_PRINTF("[CVD Debug]  vFreq= %d (* 0.1Hz) \n", cvd_timing_info.u16_VFreq);
				KADP_PRINTF("[CVD Debug]  hTotal= %d \n", cvd_timing_info.u16_HTotal);
				KADP_PRINTF("[CVD Debug]  hStart= %d \n", cvd_timing_info.u16_HStart);
				KADP_PRINTF("[CVD Debug]  vStart= %d \n", cvd_timing_info.u16_VStart);
				KADP_PRINTF("[CVD Debug]  hActive= %d \n", cvd_timing_info.u16_HSize);
				KADP_PRINTF("[CVD Debug]  vActive= %d \n", cvd_timing_info.u16_VSize);
				KADP_PRINTF("[CVD Debug]  VFreq_Stable= %d \n", cvd_timing_info.u16_VFreq_Stable);

				KADP_AFE_CVD_Read_VFreq(LX_CVD_MAIN, &isLine625); // reg_625line == 1
				if(isLine625) // reg_625line ==1 : 50Hz system
					KADP_PRINTF("[CVD Debug] Field Rate is 50Hz ");
				else
					KADP_PRINTF("[CVD Debug] Field Rate is 60Hz ");

				KADP_PRINTF("[CVD Debug] End of CVD Timing Info \n");
				break;

			case 0x05:
//				KADP_DBG_GetDecimalInput("[CVD Debug]Select Main/CHB CVD [0:main, 1:chb] = ", &input1);
				input1 = 0;

				if(input1 ==0)
					input1 = LX_CVD_MAIN;
				else if (input1 == 1)
					input1 = LX_CVD_SUB;
				else
					break;

				KADP_DBG_GetDecimalInput("[CVD Debug]Select On/Off [0:off, 1:ON] = ", &input2);
				if(input2 == 1)
				{
					KADP_DBG_GetDecimalInput("[CVD Debug]Input Timeout period (msec) = ", &input3);
					KADP_AFE_CVD_Start_Timer(input1, input3);
				}
				else
				{
					KADP_AFE_CVD_Stop_Timer(input1);
				}
				break;

			case 0x06:
				input1 = LX_CVD_MAIN;
				KADP_PRINTF("[CVD Debug] Input Color System Number to program registers \n");
				KADP_PRINTF("[CVD Debug] 0:Default, 1:NT, 2:NT_J, 3:NT_443, 4:PAL, 5:PAL_N\n");
				KADP_DBG_GetDecimalInput("[CVD Debug] 6:PAL_M, 7:PAL_CN, 8:PAL_60, 9:SECAM =", &input2);

				if(input2 > 9)
					break;

				KADP_AFE_CVD_Program_Color_System(input1, input2);

				break;

			case 0x07:
				KADP_DBG_GetDecimalInput("[CVD Debug]Select On/Off CVD [0:off, 1:on] = ", &input1);

				if(input1 ==0)
					KADP_AFE_Set_Scart_Overlay(FALSE);
				else
					KADP_AFE_Set_Scart_Overlay(TRUE);

				break;

			case 0x08:
				KADP_DBG_GetDecimalInput("[CVD Debug]Select Item to set [0:AAF, 1:ADC, 2:CRES, 3:DC, 4:CCR, 5:MD, 6:GC, 7:OD, 8:OR, 9:Blending, 10:Chroma_demode = ", &input1);

				if(input1 >= CVD_PE_NUM)
					break;

				if(input1 == CVD_PE_AAF)
					KADP_DBG_GetDecimalInput("[CVD Debug] 0:Default, 1:NTSC_M, 2:NTSC_RF, 3:PAL, 4:PAL_RF 5:SECAM 6:SECAM_RF 7:AAF_A 8:AAF_6_0 9:AAF_5_5 10:AAF_4_5 =", &input2);
				else
					KADP_DBG_GetDecimalInput("[CVD Debug] 0:Default, 1:NTSC_M, 2:NTSC_RF, 3:PAL, 4:PAL_RF 5:SECAM 6:SECAM_RF =", &input2);

				if(input2 >= LX_CVD_PQ_NUM)
					break;

				cvd_test_param_t.item_to_test =  CVD_TEST_PICTURE_ENHANCEMENT;
				cvd_test_param_t.cvd_pe_param =  input1;
				cvd_test_param_t.cvd_pe_mode =  input2;
				KADP_AFE_CVD_Test(&cvd_test_param_t);

				break;

			case 0x09:

				KADP_AFE_CVD_Get_States_Detail(&cvd_states_detail_t);
				KADP_AFE_CVD_Get_Timing_Info(LX_CVD_MAIN,&cvd_timing_info);
				KADP_PRINTF("[CVD Debug] CVD Detailed States \n");
				KADP_PRINTF("[CVD Debug] CVBS_INSEL = [0x%x], CVBS_CP = [0x%x], CVBS_PDB = [0x%x] , CVBS_ICON = [0x%x] \n", cvd_states_detail_t.cvbs_insel, cvd_states_detail_t.cvbs_cp, cvd_states_detail_t.cvbs_pdb, cvd_states_detail_t.cvbs_icon);
				KADP_PRINTF("[CVD Debug] BUF_YCM = [0x%x], BUF_SEL1 = [0x%x], BUF_PDB1 = [0x%x], VDAC_PDB= [0x%x] \n", cvd_states_detail_t.buf_ycm, cvd_states_detail_t.buf_sel1, cvd_states_detail_t.buf_pdb1, cvd_states_detail_t.vdac_pdb);
				KADP_PRINTF("[CVD Debug] BUF_SEL1[0] = [0:DTV, 1,ATV], BUF_SEL1[1] = [0:Buf Clamp ON, 1:Buf Clamp Off], BUF_SEL1[2] = [0]\n");
				KADP_PRINTF("[CVD Debug] --------------------------------------- \n");
				KADP_PRINTF("[CVD Debug] Color System= [%d] (1:NT_M, 2:NT_J, 3:NT_443, 4:PAL_BG, 5:PAL_N, 6:PAL_M, 7:PAL_Nc, 8:PAL60, 9:SECAM, 10:Non Standard) \n", cvd_timing_info.cvd_standard);
				KADP_PRINTF("[CVD Debug] No Signal = [%d], HLock = [%d], VLock = [%d]\n", cvd_states_detail_t.No_Signal_Flag, cvd_states_detail_t.HLock_Flag, cvd_states_detail_t.VLock_Flag);
				KADP_PRINTF("[CVD Debug] VLine 625 = [%d], PAL = [%d], Secam = [%d]\n", cvd_states_detail_t.VLine_625_Flag, cvd_states_detail_t.Pal_Flag, cvd_states_detail_t.Secam_Flag);
				KADP_PRINTF("[CVD Debug] Chromalock = [%d], No Burst = [%d] \n", cvd_states_detail_t.Chromalock_Flag, cvd_states_detail_t.NoBurst_Flag);
				KADP_PRINTF("[CVD Debug] FC Flag(0:Less, 1:Same, 2:More) = [%d], Cordic Freq = [%d]  \n", cvd_states_detail_t.FC_Flag, cvd_states_detail_t.cvd_cordic_freq);
				KADP_PRINTF("[CVD Debug] Status Noise = [%d] \n", cvd_states_detail_t.status_noise);
//				KADP_PRINTF("[CVD Debug] HFcnt = [%d], LFcnt = [%d] \n", cvd_states_detail_t.hfcnt_value, cvd_states_detail_t.lfcnt_value);
				KADP_PRINTF("[CVD Debug] Current State = [%d], Next State = [%d] \n", cvd_states_detail_t.Current_State, cvd_states_detail_t.Next_State);

				KADP_PRINTF("[CVD Debug] hnon_standard = [%d], vnon_standard = [%d], vdetect_vcount = [%d] \n", cvd_states_detail_t.hnon_standard, cvd_states_detail_t.vnon_standard, cvd_states_detail_t.vdetect_vcount);
				KADP_PRINTF("[CVD Debug] --------------------------------------- \n");
				KADP_PRINTF("[CVD Debug] CS0 PAL = [%d] ,CS0 Secam = [%d], CS0 Chromalock = [%d] \n"\
				, cvd_states_detail_t.Pal_Flag_CS0, cvd_states_detail_t.Secam_Flag_CS0, cvd_states_detail_t.Chromalock_Flag_CS0);
				KADP_PRINTF("[CVD Debug] CS0 FC Flag(0:Less, 1:Same, 2:More) = [%d], CS0 Cordic Freq = [%d] \n"\
				, cvd_states_detail_t.FC_Flag_CS0, cvd_states_detail_t.cvd_cordic_freq_CS0);
				KADP_PRINTF("[CVD Debug] --------------------------------------- \n");

				KADP_PRINTF("[CVD Debug] CS1 PAL = [%d] ,CS1 Secam = [%d], CS1 Chromalock = [%d] \n"\
				, cvd_states_detail_t.Pal_Flag_CS1, cvd_states_detail_t.Secam_Flag_CS1, cvd_states_detail_t.Chromalock_Flag_CS1);
				KADP_PRINTF("[CVD Debug] CS1 FC Flag(0:Less, 1:Same, 2:More) = [%d], CS1 Cordic Freq = [%d] \n"\
				, cvd_states_detail_t.FC_Flag_CS1, cvd_states_detail_t.cvd_cordic_freq_CS1);
				KADP_PRINTF("[CVD Debug] --------------------------------------- \n");
				KADP_PRINTF("[CVD Debug] CVD Color Standard = [%d], CS0 = [%d], CS1 = [%d]\n"\
				, cvd_states_detail_t.CVD_Color_System, cvd_states_detail_t.CS0_Color_System, cvd_states_detail_t.CS1_Color_System);
				KADP_PRINTF("[CVD Debug] System Supported = [0x%x] \n", cvd_states_detail_t.color_system_support);
				KADP_PRINTF("[CVD Debug] --------------------------------------- \n");
				KADP_PRINTF("[CVD Debug] tnr_x = [0x%x], tnr_s = [0x%x], s_status = [0x%x] \n", cvd_states_detail_t.tnr_x, cvd_states_detail_t.tnr_s, cvd_states_detail_t.s_status);
				KADP_PRINTF("[CVD Debug] motion_diff = [%d], global_motioin = [0x%x], pattern_found = [%d] , static_pattern_found = [%d]\n", cvd_states_detail_t.motion_diff, cvd_states_detail_t.motion_value, cvd_states_detail_t.pattern_found, cvd_states_detail_t.static_pattern_found);
				KADP_PRINTF("[CVD Debug] --------------------------------------- \n");
				KADP_PRINTF("[CVD Debug] agc_gain = [0x%x], agc_peak_en = [%d], agc_bypass = [%d] \n", cvd_states_detail_t.agc_gain, cvd_states_detail_t.agc_peak_en, cvd_states_detail_t.agc_bypass);
				KADP_PRINTF("[CVD Debug] burst_mag = [0x%x], cagc = [0x%x], saturation = [0x%x] \n", cvd_states_detail_t.burst_mag, cvd_states_detail_t.cagc, cvd_states_detail_t.saturation);
				KADP_PRINTF("[CVD Debug] tunning_mode = [0x%x], stable_count =[%d], no_signal_count = [%d] \n", cvd_states_detail_t.tunning_mode, cvd_states_detail_t.lock_stable_count, cvd_states_detail_t.no_signal_count);
				KADP_PRINTF("[CVD Debug] black level = [%d], no_color_detected =[%d] \n", cvd_states_detail_t.black_level, cvd_states_detail_t.no_color_detected);
				KADP_PRINTF("[CVD Debug] colorbar diff = [%d] , colorbar 75_100[%d]\n", cvd_states_detail_t.colorbar_diff, cvd_states_detail_t.colorbar_75_100);
				KADP_PRINTF("[CVD Debug] Internal_Demod = [%d]\n", cvd_states_detail_t.analog_demod_type);
				KADP_PRINTF("[CVD Debug] Status Clamp Up/Down  = [0x%x]\n", cvd_states_detail_t.status_clamp_updn);
				KADP_PRINTF("[CVD Debug] --------------------------------------- \n");

				KADP_PRINTF("[CVD Debug] End of CVD Timing Info \n");
				break;

			case 0x0A:
				KADP_DBG_GetDecimalInput("[CVD Debug]Select On/Off Periodic Print [0:off, 1:on] = ", &input1);

				if(input1 ==0)
					_gEnablePeriodicCVDPrint = FALSE;
				else
					_gEnablePeriodicCVDPrint = TRUE;

				break;

			case 0x0B:
				{
					UINT32 ck_vbi_detection, number_of_cs;

					KADP_AFE_CVD_Get_Crunky_Status(&ck_vbi_detection, &number_of_cs);

					KADP_PRINTF("[CVD Debug] Crunky PSP	 = [%d] \n", ck_vbi_detection);
					KADP_PRINTF("[CVD Debug] Number of CS = [%d] \n", number_of_cs);

				}
				break;
			case 0x0d :
				KADP_PRINTF("[CVD Debug] CVD Channel Power Control \n");
				KADP_DBG_GetDecimalInput("[CVD Debug]Input On/Off Value(0:off, 1:on) = ", &input1);
				KADP_AFE_CVD_POWER_CONTROL(input1);
				break;

			case 0x0e:
				{
					rc =  KADP_AFE_Open();
					KADP_PRINTF("[AFE Debug]AFE OPEN(OK - 0) = %d", rc);
				}
				break;

			case 0x0f :
				{
					rc =  KADP_AFE_Close();
					KADP_PRINTF("[AFE Debug]AFE CLOSE(OK - 0) = %d", rc);
				}
				break;

			case 0x10:
				{
					KADP_AFE_CVD_Read(0,0);
					break;
				}

			case 0x11:
				{
					KADP_AFE_CVD_Write(0, 0);
					break;
				}

			case 0x12:

					KADP_AFE_ADC_A_Die_Read(0, 0);
				break;
			case 0x13:
					KADP_AFE_ADC_A_Die_Write(0, 0);
				break;

			case 0x14:
				KADP_DBG_GetDecimalInput("[CVD Debug]Select Enable/Disable[0:disable, 1:enable = ", &input1);

				if(input1)
					_gEnableCVDPESetting = TRUE;
				else
					_gEnableCVDPESetting = FALSE;

				break;
			case 0x15:
				KADP_PRINTF(" LX_CVD_PQ_NTSC_M 	= 1,\n LX_CVD_PQ_NTSC_M_RF = 2,\n LX_CVD_PQ_PAL 		= 3,\n LX_CVD_PQ_PAL_RF	 = 4\n");
				KADP_PRINTF(" LX_CVD_PQ_SECAM 	= 5,\n LX_CVD_PQ_SECAM_RF 	= 6,\n LX_CVD_PQ_NTSC_443 	= 7,\n LX_CVD_PQ_NTSC_443_RF = 8\n");
				KADP_PRINTF(" LX_CVD_PQ_PAL_60 	= 9,\n LX_CVD_PQ_PAL_60_RF = 10,\n LX_CVD_PQ_PAL_M 	= 11,\n LX_CVD_PQ_PAL_M_RF 	= 12\n");
				KADP_PRINTF(" LX_CVD_PQ_PAL_NC 	= 13,\n LX_CVD_PQ_PAL_NC_RF = 14\n");
				KADP_DBG_GetDecimalInput("[CVD Debug]Select ", &input1);
				if ( (input1 < LX_CVD_PQ_NTSC_M) || (input1 > LX_CVD_PQ_PAL_NC_RF))
					break;

				KADP_AFE_CVD_SET_PQ_VALUE(input1);

				break;
			case 0x16:
				{
					LX_AFE_SCART_MODE_T		fb_status;
					LX_AFE_SCART_AR_T	inputMode = LX_SCART_AR_INVALID;

					KADP_AFE_Get_FB_Status(&fb_status);
					KADP_AFE_Get_Scart_AR(LX_SCART_ID_1, &inputMode);

					KADP_PRINTF("[CVD Debug] SCART Fast Blanking Status : [%s]\n", fb_status ? "SCART_MODE_CVBS":"SCART_MODE_RGB");

					if(inputMode == LX_SCART_AR_4_3)
						KADP_PRINTF("[CVD Debug] SCART Aspect Ratio Status(SCART ID) : [SCART_AR_4_3]\n");
					else if(inputMode == LX_SCART_AR_16_9)
						KADP_PRINTF("[CVD Debug] SCART Aspect Ratio Status(SCART ID) : [SCART_AR_16_9]\n");
					else
						KADP_PRINTF("[CVD Debug] SCART Aspect Ratio Status(SCART ID) : [SCART_AR_Inactive]\n");

					break;
				}
			case 0x17:
				KADP_PRINTF(" Enable [1] / Disable [0] \n");
				KADP_DBG_GetDecimalInput("[CVD Debug]Select ", &input1);

				KADP_AFE_CVD_SetSyncDetectionForTuning(input1);

				break;
			case 0x18:
				KADP_PRINTF("[CVD Debug] Input Black Level : Low[0], High[1], Auto[2] \n");
				KADP_DBG_GetDecimalInput("[CVD Debug] Input :", &input1);

				if(input1 < 3)
					KADP_AFE_CVD_Set_BlackLevel(input1);

				break;
			case 0x19:
				KADP_PRINTF("[CVD Debug] Input Stable Mode Parameter : Off[0], lpf_en only[1], Low[2], High[3]\n");
				KADP_DBG_GetDecimalInput("[CVD Debug] Input :", &input1);

				cvd_test_param_t.item_to_test = CVD_TEST_STABLE_SYNC_MODE;
				cvd_test_param_t.stable_sync_value = input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);

				break;
			case 0x1A:
				KADP_PRINTF("[CVD Debug] Input Comb2d Only Test Enable : Off[0], ON[1]\n");
				KADP_DBG_GetDecimalInput("[CVD Debug] Input :", &input1);

				if(input1)
				{
					KADP_DBG_GetDecimalInput("[CVD Debug] Input Comb2d Only On time:", &input2);
					KADP_DBG_GetDecimalInput("[CVD Debug] Input Comb2d Only Off time:", &input3);
					KADP_DBG_GetDecimalInput("[CVD Debug] Input Comb2d Only MD On time:", &input4);
					KADP_DBG_GetDecimalInput("[CVD Debug] Input Comb2d Only MD Off time:", &input5);
				}

				cvd_test_param_t.item_to_test = CVD_TEST_COMB2D_ONLY;
				cvd_test_param_t.comb2d_only_test_enable = input1;
				cvd_test_param_t.comb2d_only_on_time = input2;
				cvd_test_param_t.comb2d_only_off_time = input3;
				cvd_test_param_t.comb2d_only_md_on_time = input4;
				cvd_test_param_t.comb2d_only_md_off_time = input5;
				KADP_AFE_CVD_Test(&cvd_test_param_t);

				break;
			case 0x1B:
				{

					KADP_PRINTF("[CVD Debug] Iput Bypass Source : DTV[0], WITH_CLAMPING[1], WITH_CLAMPING_AV[2], WITHOUT_CLAMPING[3],\
							Buf_Clamping_On[4], Buf_Clamping_Off[5]\n");
					KADP_PRINTF("[CVD Debug] ABB[6], ABB after Rate Convertor[7]\n");
					KADP_DBG_GetDecimalInput("[CVD Debug] Input :", &input1);
					KADP_PRINTF("[CVD Debug] Iput CVBS Source : CVBS_IN1[0] (default), CVBS_IN3[1]\n");
					KADP_DBG_GetDecimalInput("[CVD Debug] Input :", &input2);
					KADP_PRINTF("[CVD Debug] Set all related registers??? : no(default)[0], yes[1]\n");
					KADP_DBG_GetDecimalInput("[CVD Debug] Input :", &input3);

					if(input1 < CVD_BYPASS_MAX)
					{
						KADP_AFE_CVD_Bypass_Control(input1, CVD_BYPASS_DAC, input2);
						if(input3)
						{
							KADP_AFE_CVD_VDAC_Power_Control(TRUE);

							if ( (input1 == CVD_BYPASS_DAC) || (input1 == CVD_BYPASS_ABB) ||(input1 == CVD_BYPASS_ABB_RC) )
							{
								KADP_PRINTF("[CVD Debug] SCART out from DAC\n");
							}
							else
							{
								KADP_PRINTF("[CVD Debug] SCART out from CVBS Composite\n");
							}

							if ( (input1 == CVD_BYPASS_ABB) ||(input1 == CVD_BYPASS_ABB_RC) )
							{
								if(!LX_COMP_CHIP(lx_chip_rev(), LX_CHIP_M14) && lx_chip_rev() < LX_CHIP_REV(M14, B0))
								{
									KADP_PRINTF("[CVD Debug] From ABB\n");
									KADP_ADEMOD_Set_CvbsDecCtrl(LX_DEMOD_ANALOG_ATV);
								}
							}
							else if(input1 == CVD_BYPASS_DAC)
							{
								KADP_PRINTF("[CVD Debug] From CVE\n");
								if(!LX_COMP_CHIP(lx_chip_rev(), LX_CHIP_M14) && lx_chip_rev() < LX_CHIP_REV(M14, B0))
								{
									KADP_ADEMOD_Set_CvbsDecCtrl(LX_DEMOD_ANALOG_DTV);
								}
//								KADP_DENC_OnOff(TRUE);
//								KADP_DENC_Mute_Control(FALSE);
							}
						}
					}
					else
						KADP_PRINTF("[CVD Debug] Wrong !!!\n");

				}
				break;
			case 0x1C:
				KADP_PRINTF("	[CVD Debug] Set Analog Color System \n");
				KADP_PRINTF("	[CVD Debug] NTSC:0x1,PAL:0x2,PAL_NC:0x4,PAL_M:0x8 \n");
				KADP_PRINTF("	[CVD Debug] SECAM:0x10,NTSC_443:0x20,PAL_60:0x40  \n");
				KADP_DBG_GetHexInput("Input Color System to Support (0x7F to support all color system): ", &input1);
				KADP_AFE_Set_Analog_Color_System(LX_CVD_MAIN, input1);
				break;

			case 0x1D:
				KADP_PRINTF("	[CVD Debug] Set Analog Internal/External Demod Mode \n");
				KADP_DBG_GetHexInput("Input External:0, Internal:1 ", &input1);
				if(input1)
					onoff1 = TRUE;
				else
					onoff1 = FALSE;
				KADP_AFE_Set_Internal_Demod_Mode(onoff1);
				break;

			case 0x1E:
				KADP_PRINTF("	[CVD Debug] SWITCH Analog Internal/External Demod Mux \n");
				KADP_DBG_GetHexInput("Input External:0, Internal:1 ", &input1);

				cvd_test_param_t.item_to_test = CVD_TEST_ADEMOD_TYPE;
				cvd_test_param_t.internal_demod_type = input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x20:
				KADP_DBG_GetDecimalInput("[CVD Debug]Select Enable/Disable[0:disable, 1:enable = ", &input1);

				cvd_test_param_t.item_to_test =  CVD_TEST_ENABLE_STATUS_FLD_PRINT;
				if(input1 > 0)
					cvd_test_param_t.bEnable_Status_Fld_Print =  TRUE;
				else
					cvd_test_param_t.bEnable_Status_Fld_Print =  FALSE;

				KADP_AFE_CVD_Test(&cvd_test_param_t);

				break;

			case 0x21:
				KADP_DBG_GetDecimalInput("[CVD Debug] Select Enable/Disable[0:disable, 1:enable = ", &input1);

				if(input1> 0)
				{
					KADP_DBG_GetHexInput("[CVD Debug] Input Lf_thr : ", &input3);
					KADP_DBG_GetHexInput("[CVD Debug] Input Hf_thr : ", &input2);
					KADP_DBG_GetHexInput("[CVD Debug] Input Diff_thr : ", &input4);
					KADP_DBG_GetHexInput("[CVD Debug] Input Mode for Region 30 : HW(0), LF(1), HF(2) ", &input5);
				}

				KADP_AFE_CVD_SET_SW_3DCOMB_CONTROL((UINT8)input1, input2, input3, input4, input5);

				break;

			case 0x22:
				KADP_DBG_GetDecimalInput("[CVD Debug]Select Enable/Disable[0:disable, 1:enable = ", &input1);

				cvd_test_param_t.item_to_test =  CVD_TEST_ENABLE_3DCOMB_STATE_PRINT;

				if(input1 > 0)
					cvd_test_param_t.bEnable_3DCOMB_State_Print =  TRUE;
				else
					cvd_test_param_t.bEnable_3DCOMB_State_Print =  FALSE;


				KADP_AFE_CVD_Test(&cvd_test_param_t);

				break;

			case 0x23:
				KADP_PRINTF("	[CVD Debug] Set CVD Blue Mode \n");
				KADP_DBG_GetHexInput("Input 0:Disable(default), 1:Enable, 2:Auto ", &input1);

				cvd_test_param_t.item_to_test = CVD_TEST_BLUE_MODE;
				cvd_test_param_t.blue_mode_enable = input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x24:
				KADP_PRINTF("	[CVD Debug] Set DMD DAC Out Enable\n");
				KADP_DBG_GetHexInput("Input 0:Disable(default), 1:Enable", &input1);

				cvd_test_param_t.item_to_test = CVD_TEST_DMD_OUT;
				cvd_test_param_t.dmd_out_enable = input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x25:
				KADP_PRINTF("	[CVD Debug] SCART RGB Mix Debug\n");
				KADP_DBG_GetHexInput("Input FB_EN 0:fb_en disable, 1:fb_en enable", &input1);
				KADP_DBG_GetHexInput("Input Blend Ratio 0x0 ~ 0x10(default)", &input2);

				cvd_test_param_t.item_to_test = CVD_TEST_SCART_RGB;
				cvd_test_param_t.fb_en = input1;
				cvd_test_param_t.blend_ratio = input2;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x26:
				KADP_PRINTF("	[CVD Debug] VDAC Out Mute control\n");
				KADP_DBG_GetHexInput("Input 0:Disable, 1:Enable", &input1);

				if(input1)
					onoff1 = TRUE;
				else
					onoff1 = FALSE;
				KADP_AFE_CVD_VDAC_Mute_Control(onoff1);

				break;

			case 0x27:
				KADP_PRINTF("	[CVD Debug] HSync Enhancement Test\n");
				KADP_DBG_GetHexInput("Input 0:Off, 1:NTSC(ON), 2:PAL-M(ON)", &input1);

				cvd_test_param_t.item_to_test = CVD_TEST_HSYNC_ENHANCE;
				cvd_test_param_t.hsync_enhance_value = input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x28:
				KADP_PRINTF("	[CVD Debug] VSync Enhancement Test\n");
				KADP_DBG_GetHexInput("Input 0:Off, 1:480i, 2:576i", &input1);

				cvd_test_param_t.item_to_test = CVD_TEST_VSYNC_ENHANCE;
				cvd_test_param_t.vsync_enhance_value = input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x29:
				KADP_PRINTF("	[CVD Debug] 3DCOMB(YC Seperation)Blend Control\n");
				KADP_DBG_GetHexInput("Input 0x0:2DComb, 0x5:3D Only, 0xF:Blend Mode", &input1);

				cvd_test_param_t.item_to_test = CVD_TEST_YCSEP;
				cvd_test_param_t.ycsep_blend_value = input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x2A:
				KADP_PRINTF("	[CVD Debug] Clamp Up/Down Control\n");
				KADP_DBG_GetHexInput("Input 0x0:Normal Clamping, 0x10:Clamp Off, 0x11:Clamp Down, 0x12:Clamp Up", &input1);

				cvd_test_param_t.item_to_test = CVD_TEST_CLAMP_UPDN;
				cvd_test_param_t.clamp_updn_value = input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x2B:
				KADP_PRINTF("	[CVD Debug] DC Clamp Mode Control\n");
				KADP_DBG_GetHexInput("Input 0x0:auto(default), 0x1:backporch only, 0x2:synctip only, 0x3:off", &input1);

				cvd_test_param_t.item_to_test = CVD_TEST_DC_CLAMP_MODE;
				cvd_test_param_t.dc_clamp_mode_value = input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x2C:
				KADP_PRINTF("	[CVD Debug] ABB Clamp Parameter Setting \n");
				KADP_DBG_GetHexInput("Input clamp count : (0xa/0x3f) ", &input1);
				KADP_DBG_GetHexInput("Input clamp step size : (0xf/0x1) ", &input2);

				cvd_test_param_t.item_to_test = CVD_TEST_ABB_CLAMP_PARAM;
				cvd_test_param_t.abb_clamp_count = input1;
				cvd_test_param_t.abb_clamp_step = input2;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0x30:
				KADP_DBG_GetDecimalInput("[CVD Debug]Select Enable/Disable[0:disable, 1:enable = ", &input1);
				KADP_AFE_CVD_Enable_Detection_Interrupt(input1);

				break;

			case 0x31:
				KADP_PRINTF("	[CVD Debug] SCART OUT Status  \n");
				KADP_PRINTF("	[CVD Debug] BUF1 out 0:CVD_BYPASS_DAC, 1:CVD_BYPASS_CVBS_WITH_CLAMPING, 2:CVD_BYPASS_CVBS_WITH_CLAMPING_AV\n");
				KADP_PRINTF("	[CVD Debug] BUF1 out 3:CVD_BYPASS_CVBS_WITHOUT_CLAMPING, 4:CVD_BYPASS_CVBS_BUF_CLAMPING_ON\n");
				KADP_PRINTF("	[CVD Debug] BUF1 out 5:CVD_BYPASS_CVBS_BUF_CLAMPING_OFF, 6:CVD_BYPASS_ABB, 7:CVD_BYPASS_ABB_RC\n");
				KADP_PRINTF("	[CVD Debug] BUF1 out selected : [%d] \n", _g_cvd_bypass_control_t.buf_out_1_sel);
				KADP_PRINTF("	[CVD Debug] CVBS source selected : [%d] \n", _g_cvd_bypass_control_t.cvbs_source_sel);

				break;

			case 0x32:
				KADP_PRINTF("	[CVD Debug] CVD Sync Check in ATV Auto Search \n");
				KADP_DBG_GetDecimalInput("[CVD Debug]1 : HLock, 2:VLock, 3:HLock or Vlock, 4:HLock AND VLock (default) :", &g_CVD_status_to_check_ATV_Search);
				KADP_PRINTF("	[CVD Debug] Sync Check in ATV Auto Search [0x%x] \n", g_CVD_status_to_check_ATV_Search);

				break;

			case 0x40:
				cvd_test_param_t.item_to_test =  CVD_TEST_AGC_PEAK_NOMINAL;

				KADP_DBG_GetDecimalInput("[CVD Debug] Select Enable/Disable[0:disable, 1:enable] = ", &input1);

				if(input1 > 0)
					cvd_test_param_t.agc_peak_enable =  TRUE;
				else
				{
					cvd_test_param_t.agc_peak_enable =  FALSE;
					cvd_test_param_t.agc_peak_print_en = 0;
					KADP_AFE_CVD_Test(&cvd_test_param_t);
					break;
				}
				/*

				   KADP_DBG_GetDecimalInput("[CVD Debug] white_ratio_th : ", &input1);
				   cvd_test_param_t.agc_pean_white_ratio_th =  input1;
				   KADP_DBG_GetDecimalInput("[CVD Debug] white_frame_max_th : ", &input1);
				   cvd_test_param_t.agc_pean_white_frame_max_th =  input1;
				   KADP_DBG_GetDecimalInput("[CVD Debug] white_frame_on : ", &input1);
				   cvd_test_param_t.agc_pean_white_frame_max_th =  input1;
				   KADP_DBG_GetDecimalInput("[CVD Debug] white_frame_off : ", &input1);
				   cvd_test_param_t.agc_pean_white_frame_on =  input1;
				   KADP_DBG_GetDecimalInput("[CVD Debug] Select Enable/Disable print [0:disable, 1:enable] = ", &input1);
				   cvd_test_param_t.agc_pean_white_frame_off =  input1;
				   KADP_AFE_CVD_Test(&cvd_test_param_t);
				 */
				KADP_DBG_GetHexInput("[CVD Debug] x_avg_t_th : ", &input1);
				cvd_test_param_t.agc_peak_x_avg_t_th =  input1;
				KADP_DBG_GetHexInput("[CVD Debug] x_avg_s_th : ", &input1);
				cvd_test_param_t.agc_peak_x_avg_s_th =  input1;
				KADP_DBG_GetHexInput("[CVD Debug] s_staus_th : ", &input1);
				cvd_test_param_t.agc_peak_s_staus_th =  input1;
				KADP_DBG_GetHexInput("[CVD Debug] frame_max_th : ", &input1);
				cvd_test_param_t.agc_peak_white_frame_max_th =  input1;
				KADP_DBG_GetHexInput("[CVD Debug] frame_on : ", &input1);
				cvd_test_param_t.agc_peak_white_frame_on =  input1;
				KADP_DBG_GetHexInput("[CVD Debug] frame_off : ", &input1);
				cvd_test_param_t.agc_peak_white_frame_off =  input1;
				KADP_DBG_GetHexInput("[CVD Debug] noise th : ", &input1);
				cvd_test_param_t.agc_peak_noise_th =  input1;
				KADP_DBG_GetHexInput("[CVD Debug] Select Enable/Disable print [0:disable, 1:enable] = ", &input1);
				cvd_test_param_t.agc_peak_print_en =  input1;
				KADP_AFE_CVD_Test(&cvd_test_param_t);

				break;
			case 0x41:
				{
					int		binary[4], number, pow, count, byte;
					do{
						if ( KADP_DBG_GetHexInput("SYS Read Addr", &r_addr) < 0)
							break;
						if( KADP_SYS_RegReadSimple(r_addr, &rData) )
							break;

						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("                         [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", r_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");

					} while(0);
				}
				break;
			case 0x42:
				{
					int		binary[4], number, pow, count, byte;

					do{
						if ( KADP_DBG_GetHexInput("SYS Write Addr", &w_addr) < 0)
							break;

						KADP_SYS_RegReadSimple(w_addr, &rData);

						KADP_PRINTF("SYS Current Data [0x%x] 0x%x. \n",w_addr, rData);
						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("Current                  [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", w_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");

						if ( KADP_DBG_GetHexInput("INPUT SYS Write data ", &wData) < 0)
						{
							KADP_PRINTF("---- Data[0x%x] NOT Written -----\n", wData);
							break;
						}
						if (KADP_SYS_RegWriteSimple(w_addr, wData) )
							break;

						usleep(10000);

						if (KADP_SYS_RegReadSimple(w_addr, &rData) )
							break;

						for (byte =0; byte < 4; byte++)
						{
							binary[byte] = 0;
							for (number = 0; number <8; number++)
							{
								if (rData & (1 << (number + (byte * 8)) ))
								{
									pow = 1;
									for (count=0; count < number; count++)
									{
										pow *= 10;
									}
									binary[byte] += pow;
								}
							}
						}

						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("SYS         [    addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
						KADP_PRINTF("Written !!!              [  14365870] [10987654|32109876|54321098|76543210]\n");
						KADP_PRINTF("---------------------------------------------------------------------------\n");
						KADP_PRINTF("Read Data [0x%08x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", w_addr, rData, binary[3], binary[2], binary[1], binary[0]);
						KADP_PRINTF("---------------------------------------------------------------------------\n");
					} while(0);
				}
				break;
			case 0xee :
				KADP_PRINTF("	[CVD Debug] KADP_AFE_DebugPrintCtrl \n");
				AFE_DebugPrintCtrl();
				break;

			case 0xfa:
				KADP_PRINTF("	[CVD Debug] CVD Workaround On/Off \n");
				KADP_DBG_GetDecimalInput("Input Workaround Number ", &input1);
				KADP_DBG_GetDecimalInput("Input On(1)/Off(0) ", &input2);

				cvd_test_param_t.item_to_test = CVD_TEST_WORKAROUND_CONTROL;
				cvd_test_param_t.workaround_number = input1;
				cvd_test_param_t.workaround_on_off = input2;
				KADP_AFE_CVD_Test(&cvd_test_param_t);
				break;

			case 0xfb:
				KADP_PRINTF("	[CVD Debug] CVD Fake Init \n");
				KADP_AFE_CVD_Fake_Init();
				break;

			case 0xfe :
				KADP_PRINTF("	[CVD Debug] TEST!!!! \n");
				{
					UINT32 test_value;
					KADP_AFE_CVD_Get_Noise_Status(&test_value);
					KADP_PRINTF("	[CVD Debug] TEST Result [status noise : 0x%x]!!!! \n", test_value);
				}
				break;

			case 0xFF:
				KADP_PRINTF("\n	Exit \n");
				break;
			default:
				KADP_PRINTF("\n	Invalid choice! \n");
				break;
		}
	} while (nTest != 0xFF);

	return;

}


void AFE_DebugPrintCtrl(void)
{
	UINT32 printType;
	UINT32 printColor;
	UINT32 printEnable;

	KADP_PRINTF("AFE_PRINT	[%d] \n", LX_LOGM_LEVEL_NOTI);
	KADP_PRINTF("CVD_DEBUG	[%d] \n", LX_LOGM_LEVEL_TRACE);
	KADP_PRINTF("ADC_DEBUG	[%d] \n", LX_LOGM_LEVEL_TRACE+1);
	KADP_PRINTF("AFE_TRACE	[%d] \n", LX_LOGM_LEVEL_TRACE+2);
	KADP_PRINTF("AFE_ERROR	[%d] \n", LX_LOGM_LEVEL_ERROR);
	KADP_PRINTF("CVD_CSD_DEBUG	[%d] \n", LX_LOGM_LEVEL_TRACE+3);
	KADP_PRINTF("CVD_WA_DEBUG	[%d] \n", LX_LOGM_LEVEL_TRACE+4);
	KADP_PRINTF("ADC_CALIB_DEBUG	[%d] \n", LX_LOGM_LEVEL_TRACE+5);
	KADP_PRINTF("ADC_APA_DEBUG	[%d] \n", LX_LOGM_LEVEL_TRACE+6);
	KADP_PRINTF("ADC_IFM_DEBUG	[%d] \n", LX_LOGM_LEVEL_TRACE+7);
	KADP_PRINTF("CVD_THREAD_DEBUG[%d] \n", LX_LOGM_LEVEL_TRACE+8);
	KADP_PRINTF("ADC_THREAD_DEBUG[%d] \n", LX_LOGM_LEVEL_TRACE+9);
//	KADP_DBG_GetDecimalInput("DebugPrintType : ", &printType);
	if ( KADP_AFE_GetDecimalInput("DebugPrintType", &printType) < 0)
		return;

	KADP_PRINTF("DISALBE PRINT   [0] \n");
	KADP_PRINTF("Enable  PRINT	[1] \n");
//	KADP_DBG_GetDecimalInput("Debug Disable/Enable ? : ", &printEnable);
	if ( KADP_AFE_GetDecimalInput("Debug Disable/Enable ?", &printEnable) < 0)
		return;

	/*
	KADP_PRINTF(	"COLOR_NONE[0]\n");
	KADP_PRINTF("COLOR_BLACK[1]\n");
	KADP_PRINTF(	"COLOR_RED[2]\n");
	KADP_PRINTF(	"COLOR_GREEN[3]\n");
	KADP_PRINTF(	"COLOR_YELLOW[4]\n");
	KADP_PRINTF(	"COLOR_BLUE[5]\n");
	KADP_PRINTF(	"COLOR_PURPLE[6]\n");
	KADP_PRINTF(	"COLOR_CYAN[7]\n");
	KADP_PRINTF(	"COLOR_GRAY[8]\n");
	KADP_DBG_GetHexInput("DebugPrintColor : ", &printColor);
	*/
	printColor = 0;
	KADP_PRINTF(	"%s [%d][%d][%d]\n", __func__, printType, printEnable, printColor);

	KADP_AFE_DebugPrintCtrl( printType, printColor, printEnable);
	return;
}

static void KADP_AFE_CVD_Read(int param_valid, UINT32 param_addr)
{
	UINT32 read_addr, read_data;
	char 	control_char[255] = {0,};
	int		binary[4], number, pow, count, byte;


	if(param_valid != 1)
	{
		if (KADP_AFE_CVD_Input_Address(&read_addr) < 0)
			return;
	}
	else
		read_addr = param_addr;

	do {
		KADP_AFE_Vport_Reg_Read(read_addr, &read_data);

		for (byte =0; byte < 4; byte++)
		{
			binary[byte] = 0;
			for(number = 0; number <8; number++)
			{
				if(read_data & (1 << (number + (byte * 8)) ))
				{
					pow = 1;
					for(count=0; count < number; count++)
					{
						pow *= 10;
					}
					binary[byte] += pow;
				}
			}
		}

		KADP_PRINTF("=======================================================================\n");
		KADP_PRINTF("CVD         [addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
		KADP_PRINTF("                     [  14365870] [10987654|32109876|54321098|76543210]\n");
		KADP_PRINTF("-----------------------------------------------------------------------\n");
		KADP_PRINTF("Read Data [0x%04x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", read_addr, read_data, binary[3], binary[2], binary[1], binary[0]);
		KADP_PRINTF("=======================================================================\n");
		KADP_PRINTF("Type 'h' to cHange the register address \n");
		KADP_PRINTF("Type 'm' to duMp multiple register\n");
		KADP_PRINTF("Type 'q' to exit \n");
		KADP_PRINTF("Type 'ENTER' to read the register \n");
		KADP_DBG_ReadCmdString( "KADP_AFE_CVD_Read - Input Command : ", control_char, 5);

		switch(control_char[0])
		{
			case 'q':
			case 'Q':
				KADP_PRINTF ("Exit CVD Read Mode! %c\n", control_char[0]);
				break;

			case 'm':
			case 'M':
				KADP_AFE_CVD_Register_Dump(1, read_addr);
				break;

			case 'h':
			case 'H':
				if (KADP_AFE_CVD_Input_Address(&read_addr) < 0)
					return;
				break;


			default :
				break;

		}
	} while ((control_char[0] != 'q') && (control_char[0] != 'Q') );

	return;
}

static void KADP_AFE_CVD_Write(int param_valid, UINT32 param_addr)
{
	UINT32 read_addr, read_data, write_data, prev_data;
	char 	control_char[255] = {0,};
	int		binary[4], number, pow, count, byte;

	if(param_valid != 1)
	{
		if (KADP_AFE_CVD_Input_Address(&read_addr) < 0)
			return;
	}
	else
		read_addr = param_addr;

		KADP_AFE_Vport_Reg_Read(read_addr, &prev_data);
		KADP_AFE_Vport_Reg_Read(read_addr, &write_data);

	do {
		KADP_AFE_Vport_Reg_Read(read_addr, &read_data);

		for (byte =0; byte < 4; byte++)
		{
			binary[byte] = 0;
			for(number = 0; number <8; number++)
			{
				if(read_data & (1 << (number + (byte * 8)) ))
				{
					pow = 1;
					for(count=0; count < number; count++)
					{
						pow *= 10;
					}
					binary[byte] += pow;
				}
			}
		}

		KADP_PRINTF("=======================================================================\n");
		KADP_PRINTF("CVD         [addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
		KADP_PRINTF("                     [  14365870] [10987654|32109876|54321098|76543210]\n");
		KADP_PRINTF("-----------------------------------------------------------------------\n");
		KADP_PRINTF("Read Data [0x%04x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", read_addr, read_data, binary[3], binary[2], binary[1], binary[0]);
		KADP_PRINTF("=======================================================================\n");
		KADP_PRINTF("Type 'r' to Re-write Same Value \n");
		KADP_PRINTF("Type 'u' to Undo Register Write \n");
		KADP_PRINTF("Type 'h' to cHange the register address \n");
		KADP_PRINTF("Type 'm' to duMp multiple register\n");
		KADP_PRINTF("Type 'q' to exit \n");
		KADP_PRINTF("Type 'ENTER' to read the register \n");
		KADP_DBG_ReadCmdString( "KADP_AFE_CVD_Write - Input Command or Hex Data[0x00~0xFF] : 0x", control_char, 20);

		if ( (control_char[0] < '0') || ( (control_char[0] > '9') &&  (control_char[0] < 'a')) || (control_char[0] > 'f') )
		{
			KADP_PRINTF("Not valid hex data\n");

			switch(control_char[0])
			{
				case 'r':
				case 'R':
					KADP_PRINTF ("Write [0x%08x] !!!\n", write_data);
					KADP_AFE_Vport_Reg_Write(read_addr, write_data);
					break;

				case 'u':
				case 'U':
					KADP_PRINTF ("Write [0x%08x] !!!\n", prev_data);
					KADP_AFE_Vport_Reg_Write(read_addr, prev_data);
					break;

				case 'q':
				case 'Q':
					KADP_PRINTF ("Exit A-Die Write Mode! %c\n", control_char[0]);
					break;

				case 'm':
				case 'M':
					KADP_AFE_CVD_Register_Dump(1, read_addr);
					break;

				case 'h':
				case 'H':
					if (KADP_AFE_CVD_Input_Address(&read_addr) < 0)
						return;
					KADP_AFE_Vport_Reg_Read(read_addr, &prev_data);
					KADP_AFE_Vport_Reg_Read(read_addr, &write_data);
					break;


				default :
					break;

			}
		}
		else
		{

			write_data = strtoul( control_char, (char **) NULL, 16 );
			/*
			   if ( KADP_AFE_GetHexInput("[ADC Debug] Input Data to Write (0x00 ~ 0xFF):", &write_data) < 0)
			   break;
			 */

			KADP_PRINTF ("Write [0x%08x] !!!\n", write_data);
			KADP_AFE_Vport_Reg_Read(read_addr, &prev_data);
			KADP_AFE_Vport_Reg_Write(read_addr, write_data);
		}

	} while ((control_char[0] != 'q') && (control_char[0] != 'Q') );

	return;
}

static void KADP_AFE_ADC_A_Die_Read(int param_valid, UINT32 param_addr)
{
	UINT32 read_addr, read_data;
	char 	control_char[255] = {0,};
	int		byte, binary[4], number, pow, count;

	if(param_valid != 1)
	{
		if (KADP_AFE_ADC_Input_Address(&read_addr) < 0)
			return;
	}
	else
		read_addr = param_addr;

	do {
		KADP_AFE_Read_ACE_Reg(read_addr, &read_data);

		if (lx_chip_rev() >= LX_CHIP_REV(M16P, A0))	// for M16P
		{
			for (byte =0; byte < 4; byte++)
			{
				binary[byte] = 0;
				for(number = 0; number <8; number++)
				{
					if(read_data & (1 << (number + (byte * 8)) ))
					{
						pow = 1;
						for(count=0; count < number; count++)
						{
							pow *= 10;
						}
						binary[byte] += pow;
					}
				}
			}

			KADP_PRINTF("=======================================================================\n");
			KADP_PRINTF("A-Die       [addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
			KADP_PRINTF("                     [  14365870] [10987654|32109876|54321098|76543210]\n");
			KADP_PRINTF("-----------------------------------------------------------------------\n");
			KADP_PRINTF("Read Data [0x%04x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", read_addr, read_data, binary[3], binary[2], binary[1], binary[0]);
			KADP_PRINTF("=======================================================================\n");
			KADP_PRINTF("Type 'h' to cHange the register address \n");
			KADP_PRINTF("Type 'm' to duMp multiple register\n");
			KADP_PRINTF("Type 'q' to exit \n");
			KADP_PRINTF("Type 'ENTER' to read the register \n");
			KADP_DBG_ReadCmdString( "KADP_AFE_ADC_A_Die_Read - Input Command : ", control_char, 5);
		}
		else 
		{

			binary[0] = 0;
			for(number = 0; number <8; number++)
			{
				if(read_data & (1 << number))
				{
					pow = 1;
					for(count=0; count < number; count++)
					{
						pow *= 10;
					}
					binary[0] += pow;
				}
			}

			KADP_PRINTF("-----------------------------------------\n");
			KADP_PRINTF("A-Die     [slav/regi] = [data][76543210b]\n");
			KADP_PRINTF("Read Data [0x%02x/0x%02x] = [0x%2x][%08db]\n", read_addr>>16 & 0xFF, read_addr>>24 &0xFF ,read_data, binary[0]);
			KADP_PRINTF("-----------------------------------------\n");
			KADP_PRINTF("Type 'h' to cHange the register address \n");
			KADP_PRINTF("Type 'm' to duMp multiple register\n");
			KADP_PRINTF("Type 'q' to exit \n");
			KADP_PRINTF("Type 'ENTER' to read the register \n");
			KADP_DBG_ReadCmdString( "KADP_AFE_ADC_A_Die_Read - Input Command : ", control_char, 5);
		}

		switch(control_char[0])
		{
			case 'q':
			case 'Q':
				KADP_PRINTF ("Exit A-Die Read Mode! %c\n", control_char[0]);
				break;

			case 'm':
			case 'M':
				KADP_AFE_ADC_A_Die_Register_Dump(1, read_addr);
				break;

			case 'h':
			case 'H':
				if (KADP_AFE_ADC_Input_Address(&read_addr) < 0)
					return;
				break;


			default :
				break;

		}
	} while ((control_char[0] != 'q') && (control_char[0] != 'Q') );

	return;
}

static void KADP_AFE_ADC_A_Die_Write(int param_valid, UINT32 param_addr)
{
	UINT32 read_addr, read_data, write_data, prev_data;
	char 	control_char[255] = {0,};
	int		byte, binary[4], number, pow, count;

	if(param_valid != 1)
	{
		if (KADP_AFE_ADC_Input_Address(&read_addr) < 0)
			return;
	}
	else
		read_addr = param_addr;

		KADP_AFE_Read_ACE_Reg(read_addr, &prev_data);
		KADP_AFE_Read_ACE_Reg(read_addr, &write_data);

	do {
		KADP_AFE_Read_ACE_Reg(read_addr, &read_data);

		if (lx_chip_rev() >= LX_CHIP_REV(M16P, A0))	// for M16P
		{

			for (byte =0; byte < 4; byte++)
			{
				binary[byte] = 0;
				for(number = 0; number <8; number++)
				{
					if(read_data & (1 << (number + (byte * 8)) ))
					{
						pow = 1;
						for(count=0; count < number; count++)
						{
							pow *= 10;
						}
						binary[byte] += pow;
					}
				}
			}

			KADP_PRINTF("=======================================================================\n");
			KADP_PRINTF("A-Die       [addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
			KADP_PRINTF("                     [  14365870] [10987654|32109876|54321098|76543210]\n");
			KADP_PRINTF("-----------------------------------------------------------------------\n");
			KADP_PRINTF("Read Data [0x%04x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", read_addr, read_data, binary[3], binary[2], binary[1], binary[0]);
			KADP_PRINTF("=======================================================================\n");
			KADP_PRINTF("Type 'r' to Re-write Same Value \n");
			KADP_PRINTF("Type 'u' to Undo Register Write \n");
			KADP_PRINTF("Type 'h' to cHange the register address \n");
			KADP_PRINTF("Type 'm' to duMp multiple register\n");
			KADP_PRINTF("Type 'q' to exit \n");
			KADP_PRINTF("Type 'ENTER' to read the register \n");
			KADP_DBG_ReadCmdString( "KADP_AFE_ADC_A_Die_Write - Input Command or Hex Data[0x00~0xFF] : 0x", control_char, 20);
		}
		else
		{

			binary[0] = 0;
			for(number = 0; number <8; number++)
			{
				if(read_data & (1 << number))
				{
					pow = 1;
					for(count=0; count < number; count++)
					{
						pow *= 10;
					}
					binary[0] += pow;
				}
			}

			KADP_PRINTF("-----------------------------------------\n");
			KADP_PRINTF("A-Die     [slav/regi] = [data][76543210b]\n");
			KADP_PRINTF("Read Data [0x%02x/0x%02x] = [0x%02x][%08db]\n", read_addr>>16 & 0xFF, read_addr>>24 &0xFF ,read_data, binary[0]);
			KADP_PRINTF("-----------------------------------------\n");
			KADP_PRINTF("Type 'r' to Re-write Same Value \n");
			KADP_PRINTF("Type 'u' to Undo Register Write \n");
			KADP_PRINTF("Type 'h' to cHange the register address \n");
			KADP_PRINTF("Type 'm' to duMp multiple register\n");
			KADP_PRINTF("Type 'q' to exit \n");
			KADP_PRINTF("Type 'ENTER' to read the register \n");

			KADP_DBG_ReadCmdString( "KADP_AFE_ADC_A_Die_Write - Input Command or Hex Data[0x00~0xFF] : 0x", control_char, 10);
		}

		if ( (control_char[0] < '0') || ( (control_char[0] > '9') &&  (control_char[0] < 'a')) || (control_char[0] > 'f') )
		{
			KADP_PRINTF("Not valid hex data\n");

			switch(control_char[0])
			{
				case 'r':
				case 'R':
					KADP_PRINTF ("Write [0x%02x] !!!\n", write_data);
					KADP_AFE_Write_ACE_Reg(read_addr, write_data);
					break;

				case 'u':
				case 'U':
					KADP_PRINTF ("Write [0x%02x] !!!\n", prev_data);
					KADP_AFE_Write_ACE_Reg(read_addr, prev_data);
					break;

				case 'q':
				case 'Q':
					KADP_PRINTF ("Exit A-Die Write Mode! %c\n", control_char[0]);
					break;

				case 'm':
				case 'M':
					KADP_AFE_ADC_A_Die_Register_Dump(1, read_addr);
					break;

				case 'h':
				case 'H':
					if (KADP_AFE_ADC_Input_Address(&read_addr) < 0)
						return;
					KADP_AFE_Read_ACE_Reg(read_addr, &prev_data);
					KADP_AFE_Read_ACE_Reg(read_addr, &write_data);
					break;


				default :
					break;

			}
		}
		else
		{

			write_data = strtoul( control_char, (char **) NULL, 16 );
			/*
			   if ( KADP_AFE_GetHexInput("[ADC Debug] Input Data to Write (0x00 ~ 0xFF):", &write_data) < 0)
			   break;
			 */

			if (write_data > 0xffffffff)
			{
				KADP_PRINTF("Wrong Hex Data [0x%x] \n", write_data);
				break;
			}
			KADP_PRINTF ("Write [0x%02x] !!!\n", write_data);
			KADP_AFE_Read_ACE_Reg(read_addr, &prev_data);
			KADP_AFE_Write_ACE_Reg(read_addr, write_data);
		}

	} while ((control_char[0] != 'q') && (control_char[0] != 'Q') );

	return;
}

static void KADP_AFE_ADC_A_Die_Register_Dump(int param_valid, UINT32 param_addr)
{
	UINT32 slave_addr, reg_addr, read_addr, read_data, dump_addr;
	char 	control_char[255] = {0,};
	int		binary[4], number, pow, count, byte;
	int		byte_number, byte_count, word_count;

	if(param_valid != 1)
	{
		if (KADP_AFE_ADC_Input_Address(&read_addr) < 0)
			return;

		reg_addr = (read_addr >> 24) & 0xFF;
		slave_addr = (read_addr >> 16) & 0xFF;
	}
	else
	{
		read_addr = param_addr;
		reg_addr = (read_addr >> 24) & 0xFF;
		slave_addr = (read_addr >> 16) & 0xFF;
	}

	KADP_DBG_GetDecimalInput("nput Number of Bytes to read = ", &byte_number);

	if (byte_number == 0)
	{
		KADP_PRINTF("Wrong number \n");
		return;
	}


	do {
		if (lx_chip_rev() >= LX_CHIP_REV(M16P, A0))	// for M16P
		{
			KADP_PRINTF("=======================================================================\n");
			KADP_PRINTF("A-Die       [addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
			KADP_PRINTF("                     [  14365870] [10987654|32109876|54321098|76543210]\n");
			KADP_PRINTF("-----------------------------------------------------------------------\n");

			for(word_count = 0; word_count < byte_number; word_count++)
			{
				dump_addr = read_addr + (word_count * 4);
				KADP_AFE_Read_ACE_Reg(dump_addr, &read_data);

				for (byte =0; byte < 4; byte++)
				{
					binary[byte] = 0;
					for(number = 0; number <8; number++)
					{
						if(read_data & (1 << (number + (byte * 8)) ))
						{
							pow = 1;
							for(count=0; count < number; count++)
							{
								pow *= 10;
							}
							binary[byte] += pow;
						}
					}
				}
				KADP_PRINTF("Read Data [0x%04x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", dump_addr, read_data, binary[3], binary[2], binary[1], binary[0]);

			}
		}
		else
		{

		}
		KADP_PRINTF("-----------------------------------------\n");
		//control_char = getchar();
		KADP_PRINTF("Type 'q' to exit \n");
		KADP_DBG_ReadCmdString( "KADP_AFE_ADC_A_Die_Register_Dump - Input Command : ", control_char, 5);

		switch(control_char[0])
		{
			case 'q':
			case 'Q':
				KADP_PRINTF ("Exit A-Die Register Dump mode! %c\n", control_char[0]);
				break;

			default :
				break;
		}
	} while ((control_char[0] != 'q') && (control_char[0] != 'Q') );

	return;
}

static void KADP_AFE_ADC_A_Die_Register_Dump_Number(int param_valid, UINT32 param_addr, int byte_number)
{
	UINT32 slave_addr, reg_addr, read_addr, read_data, dump_addr;
	char 	control_char[255] = {0,};
	int		binary[4], number, pow, count, byte;
	int		byte_count, word_count;

	if(param_valid != 1)
	{
		if (KADP_AFE_ADC_Input_Address(&read_addr) < 0)
			return;

		reg_addr = (read_addr >> 24) & 0xFF;
		slave_addr = (read_addr >> 16) & 0xFF;
	}
	else
	{
		read_addr = param_addr;
		reg_addr = (read_addr >> 24) & 0xFF;
		slave_addr = (read_addr >> 16) & 0xFF;
	}

//	KADP_DBG_GetDecimalInput("nput Number of Bytes to read = ", &byte_number);

	if (byte_number == 0)
	{
		KADP_PRINTF("Wrong number \n");
		return;
	}


	do {
		if (lx_chip_rev() >= LX_CHIP_REV(M16P, A0))	// for M16P
		{
			KADP_PRINTF("=======================================================================\n");
			KADP_PRINTF("M16P ACE Reg[addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
			KADP_PRINTF("                     [  14365870] [10987654|32109876|54321098|76543210]\n");
			KADP_PRINTF("-----------------------------------------------------------------------\n");

			for(word_count = 0; word_count < byte_number; word_count++)
			{
				dump_addr = read_addr + (word_count * 4);
				KADP_AFE_Read_ACE_Reg(dump_addr, &read_data);

				for (byte =0; byte < 4; byte++)
				{
					binary[byte] = 0;
					for(number = 0; number <8; number++)
					{
						if(read_data & (1 << (number + (byte * 8)) ))
						{
							pow = 1;
							for(count=0; count < number; count++)
							{
								pow *= 10;
							}
							binary[byte] += pow;
						}
					}
				}
				KADP_PRINTF("Read Data [0x%04x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", dump_addr, read_data, binary[3], binary[2], binary[1], binary[0]);

			}
		}
		else
		{
		}
		KADP_PRINTF("-----------------------------------------\n");
	} while (0);

	return;
}
static int KADP_AFE_ADC_Input_Address(UINT32 * input_addr)
{
	UINT32 slave_addr, reg_addr;

	if (lx_chip_rev() >= LX_CHIP_REV(M16P, A0))	// for M16P
	{
		if ( KADP_AFE_GetHexInput("Input Offset Address:", &slave_addr) < 0)
			return -1;

		*input_addr = slave_addr;
	}
	else
	{
	}

	return 0;
}

static int KADP_AFE_CVD_Input_Address(UINT32 * input_addr)
{
	UINT32 reg_addr;
	UINT32 comb_base;
	UINT32 comb_size;
	UINT32 reg_base;
	UINT32 reg_size;

	KADP_AFE_CVD_Get_Mem_Cfg(&comb_base, &comb_size, &reg_base, &reg_size);

	KADP_PRINTF(" ======= CVD/DE Register Read Menu ========\n");
	KADP_PRINTF(" CVD Base Address is [0x%x]\n", reg_base);

	if ( KADP_AFE_GetHexInput("Input Offset Address :", &reg_addr) < 0)
		return -1;
	if(reg_addr >= reg_size)
	{
		KADP_PRINTF(" Wrong Address [ADDR:0x%08x] \n", (reg_base + reg_addr));
		return -1;
	}


	*input_addr = reg_addr;
	return 0;
}

static void KADP_AFE_CVD_Register_Dump(int param_valid, UINT32 param_addr)
{
	UINT32  read_addr, read_data, dump_addr;
	char 	control_char[255] = {0,};
	int		binary[4], number, pow, count;
	int		word_number, word_count, byte;

	if(param_valid != 1)
	{
		if (KADP_AFE_CVD_Input_Address(&read_addr) < 0)
			return;
	}
	else
	{
		read_addr = param_addr;
	}

	KADP_DBG_GetDecimalInput("nput Number of words to read = ", &word_number);

	if (word_number == 0)
	{
		KADP_PRINTF("Wrong number \n");
		return;
	}


	do {
		KADP_PRINTF("=======================================================================\n");
		KADP_PRINTF("CVD         [addr] = [  32211000] [33222222|22221111|11111100|00000000]\n");
		KADP_PRINTF("                     [  14365870] [10987654|32109876|54321098|76543210]\n");
		KADP_PRINTF("-----------------------------------------------------------------------\n");

		for(word_count = 0; word_count < word_number; word_count++)
		{
			dump_addr = read_addr + (word_count * 4);
			KADP_AFE_Vport_Reg_Read(dump_addr, &read_data);

			for (byte =0; byte < 4; byte++)
			{
				binary[byte] = 0;
				for(number = 0; number <8; number++)
				{
					if(read_data & (1 << (number + (byte * 8)) ))
					{
						pow = 1;
						for(count=0; count < number; count++)
						{
							pow *= 10;
						}
						binary[byte] += pow;
					}
				}
			}
			KADP_PRINTF("Read Data [0x%04x] = [0x%08x] [%08d|%08d|%08d|%08d]\n", dump_addr, read_data, binary[3], binary[2], binary[1], binary[0]);

		}
		KADP_PRINTF("=======================================================================\n");
		KADP_PRINTF("Type 'q' to exit \n");
		KADP_DBG_ReadCmdString( "KADP_AFE_CVD_Register_Dump - Input Command : ", control_char, 5);

		switch(control_char[0])
		{
			case 'q':
			case 'Q':
				KADP_PRINTF ("Exit A-Die Register Dump mode! %c\n", control_char[0]);
				break;

			default :
				break;
		}
	} while ((control_char[0] != 'q') && (control_char[0] != 'Q') );

	return;
}

static int KADP_AFE_ADC_A_Die_PDB_Control(UINT32 item, UINT32 on_off)
{
	UINT32 read_data, write_data;
	KADP_PRINTF("[item:%d] <= [on_off:%d]\n", item, on_off);

	if(item >6)
		return -1;

	switch(item)
	{
		case 0:	//audio
			if(on_off)	// pdb on
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x031d0000, &read_data);
				write_data = read_data | 0x40; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x031d0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x481d0000, &read_data);
				write_data = read_data | 0x0F; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x481d0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x491d0000, &read_data);
				write_data = read_data | 0xFF; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x491d0000, write_data);
			}
			else		// pdb off
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x031d0000, &read_data);
				write_data = read_data & 0xBF; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x031d0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x481d0000, &read_data);
				write_data = read_data & 0xF0; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x481d0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x491d0000, &read_data);
				write_data = read_data & 0x00; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x491d0000, write_data);
			}
			break;
		case 1:	//cvbs
			if(on_off)	// pdb on
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x01130000, &read_data);
				write_data = read_data | 0x08; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x01130000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x03130000, &read_data);
				write_data = read_data | 0x10; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x03130000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x04130000, &read_data);
				write_data = read_data | 0x1; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x04130000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x05130000, &read_data);
				write_data = read_data | 0x1; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x05130000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x0c130000, &read_data);
				write_data = read_data | 0x4; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x0c130000, write_data);
			}
			else		// pdb off
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x01130000, &read_data);
				write_data = read_data & 0xF7; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x01130000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x03130000, &read_data);
				write_data = read_data & 0xEF; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x03130000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x04130000, &read_data);
				write_data = read_data & 0xFE; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x04130000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x05130000, &read_data);
				write_data = read_data & 0xFE; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x05130000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x0c130000, &read_data);
				write_data = read_data & 0xFB; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x0c130000, write_data);
			}

			break;
		case 2: // vdac
			if(on_off)	// pdb on
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x06130000, &read_data);
				write_data = read_data | 0x1; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x06130000, write_data);
			}
			else		// pdb off
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x06130000, &read_data);
				write_data = read_data & 0xFE; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x06130000, write_data);
			}

			break;
		case 3 : // 3ch_afe
			if(on_off)	// pdb on
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x041a0000, &read_data);
				write_data = read_data | 0x08; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x041a0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x201a0000, &read_data);
				write_data = read_data | 0x3C; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x201a0000, write_data);
			}
			else		// pdb off
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x041a0000, &read_data);
				write_data = read_data & 0xF7; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x041a0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x201a0000, &read_data);
				write_data = read_data & 0xC3; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x201a0000, write_data);
			}
			break;
		case 4:	// gbb
			if(on_off)	// pdb on
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x01120000, &read_data);
				write_data = read_data | 0x2; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x01120000, write_data);
			}
			else		// pdb off
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x01120000, &read_data);
				write_data = read_data & 0xFD; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x01120000, write_data);
			}
			break;
		case 5:	// aad
			if(on_off)	// pdb on
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x001c0000, &read_data);
				write_data = read_data | 0x01; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x001c0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x071c0000, &read_data);
				write_data = read_data | 0x04; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x071c0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x0e1c0000, &read_data);
				write_data = read_data | 0x02; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x0e1c0000, write_data);
			}
			else		// pdb off
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x001c0000, &read_data);
				write_data = read_data & 0xFE; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x001c0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x071c0000, &read_data);
				write_data = read_data & 0xFB; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x071c0000, write_data);

				KADP_AFE_Read_ACE_Reg ((UINT32)0x0e1c0000, &read_data);
				write_data = read_data & 0xFD; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x0e1c0000, write_data);
			}
			break;
		case 6:	// lvds
			if(on_off)	// pdb on
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x00110000, &read_data);
				write_data = read_data | 0x1; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x00110000, write_data);
			}
			else		// pdb off
			{
				KADP_AFE_Read_ACE_Reg ((UINT32)0x00110000, &read_data);
				write_data = read_data & 0xFE; 
				KADP_AFE_Write_ACE_Reg((UINT32)0x00110000, write_data);
			}
			break;
		default :
			break;
	}

	return 0;

}

static void	KADP_AFE_ADC_A_Die_Reg_Read_with_print(UINT32 reg_addr,UINT32 *read_data)
{
	int		binary, number, pow, count;

	KADP_AFE_Read_ACE_Reg(reg_addr, read_data);

	binary = 0;
	for(number = 0; number <8; number++)
	{
		if (*read_data & (1 << number))
		{
			pow = 1;
			for(count=0; count < number; count++)
			{
				pow *= 10;
			}
			binary += pow;
		}
	}

	KADP_PRINTF("Read Data [0x%02x/0x%02x] = [0x%2x][%08db]\n", reg_addr>>16 & 0xFF, reg_addr>>24 &0xFF ,*read_data, binary);
}

static void	KADP_AFE_ADC_A_Die_PDB_Test(void)
{
	char 	control_char[255] = {0,};
	UINT32 item, on_off;
	UINT32 cvbs_pdb, vdac_pdb, afe_3ch_pdb, gbb_pdb, audio_pdb, aad_pdb, lvds_pdb;
	UINT32 audio_reg_1, audio_reg_2, audio_reg_3;
	UINT32 cvbs_reg_1, cvbs_reg_2, cvbs_reg_3, cvbs_reg_4, cvbs_reg_5;
	UINT32 vdac_reg_1, afe_3ch_reg_1, afe_3ch_reg_2;
	UINT32 aad_reg_1, aad_reg_2, aad_reg_3, gbb_reg_1, lvds_reg_1;

	do {
		//audio

		KADP_PRINTF("-----------------------------------------\n");
		KADP_PRINTF("Audio     [slav/regi] = [data][76543210b]\n");
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x031d0000, &audio_reg_1);
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x481d0000, &audio_reg_2);
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x491d0000, &audio_reg_3);

		if ( ((audio_reg_1 & 0x40) == 0x40) && ((audio_reg_2 & 0x0F) == 0x0F) && ((audio_reg_3 & 0xFF) == 0xFF) )
			audio_pdb = 2;
		else if ( ((audio_reg_1 & 0x40) == 0x0) && ((audio_reg_2 & 0x0F) == 0x0) && ((audio_reg_3 & 0xFF) == 0x0) )
			audio_pdb = 0;
		else
			audio_pdb = 1;
		//cvbs
		KADP_PRINTF("-----------------------------------------\n");
		KADP_PRINTF("CVBS      [slav/regi] = [data][76543210b]\n");
		KADP_PRINTF("-----------------------------------------\n");
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x01130000, &cvbs_reg_1);
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x03130000, &cvbs_reg_2);
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x04130000, &cvbs_reg_3);
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x05130000, &cvbs_reg_4);
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x0c130000, &cvbs_reg_5);

		if ( ((cvbs_reg_1 & 0x08) == 0x08) && ((cvbs_reg_2 & 0x10) == 0x10) && ((cvbs_reg_3 & 0x01) == 0x01)  && ((cvbs_reg_4 & 0x01) == 0x01) )
			cvbs_pdb = 2;
		else if ( ((cvbs_reg_1 & 0x08) == 0x00) && ((cvbs_reg_2 & 0x10) == 0x00) && ((cvbs_reg_3 & 0x01) == 0x00)  && ((cvbs_reg_4 & 0x01) == 0x00) )
			cvbs_pdb = 0;
		else
			cvbs_pdb = 1;
		// vdac
		KADP_PRINTF("-----------------------------------------\n");
		KADP_PRINTF("VDAC      [slav/regi] = [data][76543210b]\n");
		KADP_PRINTF("-----------------------------------------\n");
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x06130000, &vdac_reg_1);

		if ( (vdac_reg_1 & 0x01) == 0x01) 
			vdac_pdb = 2;
		else if ( ((vdac_reg_1 & 0x01) == 0x00) )
			vdac_pdb = 0;
		else
			vdac_pdb = 1;
		// 3ch_afe
		KADP_PRINTF("-----------------------------------------\n");
		KADP_PRINTF("3CH       [slav/regi] = [data][76543210b]\n");
		KADP_PRINTF("-----------------------------------------\n");
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x041a0000, &afe_3ch_reg_1);
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x201a0000, &afe_3ch_reg_2);

		if ( ((afe_3ch_reg_1 & 0x08) == 0x08) && ((afe_3ch_reg_2 & 0x3C) == 0x3C)  )
			afe_3ch_pdb = 2;
		else if ( ((afe_3ch_reg_1 & 0x08) == 0x0) && ((afe_3ch_reg_2 & 0x3C) == 0x0)  )
			afe_3ch_pdb = 0;
		else
			afe_3ch_pdb = 1;
		// gbb
		KADP_PRINTF("-----------------------------------------\n");
		KADP_PRINTF("AAD       [slav/regi] = [data][76543210b]\n");
		KADP_PRINTF("-----------------------------------------\n");
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x01120000, &gbb_reg_1);
		KADP_PRINTF("-----------------------------------------\n");

		if ( (gbb_reg_1 & 0x02) == 0x02) 
			gbb_pdb = 2;
		else if ( ((gbb_reg_1 & 0x02) == 0x00) )
			gbb_pdb = 0;
		else
			gbb_pdb = 1;

		// aad
		KADP_PRINTF("-----------------------------------------\n");
		KADP_PRINTF("AAD       [slav/regi] = [data][76543210b]\n");
		KADP_PRINTF("-----------------------------------------\n");
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x001c0000, &aad_reg_1);
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x071c0000, &aad_reg_2);
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x0e1c0000, &aad_reg_3);
		KADP_PRINTF("-----------------------------------------\n");

		if ( ((aad_reg_1 & 0x01) == 0x01) && ((aad_reg_2 & 0x04) == 0x04) && ((aad_reg_3 & 0x02) == 0x02) )
			aad_pdb = 2;
		else if ( ((aad_reg_1 & 0x01) == 0x0) && ((aad_reg_2 & 0x04) == 0x0) && ((aad_reg_3 & 0x02) == 0x0) )
			aad_pdb = 0;
		else
			aad_pdb = 1;

		//lvds
		KADP_PRINTF("-----------------------------------------\n");
		KADP_PRINTF("LVDS      [slav/regi] = [data][76543210b]\n");
		KADP_PRINTF("-----------------------------------------\n");
		KADP_AFE_ADC_A_Die_Reg_Read_with_print ((UINT32)0x00110000, &lvds_reg_1);
		KADP_PRINTF("-----------------------------------------\n");

		if ( (lvds_reg_1 & 0x01) == 0x01) 
			lvds_pdb = 2;
		else if ( ((lvds_reg_1 & 0x01) == 0x00) )
			lvds_pdb = 0;
		else
			lvds_pdb = 1;

		KADP_PRINTF("###############################################################\n");
		KADP_PRINTF("### A-Die PDB STATUS 										###\n");
		KADP_PRINTF("### [ITEM] 	= [0:power down, 2:power On, 1:partially on]	###\n");
		KADP_PRINTF("### [0:Audio] = [%d]											###\n", audio_pdb );
		KADP_PRINTF("### [1:CVBS] 	= [%d]											###\n", cvbs_pdb );
		KADP_PRINTF("### [2:VDAC] 	= [%d]											###\n", vdac_pdb );
		KADP_PRINTF("### [3:3CH] 	= [%d]											###\n", afe_3ch_pdb );
		KADP_PRINTF("### [4:GBB] 	= [%d]											###\n", gbb_pdb );
		KADP_PRINTF("### [5:AAD] 	= [%d]											###\n", aad_pdb );
		KADP_PRINTF("### [6:LVDS] 	= [%d]											###\n", lvds_pdb );
		KADP_PRINTF("###############################################################\n");

		KADP_PRINTF("Type '0'~'6' to modify pdb status [ 0:Audio, 1:CVBS, 2:VDAC, 3:3CH, 4:GBB, 5:AAD, 6:LVDS\n");
		KADP_PRINTF("Type 'u' to power Up all pdbs\n");
		KADP_PRINTF("Type 'd' to power Down all pdbs\n");
		KADP_PRINTF("Type 'q' to exit \n");
		KADP_PRINTF("Type 'ENTER' to read the register \n");
		KADP_DBG_ReadCmdString( "KADP_AFE_ADC_A_Die_Read - Input Command : ", control_char, 5);

		switch(control_char[0])
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6' :
					item = control_char[0] - 48;
					KADP_DBG_ReadCmdString( "On[1]/Off[0] : ", control_char, 5);
					on_off = control_char[0] - 48;

					if(on_off > 1)
						break;

					KADP_AFE_ADC_A_Die_PDB_Control(item, on_off);
				break;
			case 'u':
			case 'U':
				{
					int loop;
					for(loop =0 ; loop <7;loop++)
					{
						KADP_AFE_ADC_A_Die_PDB_Control(loop, 1);
					}
				}
				break;

			case 'd':
			case 'D':
				{
					int loop;
					for(loop =0 ; loop <7;loop++)
					{
						KADP_AFE_ADC_A_Die_PDB_Control(loop, 0);
					}
				}
				break;

			case 'q':
			case 'Q':
				KADP_PRINTF ("Exit A-Die Read Mode! %c\n", control_char[0]);
				break;

			default :
				break;

		}
	} while ((control_char[0] != 'q') && (control_char[0] != 'Q') );

	return ;
}
