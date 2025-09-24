

#include <public/uah_asw_iface_v1.h>





// ******************************* CONSTRUCTOR ************************************



UAH_ASW::UAH_ASW(TEDROOMComponentID id,
		TEDROOMUInt32 roomNumMaxMens,
		TEDROOMPriority roomtaskPrio,
		TEDROOMStackSizeType roomStack,
		CEDROOMMemory *pActorMemory ) : 

		CEDROOMComponent(id,EDROOMprioMINIMUM+1,roomNumMaxMens,
				roomtaskPrio,roomStack, pActorMemory ),

		// *********** Timing service access point *********

		EDROOMtimingSAP(this, 3,&pActorMemory->TimingMemory),

		// *******************  Timers  ********************

		Timer(&EDROOMtimingSAP, 2 ),

		// ***************	Top State  *****************

		edroomTopState(*this)


{


}


//**************************************************************************
//**************************   PUBLIC METHODS  *****************************



//************************** EDROOMConfig **********************************


int UAH_ASW::EDROOMConfig()
{


 return 0; 
}



//************************** EDROOMStart **********************************

int UAH_ASW::EDROOMStart()
{


	//****************** Timing Task Start*****************

	EDROOMtimingSAP.Start();

	//***************** CEDROOMComponent::EDROOMStart*********

	CEDROOMComponent::EDROOMStart(); 	// Call to EDROOMStart method of CEDROOMComponent

	return 0;
}


//**************************************************************************
//***************************    PROTECTED METHODS *************************



//*****************************  EDROOMBehaviour ***************************



void UAH_ASW::EDROOMBehaviour()
{

	edroomTopState.EDROOMInit();
	edroomTopState.EDROOMBehaviour();

}




//********************* ComponentIsFinished **********************************


#ifdef _EDROOM_SYSTEM_CLOSE

bool UAH_ASW::EDROOMIsComponentFinished()
{


	return ( TCManager.EDROOMIsComponentFinished() && HK_FDIRMng.EDROOMIsComponentFinished() && BKGTCExec.EDROOMIsComponentFinished() && CEDROOMComponent::EDROOMIsComponentFinished());

}

#endif


//****************** EDROOMMemory::SetMemory *******************************

void UAH_ASW::CEDROOMMemory::SetMemory(TEDROOMUInt32 numMessages_ ,
		CEDROOMMessage * MessagesMem_,
		bool * MessagesMemMarks_,
		TEDROOMUInt32 numberOfNodes_,
		CEDROOMQueue::CQueueNode * QueueNodesMem_,
		bool * QueueNodesMemMarks_)
{

		CEDROOMComponentMemory::SetMemory( numMessages_,MessagesMem_, MessagesMemMarks_,
			numberOfNodes_,QueueNodesMem_, QueueNodesMemMarks_, QueueHeads);

		TimingMemory.SetMemory(3,TimerInf,&TimerInfMarks[0],TimeOutMsgs,&TimeOutMsgsMarks[0]);


}

