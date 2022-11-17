#ifndef LIBYAML_EVENT_CONSUMER_H
#define LIBYAML_EVENT_CONSUMER_H

#include <yaml.h>
 
typedef struct EventConsumer EventConsumer;
typedef SIZE_T_MACRO ConsumerStateHandleState;
typedef ConsumerStateHandleState (*EventHandle)(EventConsumer*, yaml_event_t * event);
struct EventConsumer
{
    void * data;
    EventHandle handle;
};

extern struct EventConsumer consumer_imp;
extern void init_consumer(void);
extern void clear_consumer(void);

#endif