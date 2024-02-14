#include "mongoose.h"

void *mongooseThread(void *arg)
{
  struct mg_mgr *mgr = (struct mg_mgr *)arg;
  for (;;)
    mg_mgr_poll(mgr, 1000); // Infinite event loop
  return NULL;
}

void handle_web(struct mg_connection *c, int ev, void *ev_data)
{
  if (ev == MG_EV_HTTP_MSG)
  {
    // The MG_EV_HTTP_MSG event means HTTP request. `hm` holds parsed request,
    // see https://mongoose.ws/documentation/#struct-mg_http_message
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;

    // If request is /healthz, return 200 OK with a message in the body
    if (mg_http_match_uri(hm, "/healthz"))
    {
      mg_http_reply(c, 200, "", "OK\n");
    }
    else
    {
      mg_http_reply(c, 404, "", "Not found\n");
    }
  }
}