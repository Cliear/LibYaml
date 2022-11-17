#ifndef LIBYAML_C_LANGUAGE_ENV_H
#define LIBYAML_C_LANGUAGE_ENV_H

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INT_MAX_MACRO                      INT_MAX
#define SIZE_T_MACRO                       size_t
#define UINTPTR_T_MACRO                    uintptr_t
#define PTR_DIFF_T_MACRO                   ptrdiff_t
#define OFFSET_OF_MACRO(type, member)      offset(type, member)
#define ASSERT_MACRO(value)                assert(value)
#define ASSERT_STATUS_MACRO(value)         ASSERT_MACRO(value)

#define MemSetMacro(buffer, value, length) memset(buffer, value, length)
#define MemCpyMacro(des, src, size)        memcpy(des, src, size)
#define MemMoveMacro(des, src, size)       memmove(des, src, size)
#define MemCmpMacro(s1, s2, num)           memcmp(s1, s2, num)
#define StrLenMacro(str)                   strlen(str)
#define StrDupMacro(str)                   strdup(str)
#define StrCmpMacro(s1, s2)                strcmp(s1, s2)
#define StrNCmpMacro(s1, s2, n)            strncmp(s1, s2, n)

typedef void *FileHandle;
#define FileHandleTypeMacro FileHandle

static inline SIZE_T_MACRO WriteToFile(FileHandleTypeMacro file_handle, SIZE_T_MACRO element_size,
                                       SIZE_T_MACRO element_nums, void *buffer)
{
    return fwrite(buffer, element_size, element_nums, (FILE *)file_handle);
}
#define WriteToFileMacro(file_handle, element_size, element_nums, buffer)                                              \
    WriteToFile(file_handle, element_size, element_nums, buffer)

static inline SIZE_T_MACRO ReadFromFile(FileHandleTypeMacro file_handle, SIZE_T_MACRO element_size,
                                        SIZE_T_MACRO element_nums, void *buffer, SIZE_T_MACRO * nread)
{
    return *nread = fread(buffer, element_size, element_nums, (FILE *)file_handle);
}
#define ReadFromFileMacro(file_handle, element_size, element_nums, buffer, nread)                                             \
    ReadFromFile(file_handle, element_size, element_nums, buffer, (nread))

static inline SIZE_T_MACRO FileOpIsError(SIZE_T_MACRO retval, FileHandle file)
{
    return !ferror((FILE *)file);
}
#define FileOpIsSuccessfulMacro(retval, file) FileOpIsError(retval, file)

static inline void *AllocMemory(SIZE_T_MACRO size)
{
    return malloc(size);
}
#define AllocMemoryMacro(size) AllocMemory(size)

static inline void *ReAllocMemory(void *buffer, SIZE_T_MACRO size)
{
    return realloc(buffer, size);
}
#define ReAllocMemoryMacro(buffer, size) ReAllocMemory(buffer, size)

static inline void FreeMemory(void *buffer)
{
    free(buffer);
}
#define FreeMemoryMacro(buffer) FreeMemory(buffer)

static inline SIZE_T_MACRO hex_string_to_inter_imp(const char *str)
{
    SIZE_T_MACRO value = 0;
    sscanf(str, "%zx", &value);
    return value;
}

static inline SIZE_T_MACRO oct_string_to_inter_imp(const char *str)
{
    SIZE_T_MACRO value = 0;
    sscanf(str, "%zo", &value);
    return value;
}

#define StringToInter(str)                          atoll(str)
#define HexStringToInter(str)                       hex_string_to_inter_imp(str)
#define StringSafePrintf(buffer, size, format, ...) snprintf(buffer, size, format, __VA_ARGS__)

#endif