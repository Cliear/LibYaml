#ifndef MEMORY_INFO_H
#define MEMORY_INFO_H

#include <type.h>

typedef enum { UNUSABLE_MEMORY, RAM_MEMORY, ROM_MEMORY, ACPI_RECLAIM_MEMORY, ACPI_NVS_MEMORY } DeviceMemoryType;

typedef struct
{
    RegisterSize address;
    RegisterSize size;
    RegisterSize type;  // value is DeviceMemoryType
} MemoryDescriptor;

typedef struct
{
    RegisterSize count;
    MemoryDescriptor * memory;
} DeviceMemoryInfo;

#endif