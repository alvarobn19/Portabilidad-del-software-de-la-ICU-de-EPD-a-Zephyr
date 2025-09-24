/*
 * emu_tc_programming.cpp
 *
 *  Created on: Oct 26, 2024
 *      Author: Oscar Rodriguez Polo
 */

/****************************************************************************
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,USA.
 *
 *
 ****************************************************************************/

#include <public/emu_hw_timecode_drv_v1.h>
#include <public/emu_sc_channel_drv_v1.h>
#include <public/emu_gss_v1.h>

#define OBT_START_FT_ST19 (OBT_AFTER_POWER_ON + 180)

#define FT_UAH_ASW_Event_Action_0130

//#define FT_UAH_ASW_Ev_Disabled_0140

//#define FT_UAH_ASW_Ev_Action_Disabled_0150

//#define FT_UAH_ASW_Event_Action_Delete_Action_Error_0160

#ifdef FT_UAH_ASW_Event_Action_0130

#define FT_0130_TIME_step0 (OBT_START_FT_ST19 + 2)
#define FT_0130_TIME_step1 (OBT_START_FT_ST19 + 4)
#define FT_0130_TIME_step2 (OBT_START_FT_ST19 + 6)
#define FT_0130_TIME_step3 (OBT_START_FT_ST19 + 8)
#define FT_0130_TIME_step4 (OBT_START_FT_ST19 + 10)
#define FT_0130_TIME_step5 (OBT_START_FT_ST19 + 12)

EmuGSS_TCProgram20_3_uint8 prog_FT_0130_step_0A(FT_0130_TIME_step0,
		"FT_UAH_ASW_Event_Action_0130 step 0, Set PID 15 to 5",15,5);

EmuGSS_TCProgram12_5_Limit_UINT8 prog_FT_0130_step_1(FT_0130_TIME_step1,
		"FT_UAH_ASW_Event_Action_0130 step 1, Config PMODID 0 for monitoring PID 15",
		0, 15, 1, 2, 1, 0x4001, 20, 0x4002);

EmuGSS_TCProgram19_1_Action_2_1 prog_FT_0130_step_2(FT_0130_TIME_step2,
					"FT_UAH_ASW_Event_Action_0130 step 2, 0x4002 Action is Device  Off",
					0x4002, DeviceCommandOff);


EmuGSS_TCProgram19_4 prog_FT_0130_step_3(FT_0130_TIME_step3,
					"FT_UAH_ASWEvent_Action_0130 step 3, 0x4002 Action enabled",
					0x4002);


EmuGSS_TCProgram12_1 prog_FT_0130_step_4(FT_0130_TIME_step4,
		"FT_UAH_ASW_Event_Action_0130 step 4, Enable Monitoring PMODID 0", 0);



EmuGSS_TCProgram20_3_uint8 prog_FT_0130_step_5(FT_0130_TIME_step5,
		"FT_UAH_ASW_Monitoring_0130 step 5, Update PID 15 to 99",15,99);


#endif



