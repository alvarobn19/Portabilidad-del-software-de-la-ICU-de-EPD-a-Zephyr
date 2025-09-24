// Cabeceras Zephyr para la versión 2.7.4
#include <zephyr.h>
#include <sys/printk.h>

// --- EDROOM / ASW ---
#include <public/edroom_glue.h>

#include <public/uah_asw_iface_v1.h>
#include <public/cctcmanager_iface_v1.h>
#include <public/cchk_fdirmng_iface_v1.h>
#include <public/ccbkgtcexec_iface_v1.h>
#include <public/emu_gss_v1.h>
#include <public/emu_hw_timecode_drv_v1.h>
#include <public/tc_queue_drv.h>
#include "service_libraries/emu_tc_programming/emu_tc_programming_st03.h"
#include "public/hk_fdir_executor.h"
#include "public/bkg_executor.h"

CEDROOMSystemDeployment systemDeployment;

static void inject_smoke_tc(void) {
    EmuGSS_TCProgram17_1 ping(EmuHwTimeCodeGetCurrentOBT(), "smoke");
    EmuGSS_SendProgrammedTCs();
    //printk("[SMOKE] TC enqueued via EmuGSS_SendProgrammedTCs\n");
}

extern "C" int uah_asw_mmesp_project(void)
{
    //printk("\n[ASW] Arrancando despliegue EDROOM...\n");

    // Vida PERSISTENTE: se construyen una sola vez y no se destruyen.
    static UAH_ASW      comp1(1, 13, EDROOMprioNormal, 32768,
                             systemDeployment.GetComp1Memory());
    static CCTCManager  comp2(2, 10, EDROOMprioHigh,   32768,
                             systemDeployment.GetComp2Memory());
    static CCHK_FDIRMng comp3(3, 13, EDROOMprioNormal, 32768,
                             systemDeployment.GetComp3Memory());
    static CCBKGTCExec  comp4(4, 10, EDROOMprioNormal, 32768,
                             systemDeployment.GetComp4Memory());

    systemDeployment.Config(&comp1, &comp2, &comp3, &comp4);
    systemDeployment.Start();

    start_hk_fdir_executor();
    start_bkg_executor();

    //emu_tc_register_sid11_program();

    for (int i = 0; i < 4; ++i) {
        EmuGSS_SendProgrammedTCs();
        k_msleep(10);
    }

    //k_sleep(K_SECONDS(1));
    //inject_smoke_tc();

    //printk("[ASW] Despliegue lanzado. uah_asw_mmesp_project() finaliza.\n");
    return 0;
}
