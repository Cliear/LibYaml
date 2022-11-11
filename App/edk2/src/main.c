#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>

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
    MemoryLocate: 0x100000";

CHAR16 buffer[1024];

EFI_STATUS UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Enter UEFI Successfully\n");

    yaml_parser_t parser;
    yaml_event_t event;
    yaml_parser_initialize(&parser);

    yaml_parser_set_input_string(&parser, data, sizeof(data) - 1);
    State state = STATE_START;
    init_consumer();

    do
    {
        Print(L"%d\n", __LINE__);
        int status = yaml_parser_parse(&parser, &event);
        Print(L"%d\n", __LINE__);
        if (status == 0)
        {
            Print(L"%d\n", __LINE__);
            SystemTable->ConOut->OutputString(SystemTable->ConOut, L"yaml_parser_parse error\n");
            goto ERROR;
        }
        Print(L"%d\n", __LINE__);
        state = consumer_imp.handle(&consumer_imp, &event);
        Print(L"%d\n", __LINE__);
        if (status == 0)
        {
            Print(L"%d\n", __LINE__);
            SystemTable->ConOut->OutputString(SystemTable->ConOut, L"consume_event error\n");
            goto ERROR;
        }
        Print(L"%d %d\n", __LINE__, status);
        Print(L"One loop end! The status is %d, loop exp is (%d, %d, %d)\n", 
                state, state != STATE_ERROR, state != STATE_END, state != STATE_ERROR && state != STATE_END);
        Print(L"STATE_ERROR is %d, STATE_END is %d, status is %d\n", STATE_ERROR, STATE_END, state);
        yaml_event_delete(&event);
    } while (state != STATE_ERROR && state != STATE_END);

    LoadConfig * config = (LoadConfig *)consumer_imp.data;

    AsciiStrToUnicodeStr(config->start_file_name, buffer);
    Print(L"Start parser the yaml\n");
    Print(L"The Config ptr is %p\n", config);

    LoadFiles * files = config->files;
    while (files)
    {
        AsciiStrToUnicodeStr(files->name, buffer);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, buffer);
        AsciiStrToUnicodeStr(files->path, buffer);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, buffer);
        
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"file end!\n");
    }
    
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Ok!\n");
    clear_consumer();
    yaml_parser_delete(&parser);

ERROR:
    CpuDeadLoop();
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hear\n");
    return EFI_SUCCESS;
}