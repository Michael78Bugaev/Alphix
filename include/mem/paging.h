// Alphix kernel
// Paging support for x86_64
// include/mem/paging.h

#ifndef _ALPHIX_MEM_PAGING_H
#define _ALPHIX_MEM_PAGING_H

#include <stdint.h>
#include <stddef.h>

// Флаги для записей страниц
#define PAGE_PRESENT  (1ULL << 0)
#define PAGE_RW       (1ULL << 1)
#define PAGE_USER     (1ULL << 2)
#define PAGE_PWT      (1ULL << 3)
#define PAGE_PCD      (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY    (1ULL << 6)
#define PAGE_PS       (1ULL << 7)
#define PAGE_GLOBAL   (1ULL << 8)
#define PAGE_NX       (1ULL << 63)

// Инициализация системы пагинации (identity map первых 1GB с 2MB страницами)
void paging_init(void);

#endif // _ALPHIX_MEM_PAGING_H 