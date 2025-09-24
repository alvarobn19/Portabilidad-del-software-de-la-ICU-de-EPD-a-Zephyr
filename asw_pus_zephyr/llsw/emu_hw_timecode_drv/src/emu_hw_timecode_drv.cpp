/*
 * emu_timecode_hw.cpp
 *
 *  Created on: Jan 5, 2024
 *      Author: opolo70
 */

#include "public/emu_hw_timecode_drv_v1.h"
#include <zephyr.h>
#include "public/pus_services_iface_v1.h"
#ifdef GSS_EMULATION
#include "public/emu_gss_v1.h"
#endif

uint32_t EMU_TIME_CODE_HW_NextOBTSecns=OBT_AFTER_POWER_ON + 1;
uint32_t EMU_TIME_CODE_HW_CurrentOBTSecns=OBT_AFTER_POWER_ON;

static struct k_work_delayable obt_tick_work;

static void obt_tick_work_handler(struct k_work *work)
{
        EmuHwTimeCodePassSecond();
        (void)k_work_schedule(&obt_tick_work, K_SECONDS(1));
}

void EmuHwTimeCodeStartUp(){

#ifdef GSS_EMULATION
        EmuGSS_PrintCurrentOBT();
#endif

        k_work_init_delayable(&obt_tick_work, obt_tick_work_handler);
        (void)k_work_schedule(&obt_tick_work, K_SECONDS(1));
        //EmuHwTimeCodePassSecond();
}

void EmuHwTimeCodePassSecond(){

        EMU_TIME_CODE_HW_CurrentOBTSecns =EMU_TIME_CODE_HW_NextOBTSecns;
        EMU_TIME_CODE_HW_NextOBTSecns++;

#ifdef GSS_EMULATION
        EmuGSS_PassSecond();
#endif

        pus_services_update_params();
        pus_service4_update_all_stats();
        pus_service3_do_HK();
        pus_services_do_FDIR();

}

extern "C" void EmuHwTimeCodeSetNextOBT(uint32_t nextOBTSecns){

	EMU_TIME_CODE_HW_NextOBTSecns=nextOBTSecns;
}

extern "C" uint32_t EmuHwTimeCodeGetCurrentOBT(){

	return EMU_TIME_CODE_HW_CurrentOBTSecns;

}

extern "C" uint16_t EmuHwTimeCodeGetCurrentFineTimeOBT(){

	return 0;

}
