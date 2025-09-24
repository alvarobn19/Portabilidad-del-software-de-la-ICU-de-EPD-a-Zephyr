#include "edroombp_config.h"

static_assert(EDROOM_THREAD_STACK_SIZE >= 8192,
              "EDROOM_THREAD_STACK_SIZE es demasiado pequeña para EDROOM");

#ifdef __GNUC__
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#pragma message(                                                               \
    "[EDROOM] EDROOM_THREAD_STACK_SIZE=" STRINGIFY(EDROOM_THREAD_STACK_SIZE))
#endif

#include <irq.h>
#include <kernel.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/printk.h>
#include <sys_clock.h>
#include <zephyr.h>

#include "public/config.h"
#include "public/edroombp_zephyr_wrapper.h"

extern k_thread_stack_t g_edroom_stacks[EDROOM_MAX_TASKS][K_THREAD_STACK_LEN(
    EDROOM_THREAD_STACK_SIZE)];
extern struct k_thread g_edroom_threads[EDROOM_MAX_TASKS];
extern uint8_t g_stack_in_use[EDROOM_MAX_TASKS];
extern const size_t g_edroom_pool_count;

static edroom_task_slot g_slots[EDROOM_MAX_TASKS];

Pr_Task *edroombp_self_from_current(void) {
  return static_cast<Pr_Task *>(k_thread_custom_data_get());
}

void edroombp_register_current(Pr_Task *self) {
  k_thread_custom_data_set(self);
}

extern "C" void edroombp_trampoline(void *p1, void *p2, void *p3) {
  ARG_UNUSED(p2);
  ARG_UNUSED(p3);
  Pr_Task *self = static_cast<Pr_Task *>(p1);
  edroombp_register_current(self);
  k_thread_name_set(k_current_get(), "EDROOM_Task");
  //printk("[WRAP] trampolin -> self=%p (tid=%p)\n", self, k_current_get());
  edroom_call_taskfunction(self);
}

extern "C" k_tid_t edroombp_task_create(Pr_TaskEntryFn entry, void *arg,
                                        int prio, size_t /*stack_size*/) {
  int slot = -1;
  for (size_t i = 0; i < g_edroom_pool_count; ++i)
    if (!g_stack_in_use[i]) {
      slot = (int)i;
      break;
    }
  if (slot < 0) {
    // printk("[WRAP] sin slots de stack libres\n");
    return nullptr;
  }

  g_stack_in_use[slot] = 1;
  k_thread_stack_t *stack_area = g_edroom_stacks[slot];
  size_t real_sz = K_THREAD_STACK_SIZEOF(g_edroom_stacks[slot]);
  struct k_thread *tid = &g_edroom_threads[slot];

  //printk("[WRAP] create: slot=%d tid=%p prio=%d stack=%p size=%zu (creado)\n",
         //slot, tid, prio, stack_area, real_sz);

  k_tid_t tid_created = k_thread_create(
      tid, stack_area, real_sz, edroombp_trampoline,
      /*p1*/ arg, /*p2*/ (void *)entry, /*p3*/ nullptr, prio, 0, K_FOREVER);
  g_slots[slot].tid = tid_created;
  g_slots[slot].thread = tid;
  g_slots[slot].stack = stack_area;
  g_slots[slot].stack_size = real_sz;
  g_slots[slot].self = static_cast<Pr_Task *>(arg);
  return tid_created;
}

void edroombp_task_start(struct k_thread *task) {
  //printk("[WRAP] start:  tid=%p (k_thread_start)\n", task);
  k_thread_start(task);
}

void edroombp_task_delete(struct k_thread *task) { k_thread_abort(task); }

int edroombp_task_get_current_priority(struct k_thread *task) {
  return k_thread_priority_get(task);
}

void edroombp_task_change_priority(struct k_thread *task, int newPrio) {
  k_thread_priority_set(task, newPrio);
}

int64_t edroombp_get_time(void) { return k_uptime_ticks(); }

void edroombp_wait_ticks(uint32_t ticks) {
  k_timeout_t ticksTimeOut = K_TICKS(ticks);
  k_sleep(ticksTimeOut);
}

/* ----------------- Semáforos ----------------- */
int edroombp_semaphore_create(struct k_sem *sem, unsigned int initial_count,
                              unsigned int limit) {
  return k_sem_init(sem, initial_count, limit);
}

void edroombp_semaphore_release(struct k_sem *sem) { k_sem_give(sem); }

int edroombp_semaphore_catch(struct k_sem *sem, k_timeout_t timeout) {
  return k_sem_take(sem, timeout);
}

int edroombp_semphore_count(struct k_sem *sem) { return k_sem_count_get(sem); }

int edroombp_semaphore_try_catch(struct k_sem *sem) {
  return k_sem_take(sem, K_NO_WAIT) == 0;
}

/* ----------------- Mutex ----------------- */
int edroombp_mutex_create(struct k_mutex *mutex) { return k_mutex_init(mutex); }

int edroombp_mutex_lock(struct k_mutex *mutex, k_timeout_t timeout) {
  return k_mutex_lock(mutex, timeout);
}

int32_t edroombp_mutex_try_catch(struct k_mutex *mutex) {
  return k_mutex_lock(mutex, K_NO_WAIT) == 0;
}

int edroombp_mutex_unlock(struct k_mutex *mutex) {
  return k_mutex_unlock(mutex);
}

int edroombp_mutex_count(struct k_mutex *mutex) { return mutex->lock_count; }

int edroombp_mutex_priority(struct k_mutex *mutex) {
  return mutex->owner_orig_prio;
}

/* --------------- IRQ manager --------------- */
#define ZEPHYR_INTERRUPT 1
#define RAW_INTERRUPT 0

void edroombp_enable_irqs(int /*irq*/) { irq_enable(0); }
void edroombp_disable_irqs(int /*irq*/) { irq_disable(0); }

extern "C" void edroombp_install_handler(void (*handler)(const void *),
                                         uint8_t vector_num) {
#if defined(CONFIG_DYNAMIC_INTERRUPTS)
  int ret = irq_connect_dynamic(vector_num, /*prio*/0, handler, nullptr, 0);
  if (ret < 0) {
    // printk("[WRAP] irq_connect_dynamic(%u) fallo=%d\n", vector_num, ret);
    return;
  }
  irq_enable(vector_num);
  //printk("[WRAP] IRQ %u conectado dinamicamente\n", vector_num);
#else
  #if 0
  // printk(
  //    "[WRAP] ERROR: CONFIG_DYNAMIC_INTERRUPTS desactivado; no puedo conectar IRQ %u en runtime\n",
  //     vector_num);
  #endif
#endif
}

void edroombp_deinstall_handler(uint8_t /*vector_num*/) { /* no-op */ }
void edroombp_mask_irq(uint8_t /*irq_level*/) { /* no-op */ }
void edroombp_unmask_irq(uint8_t /*irq_level*/) { /* no-op */ }

/* ----------------- Eventos por IRQ ----------------- */
extern "C" void edroombp_event_init(edroombp_event_t *e) {
  k_poll_signal_init(&e->sig);
}

extern "C" void edroombp_event_signal(edroombp_event_t *e) {
  //printk("[WRAP] edroombp_event_signal()\n");
  k_poll_signal_raise(&e->sig, 0);
}

extern "C" void edroombp_event_catch(k_tid_t *receiver_task_id,
                                     edroombp_event_t *e, int /*evento*/) {
  struct k_poll_event ev;
  k_poll_event_init(&ev, K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY,
                    &e->sig);
  int rc = k_poll(&ev, 1, K_FOREVER);
  ARG_UNUSED(rc);

  unsigned int signaled = 0;
  int result = 0;
  k_poll_signal_check(&e->sig, &signaled, &result);
  if (signaled) {
    k_poll_signal_reset(&e->sig);
  }

  if (receiver_task_id) {
    *receiver_task_id = k_current_get();
  }
}

extern "C" bool_t edroombp_event_timed_catch(uint32_t *receiver_task_id,
                                             uint32_t ticks,
                                             edroombp_event_t *e) {
  struct k_poll_event ev;
  k_poll_event_init(&ev, K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY,
                    &e->sig);

  int rc = k_poll(&ev, 1, K_TICKS(ticks));

  if (rc < 0) {
    return false;
  }

  unsigned int signaled = 0;
  int result = 0;
  k_poll_signal_check(&e->sig, &signaled, &result);
  if (signaled) {
    k_poll_signal_reset(&e->sig);
    if (receiver_task_id) {
      *receiver_task_id =
          (uint32_t)(uintptr_t)k_current_get();
    }
    return true;
  }
  return false;
}

extern "C" bool_t edroombp_event_try_catch(uint32_t *receiver_task_id,
                                           edroombp_event_t *e) {
  struct k_poll_event ev;
  k_poll_event_init(&ev, K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY,
                    &e->sig);

  int rc = k_poll(&ev, 1, K_NO_WAIT);

  if (rc < 0) {
    return false;
  }

  unsigned int signaled = 0;
  int result = 0;
  k_poll_signal_check(&e->sig, &signaled, &result);
  if (signaled) {
    k_poll_signal_reset(&e->sig);
    if (receiver_task_id) {
      *receiver_task_id =
          (uint32_t)(uintptr_t)k_current_get();
    }
    return true;
  }
  return false;
}
