/*
 * aux_pus_service3_exec_tc.c
 *
 *  Created on: Oct 24, 2024
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


#include <public/pus_sys_data_pool.h>
#include "public/adc_drv.h"

#include "public/ccsds_pus.h"
#include "public/crc.h"

#include "public/pus_tm_handler.h"
#include "public/pus_tc_handler.h"
#include <sys/byteorder.h>
#if defined(__has_include)
#if __has_include(<zephyr/kernel.h>)
#include <zephyr/kernel.h>
#else
#include <kernel.h>
#endif
#else
#include <kernel.h>
#endif

#include "../../pus_service01/include/public/pus_service1.h"
#include "public/pus_service3.h"
#include "pus_service3/aux_pus_service3_tx_tm.h"

extern struct k_mutex g_hkcfg_lock;
extern struct k_delayed_work g_hk_delayed_work[PUS_SERVICE3_MAX_NUM_OF_SIDS];

extern struct k_timer g_hk_timer[PUS_SERVICE3_MAX_NUM_OF_SIDS];

/* Find the HK configuration entry for a given SID. */
static HK_config_t *find_entry_by_sid(uint16_t sid)
{
        HK_config_t *candidate = NULL;
        for (size_t i = 0; i < PUS_SERVICE3_MAX_NUM_OF_SIDS; ++i) {
                HK_config_t *e = &HKConfig[i];
                if (e->SID != sid)
                        continue;

                if (e->status != SIDConfigUnused || e->interval > 0 ||
                    e->num_of_params > 0)
                        return e;

                if (!candidate)
                        candidate = e;
        }
        return candidate;
}

/* Enable or disable a SID using its SID value, not array index. */
static void set_sid_status(uint16_t sid, bool enable)
{
        k_mutex_lock(&g_hkcfg_lock, K_FOREVER);
        HK_config_t *e = find_entry_by_sid(sid);
        if (e) {
                int idx = (int)(e - HKConfig);

                // printk("[S3] %s sid=%u idx=%d HKConfig=%p entry@%p before status=%u period=%u params=%u\n",
                //        enable ? "ENABLE" : "DISABLE", sid, idx, (void *)HKConfig,
                //        (void *)e, e->status, e->interval, e->num_of_params);

                if (!enable) {
                        /* Por seguridad: cancela cualquier mecánica externa si la hubiera */
                        k_delayed_work_cancel(&g_hk_delayed_work[idx]);
                        k_timer_stop(&g_hk_timer[idx]);
                }

                e->status = enable ? SIDEnabled : SIDDisabled;
                e->interval_ctrl = 0;

                // printk("[S3] SID%u entry@%p after=%d\n", sid, e, (int)e->status);
        } else {
                // printk("[S3] SID=%u not found\n", sid);
        }
        k_mutex_unlock(&g_hkcfg_lock);
}

/* Update the generation period for a SID. */
static void update_sid_period(uint16_t sid, uint8_t new_period)
{
        HK_config_t *e = find_entry_by_sid(sid);
        if (e) {
                // printk("[S3] SID%u entry@%p before period=%u\n", sid, e,
                //        e->interval);
                k_mutex_lock(&g_hkcfg_lock, K_FOREVER);
                e->interval_ctrl = 0;
                e->interval = new_period;
                k_mutex_unlock(&g_hkcfg_lock);
                // printk("[S3] SID%u entry@%p after period=%u\n", sid, e,
                //        e->interval);
        } else {
                // printk("[S3] SID=%u not found\n", sid);
        }
}

static void exec_enable_disable(tc_handler_t *ptc_handler, bool enable) {

        const uint8_t *app = ptc_handler->raw_tc_mem_descriptor.p_tc_bytes +
                             UAH_PUS_TC_APP_DATA_OFFSET;
        uint16_t app_len =
                ptc_handler->raw_tc_mem_descriptor.tc_num_bytes -
                UAH_PUS_TC_APP_DATA_OFFSET;

        // printk("[S3] app_len=%u data=%02x %02x %02x\n", app_len,
        //        app_len > 0 ? app[0] : 0, app_len > 1 ? app[1] : 0,
        //        app_len > 2 ? app[2] : 0);

#ifdef EDROOMBP_DEF
        uint16_t expected_len = enable ? TC_3_5_APPDATA_LENGTH
                                       : TC_3_6_APPDATA_LENGTH;
        uint16_t usable_len = app_len;

        /* Zephyr app_len includes CRC bytes */
        if (usable_len == expected_len + 2) {
                usable_len = expected_len;
        }

        if (usable_len != expected_len) {
                pus_service1_tx_TM_1_4_short_pack_length(ptc_handler);
                return;
        }

        uint16_t sid = ((uint16_t)app[0] << 8) | app[1];
        set_sid_status(sid, enable);
        pus_service1_tx_TM_1_7(ptc_handler);
        return;
#else
        error_code_t error;
        uint8_t N;

        error = tc_handler_get_uint8_appdata_field(ptc_handler, &N);
        if (error) {
                pus_service1_tx_TM_1_4_short_pack_length(ptc_handler);
                return;
        }

        if (0 == N) {
                while (tc_handler_is_valid_next_appdata_field(ptc_handler,
                                sizeof(uint16_t))) {
                        uint16_t sid_be;
                        error = tc_handler_get_uint16_appdata_field(ptc_handler,
                                        &sid_be);
                        if (error) {
                                pus_service1_tx_TM_1_4_short_pack_length(
                                                ptc_handler);
                                return;
                        }
                        uint16_t SID = sys_be16_to_cpu(sid_be);
                        set_sid_status(SID, enable);
                }
        } else {
                for (uint8_t i = 0; i < N; ++i) {
                        byte_t sid_bytes[2];
                        error = tc_handler_get_byte_array_appdata_field(
                                        ptc_handler, sid_bytes, 2);
                        if (error) {
                                pus_service1_tx_TM_1_4_short_pack_length(
                                                ptc_handler);
                                return;
                        }
                        uint16_t SID = sys_get_be16(sid_bytes);
                        set_sid_status(SID, enable);
                }
        }

        pus_service1_tx_TM_1_7(ptc_handler);
#endif
}

void pus_service3_exec_TC_3_5(tc_handler_t *ptc_handler) {

        // printk("[S3] %s HKConfig=%p &HKConfig[0]=%p\n", __func__,
        //        HKConfig, &HKConfig[0]);
        pus_service1_tx_TM_1_3(ptc_handler);
        exec_enable_disable(ptc_handler, true);

}

void pus_service3_exec_TC_3_6(tc_handler_t *ptc_handler) {

        // printk("[S3] %s HKConfig=%p &HKConfig[0]=%p\n", __func__,
        //        HKConfig, &HKConfig[0]);
        pus_service1_tx_TM_1_3(ptc_handler);
        exec_enable_disable(ptc_handler, false);

}

void pus_service3_exec_TC_3_31(tc_handler_t *ptc_handler) {

        const uint8_t *app = ptc_handler->raw_tc_mem_descriptor.p_tc_bytes +
                             UAH_PUS_TC_APP_DATA_OFFSET;
        uint16_t app_len =
                ptc_handler->raw_tc_mem_descriptor.tc_num_bytes -
                UAH_PUS_TC_APP_DATA_OFFSET;

        // printk("[S3] app_len=%u data=%02x %02x %02x\n", app_len,
        //        app_len > 0 ? app[0] : 0, app_len > 1 ? app[1] : 0,
        //        app_len > 2 ? app[2] : 0);

#ifdef EDROOMBP_DEF
        uint16_t expected_len = TC_3_31_APPDATA_LENGTH; // SID_H SID_L rate
        uint16_t usable_len = app_len;

        if (usable_len == expected_len + 2) {
                usable_len = expected_len; // ignore CRC
        }

        if (usable_len != expected_len) {
                pus_service1_tx_TM_1_4_short_pack_length(ptc_handler);
                return;
        }

        uint16_t sid = ((uint16_t)app[0] << 8) | app[1];
        uint8_t rate = app[2];

        HK_config_t *e = find_entry_by_sid(sid);
        if (e) {
                pus_service1_tx_TM_1_3(ptc_handler);
                update_sid_period(sid, rate);
                pus_service1_tx_TM_1_7(ptc_handler);
        } else {
                pus_service1_tx_TM_1_4_SID_not_valid(ptc_handler, sid);
        }

        return;
#else
        error_code_t error;

        uint8_t N;
        byte_t sid_bytes[sizeof(uint16_t)];
        uint16_t SID;
        uint8_t collection_interval;

        // TC -> N
        error = tc_handler_get_uint8_appdata_field(ptc_handler, &N);

        // TC -> SID
        error += tc_handler_get_byte_array_appdata_field(ptc_handler,
                        sid_bytes, sizeof(sid_bytes));
        SID = sys_get_be16(sid_bytes);

        // TC -> collection interval
        error += tc_handler_get_uint8_appdata_field(ptc_handler,
                        &collection_interval);

        if (error) {

                // error in pack length
                pus_service1_tx_TM_1_4_short_pack_length(ptc_handler);

        } else {

                if (1 != N) {

                        error = pus_service1_tx_TM_1_4_num_of_instr_not_valid(ptc_handler,
                                        N);
                } else {

                        HK_config_t *e = find_entry_by_sid(SID);
                        if (e) {
                                pus_service1_tx_TM_1_3(ptc_handler);
                                update_sid_period(SID, collection_interval);
                                pus_service1_tx_TM_1_7(ptc_handler);
                        } else {
                                /* If not valid, SID */
                                pus_service1_tx_TM_1_4_SID_not_valid(ptc_handler, SID);
                        }
                }
        }
#endif
}

