#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/CpuLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Guid/FileInfo.h>
#include <Library/MemoryAllocationLib.h>

#include <yaml.h>
#include <SystemBootConfig.h>
#include <memory_ctrl.h>

#define CONFIG_FILE_PATH L"etc\\boot_info.yaml"

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * get_file_system();
EFI_FILE_HANDLE get_root_dir(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * file_system);
EFI_FILE_HANDLE open_file(EFI_FILE_HANDLE root, const CHAR16 * path);
void close_file(EFI_FILE_HANDLE file);

LoadConfig * get_config(EFI_FILE_HANDLE root);
void read_system_file(EFI_FILE_HANDLE root, LoadFiles *file_des);

EFI_STATUS UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    init_consumer();
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * fs = get_file_system();
    EFI_FILE_HANDLE root = get_root_dir(fs);
    LoadConfig * config = get_config(root);

    UINTN descriptor_size = 0;
    UINTN descriptor_count = 0;
    EFI_MEMORY_DESCRIPTOR * MemMap = get_efi_memory_descriptors(&descriptor_size, &descriptor_count);

    if (descriptor_count == 0)
    {
        return EFI_DEVICE_ERROR;
    }
    MemoryInfo * memory_info = get_memory_info(MemMap, descriptor_size, descriptor_count);

    read_system_file(root, config->files);

    FreePool(MemMap);

    clear_consumer();
    close_file(root);
    gBS->CloseProtocol(fs, &gEfiSimpleFileSystemProtocolGuid, ImageHandle, NULL);
    CpuSleep();
    CpuDeadLoop();
    return EFI_SUCCESS;
}

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * get_file_system()
{
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * simple_file_system_protocol = NULL;
    {
        EFI_STATUS status = gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (void **)(&simple_file_system_protocol));
        if (EFI_ERROR(status))
        {
            Print(L"Open file system failed!!\n");
            CHAR16 file_system_open_error[] = L"Open file system failed!!\n";
            gBS->Exit(gImageHandle, status, sizeof(file_system_open_error) - 1, file_system_open_error);
        }
    }
    return simple_file_system_protocol;
}

void close_file(EFI_FILE_HANDLE file)
{
    EFI_STATUS status = file->Close(file);
    if (EFI_ERROR(status))
    {
        Print(L"Close the file failed!\n");
#define CLOSE_FILE_FAILED_STRING L"Close file failed!\n"
        gBS->Exit(gImageHandle, status, sizeof(CLOSE_FILE_FAILED_STRING), CLOSE_FILE_FAILED_STRING);
#undef CLOSE_FILE_FAILED_STRING
    }
}

EFI_FILE_HANDLE get_root_dir(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * fs)
{
    EFI_FILE_HANDLE root;
    {
        EFI_STATUS status = fs->OpenVolume(fs, &root);
        if (EFI_ERROR(status))
        {
            Print(L"Open root dir failed!\n");
            CHAR16 root_dir_open_error[] = L"Open root dir failed!\n";
            gBS->Exit(gImageHandle, status, sizeof(root_dir_open_error) - 1, root_dir_open_error);
        }
    }
    return root;
}

EFI_FILE_HANDLE open_file(EFI_FILE_HANDLE root, const CHAR16 * path)
{
    EFI_FILE_HANDLE file;
    {
        EFI_STATUS status = root->Open(root, &file, path, EFI_FILE_MODE_READ, 0);
        if (EFI_ERROR(status))
        {
            Print(L"Not Found etc file!: < %s >\nError code: %r\n", path, status);
#define FILE_PATH_OPEN_ERROR_STRING L"Open file: < %s > failed!\nError code: %r\n"
#define INTER_STRING_LENGTH 20
            UINTN path_string_length = StrLen(path);
            UINTN string_length = path_string_length + sizeof(FILE_PATH_OPEN_ERROR_STRING) + INTER_STRING_LENGTH;
            CHAR16 * error_string = AllocatePool(string_length);
            UnicodeSPrint(error_string, string_length, FILE_PATH_OPEN_ERROR_STRING, path, status);
#undef INTER_STRING_LENGTH
#undef FILE_PATH_OPEN_ERROR_STRING
            gBS->Exit(gImageHandle, status, string_length, error_string);
        }
    }
    return file;
}

LoadConfig * get_config(EFI_FILE_HANDLE root)
{
    yaml_parser_t parser;
    yaml_event_t event;
    yaml_parser_initialize(&parser);

    EFI_FILE_HANDLE config_file = open_file(root, CONFIG_FILE_PATH);

    yaml_parser_set_input_file(&parser, config_file);

    State state = STATE_START;
    do
    {
        int status = yaml_parser_parse(&parser, &event);
        if (status == 0)
        {
            Print(L"yaml_parser_parse error in line %d\n", __LINE__ - 3);
#define YAML_PASER_FAILED_STRING L"yaml_parser_parse failed!\n"
            gBS->Exit(gImageHandle, EFI_NOT_READY, sizeof(YAML_PASER_FAILED_STRING), YAML_PASER_FAILED_STRING);
#undef YAML_PASER_FAILED_STRING
        }
        state = consumer_imp.handle(&consumer_imp, &event);
        if (state == STATE_ERROR)
        {
            Print(L"consume_event error %d\n", __LINE__ - 3);
#define CONSUMER_YAML_EVENT_FAILED_STRING L"consume_event failed!\n"
            gBS->Exit(gImageHandle, EFI_NOT_READY, sizeof(CONSUMER_YAML_EVENT_FAILED_STRING), CONSUMER_YAML_EVENT_FAILED_STRING);
#undef CONSUMER_YAML_EVENT_FAILED_STRING
        }
        yaml_event_delete(&event);
    } while (state != STATE_END);

    yaml_parser_delete(&parser);
    close_file(config_file);

    LoadConfig * config = (LoadConfig *)consumer_imp.data;
    return (LoadConfig *)consumer_imp.data;
}

void read_system_file(EFI_FILE_HANDLE root, LoadFiles *file_des)
{
    while(file_des)
    {
        CHAR16 * path = AllocatePool((AsciiStrLen(file_des->path) + 1) * 2);
        if (!path)
        {
            Print(L"Alloc Pool Failed In Line %d\n", __LINE__ - 3);
#define ALLOC_POOL_FAILED_STRING L"Alloc Pool Failed!\n"
            gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(ALLOC_POOL_FAILED_STRING), ALLOC_POOL_FAILED_STRING);
#undef ALLOC_POOL_FAILED_STRING
        }
        AsciiStrToUnicodeStr(file_des->path, path);
        EFI_FILE_HANDLE disk_file = open_file(root, path);

        UINTN struct_size = sizeof(EFI_FILE_INFO) + StrLen(path) + 1;
        EFI_FILE_INFO*  file_info = AllocatePool(struct_size);
        if (!file_info)
        {
            Print(L"Alloc Pool Failed In Line %d\n", __LINE__ - 3);
#define ALLOC_POOL_FAILED_STRING L"Alloc Pool Failed!\n"
            gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(ALLOC_POOL_FAILED_STRING), ALLOC_POOL_FAILED_STRING);
#undef ALLOC_POOL_FAILED_STRING
        }
        disk_file->GetInfo(disk_file, &gEfiFileInfoGuid, &struct_size, file_info);
        Print(L"File < '%s' > has %ld bytes, will be loaded on %lx\n", path, file_info->FileSize, file_des->memory_locate);

        EFI_PHYSICAL_ADDRESS file_memory_address = (EFI_PHYSICAL_ADDRESS)file_des->memory_locate;
        UINTN file_size = (UINTN)file_info->FileSize;
        {
            VOID * file_address = alloc_indication_memory(file_memory_address, file_size);
            if (file_address == NULL)
            {
                Print(L"Alloc Memory Position Failed!\n");
#define ALLOC_PHYSICAL_ADDRESS_FAILED_STRING L"Alloc Memory Position Failed!\n"
                gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(ALLOC_PHYSICAL_ADDRESS_FAILED_STRING), ALLOC_PHYSICAL_ADDRESS_FAILED_STRING);
#undef ALLOC_PHYSICAL_ADDRESS_FAILED_STRING
            }
        }
        {
            EFI_STATUS status = disk_file->Read(disk_file, &file_size, (void *)file_memory_address);
            if (EFI_ERROR(status))
            {
                Print(L"Read file <%s> failed! Error code : %r\n", path, status);
#define READ_FILE_FAILED_STRING L"Alloc Memory Position Failed!\n"
                gBS->Exit(gImageHandle, status, sizeof(READ_FILE_FAILED_STRING), READ_FILE_FAILED_STRING);
#undef READ_FILE_FAILED_STRING
            }
        }
        FreePool(file_info);
        close_file(disk_file);
        FreePool(path);
        file_des = file_des->next;
    }
}