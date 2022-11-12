#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/CpuLib.h>

#include <Library/UefiBootServicesTableLib.h>

#include <yaml.h>
#include <SystemBootConfig.h>

#define CONFIG_FILE_PATH L"etc\\boot_info.yaml"

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * get_file_system();
EFI_FILE_HANDLE get_root_dir(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * file_system);
EFI_FILE_HANDLE open_file(EFI_FILE_HANDLE root, const CHAR16 * path);
void close_file(EFI_FILE_HANDLE file);

EFI_STATUS UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    yaml_parser_t parser;
    yaml_event_t event;

    init_consumer();
    yaml_parser_initialize(&parser);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * fs = get_file_system();
    EFI_FILE_HANDLE root = get_root_dir(fs);
    EFI_FILE_HANDLE config_file = open_file(root, CONFIG_FILE_PATH);

    yaml_parser_set_input_file(&parser, config_file);

    State state = STATE_START;
    do
    {
        int status = yaml_parser_parse(&parser, &event);
        if (status == 0)
        {
            Print(L"yaml_parser_parse error in line %d\n", __LINE__ - 3);
            goto ERROR;
        }
        state = consumer_imp.handle(&consumer_imp, &event);
        if (state == STATE_ERROR)
        {
            Print(L"consume_event error %d\n", __LINE__ - 3);
            goto ERROR;
        }
        yaml_event_delete(&event);
    } while (state != STATE_END);

    Print(L"There are the yaml information:\n");

    LoadConfig * config = (LoadConfig *)consumer_imp.data;
    Print(L"The config start file name is %a\n", config->start_file_name);

    Print(L"There are the data of loading file:\n");
    LoadFiles * files = config->files;
    while (files)
    {
        Print(L"name:%a path: %a locate: %d\n", files->name, files->path, files->memory_locate);
        files = files->next;
    }
    Print(L"No more information! Start clear env\n");

    close_file(config_file);
    close_file(root);
    gBS->CloseProtocol(fs, &gEfiSimpleFileSystemProtocolGuid, ImageHandle, NULL);

    clear_consumer();
    yaml_parser_delete(&parser);

    Print(L"Work is end!\n");

ERROR:
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
            Print(L"Not Found etc file!: < %s >\nError code:%x\n", path, status);
#define FILE_PATH_OPEN_ERROR_STRING L"Open file: < %s > failed!\nError code:%x"
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