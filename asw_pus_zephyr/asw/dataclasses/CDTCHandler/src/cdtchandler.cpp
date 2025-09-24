/*
 * cdtchandler.cpp
 *
 *  Created on: Dec 29, 2023
 *      Author: opolo70
 */

#include <public/pus_services_iface_v1.h>
#include "public/cdtchandler.h"
#include "public/serialize.h"
#include "public/crc.h"
#include "public/sc_channel_drv_v1.h"

#include <zephyr.h>
#include <sys/printk.h>
#include <errno.h>

extern "C" {
#include "public/tmtc_dyn_mem.h"   // for tc_mem_descriptor_t
#include "public/pus_tc_handler.h"
}

extern "C" {
#include "public/pus_service3.h"
}

#include "public/cdtcexecctrl_iface_v1.h"
#include "pus_service1/aux_pus_service1_x_utils.h"
#include "public/hk_fdir_executor.h"
#include "public/bkg_executor.h"
#include "public/pus_service1.h"
#include "public/device_drv.h"

K_MUTEX_DEFINE(g_exec_mutex);

CDTCHandler::CDTCHandler() {

        mTCHandler.raw_tc_mem_descriptor.p_tc_bytes = NULL;
        mTCHandler.raw_tc_mem_descriptor.tc_num_bytes = 0;

        mIsEvAction = false;
        mBkgEnqueued = false;

}

void CDTCHandler::BuildFromDescriptor(CDTCMemDescriptor &descriptor) {

        tc_handler_build_from_descriptor(&mTCHandler, descriptor.mTCMemDescriptor);
        mIsEvAction = false;
        mBkgEnqueued = false;

}

void CDTCHandler::BuildFromRaw(const uint8_t* bytes, size_t len) {
        tc_mem_descriptor_t d;
        d.p_tc_bytes   = (uint8_t*)bytes;
        d.tc_num_bytes = (uint16_t)len;
        tc_handler_build_from_descriptor(&mTCHandler, d);
        mIsEvAction = false;
        mBkgEnqueued = false;
}

CDTCAcceptReport CDTCHandler::DoAcceptation() {

	CDTCAcceptReport acceptReport;
	acceptReport.mAcceptReport = pus_service1_tc_acceptation(&mTCHandler);

	return acceptReport;
}

void CDTCHandler::MngTCRejection(CDTCAcceptReport &acceptReport) {

	pus_service1_tx_TM_1_2(&mTCHandler, acceptReport.mAcceptReport);

	tc_handler_free_memory(&mTCHandler);
}

//Complete TC Aceptation
void CDTCHandler::MngTCAcceptation() {
        pus_service1_tx_TM_1_1(&mTCHandler);  // aceptación TM [1,1]

        CDTCExecCtrl ctrl = GetExecCtrl();

        if (ctrl.IsHK_FDIRTC()) {
                enqueue_hk_fdir_tc(this);
        } else if (ctrl.IsBKGTC()) {
                enqueue_bkg_tc(this);
                mBkgEnqueued = true;
        } else if (ctrl.IsPrioTC()) {
                // No hay ejecutor de prioridad aún
                // printk("[ROUTER] unsupported ExecCtrl for TC type %u\n",
                //        mTCHandler.tc_df_header.type);
                tc_handler_free_memory(&mTCHandler);
        } else {
                // printk("[ROUTER] unsupported ExecCtrl for TC type %u\n",
                //        mTCHandler.tc_df_header.type);
                tc_handler_free_memory(&mTCHandler);
        }
}

void CDTCHandler::SetBkgEnqueued(bool enqueued) {
        mBkgEnqueued = enqueued;
}

bool CDTCHandler::IsBkgEnqueued() const {
        return mBkgEnqueued;
}

CDTCExecCtrl CDTCHandler::GetExecCtrl() {

	//Get TC type
	uint8_t type = mTCHandler.tc_df_header.type;

	CDTCExecCtrl execCtrl;
	switch (type) {

	//TODO 10 Set ST[19] TCs as ExecCtrlHK_FDIRTC and ST[02] as prioTC
	case (3):
	case (4):
	case (5):
	case (12):
	case (19):
		execCtrl.mExecCtrl = ExecCtrlHK_FDIRTC;
		break;
	case (02):
	case (17):
		execCtrl.mExecCtrl = ExecCtrlPrioTC;
		break;
	case (20):
		execCtrl.mExecCtrl = ExecCtrlBKGTC;
		break;
	case (128):
		execCtrl.mExecCtrl = ExecCtrlReboot;
		break;

	default:
		execCtrl.mExecCtrl = ExecCtrlBKGTC;
		break;

	}

	return execCtrl;
}

void CDTCHandler::ExecTC() {
        k_mutex_lock(&g_exec_mutex, K_FOREVER);

        error_code_t error;
        error = tc_handler_start_up_execution(&mTCHandler);

        if (error) {
                pus_service1_tx_TM_1_X_no_failure_data(&mTCHandler,
                                TCVerifStageAcceptation, error);
                pus_service1_tx_TM_1_X_no_failure_data(&mTCHandler,
                                TCVerifStageExecStart, error);
                tc_handler_free_memory(&mTCHandler);
                k_mutex_unlock(&g_exec_mutex);
                return;
        }

       // printk("[ROUTER++] Enter ExecTC: type=%u ss=%u\n",
       //                mTCHandler.tc_df_header.type,
       //                mTCHandler.tc_df_header.subtype);

       uint8_t type = mTCHandler.tc_df_header.type;

       // printk("[ROUTER++] pre-dispatch: type=%u\n", type);

       switch (type) {

       //TODO 11 Add TC[02,X] &  TC[19,X] execution

       case (2):
               pus_service2_exec_tc(&mTCHandler);
               break;

       case (3):
               pus_service3_exec_tc(&mTCHandler);
               break;
       case (4):
               // printk("[ROUTER] dispatch type=4\n");
               pus_service4_exec_tc(&mTCHandler);
               break;
       case (5):
               pus_service5_exec_tc(&mTCHandler);
               break;
       case (12):
               pus_service12_exec_tc(&mTCHandler);
               break;

       case (17):
               pus_service17_exec_tc(&mTCHandler);
               break;

       case (19):
               pus_service19_exec_tc(&mTCHandler);
               break;

       case (20):
               pus_service20_exec_tc(&mTCHandler);
               break;

       case (128):
               pus_service128_exec_tc(&mTCHandler);
               break;

       default:

               break;

       }

       tc_handler_free_memory(&mTCHandler);
       k_mutex_unlock(&g_exec_mutex);
}

uint8_t CDTCHandler::GetType() const {
        return mTCHandler.tc_df_header.type;
}

uint8_t CDTCHandler::GetSubtype() const {
        return mTCHandler.tc_df_header.subtype;
}

tc_handler_t *CDTCHandler::GetTCHandler() {
        return &mTCHandler;
}

