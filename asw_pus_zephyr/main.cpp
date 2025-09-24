// Cabeceras compatibles con Zephyr 2.7 y 3.x
#if __has_include(<zephyr/zephyr.h>)
  #include <zephyr/zephyr.h>
  #include <zephyr/sys/printk.h>
#else
  #include <zephyr.h>
  #include <sys/printk.h>
#endif

extern "C" int uah_asw_mmesp_project(void);

extern "C" void __cxa_pure_virtual(void) {
    // printk("!! __cxa_pure_virtual: llamada a virtual puro detectada\n");
    // no llamar k_panic(); evita dependencia con z_fatal_error
    for (;;) {
        k_sleep(K_FOREVER);   // o simplemente: while (1) {}
    }
}

extern "C" void main(void)
{
    // printk("\nStarting...\n");
    uah_asw_mmesp_project();
    
    // Mantén vivo el hilo main para evitar retornar al wrapper del kernel
    // y posibles llamadas indirectas nulas.
    while (1) {
        k_sleep(K_FOREVER);
    }
}
