#ifndef PUBLIC_PUS_VERIFICATION_H
#define PUBLIC_PUS_VERIFICATION_H

#include "public/basic_types.h"
#include "public/pus_tc_handler.h"

error_code_t pus_verif_tc_execution_start(tc_handler_t *ptc_handler);
error_code_t pus_verif_tc_execution_completed(tc_handler_t *ptc_handler, error_code_t result);
error_code_t pus_verif_tc_execution_failed(tc_handler_t *ptc_handler, error_code_t error);

#endif /* PUBLIC_PUS_VERIFICATION_H */
