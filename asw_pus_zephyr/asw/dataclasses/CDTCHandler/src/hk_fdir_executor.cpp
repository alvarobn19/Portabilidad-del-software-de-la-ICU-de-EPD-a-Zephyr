#include <zephyr.h>
#include <sys/printk.h>
#include "public/hk_fdir_executor.h"
#include "public/cdtchandler.h"
#include <string.h>
#include "public/tmtc_dyn_mem.h"

struct HkFdirItem {
    void *fifo_reserved;      // k_fifo first
    CDTCHandler* router;
    uint8_t*     tc_copy;
    uint16_t     tc_len;
};

K_FIFO_DEFINE(hk_fdir_fifo);

static void hk_fdir_exec_thread(void*, void*, void*) {
    // printk("[HK/FDIR] thread start\n");
    while (true) {
        auto* it = static_cast<HkFdirItem*>(k_fifo_get(&hk_fdir_fifo, K_FOREVER));
        if (it && it->router && it->tc_copy && it->tc_len) {
            it->router->BuildFromRaw(it->tc_copy, it->tc_len);
            it->router->ExecTC();
        }
        if (it) {
            k_free(it->tc_copy);
            k_free(it);
        }
    }
}

K_THREAD_STACK_DEFINE(hk_fdir_stack, 2048);
static struct k_thread hk_fdir_tcb;

void start_hk_fdir_executor() {
    k_thread_create(&hk_fdir_tcb, hk_fdir_stack, K_THREAD_STACK_SIZEOF(hk_fdir_stack),
                    hk_fdir_exec_thread, nullptr, nullptr, nullptr,
                    K_PRIO_PREEMPT(8), 0, K_NO_WAIT);
    k_thread_name_set(&hk_fdir_tcb, "hk_fdir_exec");
}

void enqueue_hk_fdir_tc(CDTCHandler* router) {
    tc_handler_t* th = router->GetTCHandler();
    uint16_t n = th->raw_tc_mem_descriptor.tc_num_bytes;
    const uint8_t* p = th->raw_tc_mem_descriptor.p_tc_bytes;

    auto* it = static_cast<HkFdirItem*>(k_malloc(sizeof(HkFdirItem)));
    if (!it) return;
    it->fifo_reserved = nullptr;
    it->router = router;
    it->tc_len = n;
    it->tc_copy = (uint8_t*)k_malloc(n);
    if (!it->tc_copy) { k_free(it); return; }
    memcpy(it->tc_copy, p, n);

    k_fifo_put(&hk_fdir_fifo, it);
}

