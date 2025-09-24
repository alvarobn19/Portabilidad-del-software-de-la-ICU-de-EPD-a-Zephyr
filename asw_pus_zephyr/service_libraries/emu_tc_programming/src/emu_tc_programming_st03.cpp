/*
 * emu_tc_programming.cpp
 *
 *  Created on: Jan 13, 2017
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

#define OBT_START_FT_ST03 (OBT_AFTER_POWER_ON + 15)

#define FT_UAH_ASW_ICU_SERV_3_DISABLE_SID0_0040
#define FT_UAH_ASW_ICU_SERV_3_ENABLE_SID11_0050
#define FT_UAH_ASW_ICU_SERV_3_UPDATE_SID10_INTERVAL_0060

#ifdef FT_UAH_ASW_ICU_SERV_3_DISABLE_SID0_0040

#define FT_UAH_ASW_ICU_SERV_3_0040_TIME (OBT_START_FT_ST03 + 20)

//TODO 11 Use EmuGSS_TCProgram3_6 to disable SID0 at FT_UAH_ASW_ICU_SERV_3_0040_TIME
EmuGSS_TCProgram3_6 prog_FT_0040_step_0(FT_UAH_ASW_ICU_SERV_3_0040_TIME,
		"FT_UAH_ASW_ICU_SERV_3_0040 step 0, Disable SID 0 ",0);

#endif

#ifdef FT_UAH_ASW_ICU_SERV_3_ENABLE_SID11_0050


//TODO 12 Use EmuGSS_TCProgram3_5 to enable SID11 at OBT_START_FT_ST03 + 30
#define FT_UAH_ASW_ICU_SERV_3_0050_TIME (OBT_START_FT_ST03 + 30)

EmuGSS_TCProgram3_5 prog_FT_0050_step_0(FT_UAH_ASW_ICU_SERV_3_0050_TIME,
		"FT_UAH_ASW_ICU_SERV_3_0050 step 0, Enable SID 11 ",11);
		
#endif

#ifdef FT_UAH_ASW_ICU_SERV_3_UPDATE_SID10_INTERVAL_0060


#define FT_UAH_ASW_ICU_SERV_3_0060_TIME_step0 (OBT_START_FT_ST03 + 40)


EmuGSS_TCProgram3_31 prog_FT_0060_step_0(FT_UAH_ASW_ICU_SERV_3_0060_TIME_step0,
		"FT_UAH_ASW_ICU_SERV_3_0060 step 0, Update SID 10 Rate to 2",10,2);


#endif






