/* include (minimum required) event broker header files */
#include "../include/nebmodules.h"
#include "../include/nebcallbacks.h"

/* include other event broker header files that we need for our work */
#include "../include/nebstructs.h"
#include "../include/broker.h"

/* include some Nagios stuff as well */
#include "../include/config.h"
#include "../include/common.h"
#include "../include/nagios.h"

/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION);


static int nagioseasiser_query_handler(int sd, char *buf, unsigned int len);

/* this function gets called when the module is loaded by the event broker */
int nebmodule_init(int flags, char *args, nebmodule *handle)
{
  return qh_register_handler("nagioseasier", "The nagioseasier query handler", 0, nagioseasier_query_handler);
}

/* this function gets called when the module is unloaded by the event broker */
int nebmodule_deinit(int flags, int reason)
{
  return qh_deregister_handler("nagioseasier");
}

static int nagioseasier_query_handler(int sd, char *buf, unsigned int len)
{
  char *space;

  if (!*buf || !strcmp(buf, "help")) {
    nsock_printf_nul(sd, "Query handler for actually doing useful shit with this socket.\n"
      "Available commands:\n"
      "  enable_notifications    Enable notifications for a host or host-service\n"
      "  disable_notifications   Disable notifications for a host or host-service\n"
    );
    return 0;
  }

  const char *obj = "ops-breakin1-pe1-prd/last_puppet_run";

  if ((space = memchr(buf, ' ', len)))
    *(space++) = 0;

  if (!space && !strcmp(buf, "enable_notifications")) {
    host    *hst = find_host(obj);
    service *svc = find_service(obj);

    if (svc) {
      enable_service_notifications(svc);
      nsock_printf_nul(sd, "NOTIFICATIONS ENABLED FOR SERVICE: %s", svc->display_name);
      return 200;
    }

    if (hst) {
      enable_host_notifications(hst);
      nsock_printf_nul(sd, "NOTIFICATIONS ENABLED FOR HOST: %s", hst->display_name);
      return 200;
    }

    nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR: %s", obj);
    return 404;
  }


  if (!space && !strcmp(buf, "disable_notifications")) {
    host    *hst = find_host(obj);
    service *svc = find_service(obj);

    if (svc) {
      disable_service_notifications(svc);
      nsock_printf_nul(sd, "NOTIFICATIONS DISABLED FOR SERVICE: %s", svc->display_name);
      return 200;
    }

    if (hst) {
      disable_host_notifications(hst);
      nsock_printf_nul(sd, "NOTIFICATIONS DISABLED FOR HOST: %s", hst->display_name);
      return 200;
    }

    nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR: %s", obj);
    return 404;
  }

  nsock_printf_nul(sd, "UNKNOWN COMMAND");
  return 400;
}
