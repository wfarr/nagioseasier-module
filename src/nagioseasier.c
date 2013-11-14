/* system shit */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <nagios/common.h>
#include <nagios/config.h>
#include <nagios/downtime.h>
#include <nagios/objects.h>
#include <nagios/nagios.h>
#include <nagios/nebmodules.h>
#include <nagios/nebcallbacks.h>

#include "commands.h"
#include "helper.h"

/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION)

/* prototype the query handler functions to keep the compiler happy */
int qh_deregister_handler(const char* name);
int qh_register_handler(const char* name, const char* description, unsigned int options, qh_handler handler);

static int nagioseasier_query_handler(int sd, char* buf, unsigned int len);

/* this function gets called when the module is loaded by the event broker */
int
nebmodule_init(int flags, char* args, nebmodule* handle)
{
  (void)flags;
  (void)args;
  (void)handle;

  return qh_register_handler("nagioseasier", "The nagioseasier query handler", 0, nagioseasier_query_handler);
}

/* this function gets called when the module is unloaded by the event broker */
int
nebmodule_deinit(int flags, int reason)
{
  (void)flags;
  (void)reason;

  return qh_deregister_handler("nagioseasier");
}

static int
nagioseasier_query_handler(int sd, char* buf, unsigned int len)
{
  (void)len;

  char* action = buf;
  char* obj;

  /* separate our action and obj */
  if ((obj = strchr(buf, ' '))) {
    *(obj++) = 0;
  }

  /* shove the rest of the input into rest, leave obj alone */
  char* rest;
  if ((rest = strchr(obj, ' '))) {
    *(rest++) = 0;
  }

  return nez_lookup_command(action)(sd, obj, rest);
}
