#include <zephyr.h>
#include <sys/printk.h>
#include "public/bkg_executor.h"
#include "public/cdtchandler.h"
#include <string.h>

struct BkgItem {
    void *fifo_reserved; /* Required by k_fifo */
    CDTCHandler* router;
    uint8_t*     tc_copy;
    uint16_t     tc_len;
};

K_FIFO_DEFINE(bkg_fifo);

static void bkg_exec_thread(void*, void*, void*) {
    while (true) {
        auto* it = static_cast<BkgItem*>(k_fifo_get(&bkg_fifo, K_FOREVER));
        if (it && it->router && it->tc_copy && it->tc_len) {
            it->router->BuildFromRaw(it->tc_copy, it->tc_len);
            tc_handler_t* th = it->router->GetTCHandler();
            uint16_t apid = ccsds_pus_tc_get_APID(th->tc_packet_header.packet_id);
            uint16_t seq  = ccsds_pus_tmtc_get_seq_count(th->tc_packet_header.packet_seq_ctrl);
            printk("[BKG_EXEC] ReqID(%u,%u,%u,%u) handler=%p\n",
                   apid, th->tc_df_header.type, th->tc_df_header.subtype, seq, th);
            it->router->ExecTC();
        }
        if (it) {
            k_free(it->tc_copy);
            k_free(it);
        }
    }
}

K_THREAD_STACK_DEFINE(bkg_stack, 2048);
static struct k_thread bkg_tcb;

void start_bkg_executor() {
    k_thread_create(&bkg_tcb, bkg_stack, K_THREAD_STACK_SIZEOF(bkg_stack),
                    bkg_exec_thread, nullptr, nullptr, nullptr,
                    K_PRIO_PREEMPT(8), 0, K_NO_WAIT);
    k_thread_name_set(&bkg_tcb, "bkg_exec");
}

void enqueue_bkg_tc(CDTCHandler* router) {
    tc_handler_t* th = router->GetTCHandler();
    uint16_t n = th->raw_tc_mem_descriptor.tc_num_bytes;
    const uint8_t* p = th->raw_tc_mem_descriptor.p_tc_bytes;

    uint16_t apid = ccsds_pus_tc_get_APID(th->tc_packet_header.packet_id);
    uint16_t seq  = ccsds_pus_tmtc_get_seq_count(th->tc_packet_header.packet_seq_ctrl);
    /*printk("[BKG_ENQ] ReqID(%u,%u,%u,%u) handler=%p\n",
           apid, th->tc_df_header.type, th->tc_df_header.subtype, seq, th);*/

    auto* it = static_cast<BkgItem*>(k_malloc(sizeof(BkgItem)));
    if (!it) return;
    it->fifo_reserved = nullptr;
    it->router = router;
    it->tc_len = n;
    it->tc_copy = (uint8_t*)k_malloc(n);
    if (!it->tc_copy) { k_free(it); return; }
    memcpy(it->tc_copy, p, n);

    k_fifo_put(&bkg_fifo, it);
}
