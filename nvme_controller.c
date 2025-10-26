/*
 * NVMe Controller Module - Implementation
 * NVMe storage controller for BlackBox DPU
 */

#include "nvme_controller.h"

/* ============================================================================
 * NVME CONTROLLER MODEL
 * ============================================================================ */

void nvme_write_data(BlackBoxSoC* soc) {
    NVMeController* nvme = &soc->nvme;
    
    uint8_t* src = memory_translate(&soc->memory, nvme->write_buf_addr);
    if (!src || !nvme->storage_file) return;
    
    fwrite(src, 1, nvme->write_buf_len, nvme->storage_file);
    fflush(nvme->storage_file);
    
    nvme->bytes_written += nvme->write_buf_len;
    nvme->writes_completed++;
    
    soc->noc_stats.nvme_path_bytes += nvme->write_buf_len;
    
    if (soc->verbose) {
        printf("[%lu ns] NVMe: Wrote %u bytes to storage (total: %lu bytes)\n",
               soc->event_queue.current_time, nvme->write_buf_len, nvme->bytes_written);
    }
}
