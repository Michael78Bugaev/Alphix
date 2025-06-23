// Alphix kernel
// 4-level Paging setup for x86_64 with 2MB identity-mapping
// mem/paging.c

#include <mem/paging.h>
#include <cpu/cpu.h>
#include <string.h>

// Аллокаторы для таблиц (выравнено на 4K)
static uint64_t pml4[512]  __attribute__((aligned(4096)));
static uint64_t pdpt[512]  __attribute__((aligned(4096)));
// PD-таблицы для identity map первых 4GB (4 * 512 * 2MB)
static uint64_t pd0[512]   __attribute__((aligned(4096)));
static uint64_t pd1[512]   __attribute__((aligned(4096)));
static uint64_t pd2[512]   __attribute__((aligned(4096)));
static uint64_t pd3[512]   __attribute__((aligned(4096)));

void paging_init(void)
{
    // Очистим таблицы
    memset(pml4, 0, sizeof(pml4));
    memset(pdpt, 0, sizeof(pdpt));
    memset(pd0, 0, sizeof(pd0));
    memset(pd1, 0, sizeof(pd1));
    memset(pd2, 0, sizeof(pd2));
    memset(pd3, 0, sizeof(pd3));

    // PML4[0] -> PDPT
    pml4[0] = (uint64_t)pdpt | PAGE_PRESENT | PAGE_RW;
    // PDPT[0..3] -> PD0..PD3
    pdpt[0] = (uint64_t)pd0 | PAGE_PRESENT | PAGE_RW;
    pdpt[1] = (uint64_t)pd1 | PAGE_PRESENT | PAGE_RW;
    pdpt[2] = (uint64_t)pd2 | PAGE_PRESENT | PAGE_RW;
    pdpt[3] = (uint64_t)pd3 | PAGE_PRESENT | PAGE_RW;

    // PDx -> identity map первые 4GB (4 * 512 * 2MB)
    for (int j = 0; j < 4; ++j) {
        uint64_t *pd = (j==0 ? pd0 : j==1 ? pd1 : j==2 ? pd2 : pd3);
        for (int i = 0; i < 512; ++i) {
            uint64_t addr = ((uint64_t)j * 512 + i) * 0x200000ULL;
            pd[i] = addr | PAGE_PRESENT | PAGE_RW | PAGE_PS;
        }
    }

    // Включение PAE (для long mode)
    uint64_t cr4;
    __asm__ __volatile__("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1UL << 5); // CR4.PAE
    __asm__ __volatile__("mov %0, %%cr4" :: "r"(cr4));

    // Загрузка CR3 с адресом PML4
    cpu_write_cr3((uint64_t)pml4);

    // Включение Paging
    uint64_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= (1UL << 31); // CR0.PG
    __asm__ __volatile__("mov %0, %%cr0" :: "r"(cr0));
} 