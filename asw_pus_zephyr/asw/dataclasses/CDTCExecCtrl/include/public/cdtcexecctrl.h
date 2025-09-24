#ifndef FCDTCExecCtrlH
#define FCDTCExecCtrlH

enum TTCExecCtrl {
	ExecCtrlPrioTC, ExecCtrlReboot, ExecCtrlHK_FDIRTC, ExecCtrlBKGTC
};

class CDTCExecCtrl{
	
	friend class CDTCHandler;

	protected:
	
		enum TTCExecCtrl mExecCtrl;

	public:

	/**
	 * \brief Check if it is a reboot telecommand
	 *
	 * \return true if it is a reboot telecommand
	 *
	 */
	bool IsRebootTC(){return (ExecCtrlReboot==mExecCtrl);}

	/**
	 * \brief Check if it is a priority telecommand
	 *
	 * \return true if it is a priority telecommand
	 *
	 */
	bool IsPrioTC(){return (ExecCtrlPrioTC==mExecCtrl);}

	/**
	 * \brief Check if is a HK_FDIR telecommand
	 *
	 * \return true if it is a HK_FDIR telecommand
	 *
	 */
	bool IsHK_FDIRTC(){return (ExecCtrlHK_FDIRTC==mExecCtrl);}


	/**
	 * \brief Check if is a Background telecommand
	 *
	 * \return true if it is a Background telecommand
	 *
	 */
	bool IsBKGTC() {return (ExecCtrlBKGTC==mExecCtrl);};
	
	
};
#endif
