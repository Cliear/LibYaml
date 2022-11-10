#ifndef LIBYAML_EDK2_H
#define LIBYAML_EDK2_H

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SimpleFileSystem.h>

#define INT_MAX_MACRO                 (~0u >> 1)
#define SIZE_T_MACRO                  INTN
#define UINTPTR_T_MACRO               PHYSICAL_ADDRESS
#define PTR_DIFF_T_MACRO              INTN
#define OFFSET_OF_MACRO(type, member) OFFSET_OF(type, member)
#define ASSERT_MACRO(value)           ASSERT(value)
#define ASSERT_STATUS_MACRO(value)    ASSERT_EFI_ERROR(value)

typedef EFI_FILE_HANDLE FileHandle;
#define FileHandleTypeMacro FileHandle

typedef struct
{
    UINTN size;
    char  data[];
} InterfaceInternalMemoryType;

#define InterfaceInternalMemoryTypeDataOffset (OFFSET_OF_MACRO(InterfaceInternalMemoryType, data))

#define BaseAllocMemoryAddress(address)                                                                                \
    ((InterfaceInternalMemoryType *)((UINT8 *)address - InterfaceInternalMemoryTypeDataOffset))

static inline SIZE_T_MACRO WriteToFile(FileHandleTypeMacro file_handle, UINTN element_size, UINTN element_nums,
                                       void *buffer)
{
    UINTN      size   = element_size * element_nums;
    EFI_STATUS status = file_handle->Write(file_handle, &size, buffer);
    return status;
}
#define WriteToFileMacro(file_handle, element_size, element_nums, buffer)                                              \
    WriteToFile(file_handle, element_size, element_nums, buffer)

static inline SIZE_T_MACRO ReadFromFile(FileHandleTypeMacro file_handle, UINTN element_size, UINTN element_nums,
                                        void *buffer)
{
    UINTN      size   = element_size * element_nums;
    EFI_STATUS status = file_handle->Read(file_handle, &size, buffer);
    return status;
}
#define ReadFromFileMacro(file_handle, element_size, element_nums, buffer)                                             \
    ReadFromFile(file_handle, element_size, element_nums, buffer)

static inline int FileOpIsError(SIZE_T_MACRO retval, FileHandle file)
{
    return retval == EFI_SUCCESS;
}
#define FileOpIsSuccessfulMacro(retval, file) FileOpIsError(retval, file)

static inline void *AllocMemory(UINTN size)
{
    UINTN                        real_size = size + sizeof(InterfaceInternalMemoryType);
    InterfaceInternalMemoryType *memory    = (InterfaceInternalMemoryType *)AllocatePool(real_size);
    if (memory)
    {
        memory->size = real_size;
        return memory->data;
    }
    return memory;
}
#define AllocMemoryMacro(size) AllocMemory(size)

static inline void *ReAllocMemory(void *buffer, UINTN size)
{
    UINTN                        real_size  = size + sizeof(InterfaceInternalMemoryType);
    InterfaceInternalMemoryType *memory_src = BaseAllocMemoryAddress(buffer);
    InterfaceInternalMemoryType *memory =
        (InterfaceInternalMemoryType *)ReallocatePool(memory_src->size, real_size, memory_src->data);
    if (memory)
    {
        memory->size = real_size;
        return memory->data;
    }
    return memory;
}
#define ReAllocMemoryMacro(buffer, size) ReAllocMemory(buffer, size)

static inline void FreeMemory(void *buffer)
{
    InterfaceInternalMemoryType *memory = BaseAllocMemoryAddress(buffer);
    FreePool(memory);
}
#define FreeMemoryMacro(buffer) FreeMemory(buffer)

#define StringToInter(str) AsciiStrDecimalToUint64(str)
#define StringSafePrintf(buffer, size, format, ...) AsciiSPrint(buffer, size, format, __VA_ARGS__)

#endif