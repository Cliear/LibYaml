#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <yaml.h>
#include <SystemBootConfig.h>

int main(int argc, char *argv[])
{
    int code;
    yaml_parser_t parser;
    yaml_event_t event;

    yaml_parser_initialize(&parser);
    FILE * file;
    if(argc == 1)
    {
        file = fopen("system_boot.yaml", "r");
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
    yaml_parser_set_input_file(&parser, file);
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

    /* Output the parsed data. */
    LoadConfig * config = (LoadConfig *)consumer_imp.data;
    printf("The config start file name is %s\n", config->start_file_name);

    printf("There are the data of loading file:\n");
    LoadFiles * files = config->files;
    while (files)
    {
        printf("name:%s path: %s locate: %zu\n", files->name, files->path, files->memory_locate);
        files = files->next;
    }
    code = EXIT_SUCCESS;
    fclose(file);

done:
    clear_consumer();
    yaml_parser_delete(&parser);
    return code;
}
