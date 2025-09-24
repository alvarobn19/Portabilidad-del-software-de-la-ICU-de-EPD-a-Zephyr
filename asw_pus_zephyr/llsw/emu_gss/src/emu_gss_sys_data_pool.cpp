/*
 * emu_gss_sys_data_pool.cpp
 *
 *  Created on: Nov 7, 2024
 *      Author: opolo70
 */

#include "emu_gss/emu_gss_sys_data_pool.h"

float SID0_Slope[5] = { 20.0 / 4096, 20.0 / 4096, 20.0 / 4096, 20.0 / 4096,
		10.0 / 4096 };

float SID0_Intercept[5] = { -10, -10, -10, -10, -10 };

void GSSSysDataPool::ShowPIDValue(uint16_t pid,
		data_pool_item_t data_item_raw) {

	data_pool_item_type_t data_type;
	data_type = sys_data_pool_item_type(pid);

	switch (data_type) {

	case (uint32_item_type):

		if ((0 <= pid) && (pid < 5)) {

			float pid_value = data_item_raw.uint32_data * SID0_Slope[pid]
					+ SID0_Intercept[pid];

			printf("%.2f", pid_value);

		} else {

			printf("0x%X", data_item_raw.uint32_data);

		}

		break;

	case (uint8_item_type):

		if (pid > 15) {
			printf("0x%x", data_item_raw.uint8_data);
		}else{
			printf("%d", data_item_raw.uint8_data);
		}
		break;
	default:
		break;
	}

}

