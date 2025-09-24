/*
 * pus_service4.c
 *
 *  Created on: Nov 20, 2024
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

#include "../../pus_service04/include/public/pus_service4.h"

#include "public/pus_sys_data_pool.h"

#include "public/ccsds_pus.h"
#include "public/crc.h"
#include "public/pus_tm_handler.h"
#include "../../pus_service01/include/public/pus_service1.h"
#include "../../pus_service04/include/pus_service4/aux_pus_service4_exec_tc.h"
#include "../../pus_service04/include/pus_service4/aux_pus_service4_utils.h"

#if defined(__has_include)
#if __has_include(<zephyr/sys/printk.h>)
#include <zephyr/sys/printk.h>
#else
#include <sys/printk.h>
#endif
#else
#include <sys/printk.h>
#endif

void pus_service4_startup(){

        /* Reset stats window for all defined and valid PIDs */
        /* Coherente con Svc3: arrancamos limpiando ventanas como en HK */
        for(uint8_t i=0; i < PUS_SERVICE4_MAX_NUM_OF_STATS; i++){

                //stats defined and PID is valid
                if(pus_service4_is_stats_index_defined(i)){

                        pus_service4_reset_stats_index(i);
                }


        }



}

void pus_service4_update_all_stats() {
        //Check HK SID generation (igual que Svc3)
	for (int i = 0; i < PUS_SERVICE4_MAX_NUM_OF_STATS; i++) {

		if (pus_service4_is_stats_index_defined(i)) {

			pus_service4_update_PID_stats(i);

		}
	}
}

void pus_service4_exec_tc(tc_handler_t *ptc_handler) {
       // printk("*** S4 DISPATCH *** type=%u ss=%u\n",
       //        ptc_handler->tc_df_header.type,
       //        ptc_handler->tc_df_header.subtype);


        switch (ptc_handler->tc_df_header.subtype) {

        case (1):
                // printk("*** S4 CASE 1 ***\n");
                pus_service4_exec_TC_4_1(ptc_handler);
                break;
        case (6):
                // printk("*** S4 CASE 6 ***\n");
                pus_service4_exec_TC_4_6(ptc_handler);
                break;
        case (7):
                // printk("*** S4 CASE 7 ***\n");
                pus_service4_exec_TC_4_7(ptc_handler);
                break;

        default:
                break;

        }

}

