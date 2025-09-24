#include "public/pus_verification.h"
#include "public/pus_service1.h"
#include "pus_service1/aux_pus_service1_x_utils.h"

#ifndef PUS_NO_ERROR
#define PUS_NO_ERROR 0
#endif

error_code_t pus_verif_tc_execution_start(tc_handler_t *ptc_handler) {
    if (tc_handler_is_tc_start_exec_ack_enabled(ptc_handler)) {
        return pus_service1_tx_TM_1_3(ptc_handler);
    }
    return PUS_NO_ERROR;
}

error_code_t pus_verif_tc_execution_completed(tc_handler_t *ptc_handler, error_code_t result) {
    if (tc_handler_is_tc_completion_exec_ack_enabled(ptc_handler)) {
        if (PUS_NO_ERROR == result) {
            return pus_service1_tx_TM_1_7(ptc_handler);
        } else {
            return pus_service1_tx_TM_1_X_no_failure_data(ptc_handler,
                                                         TCVerifStageExecCompletion,
                                                         result);
        }
    }
    return PUS_NO_ERROR;
}

error_code_t pus_verif_tc_execution_failed(tc_handler_t *ptc_handler, error_code_t error) {
    if (tc_handler_is_tc_start_exec_ack_enabled(ptc_handler)) {
        return pus_service1_tx_TM_1_X_no_failure_data(ptc_handler,
                                                     TCVerifStageExecStart,
                                                     error);
    }
    return PUS_NO_ERROR;
}
