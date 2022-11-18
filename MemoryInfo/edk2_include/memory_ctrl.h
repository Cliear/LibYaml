#ifndef MEMORY_INFO_MEMORY_CTRL_H
#define MEMORY_INFO_MEMORY_CTRL_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <memory_info.h>

static inline DeviceMemoryType get_memory_type(EFI_MEMORY_TYPE type)
{
    switch (type)
    {
        case EfiReservedMemoryType:
        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
        case EfiPalCode:
        {
            // ROM or Reserved
            return ROM_MEMORY;
        }
        case EfiUnusableMemory:
        {
            // unusable
            return UNUSABLE_MEMORY;
        }
        case EfiACPIReclaimMemory:
        {
            return ACPI_RECLAIM_MEMORY;
        }
        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        case EfiConventionalMemory:
        case EfiPersistentMemory:
        {
            // ram
            return RAM_MEMORY;
        }
        case EfiACPIMemoryNVS:
        {
            // ACPI NVS memory
            return ACPI_NVS_MEMORY;
        }
        default:
        {
            Print(L"Invalid Memory Type!!\n");
#define EFI_MEMORY_TYPE_ERROR L"Invalid Memory Type!\n"
            gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(EFI_MEMORY_TYPE_ERROR), EFI_MEMORY_TYPE_ERROR);
#undef EFI_MEMORY_TYPE_ERROR
        }
    }
}

EFI_MEMORY_DESCRIPTOR * get_efi_memory_descriptors(UINTN * descriptor_size, UINTN * count);
DeviceMemoryInfo * get_memory_info(EFI_MEMORY_DESCRIPTOR * memory_descriptors, UINTN descriptor_size, UINTN count);

typedef struct {EFI_MEMORY_TYPE type; UINTN valid;} efi_memory_type_info;
efi_memory_type_info get_efi_memory_type_in_scope(EFI_MEMORY_DESCRIPTOR * memory_descriptors, UINTN descriptor_size, UINTN count, EFI_PHYSICAL_ADDRESS address, UINTN size);

VOID * alloc_indication_memory(EFI_PHYSICAL_ADDRESS address, UINTN size);

#endif