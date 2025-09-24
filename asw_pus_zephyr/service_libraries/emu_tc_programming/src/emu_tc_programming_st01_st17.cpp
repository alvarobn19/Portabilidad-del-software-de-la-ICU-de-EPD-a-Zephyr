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

#define OBT_START_FT_ST01_ST17 OBT_AFTER_POWER_ON

#define FT_UAH_ASW_SERV_17_0010
#define FT_UAH_ASW_SERV_1_DISABLE_1_7_0020
#define FT_UAH_ASW_SERV_1_REJ_TC_0030
//#define FT_SOLO_EPD_TCContainment_0035



#ifdef FT_UAH_ASW_SERV_17_0010


#define FT_UAH_ASW_0010_TIME_step0 (OBT_START_FT_ST01_ST17 + 3)


EmuGSS_TCProgram17_1 prog_FT_0010_step_0(FT_UAH_ASW_0010_TIME_step0,
					"FT_UAH_ASW_SERV_17_0010 step 0, Connection Test");


#endif

#ifdef FT_UAH_ASW_SERV_1_DISABLE_1_7_0020


//TODO 08 Program TC[17,1] at POWER_ON + 8
#define FT_UAH_ASW_0020_TIME_step0 (OBT_START_FT_ST01_ST17 + 8)

EmuGSS_TCProgram17_1_DISABLE_1_7 prog_FT_0020_step_0(FT_UAH_ASW_0020_TIME_step0,
					"FT_UAH_ASW_SERV_1_DISABLE_1_7_0020 step 0, Disable TM[1,7]");

#endif


#ifdef FT_UAH_ASW_SERV_1_REJ_TC_0030

//TODO 09 Program TC[17,1] at POWER_ON + 10
#define FT_UAH_ASW_0030_TIME_step0 (OBT_START_FT_ST01_ST17 + 10)

EmuGSS_TCProgram17_3 prog_FT_0030_step_0(FT_UAH_ASW_0030_TIME_step0,
		"FT_UAH_ASW_SERV_1_0030 step 0, Rejected TC",0xFF);

#endif

#ifdef FT_SOLO_EPD_TCContainment_0035

//TODO 10 Program TC[17,1] at POWER_ON + 12

#define FT_UAH_ASW_0035_TIME_step0 (OBT_START_FT_ST01_ST17 + 12)

EmuGSS_TCProgram17_1 prog_FT_0035_step_0(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 0");

EmuGSS_TCProgram17_1 prog_FT_0035_step_1(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 1");

EmuGSS_TCProgram17_1 prog_FT_0035_step_2(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 2");

EmuGSS_TCProgram17_1 prog_FT_0035_step_3(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 3");

EmuGSS_TCProgram17_1 prog_FT_0035_step_4(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 4");

EmuGSS_TCProgram17_1 prog_FT_0035_step_5(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 5");

EmuGSS_TCProgram17_1 prog_FT_0035_step_6(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 6");

EmuGSS_TCProgram17_1 prog_FT_0035_step_7(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 7");

EmuGSS_TCProgram17_1 prog_FT_0035_step_8(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 8");

EmuGSS_TCProgram17_1 prog_FT_0035_step_9(FT_UAH_ASW_0035_TIME_step0,
					"FT_SOLO_EPD_TCContainment_0035 step 9");


#endif





