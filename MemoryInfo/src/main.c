#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

#include <memory_ctrl.h>

#define MemoryTypeToStringContext(name) [name] = L## #name
static const CHAR16 *WEfiMemoryTypeToString[] =
{
    MemoryTypeToStringContext(EfiReservedMemoryType),
    MemoryTypeToStringContext(EfiLoaderCode),
    MemoryTypeToStringContext(EfiLoaderData),
    MemoryTypeToStringContext(EfiBootServicesCode),
    MemoryTypeToStringContext(EfiBootServicesData),
    MemoryTypeToStringContext(EfiRuntimeServicesCode),
    MemoryTypeToStringContext(EfiRuntimeServicesData),
    MemoryTypeToStringContext(EfiConventionalMemory),
    MemoryTypeToStringContext(EfiUnusableMemory),
    MemoryTypeToStringContext(EfiACPIReclaimMemory),
    MemoryTypeToStringContext(EfiACPIMemoryNVS),
    MemoryTypeToStringContext(EfiMemoryMappedIO),
    MemoryTypeToStringContext(EfiMemoryMappedIOPortSpace),
    MemoryTypeToStringContext(EfiPalCode),
    MemoryTypeToStringContext(EfiPersistentMemory),
    MemoryTypeToStringContext(EfiMaxMemoryType),
};
static const CHAR16 *WMemoryTypeName[] =
{
    MemoryTypeToStringContext(UNUSABLE_MEMORY),
    MemoryTypeToStringContext(RAM_MEMORY),
    MemoryTypeToStringContext(ROM_MEMORY),
    MemoryTypeToStringContext(ACPI_RECLAIM_MEMORY),
    MemoryTypeToStringContext(ACPI_NVS_MEMORY),
};
#undef MemoryTypeToStringContext

// EFI_STATUS UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
// {
//     UINTN descriptor_size = 0;
//     UINTN descriptor_count = 0;
//     EFI_MEMORY_DESCRIPTOR * MemMap = get_efi_memory_descriptors(&descriptor_size, &descriptor_count);

//     if (descriptor_count == 0)
//     {
//         return EFI_DEVICE_ERROR;
//     }

//     MemoryInfo * memory_info = get_memory_info(MemMap, descriptor_size, descriptor_count);
//     FreePool(MemMap);

//     return EFI_SUCCESS;
// }

EFI_MEMORY_DESCRIPTOR * get_efi_memory_descriptors(UINTN * descriptor_size, UINTN * count)
{
    UINTN MemMapSize = 0;
    EFI_MEMORY_DESCRIPTOR * MemMap = NULL;
    UINTN MapKey = 0;
    UINTN DescriptorSize = 0;
    UINT32 DesVersion = 0;

    gBS->GetMemoryMap(&MemMapSize, MemMap, &MapKey, &DescriptorSize, &DesVersion);
    MemMap = AllocatePool(MemMapSize);
    {
        if (MemMap == NULL)
        {
            Print(L"Alloc Memory failed!!\n");
#define ALLOC_MEMORY_FAILED L"Alloc Memory failed!\n"
            gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(ALLOC_MEMORY_FAILED), ALLOC_MEMORY_FAILED);
#undef ALLOC_MEMORY_FAILED
        }
    }
    gBS->GetMemoryMap(&MemMapSize, MemMap, &MapKey, &DescriptorSize, &DesVersion);

    *count = MemMapSize / DescriptorSize;
    *descriptor_size = DescriptorSize;
    if (*count == 0)
    {
        Print(L"Can't load the memory info!!\n");
#define GET_MEMORY_MAP_ERROR L"Can't load the memory info, the size is 0!\n"
        gBS->Exit(gImageHandle, EFI_LOAD_ERROR, sizeof(GET_MEMORY_MAP_ERROR), GET_MEMORY_MAP_ERROR);
#undef GET_MEMORY_MAP_ERROR
    }

    return MemMap;
}

DeviceMemoryInfo * get_memory_info(EFI_MEMORY_DESCRIPTOR * efi_memory_descriptors, UINTN descriptor_size, UINTN descriptor_count)
{
    if (descriptor_count == 0)
    {
        return EFI_DEVICE_ERROR;
    }

    DeviceMemoryInfo * memory_info = AllocatePool(sizeof(DeviceMemoryInfo));
    if (memory_info == NULL)
    {   
        Print(L"Alloc Memory failed!!\n");
#define ALLOC_MEMORY_FAILED L"Alloc Memory failed!\n"
        gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(ALLOC_MEMORY_FAILED), ALLOC_MEMORY_FAILED);
#undef ALLOC_MEMORY_FAILED
    }
    memory_info->count = 0;
    memory_info->memory = AllocatePool(descriptor_count * sizeof(MemoryDescriptor));
    
    if (memory_info->memory == NULL)
    {   
        Print(L"Alloc Memory failed!!\n");
#define ALLOC_MEMORY_FAILED L"Alloc Memory failed!\n"
        gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(ALLOC_MEMORY_FAILED), ALLOC_MEMORY_FAILED);
#undef ALLOC_MEMORY_FAILED
    }

    UINTN last = 0;
    for (int i = 0; i < descriptor_count; i++)
    {
        EFI_MEMORY_DESCRIPTOR * efi_memory_descriptor = (EFI_MEMORY_DESCRIPTOR *)((CHAR8 *)efi_memory_descriptors + i * descriptor_size);
        Print(L"<Physical Address: %016lx> <END in: %016lx> <Numbers: %16d> <Type: %s>\n", 
            efi_memory_descriptor->PhysicalStart, efi_memory_descriptor->PhysicalStart + ((efi_memory_descriptor->NumberOfPages) << EFI_PAGE_SHIFT),
            efi_memory_descriptor->NumberOfPages, WEfiMemoryTypeToString[efi_memory_descriptor->Type]);
        {
            UINTN memory_index = memory_info->count;
            MemoryDescriptor * descriptor = memory_info->memory + memory_index;
            DeviceMemoryType current_memory_type = get_memory_type(efi_memory_descriptor->Type);
            UINTN current_memory_address = efi_memory_descriptor->PhysicalStart;
            UINTN current_memory_size = efi_memory_descriptor->NumberOfPages << EFI_PAGE_SHIFT;
            if (last == 0)
            {
                last = 1;
                descriptor->address = current_memory_address;
                descriptor->size = current_memory_size;
                descriptor->type = current_memory_type;
                memory_info->count++;
            }
            else
            {
                UINTN last_index = memory_index - 1;
                MemoryDescriptor * last_descriptor = memory_info->memory + last_index;
                if (last_descriptor->type == current_memory_type && 
                    last_descriptor->address + last_descriptor->size == current_memory_address)
                {
                    last_descriptor->size += current_memory_size;
                }
                else
                {
                    descriptor->address = current_memory_address;
                    descriptor->size = current_memory_size;
                    descriptor->type = current_memory_type;
                    memory_info->count++;
                }
            }
        }
    }

    Print(L"There are the memory info:\n");
    for (int i = 0; i < memory_info->count; i++)
    {
        const MemoryDescriptor * descriptor = memory_info->memory + i;
        Print(L"<%16lx> <%-16lx> <%s>\n", descriptor->address, descriptor->size, WMemoryTypeName[descriptor->type]);
    }
    return memory_info;
}

efi_memory_type_info get_efi_memory_type_in_scope(EFI_MEMORY_DESCRIPTOR * memory_descriptors, UINTN descriptor_size, UINTN count, EFI_PHYSICAL_ADDRESS address, UINTN size)
{
    for (int i = 0; i < count; i++)
    {
        EFI_MEMORY_DESCRIPTOR * efi_memory_descriptor = (EFI_MEMORY_DESCRIPTOR *)((CHAR8 *)memory_descriptors + i * descriptor_size);
        EFI_MEMORY_TYPE current_memory_type = efi_memory_descriptor->Type;
        EFI_PHYSICAL_ADDRESS current_address = efi_memory_descriptor->PhysicalStart;
        UINTN page_size = efi_memory_descriptor->NumberOfPages << EFI_PAGE_SHIFT;
        EFI_PHYSICAL_ADDRESS current_address_end = current_address + page_size;
        if (current_address <= address && current_address_end >= address + size)
        {
            return (efi_memory_type_info){.type = current_memory_type, .valid = 1};
        }
    }

    return (efi_memory_type_info){.valid = 0};
}

VOID * alloc_indication_memory(EFI_PHYSICAL_ADDRESS address, UINTN size)
{
    UINTN descriptor_size = 0;
    UINTN descriptor_count = 0;
    EFI_MEMORY_DESCRIPTOR * MemMap = get_efi_memory_descriptors(&descriptor_size, &descriptor_count);
    efi_memory_type_info memory_type = get_efi_memory_type_in_scope(MemMap, descriptor_size, descriptor_count, address, size);
    FreePool(MemMap);

    if (memory_type.valid == 0)
    {
        Print(L"Can't find the memory location!!\n");
#define INVALID_MEMORY_POSITION L"Can't find the memory location!!\n"
        gBS->Exit(gImageHandle, EFI_INVALID_PARAMETER, sizeof(INVALID_MEMORY_POSITION), INVALID_MEMORY_POSITION);
#undef INVALID_MEMORY_POSITION
    }
    else
    {
        EFI_PHYSICAL_ADDRESS memory = address;
        switch (memory_type.type)
        {
            case EfiLoaderData:
            {
                EFI_STATUS status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, (size + (1U << EFI_PAGE_SHIFT)) >> EFI_PAGE_SHIFT, &memory);
                if (EFI_ERROR(status))
                {
                    Print(L"Alloc Memory Position Failed! Error code : %r\n", status);
#define ALLOC_PHYSICAL_ADDRESS_FAILED_STRING L"Alloc Memory Position Failed!\n"
                    gBS->Exit(gImageHandle, status, sizeof(ALLOC_PHYSICAL_ADDRESS_FAILED_STRING), ALLOC_PHYSICAL_ADDRESS_FAILED_STRING);
#undef ALLOC_PHYSICAL_ADDRESS_FAILED_STRING
                }
                return (VOID *)memory;
            }
            case EfiBootServicesData:
            {
                EFI_STATUS status = gBS->AllocatePages(AllocateAddress, EfiBootServicesData, (size + (1U << EFI_PAGE_SHIFT)) >> EFI_PAGE_SHIFT, &memory);
                if (EFI_ERROR(status))
                {
                    Print(L"Alloc Memory Position Failed! Error code : %r\n", status);
#define ALLOC_PHYSICAL_ADDRESS_FAILED_STRING L"Alloc Memory Position Failed!\n"
                    gBS->Exit(gImageHandle, status, sizeof(ALLOC_PHYSICAL_ADDRESS_FAILED_STRING), ALLOC_PHYSICAL_ADDRESS_FAILED_STRING);
#undef ALLOC_PHYSICAL_ADDRESS_FAILED_STRING
                }
                return (VOID *)memory;
            }
            case EfiRuntimeServicesData:
            {
                EFI_STATUS status = gBS->AllocatePages(AllocateAddress, EfiRuntimeServicesData, (size + (1U << EFI_PAGE_SHIFT)) >> EFI_PAGE_SHIFT, &memory);
                if (EFI_ERROR(status))
                    {
                    Print(L"Alloc Memory Position Failed! Error code : %r\n", status);
#define ALLOC_PHYSICAL_ADDRESS_FAILED_STRING L"Alloc Memory Position Failed!\n"
                    gBS->Exit(gImageHandle, status, sizeof(ALLOC_PHYSICAL_ADDRESS_FAILED_STRING), ALLOC_PHYSICAL_ADDRESS_FAILED_STRING);
#undef ALLOC_PHYSICAL_ADDRESS_FAILED_STRING
                    }
                    return (VOID *)memory;
            }
            case EfiConventionalMemory:
            {
                return (VOID *)address;
            }
            default:
            {
                Print(L"Can't find the memory location!!\n");
#define INVALID_MEMORY_POSITION L"Can't find the memory location!!\n"
                gBS->Exit(gImageHandle, EFI_INVALID_PARAMETER, sizeof(INVALID_MEMORY_POSITION), INVALID_MEMORY_POSITION);
#undef INVALID_MEMORY_POSITION
            }
        }
    }
}