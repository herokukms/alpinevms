#ifndef WEB_H
#define WEB_H
#include "mongoose.h"
#include <pthread.h>

void handle_web(struct mg_connection *nc, int ev, void *ev_data);
void* mongooseThread(void* arg);

#endif // WEB_H