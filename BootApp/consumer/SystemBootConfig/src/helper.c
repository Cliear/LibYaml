// #include <lib.h>
#include "helper.h"

/* Helper to bail on error. */
void bail(const char *msg)
{   
    return;
}

/* Helper to allocate memory or bail. */
void *bail_alloc(size_t size)
{
    void *p = AllocMemoryMacro(size);
    if (!p)
    {
        bail("out of memory");
    }
    return p;
}

/* Helper to copy a string or bail. */
char *bail_strdup(const char *s)
{
    char *c = StrDupMacro(s ? s : "");
    if (!c)
    {
        bail("out of memory");
    }
    return c;
}

void add_LoadConfig(LoadConfig **load_config, const char *name, LoadFiles *files)
{
    LoadConfig *f = bail_alloc(sizeof(*f));
    f->start_file_name = bail_strdup(name);
    f->files = files;

    /* Append to list. */
    if (!(*load_config))
    {
        *load_config = f;
    }
    else
    {
        FreeMemoryMacro((void *)(f->start_file_name));
        FreeMemoryMacro(f);
    }
}

void add_file(LoadFiles **files, const char *name, const char *path, UINTPTR_T_MACRO locate)
{
    /* Create variety object. */
    LoadFiles *v = bail_alloc(sizeof(*v));
    v->name = bail_strdup(name);
    v->path = bail_strdup(path);
    v->memory_locate = locate;
    v->next = NULL;

    /* Append to list. */
    if (!*files)
    {
        *files = v;
    }
    else
    {
        LoadFiles *tail = *files;
        while (tail->next)
        {
            tail = tail->next;
        }
        tail->next = v;
    }
}

void add_device_info(DeviceInfo **device_info, UINTPTR_T_MACRO image_number, UINTPTR_T_MACRO location)
{
    /* Create variety object. */
    DeviceInfo *info = bail_alloc(sizeof(*info));
    info->image_number = image_number;
    info->loaction = location;

    if (!*device_info)
    {
        *device_info = info;
    }
    else
    {
        FreeMemoryMacro(info);
        (*device_info)->image_number = image_number;
        (*device_info)->loaction = location;
    }
}


void destroy_files(LoadFiles **files)
{
    if (*files != NULL)
    {
        for (LoadFiles *v = *files; v; v = *files)
        {
            *files = v->next;
            FreeMemoryMacro((void *)(v->name));
            v->name = NULL;
            FreeMemoryMacro((void *)(v->path));
            v->path = NULL;
            FreeMemoryMacro(v);
        }
    }
}

void destroy_load_config(LoadConfig **config)
{
    if (*config != NULL)
    {
        destroy_files(&(*config)->files);
        FreeMemoryMacro((void *)((*config)->start_file_name));
        (*config)->start_file_name = NULL;
    }
}

void destroy_device_info(DeviceInfo **device_info)
{
    if (*device_info != NULL)
    {
        FreeMemoryMacro((void *)(*device_info));
        *device_info = NULL;
    }
}