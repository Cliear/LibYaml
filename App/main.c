#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char *argv[])
{
    int code;
    yaml_parser_t parser;
    yaml_event_t event;

    yaml_parser_initialize(&parser);
    FILE * file;
    if(argc == 1)
    {
        file = fopen("D:/2022/C/yaml/yaml/system_boot.yaml", "r");
    }
    else
    {
        file = fopen(argv[1], "r");
    }
    if (file == NULL)
    {
        fprintf(stderr, "Open File Failed!\n");
        return 0;
    }
    yaml_parser_set_input_string(&parser, (unsigned char *)data, sizeof(data) - 1);
    State state = STATE_START;
    init_consumer();
    do
    {
        int status = yaml_parser_parse(&parser, &event);
        if (status == 0)
        {
            fprintf(stderr, "yaml_parser_parse error\n");
            code = EXIT_FAILURE;
            goto done;
        }
        state = consumer_imp.handle(&consumer_imp, &event);
        // status = consume_event(&state, &event);
        if (state == 0)
        {
            fprintf(stderr, "consume_event error\n");
            code = EXIT_FAILURE;
            goto done;
        }
        yaml_event_delete(&event);
    } while (state != STATE_ERROR && state != STATE_END);

    // /* Output the parsed data. */
    // for (struct Fruit *f = state.flist; f; f = f->next)
    // {
    //     printf("fruit: name=%s, color=%s, count=%d\n", f->name, f->color, f->count);
    //     for (struct variety *v = f->varieties; v; v = v->next)
    //     {
    //         printf("  variety: name=%s, color=%s, seedless=%s\n", v->name, v->color, v->seedless ? "true" : "false");
    //     }
    // }
    code = EXIT_SUCCESS;
    fclose(file);

done:
    clear_consumer();
    yaml_parser_delete(&parser);
    return code;
}
