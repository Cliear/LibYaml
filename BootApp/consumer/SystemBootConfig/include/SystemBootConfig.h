#ifndef LIBYAML_EVENT_CONSUMER_SYSTEM_BOOT_CONFIG_H
#define LIBYAML_EVENT_CONSUMER_SYSTEM_BOOT_CONFIG_H

#include <yaml_event_consumer.h>

#ifndef __cplusplus
typedef
#endif
struct LoadConfig
{
    const char *start_file_name;
    struct LoadFiles *files;
}
#ifndef __cplusplus
LoadConfig
#endif
;

#ifndef __cplusplus
typedef
#endif
struct LoadFiles
{
    const char *name;
    const char *path;
    SIZE_T_MACRO memory_locate;
    struct LoadFiles *next;
}
#ifndef __cplusplus
LoadFiles
#endif
;

#ifndef __cplusplus
typedef
#endif
/* Our example parser states. */
enum State
{
    STATE_ERROR, /* ERROR state */
    STATE_START,    /* start state */
    STATE_STREAM,   /* start/end stream */
    STATE_DOCUMENT, /* start/end document */
    STATE_SECTION,  /* top level */

    STATE_CONFIG_LIST,   /* Config list */
    STATE_CONFIG_VALUES, /* Config key-value pairs */
    STATE_CONFIG_KEY,    /* Config key */
    STATE_CONFIG_NAME,   /* Config name value */

    STATE_FLIST,     /* File list */
    STATE_FVALUES,   /* File key-value pairs */
    STATE_FKEY,      /* File key */
    STATE_FNAME,     /* File name */
    STATE_FPATH,  /* File color value */
    STATE_FMEMORY_LOCATE,  /* File count value */

    STATE_END, /* end state */
}
#ifndef __cplusplus
State
#endif
;

#endif