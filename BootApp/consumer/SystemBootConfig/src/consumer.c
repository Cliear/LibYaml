#include "SystemBootConfig.h"
#include "helper.h"
#include "yaml_event_consumer.h"
#include <interface.h>
// #include <lib.h>
#include <yaml.h>

static struct ConsumerContainer
{
    LoadConfig config;
    LoadFiles  file;
    LoadFiles *files;
    DeviceInfo device_info;
} container;

ConsumerStateHandleState start(EventConsumer *consumer, yaml_event_t *event);
ConsumerStateHandleState config_list(EventConsumer *consumer, yaml_event_t *event);
ConsumerStateHandleState section(EventConsumer *consumer, yaml_event_t *event);
ConsumerStateHandleState stream(EventConsumer *consumer, yaml_event_t *event);
ConsumerStateHandleState config_values(EventConsumer *consumer, yaml_event_t *event);
ConsumerStateHandleState config_key(EventConsumer *consumer, yaml_event_t *event);
ConsumerStateHandleState file_key(EventConsumer *consumer, yaml_event_t *event);
ConsumerStateHandleState file_value(EventConsumer *consumer, yaml_event_t *event);

ConsumerStateHandleState device_info_key(EventConsumer *consumer, yaml_event_t *event);
ConsumerStateHandleState device_info_list(EventConsumer *consumer, yaml_event_t *event);

static char *temp_ptr = NULL;

ConsumerStateHandleState end(EventConsumer *consumer, yaml_event_t *event)
{
    consumer->handle = start;
    return STATE_END;
}

ConsumerStateHandleState error(EventConsumer *consumer, yaml_event_t *event)
{
    consumer->handle = start;
    return STATE_ERROR;
}

ConsumerStateHandleState file_name(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
        {
            if (container.file.name)
            {
                FreeMemoryMacro((void *)(container.file.name));
                container.file.name = NULL;
            }
            container.file.name = bail_strdup((char *)event->data.scalar.value);

            consumer->handle    = file_key;
            return STATE_FKEY;
            default:
            {
                consumer->handle = error;
                return STATE_ERROR;
            }
        }
    }
}

ConsumerStateHandleState file_path(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
            if (container.file.path)
            {
                FreeMemoryMacro((void *)(container.file.path));
                container.file.path = NULL;
            }
            container.file.path = bail_strdup((char *)event->data.scalar.value);

            consumer->handle    = file_key;
            return STATE_FKEY;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

static SIZE_T_MACRO internel_string_to_inter(const char * str)
{
    if (str[0] == '0')
    {
        if (str[1] == 'x' || str[1] == 'X')
        {
            return (SIZE_T_MACRO)HexStringToInter(str);
        }
        else
        {
            return (SIZE_T_MACRO)StringToInter(str + 1);
        }
    }
    else
    {
        return (SIZE_T_MACRO)StringToInter(str);
    }
}

ConsumerStateHandleState file_locate(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
        {
            const char *number_str = (char *)event->data.scalar.value;
            SIZE_T_MACRO value = 0;
            if (event->data.scalar.length > 2 && number_str[0] == '-')
            {
                value = internel_string_to_inter(number_str + 1);
            }
            else
            {
                value = internel_string_to_inter(number_str);
            }
            container.file.memory_locate = value;
            consumer->handle             = file_key;
            return STATE_FKEY;
        }
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState file_key(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
            temp_ptr = (char *)event->data.scalar.value;
            if (StrCmpMacro(temp_ptr, "name") == 0)
            {
                consumer->handle = file_name;
                return STATE_FNAME;
            }
            else if (StrCmpMacro(temp_ptr, "Path") == 0)
            {
                consumer->handle = file_path;
                return STATE_FPATH;
            }
            else if (StrCmpMacro(temp_ptr, "MemoryLocate") == 0)
            {
                consumer->handle = file_locate;
                return STATE_FMEMORY_LOCATE;
            }
            else
            {
                consumer->handle = error;
                return STATE_ERROR;
            }
        case YAML_MAPPING_END_EVENT:
            add_file(&(container.files), container.file.name, container.file.path, container.file.memory_locate);
            FreeMemoryMacro((void *)(container.file.name));
            FreeMemoryMacro((void *)(container.file.path));
            container.file.name = NULL;
            container.file.path = NULL;
            consumer->handle    = file_value;
            return STATE_FVALUES;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState file_value(EventConsumer *consumer, yaml_event_t *event)
{

    switch (event->type)
    {
        case YAML_MAPPING_START_EVENT: consumer->handle = file_key; return STATE_FKEY;
        case YAML_SEQUENCE_END_EVENT: consumer->handle = config_key; return STATE_CONFIG_KEY;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState config_start_file_name(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
            if (container.config.start_file_name)
            {
                FreeMemoryMacro((void *)(container.config.start_file_name));
                container.config.start_file_name = NULL;
            }
            container.config.start_file_name = bail_strdup((char *)event->data.scalar.value);
            consumer->handle                 = config_key;
            return STATE_CONFIG_KEY;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState config_load_files(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SEQUENCE_START_EVENT: consumer->handle = file_value; return STATE_FVALUES;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState config_key(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
        {
            temp_ptr = (char *)event->data.scalar.value;
            if (StrCmpMacro(temp_ptr, "StartFile") == 0)
            {
                consumer->handle = config_start_file_name;
                return STATE_CONFIG_NAME;
            }
            else if (StrCmpMacro(temp_ptr, "Files") == 0)
            {
                consumer->handle = config_load_files;
                return STATE_FLIST;
            }
            else
            {
                consumer->handle = error;
                return STATE_ERROR;
            }
        }
        case YAML_MAPPING_END_EVENT:
        {
            SystemBootConfig * system_boot_config = (SystemBootConfig *)consumer->data;
            add_LoadConfig(&(system_boot_config->load_config), container.config.start_file_name, container.files);
            container.files  = NULL;
            consumer->handle = config_values;
            return STATE_CONFIG_VALUES;
        }
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState config_values(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_MAPPING_START_EVENT: consumer->handle = config_key; return STATE_CONFIG_KEY;
        case YAML_SEQUENCE_END_EVENT: consumer->handle = config_list; return STATE_CONFIG_LIST;
        case YAML_MAPPING_END_EVENT: consumer->handle = section; return STATE_SECTION;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState config_list(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_MAPPING_START_EVENT: consumer->handle = config_key; return STATE_CONFIG_VALUES;
        case YAML_MAPPING_END_EVENT: consumer->handle = section; return STATE_SECTION;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState device_info_image_number(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
        {
            const char *number_str = (char *)event->data.scalar.value;
            SIZE_T_MACRO value = 0;
            if (event->data.scalar.length > 2 && number_str[0] == '-')
            {
                value = internel_string_to_inter(number_str + 1);
            }
            else
            {
                value = internel_string_to_inter(number_str);
            }
            container.device_info.image_number = value;
            consumer->handle             = device_info_key;
            return STATE_FKEY;
        }
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState device_info_locate(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
        {
            const char *number_str = (char *)event->data.scalar.value;
            SIZE_T_MACRO value = 0;
            if (event->data.scalar.length > 2 && number_str[0] == '-')
            {
                value = internel_string_to_inter(number_str + 1);
            }
            else
            {
                value = internel_string_to_inter(number_str);
            }
            container.device_info.loaction = value;
            consumer->handle             = device_info_key;
            return STATE_FKEY;
        }
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState device_map_end(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_MAPPING_START_EVENT: consumer->handle = device_info_key; return STATE_CONFIG_KEY;
        case YAML_SEQUENCE_END_EVENT: consumer->handle = device_info_list; return STATE_CONFIG_LIST;
        case YAML_MAPPING_END_EVENT: consumer->handle = end; return STATE_END;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState device_info_key(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
        {
            temp_ptr = (char *)event->data.scalar.value;
            if (StrCmpMacro(temp_ptr, "ImageNumber") == 0)
            {
                consumer->handle = device_info_image_number;
                return STATE_DEVICE_INFO_IMAGE_NUMBER;
            }
            else if (StrCmpMacro(temp_ptr, "Location") == 0)
            {
                consumer->handle = device_info_locate;
                return STATE_DEVICE_INFO_LOCATION;
            }
            else
            {
                consumer->handle = error;
                return STATE_ERROR;
            }
        }
        case YAML_MAPPING_END_EVENT:
        {
            SystemBootConfig * system_boot_config = (SystemBootConfig *)consumer->data;
            add_device_info(&(system_boot_config->device_info), container.device_info.image_number, container.device_info.loaction);
            container.device_info.image_number = 0;
            container.device_info.loaction = 0;
            consumer->handle = device_map_end;
            return STATE_DEVICE_INFO_VALUES;
        }
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState device_info_list(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_MAPPING_START_EVENT: consumer->handle = device_info_key; return STATE_DEVICE_INFO_VALUES;
        case YAML_MAPPING_END_EVENT: consumer->handle = section; return STATE_SECTION;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState section(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_SCALAR_EVENT:
        {
            temp_ptr = (char *)event->data.scalar.value;
            if (StrCmpMacro(temp_ptr, "LoadFiles") == 0)
            {
                consumer->handle = config_list;
                return STATE_CONFIG_LIST;
            }
            else if (StrCmpMacro(temp_ptr, "DeviceInfo") == 0)
            {
                consumer->handle = device_info_list;
                return STATE_DEVICE_INFO_LIST;
            }
            else
            {
                consumer->handle = error;
                return STATE_ERROR;
            }
        }
        case YAML_DOCUMENT_END_EVENT: consumer->handle = stream; return STATE_STREAM;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState document(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_MAPPING_START_EVENT: consumer->handle = section; return STATE_SECTION;
        case YAML_DOCUMENT_END_EVENT: consumer->handle = stream; return STATE_STREAM;
        case YAML_SCALAR_EVENT:
        {
            const char * name = (char *)event->data.scalar.value;
            if (StrCmpMacro(name, "") == 0)
            {
                consumer->handle = document;
                return STATE_DOCUMENT;
            }
        }
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState stream(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_DOCUMENT_START_EVENT: consumer->handle = document; return STATE_DOCUMENT;
        case YAML_STREAM_END_EVENT: consumer->handle = end; return STATE_END;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

ConsumerStateHandleState start(EventConsumer *consumer, yaml_event_t *event)
{
    switch (event->type)
    {
        case YAML_STREAM_START_EVENT: consumer->handle = stream; return STATE_STREAM;
        default:
        {
            consumer->handle = error;
            return STATE_ERROR;
        }
    }
}

struct EventConsumer consumer_imp = {.handle = start, .data = NULL};

void init_consumer(void)
{
    if (consumer_imp.data)
    {
        FreeMemoryMacro(consumer_imp.data);
        consumer_imp.data = NULL;
    }
    SystemBootConfig *system_boot_config = bail_alloc(sizeof(SystemBootConfig));
    system_boot_config->load_config = NULL;
    system_boot_config->device_info = NULL;
    consumer_imp.data = system_boot_config;
    consumer_imp.handle = start;
}

void clear_consumer(void)
{
    if (container.config.start_file_name)
    {
        FreeMemoryMacro((void *)(container.config.start_file_name));
        container.config.start_file_name = NULL;
    }
    if (container.file.name)
    {
        FreeMemoryMacro((void *)(container.file.name));
        container.file.name = NULL;
    }
    if (container.file.path)
    {
        FreeMemoryMacro((void *)(container.file.path));
        container.file.path = NULL;
    }

    // destroy_files((&((LoadConfig *)(consumer.data))->files));
    SystemBootConfig * system_boot_config = (SystemBootConfig *)consumer_imp.data;
    destroy_load_config(&(system_boot_config->load_config));
    destroy_device_info(&(system_boot_config->device_info));

    if (consumer_imp.data)
    {
        FreeMemoryMacro(consumer_imp.data);
        consumer_imp.data = NULL;
    }
    consumer_imp.data   = NULL;
    consumer_imp.handle = start;

    temp_ptr            = NULL;
}