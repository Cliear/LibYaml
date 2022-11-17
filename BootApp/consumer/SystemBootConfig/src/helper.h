
#include <SystemBootConfig.h>

void bail(const char *msg);
void *bail_alloc(size_t size);
char *bail_strdup(const char *s);
void add_LoadConfig(LoadConfig **load_config, const char *name, LoadFiles *files);
void add_file(LoadFiles **varieties, const char *name, const char *path, UINTPTR_T_MACRO locate);
void destroy_load_config(LoadConfig **config);
void destroy_files(LoadFiles **varieties);