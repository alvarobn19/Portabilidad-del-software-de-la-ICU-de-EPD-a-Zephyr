/*
 * asw_pus_service3.h
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


#ifndef SERVICE_LIBRARIES_ASW_PUS_SERVICES_ASW_PUS_SERVICE3_INCLUDE_ASW_PUS_SERVICE3_H_
#define SERVICE_LIBRARIES_ASW_PUS_SERVICES_ASW_PUS_SERVICE3_INCLUDE_ASW_PUS_SERVICE3_H_

#include "public/config.h"
#include "public/basic_types.h"
#include <stdint.h>

#include "public/pus_tc_handler.h"
#include "public/pus_tm_handler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PUS_SERVICE3_MAX_NUM_OF_SIDS 16

#define PUS_SERVICE3_MAX_NUM_OF_PID_PER_SID 12

#ifdef EDROOMBP_DEF
#define TC_3_5_APPDATA_LENGTH   2  /* [SID_H][SID_L] */
#define TC_3_6_APPDATA_LENGTH   2  /* [SID_H][SID_L] */
#define TC_3_31_APPDATA_LENGTH  3  /* [SID_H][SID_L][rate] */
#else
#define TC_3_5_APPDATA_LENGTH   3  /* [N][SID] */
#define TC_3_6_APPDATA_LENGTH   3  /* [N][SID] */
#define TC_3_31_APPDATA_LENGTH  4  /* [N][SID][rate] */
#endif

/*
 * Housekeeping (HK) SID status values. The scheduler logic expects
 * 0 to mean "enabled" and 1 to mean "disabled".  Explicitly assign
 * the numeric values to avoid any ambiguity.
 */
enum {
    SIDEnabled      = 0, /* SID is active */
    SIDDisabled     = 1, /* SID is inactive */
    SIDConfigUnused = 2  /* Entry not configured */
};

typedef uint8_t SID_config_status_t;

struct HK_config{
  SID_config_status_t status;   /* now exactly 1 byte */
  uint16_t SID;
  uint8_t interval;
  uint8_t interval_ctrl;
  uint8_t num_of_params;
  uint16_t param_IDs[PUS_SERVICE3_MAX_NUM_OF_PID_PER_SID];
};

typedef struct HK_config HK_config_t;

/* Compile-time checks to detect unexpected padding or field sizes */
#if defined(__cplusplus)
static_assert(sizeof(((HK_config_t *)0)->status) == sizeof(uint8_t),
              "HK_config_t status size unexpected");
static_assert(sizeof(HK_config_t) >=
                     (1 + 2 + 1 + 1 + 1 +
                      2 * PUS_SERVICE3_MAX_NUM_OF_PID_PER_SID),
              "HK_config_t size unexpected");
#else
_Static_assert(sizeof(((HK_config_t *)0)->status) == sizeof(uint8_t),
               "HK_config_t status size unexpected");
_Static_assert(sizeof(HK_config_t) >=
                       (1 + 2 + 1 + 1 + 1 +
                        2 * PUS_SERVICE3_MAX_NUM_OF_PID_PER_SID),
               "HK_config_t size unexpected");
#endif

extern HK_config_t HKConfig[PUS_SERVICE3_MAX_NUM_OF_SIDS];

/**
 * \brief Do HK, managing the generation of TM[3,25]
 */
void pus_service3_do_HK();

/**
 * \brief executes a TC[3,X] telecommand
 * \param ptc_handler pointer to the tc handler
 */
void pus_service3_exec_tc(tc_handler_t *ptc_handler);
void pus_service3_exec_TC_3_5(tc_handler_t *ptc_handler);
void pus_service3_exec_TC_3_6(tc_handler_t *ptc_handler);
void pus_service3_exec_TC_3_31(tc_handler_t *ptc_handler);

#ifdef __cplusplus
}
#endif


#endif /* SERVICE_LIBRARIES_ASW_PUS_SERVICES_ASW_PUS_SERVICE3_INCLUDE_ASW_PUS_SERVICE3_H_ */
