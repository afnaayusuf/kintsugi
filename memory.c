/*
 * Memory Subsystem - Implementation
 */

#include "memory.h"

void memory_init(MemoryModel* mem) {
    mem->boot_rom = (uint8_t*)calloc(BOOT_ROM_SIZE, 1);
    mem->sbm = (uint8_t*)calloc(SBM_SIZE, 1);
    mem->apu_l2_cache = (uint8_t*)calloc(APU_L2_CACHE_SIZE, 1);
    mem->rpu_tcm = (uint8_t*)calloc(RPU_TCM_SIZE, 1);
    mem->dram = (uint8_t*)calloc(DRAM_SIZE, 1);
}

void memory_cleanup(MemoryModel* mem) {
    free(mem->boot_rom);
    free(mem->sbm);
    free(mem->apu_l2_cache);
    free(mem->rpu_tcm);
    free(mem->dram);
}

uint8_t* memory_translate(MemoryModel* mem, uint32_t addr) {
    (void)mem;
    if (addr >= BOOT_ROM_BASE && addr < BOOT_ROM_BASE + BOOT_ROM_SIZE) {
        return &mem->boot_rom[addr - BOOT_ROM_BASE];
    } else if (addr >= SBM_BASE && addr < SBM_BASE + SBM_SIZE) {
        return &mem->sbm[addr - SBM_BASE];
    } else if (addr >= APU_L2_CACHE_BASE && addr < APU_L2_CACHE_BASE + APU_L2_CACHE_SIZE) {
        return &mem->apu_l2_cache[addr - APU_L2_CACHE_BASE];
    } else if (addr >= RPU_TCM_BASE && addr < RPU_TCM_BASE + RPU_TCM_SIZE) {
        return &mem->rpu_tcm[addr - RPU_TCM_BASE];
    } else if (addr >= DRAM_BASE && addr < DRAM_BASE + DRAM_SIZE) {
        return &mem->dram[addr - DRAM_BASE];
    }
    return NULL;
}

uint32_t memory_get_region_remaining(MemoryModel* mem, uint32_t addr)
{
    (void)mem; // mem pointer not required for region sizes
    if (addr >= BOOT_ROM_BASE && addr < BOOT_ROM_BASE + BOOT_ROM_SIZE) {
        return (BOOT_ROM_BASE + BOOT_ROM_SIZE) - addr;
    } else if (addr >= SBM_BASE && addr < SBM_BASE + SBM_SIZE) {
        return (SBM_BASE + SBM_SIZE) - addr;
    } else if (addr >= APU_L2_CACHE_BASE && addr < APU_L2_CACHE_BASE + APU_L2_CACHE_SIZE) {
        return (APU_L2_CACHE_BASE + APU_L2_CACHE_SIZE) - addr;
    } else if (addr >= RPU_TCM_BASE && addr < RPU_TCM_BASE + RPU_TCM_SIZE) {
        return (RPU_TCM_BASE + RPU_TCM_SIZE) - addr;
    } else if (addr >= DRAM_BASE && addr < DRAM_BASE + DRAM_SIZE) {
        return (DRAM_BASE + DRAM_SIZE) - addr;
    }
    return 0;
}
