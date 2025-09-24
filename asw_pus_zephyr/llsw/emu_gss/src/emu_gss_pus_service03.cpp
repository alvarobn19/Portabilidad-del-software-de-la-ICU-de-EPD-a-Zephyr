/*
 * emu_sc_pus_service3.cpp
 *
 *
 *  Created on: Apr 27, 2024
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

#include <stdio.h>

#if defined(__has_include)
#if __has_include(<zephyr/sys/printk.h>)
#include <zephyr/sys/printk.h>
#elif __has_include(<sys/printk.h>)
#include <sys/printk.h>
#endif
#else
#include <sys/printk.h>
#endif

#include "public/emu_gss_v1.h"
#include "emu_gss/emu_gss_sys_data_pool.h"

#include "public/pus_services_iface_v1.h"

extern "C" {
#include "public/pus_service3.h"
#include "pus_service3/aux_pus_service3_utils.h"
}

#ifdef EDROOMBP_DEF
#define TC_3_5_APPDATA_LENGTH 2      // [SID_H][SID_L]
#define TC_3_6_APPDATA_LENGTH 2      // [SID_H][SID_L]
#define TC_3_31_APPDATA_LENGTH 3     // [SID_H][SID_L][rate]
#else
#define TC_3_5_APPDATA_LENGTH 3      // [N][SID]
#define TC_3_6_APPDATA_LENGTH 3      // [N][SID]
#define TC_3_31_APPDATA_LENGTH 4     // [N][SID][rate]
#endif

EmuGSS_TCProgram3_5::EmuGSS_TCProgram3_5(uint32_t uniTime2YK, const char *brief,
                uint16_t sid) :
                EmuGSS_TCProgram(uniTime2YK, 3, 5,
                TC_3_5_APPDATA_LENGTH, brief) {

        // printk("[HK] ctor prog_FT_0050_step_0 (enable SID=11)\n");

        mSID = sid;
        NewProgram(this);
        // printk ("SID = %u\n", mSID);
}

void EmuGSS_TCProgram3_5::BuildTCAppData(tc_mem_descriptor_t &tc_descriptor) {

#ifdef EDROOMBP_DEF
        // printk("[PROG3,5] SID=%u -> bytes %02x %02x\n", mSID,
        //        (uint8_t)((mSID >> 8) & 0xFF), (uint8_t)(mSID & 0xFF));
        SetNextUInt8((mSID >> 8) & 0xFF);
        SetNextUInt8(mSID & 0xFF);
#else
        uint8_t n = 1;
        // printk("[PROG3,5] N=%u, SID=%u -> bytes %02x %02x\n",
        //        n, mSID, (uint8_t)((mSID >> 8) & 0xFF), (uint8_t)(mSID & 0xFF));
        SetNextUInt8(n);
        SetNextUInt16(mSID);
#endif

}

EmuGSS_TCProgram3_6::EmuGSS_TCProgram3_6(uint32_t uniTime2YK, const char *brief,
		uint16_t sid) :
		EmuGSS_TCProgram(uniTime2YK, 3, 6,
		TC_3_6_APPDATA_LENGTH, brief) {

	mSID = sid;
        // printk ("SID = %u\n", mSID);
	NewProgram(this);
}

void EmuGSS_TCProgram3_6::BuildTCAppData(tc_mem_descriptor_t &tc_descriptor) {

#ifdef EDROOMBP_DEF
        // printk("[PROG3,6] SID=%u -> bytes %02x %02x\n", mSID,
        //        (uint8_t)((mSID >> 8) & 0xFF), (uint8_t)(mSID & 0xFF));
        SetNextUInt8((mSID >> 8) & 0xFF);
        SetNextUInt8(mSID & 0xFF);
#else
        uint8_t n = 1;                              // <-- N = número de SIDs
        SetNextUInt8(n);
        // printk("[PROG3,6] N=%u, SID=%u -> bytes %02x %02x\n",
        //        n, mSID, static_cast<uint8_t>((mSID >> 8) & 0xFF),
        //        static_cast<uint8_t>(mSID & 0xFF));
        SetNextUInt16(mSID);
#endif

}

EmuGSS_TCProgram3_31::EmuGSS_TCProgram3_31(uint32_t uniTime2YK,
		const char *brief, uint16_t sid, uint8_t collecInterval) :
		EmuGSS_TCProgram(uniTime2YK, 3, 31,
		TC_3_31_APPDATA_LENGTH, brief) {

	mSID = sid;
	mCollectInterval = collecInterval;

	NewProgram(this);
}

void EmuGSS_TCProgram3_31::BuildTCAppData(tc_mem_descriptor_t &tc_descriptor) {

#ifdef EDROOMBP_DEF
        // printk("[PROG3,31] SID=%u, rate=%u -> bytes %02x %02x %02x\n",
        //        mSID, mCollectInterval,
        //        (uint8_t)((mSID >> 8) & 0xFF), (uint8_t)(mSID & 0xFF),
        //        mCollectInterval);
        SetNextUInt8((mSID >> 8) & 0xFF);
        SetNextUInt8(mSID & 0xFF);
        SetNextUInt8(mCollectInterval);
#else
        uint8_t n = 1;
        // printk("[PROG3,31] N=%u, SID=%u, rate=%u -> bytes %02x %02x %02x\n",
        //        n, mSID, mCollectInterval,
        //        (uint8_t)((mSID >> 8) & 0xFF), (uint8_t)(mSID & 0xFF),
        //        mCollectInterval);
        SetNextUInt8(n);
        SetNextUInt16(mSID);
        SetNextUInt8(mCollectInterval);
#endif

}

//********************************************************+
void EmuGSS_ShowServ3TM(const struct tm_mem_descriptor *pTMDescriptor) {

	GSSServ3TMHandler serv3TMHandler(pTMDescriptor->p_tm_bytes);
	serv3TMHandler.ShowTM();

}

GSSServ3TMHandler::GSSServ3TMHandler(const uint8_t *pTMBytes) :
		GSSTMHandler(pTMBytes) {

}

void GSSServ3TMHandler::ShowTM() {

	switch (mDFHeader.subtype) {
	case (25):
		ShowTM_3_25();
		break;

	}
}

void GSSServ3TMHandler::ShowTM_3_25() {

	uint16_t sid = GetNextUInt16AppDataField();

	printf(" SID %d Param Values", sid);

	ShowTM_3_25_SID(sid);

}


void GSSServ3TMHandler::ShowTM_3_25_SID(uint16_t SID) {

	uint8_t index;
	error_code_t error = pus_service3_get_SID_index(SID, &index);

	if (!error) {

		int num_params = HKConfig[index].num_of_params;

		for (uint8_t i = 0; i < num_params; i++) {

			data_pool_item_type_t data_type;
			data_pool_item_t data_item_raw;
			uint16_t pid = HKConfig[index].param_IDs[i];

			data_type = sys_data_pool_item_type(pid);

			error_code_t error = 0;
			switch (data_type) {

			case (uint32_item_type):
				data_item_raw.uint32_data = GetNextUInt32AppDataField();

				break;

			case (uint8_item_type):
				data_item_raw.uint8_data = GetNextUInt8AppDataField();

				break;
			default:
				error = 2;

			}

			if (0 == error) {

				if (0 == i) {
					printf(" {");
					GSSSysDataPool::ShowPIDValue(pid, data_item_raw);

				} else if ((num_params - 1) == i) {
					printf(",");
					GSSSysDataPool::ShowPIDValue(pid, data_item_raw);
					printf("}");

				} else {

					printf(",");
					GSSSysDataPool::ShowPIDValue(pid, data_item_raw);
				}
			}

		}
	}

}

