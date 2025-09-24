/*
 * \file	edroombp.h created for edroombp project
 * \brief
 *
 * \author	Alberto Carrasco Gallardo, <acarrasco@srg.aut.uah.es>
 * \author	Óscar Rodríguez Polo, <o.rodriguez@uah.es>
 * \author	Alvaro González Garcia, <alvaro.gonzalezgarci@uah.es>
  *
 * \internal
 * Created: 	23-mar-2011
 * Compiler: 	gcc/g++
 * Company:  Space Research Group, Universidad de Alcal�.
 * Copyright: Copryright (c) 2011, Alberto Carrasco Gallardo
 *
 * For further information, please visit http://srg.aut.uah.es
 *
 * This software is provided under the terms of the GNU General Public v2
 * Licence. A full copy of the GNU GPL is provided in the file COPYING
 * found in the development root of ERCOS-RT project.
 *
 */
#ifndef __EDROOM__BP_H__
#define __EDROOM__BP_H__

#include <zephyr.h>
#include <sys/printk.h>
#include <stdio.h>
#include <stdint.h>
#include <public/basic_types.h>
#include <public/edroomsl_types.h>
#include <public/config.h>
#include <public/edroombp_zephyr_wrapper.h>

#define EDROOMPrioWRAPPER (EDROOMprioIDLE + 1)


/*	EDROOM priorities	*/
enum TEDROOMPriority{
    EDROOMprioURGENT = 1,
    EDROOMprioVeryHigh,
    EDROOMprioHigh,
    EDROOMprioNormal,
    EDROOMprioLow,
    EDROOMprioVeryLow,
    EDROOMprioMINIMUM,
    EDROOMprioIDLE,
};

/**	Structure for time handling	*/
typedef struct
{
   uint32_t seconds;
   uint32_t microseconds;
}EDROOMTimeSpec;

#define EDROOM_PRIO_MINIMUM EDROOMprioIDLE

typedef unsigned TEDROOMStackSizeType;

#define EDROOMClockTicksType uint32_t 



typedef unsigned char EDROOMByte;
typedef void Pr_TaskRV_t;           /*return value of the test task*/
typedef uintptr_t Pr_TaskP_t;			/*parameter passed to the task*/
typedef void Pr_IRQHandler_RetType; /*	return type for IRQ handlers	*/
typedef k_thread_stack_t Pr_Task_Stack_t;

extern const size_t g_edroom_pool_count;


//********************************************************
//********************  Pr_Kernel  ***********************
//********************************************************

class Pr_Kernel
{
public:

    Pr_Kernel();
    ~Pr_Kernel(){};

    /**
     * Kernel start method
     *
     * This methos may be invoqued to start the kernel threads.
     */
    void Start();
};


//********************************************************
//********************  Pr_Semaphore  ********************
//********************************************************

class Pr_Time;

/**
 * \class Pr_Semaphore
 * \brief	semaphore class declaration
 */
class Pr_Semaphore
{

public:
    /**
     * \todo Default constructor.
     *
     * \param _value :   This parameter is the semaphore counter value.
     *
     * The default semaphore value is 1.
     */
    Pr_Semaphore(uint32_t _value = 0);
   
    /**
     * Semaphore Signal
     */
    void Signal();

     /**
     * Semaphore Wait
     */
    void Wait();

    /**
     * Semaphore conditional wait.
     *
     * The method catch the semaphore if it is possible. In any other case the
     * thread is not blocked.
     *
     * \return The method return 1 in case of the semaphore is catched. In any
     * other case the method returns 0.
     */
    int WaitCond();

    /**
     * \brief	the method try to catch the semaphore during a period of time.
     *	\param	interval	interval to wait
     * \return true if catched, false otherwise.
     */
    bool WaitTimed(const Pr_Time & interval);

    /**
     * Destructor
     */
    ~Pr_Semaphore(){
    }
};

//********************************************************
//********************  Pr_SemaphoreBin  *****************
//********************************************************

/**
 * \class Pr_SemaphoreBin
 * \brief	Binary semaphore class declaration
 */
class Pr_SemaphoreBin : public Pr_Semaphore
{

protected:
    /** This attribute is the class semaphore   */
    struct k_sem sem;

public:
    /**
     * Default Constructor.
     */
    Pr_SemaphoreBin(uint32_t _value = 0);

    Pr_SemaphoreBin(const Pr_SemaphoreBin&) = delete;
    Pr_SemaphoreBin& operator=(const Pr_SemaphoreBin&) = delete;
    Pr_SemaphoreBin(Pr_SemaphoreBin&&) = delete;
    Pr_SemaphoreBin& operator=(Pr_SemaphoreBin&&) = delete;

    /**
     * Semaphore Signal
     */
    void Signal();

    /**
     * Semaphore Wait
     */
    void Wait();

    /**
     * Semaphore conditional wait.
     *
     * The method catch the semaphore if it is possible. In any other case the
     * thread is not blocked.
     *
     * \return The method return 1 in case of the semaphore is catched. In any
     * other case the method returns 0.
     */
    int32_t WaitCond();

    /**
	 * \brief	the method try to catch the semaphore during a period of time.
	 *	\param	interval	interval to wait
	 *
	 * \return true if catched, false otherwise.
	 */
    bool WaitTimed(const Pr_Time & interval);

    int countSem();

    /**
     * Default Destructor.
     *
     * The destructor destroys the semaphore.
     */
    ~Pr_SemaphoreBin();
};

//********************************************************
//********************  Pr_SemaphoreRec  *****************
//********************************************************

/**
 * \class Pr_SemaphoreRec
 * \brief	Resource semaphore class declaration
 */

#define Pr_Mutex Pr_SemaphoreRec

class Pr_SemaphoreRec : public Pr_Semaphore
{
private:
    /** This attribute is the class mutex   */
    struct k_mutex mutex;

    //NO SE UTILIZAN
    uint32_t prio_type;
	uint32_t prio_ceiling;

public:
    /** Default constructor */
    Pr_SemaphoreRec();

    //NO SE UTILIZA
    /** Constructor for priority ceiling resource sempahore */
    Pr_SemaphoreRec(int32_t prioceiling);

    Pr_SemaphoreRec(const Pr_SemaphoreRec&) = delete;
    Pr_SemaphoreRec& operator=(const Pr_SemaphoreRec&) = delete;
    Pr_SemaphoreRec(Pr_SemaphoreRec&&) = delete;
    Pr_SemaphoreRec& operator=(Pr_SemaphoreRec&&) = delete;

    /**
     * Semaphore signal.
     */
    void Signal(void);

    /**
     * Semaphore wait.
     */
    void Wait(void);

    /**
     * Semaphore conditional wait.
     *
     * The method catch the semaphore if it is possible. In any other case the
     * thread is not blocked.
     *
     * \return The method return 1 in case of the semaphore is catched. In any
     * other case the method returns 0.
     */
    int32_t WaitCond(void);

    /* Not implemented	*/
    bool WaitTimed(Pr_Time & interval);
    
    /**
     * The method returns the actual counter of the mutex
     * 
     * \return the counter of the mutex
     */
    int countSem();

    /**
     * The method returns the actual priority of the mutex
     * 
     * \return the priority of the mutex
     */
    int prioritySem();

    /**
     * Default Destructor
     */
    ~Pr_SemaphoreRec();
};


//********************************************************
//********************  Pr_Task  *************************
//********************************************************

class Pr_Task
{
/**
 * This method sends information to any other task.
 *
 * \param _task :     This is the receiver task.
 * \param _p_data :   This parameter is the data pointer to be sent.
 */
friend void Pr_Send(Pr_Task &_task, void *_p_data);

friend void edroom_call_taskfunction(void*);

/**
 * This method receives a data from sent by other task.
 *
 * \param _p_data :      This parameter is the pointer to the received data.
 * \param _datalength :   This parameter is the data length.
 */
friend void Pr_Receive(void *_p_data, unsigned _datalength);

private:
    //prioridad antigua
    TEDROOMPriority old_prio;

    /** This member is the task identifier  */
    struct k_thread taskID;
    k_tid_t k_tid;
    
    k_thread_stack_t * taskStackPointer; 

    /** Pointer to the data sent through Pr_Send method  */
    void *p_data;

    //*************   SEMAPHORES *****************
    /**
     * This semaphore is used in the Pr_Send/Pr_Receive methods to synchronize
     * the sender and receiver threads
     */
    Pr_SemaphoreBin semSend;

    /**
     * This semaphore is used in the Pr_Send/Pr_Receive methods to synchronize
     * the sender and receiver threads
     */
    Pr_SemaphoreBin semReceive;

    /**
     * Change the task priority
     *
     * \param _priority :    This parameter is the task priority
     */
    void ChangePriority(TEDROOMPriority _priority);

    /**
     * This member is the IP task pointer.
     */
    Pr_TaskRV_t (*TaskIP)(Pr_TaskP_t);


    static Pr_TaskRV_t TaskFunction(void *pPr_Task, void *p2, void *p3) ;

protected:
    /**
     * Message priority.
     *
     * This is the message treatment priority
     */
    TEDROOMPriority priorityMsg;

    /**
     * Temporal priority.
     */
    TEDROOMPriority priorityTmp;

public:
    /**
     * \brief return the temp prio of the TASK
     * @return
     */
    TEDROOMPriority GetTmpTaskPrio();

    /**
     * Default constructor.
     *
     * Initializes internal semaphores and pointers so that a task
     * instance is always in a known state even if no thread is
     * created yet.
    */
    Pr_Task();

    Pr_Task(const Pr_Task&) = delete;
    Pr_Task& operator=(const Pr_Task&) = delete;
    Pr_Task(Pr_Task&&) = delete;
    Pr_Task& operator=(Pr_Task&&) = delete;

    /**
     * Debug Constructor
     *
     * \param _taskCode :    This parameter is the pointer to the task code
     * \param _priority :    This parameter is the task priority
     * \param _stackSize :   This parameter is the task stack size
     *
     * The constructor creates a task. Assigns the needed stack size and the
     * IP code pointer to the task structure.
     */
    Pr_Task(Pr_TaskRV_t (*_taskCode)(Pr_TaskP_t), /*  Task IP */
            TEDROOMPriority _priority,            /*  task priority   */
            unsigned _stackSize);                 /*  stack size  */

    /**
     * This methos stablis the task priority
     *
     * \param _priority :    This parameter is the task priority
     */
    void SetPriority(TEDROOMPriority _priority);

    /**
     * This method modifies temporally the task priority.
     *
     * \param _priority :    the task priority
     *
     * The task priority is only modify when the priority pass argument is
     * greater than the current task priority. The attribute priorityTmp takes
     * the mayor value between the priority passed an its value.
     * The attribute priorityMsg is not modified.
     */
    void SetMaxPrioTmp(TEDROOMPriority _priority);
    /**
     * This method stablish the message treatment priority.
     *
     * \param _priority :    the task priority
     *
     * The priorityMsg attribute is modified only if the priority passed
     * argument is greater than priorityMsg.
     * The attributes priorityMsg and priorityTmp takes the greater value
     * between these and the priority passed argument.
     */
    void SetMaxPrioMsg(TEDROOMPriority _priority);

    /**
     * This method restores the task priority to the message priority.
     *
     * The method is invoked after been modified the temporal priority task
     * (priorityTmp). The priorityTmp takes the priorityMsg value to maintain
     * the coherence between the real priority task and the priorityTmp value.
     */
    void RestorePrioMsg(void);

    /**
     * Returns the task priority
     *
     * \return This method returns the current task priority.
     */
    TEDROOMPriority GetTaskPrio();

     /**
     * Defaul Destructor
     */
    ~Pr_Task() {}
};

void Pr_Send(Pr_Task &task, void *p_data);
void Pr_Receive(void *ap_data, unsigned datalength);

//********************************************************
//********************  Pr_Time  *************************
//********************************************************

class Pr_Time
{
friend class Pr_Kernel;
friend class Pr_SemaphoreBin;
friend class Pr_IRQEvent;

private:

    /** This parameter stores the system time    */
    EDROOMTimeSpec time;

public:

    /**
     * Default Constructor
     */
    Pr_Time();

    /**
     * Constructor.
     *
     * \param   _time	the time value
     *
     * The constructor inits the object with the time value and units passed
     * as parameters.
     */
    Pr_Time(EDROOMTimeSpec _time);

    /**
	 * Constructor.
	 *
	 * \param   _secs	the seconds value
	 *	\param   _usecs	the microseconds value
	 * The constructor inits the object with the time value and units passed
	 * as parameters.
	 */
    Pr_Time(uint32_t _secs, uint32_t _usecs);

    /**
     * Constructor.
     *
     * \param   _time :  This parameter is a Pr_Time object used to init the
     * current object.
     *
     * This contructor inits the current object with the same value of the object
     * passed as parameter.
     */
    Pr_Time(const Pr_Time & _time);

    /**
     * \brief	Relative Delay method.
     *
     * \param   _interval :  This parameter is the time delay
     *
     * The method sleeps the caller task during the delay passed as parameter.
     */
    friend void Pr_DelayIn( const Pr_Time & _interval);

    /**
     * \brief	Absolute Delay method.
     *
     * \param   _time :  This parameter is the time to be awaked
     *
     * The method sleeps the caller. The system awake the task at the specified
     * time  (_time)
     */
    friend void Pr_DelayAt( const Pr_Time & _time);

    /**
     * Get the current system time.
     *
     * This method stores in the _time object the current system time.
     */
    void GetTime(void);

    /**
     * + Operator overload
     *
     * \param   _time :  This parameter is the time structure.
     */
    Pr_Time& operator+=(const Pr_Time& _time);

    /**
     * - Operator overload
     *
     * \param   _time :  This parameter is the time structure.
     */
    Pr_Time& operator-=(const Pr_Time& _time);

    /**
     * = operator overload
     *
     * \param   _time :  This parameter is the time structure.
     */
    Pr_Time& operator=(const Pr_Time& _time);

  /**
     * == operator overload
     *
     * \param   _time :  This parameter is the time structure.
     */
    int operator==(const Pr_Time& _time);

    /**
     * != operator overload
     *
     * \param   _time :  This parameter is the time structure.
     */
    int operator!=(const Pr_Time & _time);

    /**
     * > operator overload
     *
     * \param   _time :  This parameter is the time structure.
     */
    int operator>(const Pr_Time& _time);

    /**
     * < operator overload
     *
     * \param   _time :  This parameter is the time structure.
     */
    int operator<(const Pr_Time& _time);

    /**
     * >= operator overload
     *
     * \param   _time :  This parameter is the time structure.
     */
    int operator>=(const Pr_Time& _time);

    /**
     * <= operator overload
     *
     * \param   _time :  This parameter is the time structure.
     */
    int operator<=(const Pr_Time& _time);

    /**
     * Value of Pr_Time in ticks
     *
     * \return This method returns the value of variable in ticks.
     */
    EDROOMClockTicksType Ticks() const;

    /**
      * Round up or down to the closer ticks
      */
    void RoudMicrosToTicks();

    /**
       * \brief Get Value of the time in seconds + microseconds
       *
 	   * \param   _secs	returns the seconds
	   *	\param   _usecs	returns the microseconds
       */
    void GetValue(uint32_t &_secs, uint32_t &_usecs){
         _secs=time.seconds;
         _usecs=time.microseconds;
    }

    /**
        * Value of the system clock in ticks
        *
        * \return This method returns the value of the system clock in ticks.
        */
    EDROOMClockTicksType GetTicks();
};

//********************************************************
//********************  Pr_IRQManager ********************
//********************************************************

typedef void (*Pr_IRQHandler)(const void *);

typedef uint8_t TEDROOMIRQMask;

class Pr_IRQManager{


public:

    /*!
 	 * \brief Set a new IRQ Mask
 	 * @param irqMask
 	 */
	static inline void NewIRQMask(TEDROOMIRQMask irqMask) {};

    /*!
     * \brief Get the current IRQ Mask
     * \todo
     */
	static inline TEDROOMIRQMask GetIRQMask(void) { return 0; };

    /*!
     * \brief This method disables all interrupts but does not change the IRQ
     * mask
     */
	static void DisableAllIRQs(void);

    /*!
     * \brief This method apply the curent IRQ Mask that was in NewIRQMask
     */
	static void ApplyCurrentIRQMask(void);

    /*!
     * \brief This method installs a user interrupt service routine
     * @param handler	pointer to the handler
     * @param IRQLevel	level of the IRQ
     * @param IRQVectorNumber number of the IRQ
     */
	static void InstallIRQHandler(Pr_IRQHandler handler,
                        uint8_t IRQLevel,
                        uint8_t IRQVectorNumber );

    /*!
     * \brief This method deinstall therestore the original service routines
     * @param IRQLevel level of the IRQ
     * @param IRQVectorNumber number of the IRQ
     */
	static void DeinstallIRQHandler(unsigned char IRQLevel
   											,unsigned char IRQVectorNumber);

    /**
     * Prologue hook for an interrupt handler
     *
     * This method must be called from the first line of an application-defined
     * interrupt service routine.  Its implementation may call (if needed) to
     * to the kernel isr prologue.
     *
     * Note that, for the EDROOMBP implementation over ERCOS, this is a dummy
     * method
     */
    static inline void HandlerEntry (void) {};

     /**
     * Epilogue hook for an interrupt handler
     *
     * This method must be called from the last line of an application-defined
     * interrupt service routine.  Its implementation may call (if needed) to
     * to the kernel isr prologue.
     *
     * Note that, for the EDROOMBP implementation over ERCOS, this is a dummy
     * method
     */
    static inline void HandlerExit (void) {};
    
    /**
     * \brief	 disable an IRQ
     * @param IRQVectorNumber	the number of IRQ
     */
    static void DisableIRQ(uint32_t IRQVectorNumber);
    
    /**
     * \brief	enable an IRQ
     * @param IRQVectorNumber the number of IRQ
     */
    static void EnableIRQ(uint32_t IRQVectorNumber);

};

//********************************************************
//********************  Pr_IRQEvent **********************
//********************************************************


class Pr_IRQEvent{

private:
        uint32_t irq_event;
    edroombp_event_t evento;

public:

    /**
     * Constructor.
     *
     * \param IRQ :  This parameter is the IRQ vector number.
     */
    Pr_IRQEvent(uint8_t irq_event);

    /**
     * Signal event.
     *
     * This method signals an interrupt. The signaling provokes the event
     * propagation to the waiting threads.
     */
    void Signal();

    /**
     * Wait for an event.
     *
     * This method allows threads to wait for an interrupt event.
     */
    void Wait();

    /**
     * Conditional Wait for an event.
     *
     * This method allows threads to wait for an interrupt event.
     */
    bool_t WaitCond();

    /**
     * Timed Waitting for an event.
     *
     * This method allows threads to wait for an interrupt event for a time.
     * \param time interval to wait.
     */
    bool_t WaitTimed(Pr_Time time);

};

#endif
