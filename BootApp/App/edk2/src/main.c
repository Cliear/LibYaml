#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/CpuLib.h>
#include <Library/BaseMemoryLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Guid/FileInfo.h>
#include <Library/MemoryAllocationLib.h>

#include <yaml.h>
#include <SystemBootConfig.h>
#include <memory_ctrl.h>

#include <device_boot_info.h>

#define CONFIG_FILE_PATH L"etc\\boot_info.yaml"

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * get_file_system();
EFI_FILE_HANDLE get_root_dir(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * file_system);
EFI_FILE_HANDLE open_file(EFI_FILE_HANDLE root, const CHAR16 * path);
void close_file(EFI_FILE_HANDLE file);

SystemBootConfig * get_system_boot_config(EFI_FILE_HANDLE root);
void read_system_file(EFI_FILE_HANDLE root, LoadFiles *file_des);
LoadFiles* find_startup_file(LoadConfig * config);

int init_video(DeviceVideoInfo * info);
void SetDisplayMode(int mode_num);
EFI_STATUS init_memory(DeviceInfo info);

NORETURN void longjmp(void (*position)(void));

EFI_STATUS UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    init_consumer();
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * fs = get_file_system();
    EFI_FILE_HANDLE root = get_root_dir(fs);
    SystemBootConfig * config = get_system_boot_config(root);

    init_memory(*(config->device_info));
    DeviceBootInfo * boot_info = (DeviceBootInfo *)config->device_info->loaction;
    boot_info->image_number = config->device_info->image_number;
    int display_mode_num = init_video(&(boot_info->video));

    read_system_file(root, config->load_config->files);

    clear_consumer();
    close_file(root);
    gBS->CloseProtocol(fs, &gEfiSimpleFileSystemProtocolGuid, ImageHandle, NULL);

    LoadFiles * startup_file = find_startup_file(config->load_config);
    void (*des_position)() = (void (*)())startup_file->memory_locate;

    SetDisplayMode(display_mode_num);

    longjmp(des_position);

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

SystemBootConfig * get_system_boot_config(EFI_FILE_HANDLE root)
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

    SystemBootConfig * config = (SystemBootConfig *)consumer_imp.data;
    return config;
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
            if (file_memory_address != NULL && file_address == NULL)
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

LoadFiles* find_startup_file(LoadConfig * config)
{
    if (config == NULL)
    {
        Print(L"Can't detect the config info!!!\n!");
#define DETECT_CONFIG_FAILED L"Can't detect the config info!!!\n\n"
        gBS->Exit(gImageHandle, EFI_LOAD_ERROR, sizeof(DETECT_CONFIG_FAILED), DETECT_CONFIG_FAILED);
#undef DETECT_CONFIG_FAILED
    }
    const char * startup_file_name = config->start_file_name;
    LoadFiles * file = config->files;
    while (file != NULL)
    {
        if (AsciiStrCmp(startup_file_name, file->name) == 0)
        {
            return file;
        }
    }

    {
        Print(L"Can't detect the startup file!!!\n!");
#define DETECT_STARTUP_FILE_FAILED L"Can't detect the startup file!!!\n\n"
        gBS->Exit(gImageHandle, EFI_LOAD_ERROR, sizeof(DETECT_STARTUP_FILE_FAILED), DETECT_STARTUP_FILE_FAILED);
#undef DETECT_STARTUP_FILE_FAILED
    }
}

int init_video(DeviceVideoInfo * info)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL * video_protocol = NULL;
    EFI_STATUS res = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, 
        NULL, &video_protocol);
    if (EFI_ERROR(res))
    {
        Print(L"Locate the GraphicsOutputProtocol failed\n!");
#define LOCATE_VIDEO_PROTOCOL_FAILED L"Locate the GraphicsOutputProtocol failed!!\n\n"
        gBS->Exit(gImageHandle, res, sizeof(LOCATE_VIDEO_PROTOCOL_FAILED), LOCATE_VIDEO_PROTOCOL_FAILED);
#undef LOCATE_VIDEO_PROTOCOL_FAILED
    }
    
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION * video_info = NULL;
    UINTN info_size;
    // UINT32 Mode;
    UINT32 num = MAX_UINT32;
    UINTN resolution = 0;

    for (UINT32 i = 0; i < video_protocol->Mode->MaxMode; i++)
    {
        video_protocol->QueryMode(video_protocol, i, &info_size, &video_info);
        Print
        (
            L"Mode: %-4d, Version:%x, Format: %d, Horizontal: %d, Vertical: %d, ScanfLine:%d\n",
            i, video_info->Version, video_info->PixelFormat,
            video_info->HorizontalResolution, video_info->VerticalResolution, video_info->PixelsPerScanLine 
        );
        if (video_info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor ||
            video_info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor)
        {
            UINTN current_mode_resolution = video_info->HorizontalResolution * video_info->VerticalResolution;
            if (current_mode_resolution > resolution)
            {
                resolution = current_mode_resolution;
                num = i;
            }
        }
        gBS->FreePool(video_info);
    }

    if (num != MAX_UINT32)
    {
        info->frame_buff      = video_protocol->Mode->FrameBufferBase;
        info->buff_size       = video_protocol->Mode->FrameBufferSize;
        info->width           = video_protocol->Mode->Info->HorizontalResolution;
        info->height          = video_protocol->Mode->Info->VerticalResolution;
        info->pixs_per_line   = video_protocol->Mode->Info->PixelsPerScanLine;
        info->format          = video_protocol->Mode->Info->PixelFormat;
    }
    gBS->CloseProtocol(video_protocol, &gEfiGraphicsOutputProtocolGuid, gImageHandle, NULL);

    return num;
}

void SetDisplayMode(int mode_num)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL * video_protocol = NULL;
    {
        EFI_STATUS res = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, 
            NULL, &video_protocol);
        if (EFI_ERROR(res))
        {
#define LOCATE_VIDEO_PROTOCOL_FAILED L"Locate the GraphicsOutputProtocol failed\n!\n"
            gBS->Exit(gImageHandle, res, sizeof(LOCATE_VIDEO_PROTOCOL_FAILED), LOCATE_VIDEO_PROTOCOL_FAILED);
#undef LOCATE_VIDEO_PROTOCOL_FAILED
        }
    }
    {
        EFI_STATUS res = video_protocol->SetMode(video_protocol, mode_num);
        if (EFI_ERROR(res))
        {
#define SET_DISPLAY_MODE_FAILED L"Set display mode failed\n!\n"
            gBS->Exit(gImageHandle, res, sizeof(SET_DISPLAY_MODE_FAILED), SET_DISPLAY_MODE_FAILED);
#undef SET_DISPLAY_MODE_FAILED
        }
    }

    gBS->CloseProtocol(video_protocol, &gEfiGraphicsOutputProtocolGuid, gImageHandle, NULL);
}

EFI_STATUS init_memory(DeviceInfo info)
{
    UINTN descriptor_size = 0;
    UINTN descriptor_count = 0;
    EFI_MEMORY_DESCRIPTOR * MemMap = get_efi_memory_descriptors(&descriptor_size, &descriptor_count);
    if (descriptor_count == 0)
    {
        return EFI_DEVICE_ERROR;
    }
    DeviceMemoryInfo * memory_info = get_memory_info(MemMap, descriptor_size, descriptor_count);
    if (memory_info->count == 0)
    {
        Print(L"Init memory info failed!\n");
#define GET_MEMORY_INFO_FAILED L"Init memory info failed!\n"
        gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(GET_MEMORY_INFO_FAILED), GET_MEMORY_INFO_FAILED);
#undef GET_MEMORY_INFO_FAILED
    }

    DeviceBootInfo * device_boot_info = alloc_indication_memory(info.loaction, 
                            memory_info->count * sizeof(*(memory_info->memory)) + sizeof(DeviceMemoryInfo));
    if (info.loaction != NULL && device_boot_info == NULL)
    {
        Print(L"Alloc Memory Position Failed!\n");
#define ALLOC_PHYSICAL_ADDRESS_FAILED_STRING L"Alloc Memory Position Failed!\n"
        gBS->Exit(gImageHandle, EFI_OUT_OF_RESOURCES, sizeof(ALLOC_PHYSICAL_ADDRESS_FAILED_STRING), ALLOC_PHYSICAL_ADDRESS_FAILED_STRING);
#undef ALLOC_PHYSICAL_ADDRESS_FAILED_STRING
    }
    device_boot_info->memory.count = memory_info->count;
    device_boot_info->memory.memory = (UINTN)device_boot_info + sizeof(*device_boot_info);
    CopyMem(device_boot_info->memory.memory, memory_info->memory, memory_info->count * sizeof(*(memory_info->memory)));

    FreePool(MemMap);
}

NORETURN void longjmp(void (*position)(void))
{
    UINTN MapKey = 0;
    UINTN MemMapSize = 0;
    EFI_MEMORY_DESCRIPTOR * MemMap = 0;
    UINTN DescriptorSize = 0;
    UINT32 DesVersion = 0;
    gBS->GetMemoryMap(&MemMapSize, MemMap, &MapKey, &DescriptorSize, &DesVersion);
    gBS->ExitBootServices(gImageHandle, MapKey);

    position();
}