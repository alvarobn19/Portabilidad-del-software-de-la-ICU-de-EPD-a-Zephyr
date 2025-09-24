#ifndef FCDTCHandlerH
#define FCDTCHandlerH

#include "public/config.h"
#include "public/basic_types.h"
#include "public/ccsds_pus.h"
#include "public/tmtc_dyn_mem.h"
#include "public/pus_tc_handler.h"

#include "public/cdtcmemdescriptor_iface_v1.h"
#include "public/cdtcacceptreport_iface_v1.h"
#include "public/cdtcexecctrl_iface_v1.h"


class CDTCHandler {

	friend class CDEvAction;


protected:

        tc_handler_t mTCHandler;
        bool        mIsEvAction;
        bool        mBkgEnqueued;

public:

	//!Constructor
	CDTCHandler();

        //!Build From Descriptor
        void BuildFromDescriptor(CDTCMemDescriptor &descriptor);

        //!Build From Raw Buffer
        void BuildFromRaw(const uint8_t* bytes, size_t len);

        //!Do TC Acceptation
        CDTCAcceptReport DoAcceptation();

        //!Mng TC Rejection
        void MngTCRejection(CDTCAcceptReport & acceptReport);

	//!Mng TC Accetation
	void MngTCAcceptation();

	//!Set Execution Control 
	CDTCExecCtrl GetExecCtrl();

        //!Exec the telecommand
        void ExecTC();

        //!Get TC type
        uint8_t GetType() const;

        //!Get TC subtype
        uint8_t GetSubtype() const;

        //!Get pointer to underlying tc_handler_t
        tc_handler_t *GetTCHandler();

        //!Background queue status helpers
        void SetBkgEnqueued(bool enqueued);
        bool IsBkgEnqueued() const;


};

#endif
