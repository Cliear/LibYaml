#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/CpuLib.h>

#include <yaml.h>
#include <SystemBootConfig.h>

char data[] =
"LoadFiles:\n\
  StartFile: System\n\
  Files:\n\
  - \n\
    name: System\n\
    Path: abc/def.ddd\n\
    MemoryLocate: 0x100000\n\
  - \n\
    name: Boot\n\
    Path: abc/def.ddd\n\
    MemoryLocate: 0x200000";

CHAR16 buffer[1024];

EFI_STATUS UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    yaml_parser_t parser;
    yaml_event_t event;

    init_consumer();
    yaml_parser_initialize(&parser);

    yaml_parser_set_input_string(&parser, data, sizeof(data) - 1);
    State state = STATE_START;
    do
    {
        int status = yaml_parser_parse(&parser, &event);
        if (status == 0)
        {
            ErrorPrint(L"yaml_parser_parse error in line %d\n", __LINE__ - 3);
            goto ERROR;
        }
        state = consumer_imp.handle(&consumer_imp, &event);
        if (status == 0)
        {
            ErrorPrint(L"consume_event error %d\n", __LINE__ - 3);
            goto ERROR;
        }
        yaml_event_delete(&event);
    } while (state != STATE_ERROR && state != STATE_END);

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
    clear_consumer();
    yaml_parser_delete(&parser);

    Print(L"Work is end!\n");

ERROR:
    CpuSleep();
    CpuDeadLoop();
    return EFI_SUCCESS;
}