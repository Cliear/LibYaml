#ifndef MEMORY_INFO_H
#define MEMORY_INFO_H

#include <type.h>

typedef enum { UNUSABLE_MEMORY, RAM_MEMORY, ROM_MEMORY, ACPI_RECLAIM_MEMORY, ACPI_NVS_MEMORY } MemoryType;

typedef struct
{
    RegisterSize address;
    RegisterSize size;
    RegisterSize type;
} MemoryDescriptor;

typedef struct
{
    RegisterSize count;
    MemoryDescriptor * memory;
} MemoryInfo;

#endif