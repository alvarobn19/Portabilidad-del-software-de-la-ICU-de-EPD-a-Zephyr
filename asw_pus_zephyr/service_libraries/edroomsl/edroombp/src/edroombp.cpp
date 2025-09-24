#include "edroombp_config.h"

#include <zephyr.h>

#include <init.h>

#include "public/edroombp_zephyr_wrapper.h"
#include "public/edroombp.h"
#include "public/config.h"
#include "autoconf.h"
#include <stdint.h>

typedef uint32_t OS_TIME_t;



#define EDROOMBP_TICKS_TO_TIMESPEC(_timespec, _ticks)                  \
	do                                                                 \
	{                                                                  \
		_timespec->seconds = _ticks / (CLICKS_PER_SEC * 1);            \
		_ticks -= _timespec->seconds * (CLICKS_PER_SEC * 1);           \
		_timespec->microseconds = _ticks * 1000000 / (CLICKS_PER_SEC); \
	} while (0)

#define EDROOMBP_TIMESPEC_NORMALIZE(t)          \
	{                                           \
		if ((t)->microseconds >= USECS_PER_SEC) \
		{                                       \
			(t)->microseconds -= USECS_PER_SEC; \
			(t)->seconds++;                     \
		}                                       \
	}

#define EDROOMBP_TIMESPEC_ADD(t1, t2)             \
	do                                            \
	{                                             \
		(t1)->microseconds += (t2)->microseconds; \
		(t1)->seconds += (t2)->seconds;           \
		EDROOMBP_TIMESPEC_NORMALIZE(t1);          \
	} while (0)

#define EDROOMBP_TIMESPEC_ADD_NS(t, n)  \
	do                                  \
	{                                   \
		(t)->microseconds += (n);       \
		EDROOMBP_TIMESPEC_NORMALIZE(t); \
	} while (0)

#define EDROOMBP_TIMESPEC_NZ(t) ((t)->seconds != 0 || (t)->microseconds != 0)

#define EDROOMBP_TIMESPEC_LT(t1, t2) ((t1)->seconds < (t2)->seconds ||   \
									  ((t1)->seconds == (t2)->seconds && \
									   (t1)->microseconds < (t2)->microseconds))

#define EDROOMBP_TIMESPEC_GT(t1, t2) (EDROOMBP_TIMESPEC_LT(t2, t1))

#define EDROOMBP_TIMESPEC_GE(t1, t2) (0 == EDROOMBP_TIMESPEC_LT(t1, t2))

#define EDROOMBP_TIMESPEC_LE(t1, t2) (0 == EDROOMBP_TIMESPEC_GT(t1, t2))

#define EDROOMBP_TIMESPEC_EQ(t1, t2) ((t1)->seconds == (t2)->seconds && \
									  (t1)->microseconds == (t2)->microseconds)

#define YEAR 2000
#define MONTH 1
#define DAY 1

//******************************************************
//****************  Pr_Kernel **************************
//******************************************************

/*				PR_KERNEL FUNCTIONALITY		*/
int edroombp_system_init(int level)
{
	/* Si no haces init real por ahora, devuelve OK */
    return 0;
}

Pr_Kernel::Pr_Kernel()
{
}

void Pr_Kernel::Start()
{
}


//******************************************************
//****************  Pr_Task ****************************
//******************************************************

//****************  CONSTRUCTORS ***********************

/*k_tid_t k_thread_create(struct k_thread *new_thread,
							k_thread_stack_t *stack, size_t stack_size,
							 k_thread_entry_t entry, void *p1,
							 void *p2, void *p3, int prio,
							 uint32_t options, k_timeout_t delay)

	new_thread – Pointer to uninitialized struct k_thread
	stack – Pointer to the stack space.
	stack_size – Stack size in bytes.
	entry – Thread entry function.
	p1 – 1st entry point parameter.
	p2 – 2nd entry point parameter.
	p3 – 3rd entry point parameter.
	prio – Thread priority.
	options – Thread options.
	delay – Scheduling delay, or K_NO_WAIT (for no delay).*/

// --- Pool de pilas por hilo (una pila completa por tarea) ---
K_THREAD_STACK_ARRAY_DEFINE(g_edroom_stacks, EDROOM_MAX_TASKS, EDROOM_THREAD_STACK_SIZE);
struct k_thread g_edroom_threads[EDROOM_MAX_TASKS];
uint8_t g_stack_in_use[EDROOM_MAX_TASKS] = {};
extern const size_t g_edroom_pool_count = EDROOM_MAX_TASKS;

Pr_Task::Pr_Task()
    : old_prio((TEDROOMPriority)0),
      k_tid(nullptr),
      taskStackPointer(nullptr),
      p_data(nullptr),
      semSend(0),
      semReceive(0),
      TaskIP(nullptr),
      priorityMsg((TEDROOMPriority)0),
      priorityTmp((TEDROOMPriority)0) {}

Pr_Task::Pr_Task(Pr_TaskRV_t (*_taskCode)(Pr_TaskP_t ignored), /*  Task IP */
                        TEDROOMPriority _priority, /*  task priority   */
                        unsigned _stackSize)
    : old_prio(_priority),
      k_tid(nullptr),
      taskStackPointer(nullptr),
      p_data(nullptr),
      semSend(0),
      semReceive(0),
      TaskIP(_taskCode),
      priorityMsg(_priority),
      priorityTmp(_priority)
{
        //Check if taskCode is not null
    if (_taskCode){

                /* Ensure a minimum stack size for the task */
        const unsigned min_stack = 1024; /* bytes */
        unsigned stackSize = _stackSize < min_stack ? min_stack : _stackSize;

        // 2) La pila se gestiona dentro del wrapper
        taskStackPointer = nullptr;

        // 3) (Opcional) recortar el tamaño solicitado al del pool
        unsigned effective_stack = (stackSize > EDROOM_THREAD_STACK_SIZE) ? EDROOM_THREAD_STACK_SIZE : stackSize;

        // 4) Crear y arrancar el hilo a través del wrapper
        this->k_tid = edroombp_task_create(
                        Pr_Task::TaskFunction,   // entry: firma (void*, void*, void*)
                        this,                     // arg: primer argumento que recibirá TaskFunction
                        (int)priorityTmp,         // prioridad Zephyr
                        effective_stack           // tamaño de pila solicitado (el wrapper decide si usa pool)
        );

        if (this->k_tid) {
          //printk("[WRAP] start:  tid=%p (k_thread_start)\n", this->k_tid);
          k_thread_start(this->k_tid);
          
          
          /* Cede CPU 1 tick para que el hilo nuevo llegue a su Pr_Receive() */
                  k_yield();                  // o k_msleep(1);
        }

        }
}

/*static void print_stack_left(const char* tag) {
#if defined(CONFIG_THREAD_STACK_INFO)
    size_t unused = 0;
    k_thread_stack_space_get(k_current_get(), &unused);
    // printk("[STACK] %s: libre=%u bytes\n", tag, (unsigned)unused);
#else
    // printk("[STACK] %s: (STACK_INFO no disponible)\n", tag);
#endif
}*/

TEDROOMPriority Pr_Task::GetTmpTaskPrio()
{
        return (TEDROOMPriority)edroombp_task_get_current_priority(this->k_tid);
}

Pr_TaskRV_t Pr_Task::TaskFunction(void *pPr_Task , void *p2, void *p3)
{
	ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    
    //printk("[EDR] TF enter: pointerTask=%p\n", pPr_Task);
    Pr_Task* self = static_cast<Pr_Task*>(pPr_Task);
    
    //void** vtbl = *(void***)self;
    //printk("[EDR] self=%p vtbl=%p first=%p\n", self, vtbl, vtbl ? vtbl[0] : nullptr);
    
    
    if (!self) {
        //printk("[EDR] TaskFunction: self=NULL\n");
        return;
    }
    if (!self->TaskIP) {
        //printk("[EDR] TaskFunction: TaskIP=NULL (self=%p)\n", self);
        return;
    }

    // PASA EL CONTEXTO CORRECTO (self), NO 0
    //const Pr_TaskP_t ctx = (Pr_TaskP_t)(uintptr_t)self;
    //printk("[EDR] TaskIP=%p ctx=%p\n", (void*)self->TaskIP, (void*)(uintptr_t)ctx);

    //self->TaskIP(ctx);   // <-- antes era self->TaskIP(0);

	self->TaskIP(static_cast<Pr_TaskP_t>(reinterpret_cast<uintptr_t>(self)));
    //return;
}

extern "C" void edroom_call_taskfunction(void *self_void)
{
    Pr_Task *self = static_cast<Pr_Task *>(self_void);
    //printk("[EDR] call_taskfunction self=%p\n", self);
    // Aquí se llama al método C++ correcto:
    self->TaskFunction(self, nullptr, nullptr);
}

void Pr_Task::SetPriority(TEDROOMPriority _priority)
{
	priorityMsg = _priority;
	if (priorityTmp != priorityMsg)
	{
		priorityTmp = priorityMsg;
		ChangePriority(_priority);
	}
}

void Pr_Task::SetMaxPrioTmp(TEDROOMPriority _priority)
{
	if (_priority < priorityTmp)
	{
		priorityTmp = _priority;
		ChangePriority(_priority);
	}
}

void Pr_Task::SetMaxPrioMsg(TEDROOMPriority _priority)
{
	if (_priority < priorityMsg)
	{
		priorityMsg = _priority;
		if (priorityMsg < priorityTmp)
		{
			priorityTmp = priorityMsg;
			ChangePriority(_priority);
		}
	}
}

void Pr_Task::RestorePrioMsg(void)
{
	if (priorityTmp != priorityMsg)
	{
		priorityTmp = priorityMsg;
		ChangePriority(priorityMsg);
	}
}

TEDROOMPriority Pr_Task::GetTaskPrio()
{
	return priorityMsg;
}

void Pr_Task::ChangePriority(TEDROOMPriority _priority)
{
        edroombp_task_change_priority(this->k_tid, _priority);
}



//******************************************************
//****************  Pr_Time ****************************
//******************************************************

//****************  CONSTRUCTORS ***********************


Pr_Time::Pr_Time()
{
	time.seconds = 0;
	time.microseconds = 0;
}

Pr_Time::Pr_Time(const Pr_Time &_time)
{
	time.microseconds = _time.time.microseconds;
	time.seconds = _time.time.seconds;
}

Pr_Time::Pr_Time(EDROOMTimeSpec _time)
{
	time.seconds = _time.seconds;

	time.microseconds = _time.microseconds;
}

Pr_Time::Pr_Time(uint32_t _secs, uint32_t _usecs)
{
	time.seconds =  _secs;
	time.microseconds = _usecs;
}


void Pr_Time::GetTime(void)
{
	uint32_t currentTicks = k_uptime_ticks();
	time.seconds = currentTicks/CONFIG_SYS_CLOCK_TICKS_PER_SEC;
    time.microseconds =	(currentTicks-((uint32_t)time.seconds*CONFIG_SYS_CLOCK_TICKS_PER_SEC))*USEC_PER_TICK;
}

EDROOMClockTicksType Pr_Time::GetTicks()
{
	return (EDROOMClockTicksType)k_uptime_ticks();
}


Pr_Time& Pr_Time::operator+=(const Pr_Time &_time)
{

	EDROOMBP_TIMESPEC_ADD(&time, &(_time.time));

	return *this;
}

Pr_Time& Pr_Time::operator-=(const Pr_Time &_time)
{
	//Check if the left operand seconds is higher than the right operand's
	if (_time.time.seconds <= time.seconds)
	{
		//substract the seconds.
		time.seconds -= _time.time.seconds;
		//Check the microseconds in the left operand against the right operand's
		if (_time.time.microseconds > time.microseconds)
		{
			if (time.seconds > 0)
			{
				time.seconds--;
				time.microseconds += USECS_PER_SEC - _time.time.microseconds;
			} else
			{
				time.seconds = time.microseconds = 0;
			}
		} else
		{
			time.microseconds -= _time.time.microseconds;
		}
	} else
	{
		time.seconds = time.microseconds = 0;
	}

	return *this;
}

Pr_Time& Pr_Time::operator=(const Pr_Time &_time)
{
	time.microseconds = _time.time.microseconds;
	time.seconds = _time.time.seconds;

	return *this;
}

int Pr_Time::operator==(const Pr_Time &_time)
{
	return (EDROOMBP_TIMESPEC_EQ(&time, &(_time.time)));
}

int Pr_Time::operator!=(const Pr_Time &_time)
{
	return (0 == EDROOMBP_TIMESPEC_EQ(&time, &(_time.time)));
}

int Pr_Time::operator>(const Pr_Time &_time)
{
	return (EDROOMBP_TIMESPEC_GT(&time, &(_time.time)));
}

int Pr_Time::operator<(const Pr_Time &_time)
{
	return (EDROOMBP_TIMESPEC_LT(&time, &(_time.time)));
}

int Pr_Time::operator>=(const Pr_Time &_time)
{
	return (EDROOMBP_TIMESPEC_GE(&time, &(_time.time)));
}

int Pr_Time::operator<=(const Pr_Time &_time)
{
	return (EDROOMBP_TIMESPEC_LE(&time, &(_time.time)));
}


EDROOMClockTicksType Pr_Time::Ticks() const
{
	EDROOMClockTicksType TimeInTicks;
	TimeInTicks = time.seconds;
	TimeInTicks = ((TimeInTicks * CONFIG_SYS_CLOCK_TICKS_PER_SEC)
	+ (time.microseconds / USEC_PER_TICK));

	return TimeInTicks;
}

void Pr_Time::RoudMicrosToTicks()
{

	uint32_t ticksFromMicroseconds;
	uint32_t microsecondsFromTicks;

	ticksFromMicroseconds = time.microseconds / USEC_PER_TICK;
	microsecondsFromTicks = ticksFromMicroseconds * USEC_PER_TICK;

	if (time.microseconds != microsecondsFromTicks)
	{
		microsecondsFromTicks += USEC_PER_TICK / 2;
		if (microsecondsFromTicks <= time.microseconds)
		{
			time.microseconds += USEC_PER_TICK;
			EDROOMBP_TIMESPEC_NORMALIZE(&time);
		}
	}
}

void Pr_DelayIn(const Pr_Time &_interval)
{
	EDROOMClockTicksType TimeInTicks;
    //calculate the ticks to sleep.
    TimeInTicks = _interval.Ticks();
    edroombp_wait_ticks(TimeInTicks);
}

void Pr_DelayAt(const Pr_Time &_time)
{

/*	EDROOMTimeSpec time;
	float sec1, sec2;
	EDROOMClockTicksType TimeInTicks;

	uint32_t currentTicks = k_uptime_ticks();
	time.seconds = currentTicks/CONFIG_SYS_CLOCK_TICKS_PER_SEC;
    time.microseconds =	(currentTicks-((uint32_t)time.seconds*CONFIG_SYS_CLOCK_TICKS_PER_SEC))*USEC_PER_TICK;
	

	if (start_time.seconds <= time.seconds)
	{
		time.seconds -= start_time.seconds;
		if (start_time.microseconds > time.microseconds)
		{
			if (time.seconds > 0)
			{

				time.seconds--;
				time.microseconds += USEC_PER_TICK - start_time.microseconds;
			} else
			{

				time.seconds = time.microseconds = 0;

			}
		} else
		{

	time.microseconds -= start_time.microseconds;
		}
	} else
	{
		time.seconds = time.microseconds = 0;
	}

	//------------ calculate the value of time to wake the task.

	sec1 = time.seconds + (float) (time.microseconds / USEC_PER_TICK);
	sec2 = _time.time.seconds
		+ (float) (_time.time.microseconds/ USEC_PER_TICK);

	if (sec2 > sec1)
	{
		sec2 -= sec1;

		time.seconds = (uint32_t) sec2;
		time.microseconds = (uint32_t) ((sec2 - time.seconds)* USEC_PER_TICK);
	} else
	{
		DEBUG("too late!");
		return; // zero delay
	}
	//interval to wait expressed in ticks
    TimeInTicks = time.seconds * CONFIG_SYS_CLOCK_TICKS_PER_SEC +
                  time.microseconds / USEC_PER_TICK;
    //call to sleep.
    edroombp_wait_ticks(TimeInTicks);*/
    
    EDROOMClockTicksType nowTicks = k_uptime_ticks();
        EDROOMClockTicksType targetTicks = _time.Ticks();
        if (targetTicks > nowTicks) {
                edroombp_wait_ticks(targetTicks - nowTicks);
        } else {
                DEBUG("too late!");
        }

}

//********************************************************
//********************  Pr_Semaphore  ********************
//********************************************************

Pr_Semaphore::Pr_Semaphore(uint32_t _value)
{
	
}

//********************************************************
//********************  Pr_SemaphoreBin  *****************
//********************************************************

Pr_SemaphoreBin::Pr_SemaphoreBin(uint32_t _value) : Pr_Semaphore(_value)
{
	edroombp_semaphore_create(&sem, (int) _value, MAX_SEM_LIMIT);
}

void Pr_SemaphoreBin::Signal()
{
	edroombp_semaphore_release(&sem);
}

void Pr_SemaphoreBin::Wait()
{
	edroombp_semaphore_catch(&sem, K_FOREVER);
}


int32_t Pr_SemaphoreBin::WaitCond()
{
        return (k_sem_take(&sem, K_NO_WAIT) == 0);
}

bool Pr_SemaphoreBin::WaitTimed(const Pr_Time &_waittime)
{
        uint32_t TimeInTicks;
        //ticks to wait.
        TimeInTicks = (uint32_t) _waittime.Ticks();

        //call to sleep the task.
        return (edroombp_semaphore_catch(&sem, K_TICKS(TimeInTicks)) == 0);
}

Pr_SemaphoreBin::~Pr_SemaphoreBin()
{

}

int Pr_SemaphoreBin::countSem(){
	return edroombp_semphore_count(&sem);
}
//********************************************************
//********************  Pr_SemaphoreRec  *****************
//********************************************************

Pr_SemaphoreRec::Pr_SemaphoreRec()
{
	edroombp_mutex_create(&mutex);
}

Pr_SemaphoreRec::Pr_SemaphoreRec(int32_t prioceiling)
{
	edroombp_mutex_create(&mutex);
}

void Pr_SemaphoreRec::Signal()
{
	edroombp_mutex_unlock(&mutex);
}

void Pr_SemaphoreRec::Wait()
{
	edroombp_mutex_lock(&mutex, K_FOREVER);
}

int32_t Pr_SemaphoreRec::WaitCond()
{
	return edroombp_mutex_try_catch(&mutex);
}

void Pr_Send(Pr_Task &_task, void *_p_data)
{
        //copy the pointer to the task's attribute.
        _task.p_data = _p_data;

        //signal the received data.
        _task.semReceive.Signal();
        //catch the send semaphore.
        _task.semSend.Wait();

}

void Pr_Receive(void * _p_data, unsigned _datalength)
{
        Pr_Task *receiver = edroombp_self_from_current();
        __ASSERT(receiver != nullptr, "Pr_Receive sin self");
        if (!receiver) {
                //printk("[EDR] Pr_Receive sin self\n");
                return;
        }

        //catch the recieve semaphore.
        receiver->semReceive.Wait();

        //copy the pointer provided by sender (size checked by caller).
        *(void**)_p_data = receiver->p_data;

        //release the send semaphore.
        receiver->semSend.Signal();
}

Pr_SemaphoreRec::~Pr_SemaphoreRec()
{
}

int Pr_SemaphoreRec::countSem(){
	return edroombp_mutex_count(&mutex);
}

int Pr_SemaphoreRec::prioritySem(){
	return edroombp_mutex_priority(&mutex);
}


//********************************************************
//********************  Pr_IRQManager ********************
//********************************************************

int key;
void Pr_IRQManager::DisableAllIRQs(void)
{
	//disable all the IRQs.
	edroombp_disable_irqs(1);
}

void Pr_IRQManager::ApplyCurrentIRQMask(void)
{
	//enable all the IRQs.
	edroombp_enable_irqs(1);
}


void Pr_IRQManager::InstallIRQHandler(Pr_IRQHandler handler,
        uint8_t IRQLevel, uint8_t IRQVectorNumber)
{
        ARG_UNUSED(IRQLevel);
        edroombp_install_handler(
            reinterpret_cast<void (*)(const void *)>(handler),
            IRQVectorNumber);
}

void Pr_IRQManager::DeinstallIRQHandler(uint8_t IRQLevel,
		uint8_t IRQVectorNumber)
{
	//Deinstall the handler for the given IRQ vector.
	edroombp_deinstall_handler(IRQVectorNumber);
}

void Pr_IRQManager::DisableIRQ(uint32_t IRQVectorNumber)
{
	ASSERT(IRQVectorNumber>=0x10);
	ASSERT(IRQVectorNumber<0x20);


	edroombp_mask_irq(IRQVectorNumber-0x10);

}

void Pr_IRQManager::EnableIRQ(uint32_t IRQVectorNumber)
{
	ASSERT(IRQVectorNumber>=0x10);
	ASSERT(IRQVectorNumber<0x20);

	edroombp_unmask_irq(IRQVectorNumber-0x10);

}

//********************************************************
//********************  Pr_IRQEvent ********************
//********************************************************

Pr_IRQEvent::Pr_IRQEvent(uint8_t irq_event) : irq_event(0)
{
       //Check the event id is correct.
       ASSERT(irq_event < 32);
       //set the flag.
       this->irq_event = (1 << irq_event);

       edroombp_event_init(&evento);
}


void Pr_IRQEvent::Signal()
{
        //Check irq_event is not zero
        ASSERT(irq_event != 0);
        //Signal the IRQ event.
        edroombp_event_signal(&evento);

}

void Pr_IRQEvent::Wait()
{
        //Check that the irq_event is not zero.
        ASSERT(irq_event != 0);
        //catch the IRQ event.
        edroombp_event_catch(nullptr, &evento, irq_event);

}

bool Pr_IRQEvent::WaitTimed(Pr_Time _time)
{
	uint32_t TimeInTicks;
	//Check that the irq_event is not zero.
	ASSERT(irq_event != 0);
	//convert the time to Ticks.
	TimeInTicks = (uint32_t) _time.Ticks();
	//try to catch the IRQ event during a given period of time.
        return edroombp_event_timed_catch(nullptr, TimeInTicks, &evento);
}

bool Pr_IRQEvent::WaitCond()
{
        //Check irq_event is not zero
        ASSERT(irq_event != 0);
        //try to catch the IRQ event.
        return edroombp_event_try_catch(nullptr, &evento);

}

