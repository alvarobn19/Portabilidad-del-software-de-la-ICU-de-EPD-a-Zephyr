/*
 * asw_pus_service3.c
 *
 *  Created on: Oct 21, 2024
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

#include "public/pus_service3.h"

#include <public/pus_sys_data_pool.h>
#include "public/adc_drv.h"

#include "public/ccsds_pus.h"
#include "public/crc.h"
#include "public/pus_tm_handler.h"
#include "public/pus_service1.h"
#include "pus_service3/aux_pus_service3_exec_tc.h"
#include "pus_service3/aux_pus_service3_tx_tm.h"
#if defined(__has_include)
#if __has_include(<zephyr/kernel.h>)
#include <zephyr/kernel.h>
#else
#include <kernel.h>
#endif
#else
#include <kernel.h>
#endif
#if defined(__has_include)
#if __has_include(<zephyr/sys/printk.h>)
#include <zephyr/sys/printk.h>
#else
#include <sys/printk.h>
#endif
#else
#include <sys/printk.h>
#endif



HK_config_t HKConfig[PUS_SERVICE3_MAX_NUM_OF_SIDS] = {
		{ SIDEnabled, 0, 10, 0, 5, { 0, 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDDisabled, 10, 4, 0, 3, { 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDDisabled, 11, 5, 0, 2, { 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
		{ SIDConfigUnused, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

};

K_MUTEX_DEFINE(g_hkcfg_lock);

/* Zephyr 2.7.4 usa k_delayed_work */
struct k_delayed_work g_hk_delayed_work[PUS_SERVICE3_MAX_NUM_OF_SIDS];
struct k_timer        g_hk_timer[PUS_SERVICE3_MAX_NUM_OF_SIDS];

/* Handlers "dummy" para inicializar; no encolamos nada aquí */
static void hk_dummy_work(struct k_work *w) { ARG_UNUSED(w); }
static void hk_dummy_timer(struct k_timer *t) { ARG_UNUSED(t); }

/* Inicializa las estructuras para que cancel/stop sean seguras */
static int pus_service3_init(const struct device *unused)
{
        ARG_UNUSED(unused);
        for (int i = 0; i < PUS_SERVICE3_MAX_NUM_OF_SIDS; ++i) {
                k_delayed_work_init(&g_hk_delayed_work[i], hk_dummy_work);
                k_timer_init(&g_hk_timer[i], hk_dummy_timer, NULL);
        }
        return 0;
}

/* Ejecuta init en el arranque */
SYS_INIT(pus_service3_init, APPLICATION, 50);


void pus_service3_do_HK()
{
        k_mutex_lock(&g_hkcfg_lock, K_FOREVER);

        for (size_t i = 0; i < PUS_SERVICE3_MAX_NUM_OF_SIDS; ++i) {
                HK_config_t *e = &HKConfig[i];

#if CONFIG_PRINTK


         /*printk("[CHK] HKConfig@%p SID%u entry@%p before: status=%u cnt=%u period=%u\n",
                       (void *)HKConfig, e->SID, (void *)e, e->status,
                       e->interval_ctrl, e->interval);*/
#endif

                if (e->status == SIDEnabled && e->interval > 0) {
                        if (++e->interval_ctrl >= e->interval) {
                                e->interval_ctrl = 0;

                                /* Re-check bajo el mismo lock: si sigue habilitado, emitimos */
                                if (e->status == SIDEnabled) {
                                        pus_service3_tx_TM_3_25(i);
                                }
                        }
                }
        }

        k_mutex_unlock(&g_hkcfg_lock);
}

void pus_service3_exec_tc(tc_handler_t *ptc_handler) {
#if CONFIG_PRINTK

        // printk("### S3 router, subtype=%u ###\n", ptc_handler->tc_df_header.subtype);
#endif

        switch (ptc_handler->tc_df_header.subtype) {
        case 5:
                pus_service3_exec_TC_3_5(ptc_handler);
                break;
        case 6:
#if CONFIG_PRINTK

                // printk("### S3.6 handler entry ###\n");
#endif
                pus_service3_exec_TC_3_6(ptc_handler);
                break;
        case 31:
                pus_service3_exec_TC_3_31(ptc_handler);
                break;
        default:
#if CONFIG_PRINTK

                // printk("### S3 unknown subtype=%u ###\n",
                //        ptc_handler->tc_df_header.subtype);
#endif
                break;
        }
}


