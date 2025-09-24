/*
 * cdevaction.cpp
 *
 *  Created on: Nov 12, 2024
 *      Author: opolo70
 */

#include "public/cdevaction_iface_v1.h"
#include "public/pus_service19.h"
extern "C" {
#include "public/tmtc_dyn_mem.h"   // tmtc_pool_free
}

void CDEvAction::ExtractEvActionFromQueue(){

        mTCHandler.mTCHandler.raw_tc_mem_descriptor.p_tc_bytes=tmtc_pool_alloc();

        pus_service19_extract_next_ev_action(&mTCHandler.mTCHandler);

        tc_handler_build_from_descriptor(&mTCHandler.mTCHandler,
                        mTCHandler.mTCHandler.raw_tc_mem_descriptor);

        mTCHandler.mIsEvAction = true;


}


extern "C" void cdevaction_exec_next_from_queue(void)
{
        CDEvAction ev;
        ev.ExtractEvActionFromQueue();
        // Ejecutar solo si existe una Event Action realmente encolada
        tc_mem_descriptor_t *desc =
                &ev.mTCHandler.GetTCHandler()->raw_tc_mem_descriptor;
        if (desc->tc_num_bytes > 0) {
                ev.mTCHandler.ExecTC();
        } else {
                // Nada que ejecutar: liberar pool y salir sin ruido
                if (desc->p_tc_bytes) {
                        tmtc_pool_free(desc->p_tc_bytes);
                        desc->p_tc_bytes = NULL;
                }
        }
}


