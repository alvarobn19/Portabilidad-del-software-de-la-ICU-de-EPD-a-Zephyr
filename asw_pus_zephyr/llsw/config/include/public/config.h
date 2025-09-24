#ifndef __PUBLIC__CONFIG_H__
#define __PUBLIC__CONFIG_H__

/*
 * config.h — EDROOM + Zephyr
 * Unifica el viejo config (FreeRTOS/RTEMS) con el estilo del ping-pong,
 * pero usando SIEMPRE los parámetros REALES de Zephyr.
 */

/* === Compatibilidad Zephyr 2.7.x / 3.x+ === */
#if defined(__has_include)
  /* Solo activamos el prefijo si existen las cabeceras nuevas */
  #if __has_include(<zephyr/sys/printk.h>)
    #define Z_HAS_PREFIX 1
  #else
    #define Z_HAS_PREFIX 0
  #endif
#else
  /* Compiladores sin __has_include -> asume 2.7 */
  #define Z_HAS_PREFIX 0
#endif

#if Z_HAS_PREFIX
  #include <zephyr/kernel.h>
  #include <zephyr/device.h>
  #include <zephyr/sys/printk.h>
  #include <zephyr/sys/util.h>
  #include <zephyr/sys/__assert.h>
  #include <zephyr/logging/log.h>
#else
  #include <kernel.h>
  #include <device.h>
  #include <sys/printk.h>
  #include <sys/util.h>
  #include <sys/__assert.h>
  #include <logging/log.h>
#endif


/* --------------------------------------------------------------------------
 *  TIEMPO DEL SISTEMA (coherente con Zephyr)
 *  - En el ping-pong fijaba 10ms; aquí derivamos del board/prj.conf.
 *  - Si en prj.conf defines un tick de 10ms, esto reflejará 10.000 µs.
 * -------------------------------------------------------------------------- */
#define USECS_PER_SEC   1000000U
#define CLICKS_PER_SEC  CONFIG_SYS_CLOCK_TICKS_PER_SEC      /* ticks por segundo (Zephyr) */
#define USEC_PER_TICK   (USECS_PER_SEC / CLICKS_PER_SEC)    /* µs por tick (entero) */

/* Compat: nombre que usabas en ping-pong (opcional) */
#define CONFIG_PLATFORM_ZEPHYR_USECS_PER_TICK  USEC_PER_TICK

/* Aviso útil si el tick no divide exactamente 1s (posible truncado entero) */
#if ((USECS_PER_SEC % CONFIG_SYS_CLOCK_TICKS_PER_SEC) != 0)
#warning "USEC_PER_TICK no es entero exacto; hay truncado. Revisa CONFIG_SYS_CLOCK_TICKS_PER_SEC."
#endif

/* --------------------------------------------------------------------------
 *  SINCRONIZACIÓN (semaforos)
 *  - En el código EDROOM usas MAX_SEM_LIMIT al crear semáforos.
 *  - En Zephyr es idiomático usar K_SEM_MAX_LIMIT.
 * -------------------------------------------------------------------------- */
#ifndef MAX_SEM_LIMIT
#define MAX_SEM_LIMIT   K_SEM_MAX_LIMIT
#endif

/* --------------------------------------------------------------------------
 *  PARÁMETROS EDROOM (del primer config, independientes del SO)
 *  Ajusta estos números a tu despliegue real si difieren.
 * -------------------------------------------------------------------------- */
#define CONFIG_EDROOMDEPLOYMENT_NEED_TASK

/* Núcleo EDROOM BP */
#define CONFIG_EDROOMBP_SWR_EDROOMBP
#define CONFIG_EDROOMBP_SWR_EDROOM_MAX_PRIORITY    255
#define CONFIG_EDROOMBP_SWR_EDROOM_MIN_PRIORITY    1
#define CONFIG_EDROOMBP_SWR_EDROOM_MIN_STACK_SIZE  2048
#define CONFIG_EDROOMBP_SWR_EDROOM_MAX_TASKS       100

/* Demanda estática (estimaciones por componente) */
#define CONFIG_EDROOMSL_SWR_DEMANDED_CEDROOMCOMPONENT   6   /* nº de componentes EDROOM previstos */
#define CONFIG_EDROOMBP_SWR_DEMANDED_PR_KERNEL          1
#define CONFIG_EDROOMBP_SWR_DEMANDED_PR_BINSEM          (CONFIG_EDROOMSL_SWR_DEMANDED_CEDROOMCOMPONENT * 5)
#define CONFIG_EDROOMBP_SWR_DEMANDED_PR_SEMREC          (CONFIG_EDROOMSL_SWR_DEMANDED_CEDROOMCOMPONENT * 5)
#define CONFIG_EDROOMBP_SWR_DEMANDED_PR_TASK            (CONFIG_EDROOMSL_SWR_DEMANDED_CEDROOMCOMPONENT * 2)
#define CONFIG_EDROOMBP_SWR_DEMANDED_PR_IRQEVENT        0
#define CONFIG_EDROOMBP_SWR_DEMANDED_NUM_PRIO           255

/* Parámetros EDROOM genéricos */
#ifndef EDROOM_MAX_TASKS
#define EDROOM_MAX_TASKS CONFIG_EDROOMBP_SWR_EDROOM_MAX_TASKS
#endif

#ifndef EDROOM_THREAD_STACK_SIZE
#define EDROOM_THREAD_STACK_SIZE CONFIG_EDROOMBP_SWR_EDROOM_MIN_STACK_SIZE /* ajustar si se requiere un valor mayor */
#endif

/* --------------------------------------------------------------------------
 *  LOGS / ASSERTS (ajusta a tu preferencia)
 * -------------------------------------------------------------------------- */
#ifndef PRINTF_LOGS
#define PRINTF_LOGS
#endif

#ifdef PRINTF_LOGS
#if Z_HAS_PREFIX
  #include <zephyr/sys/printk.h>
#else
  #include <sys/printk.h>
#endif
  #define DEBUG_PRINTK(fmt, ...)  // printk("[EDROOM] " fmt, ##__VA_ARGS__)
#else
  #define DEBUG_PRINTK(fmt, ...)  ((void)0)
#endif

#define DEBUG(...) DEBUG_PRINTK(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

#if Z_HAS_PREFIX
#include <zephyr/zephyr.h>
#include <zephyr/sys/printk.h>
#else
#include <zephyr.h>
#include <sys/printk.h>
#endif

#ifdef __cplusplus
}
#endif

static inline __attribute__((noreturn)) void edroom_abort_here(const char *file, int line)
{
    // printk("[EDROOM][ASSERT] fallo en %s:%d\n", file, line);
    for (;;) {
        k_sleep(K_FOREVER);
    }
}

#undef ASSERT
#define ASSERT(cond) do {                       \
    if (!(cond)) {                              \
        edroom_abort_here(__FILE__, __LINE__);  \
    }                                           \
} while (0)

/* --------------------------------------------------------------------------
 *  FLAGS DE PROYECTO (mantén los que usabas en el primero si aplican)
 * -------------------------------------------------------------------------- */
#define _EDROOM_HANDLE_IRQS

/* Habilita la emulación de GSS salvo que se haya definido externamente */
#ifndef GSS_EMULATION
#define GSS_EMULATION     /* Emulación de GSS si la usabas en el ASW */
#endif

#endif /* __PUBLIC__CONFIG_H__ */
