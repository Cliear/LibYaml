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


void destroy_files(LoadFiles **files)
{
    for (LoadFiles *v = *files; v; v = *files)
    {
        *files = v->next;
        FreeMemoryMacro((void *)(v->name));
        FreeMemoryMacro((void *)(v->path));
        FreeMemoryMacro(v);
    }
}

void destroy_load_config(LoadConfig **config)
{
    destroy_files(&(*config)->files);
    FreeMemoryMacro((void *)((*config)->start_file_name));
}