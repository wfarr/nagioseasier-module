#include <string.h>

#include <nagios/downtime.h>
#include <nagios/nagios.h>

#include "../helper.h"
#include "downtime.h"

static int
schedule_downtime_for_obj(int sd, host* hst, service* svc, unsigned long minutes, char* comment_data)
{
  if (hst || svc) {
    int           typedowntime = (svc ? SERVICE_DOWNTIME : HOST_DOWNTIME);
    char*         hst_name     = (svc ? svc->host_name : hst->name);
    char*         svc_name     = (svc ? svc->description : NULL);
    time_t        entry_time   = time(NULL);
    char*         author       = "nagioseasier";
    time_t        start_time   = time(NULL);
    time_t        end_time     = 0L; /* only used for fixed downtime, TODO some other time */
    int           fixed        = 0;
    unsigned long triggered_by = 0L; /* assume triggered by no obj in system? */
    unsigned long duration     = minutes * 60L; /* assuming duration is seconds */
    unsigned long downtime_id  = 0L;


    end_time = start_time + duration;

    if (svc_name) {
      nsock_printf_nul(sd, "SCHEDULED %lu MINUTES OF DOWNTIME FOR %s/%s WITH MESSAGE: %s\n", minutes, hst_name, svc_name, comment_data);
    } else {
      nsock_printf_nul(sd, "SCHEDULED %lu MINUTES OF DOWNTIME FOR %s WITH MESSAGE: %s\n", minutes, hst_name, comment_data);
    }

    int retval = schedule_downtime(typedowntime,
      hst_name,
      svc_name,
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      triggered_by,
      duration,
      &downtime_id);

    return (retval == OK ? 200 : 400);
  }

  return 404;
}

int
nez_cmd_schedule_downtime(int sd, char* object, char* rest)
{
  unsigned long minutes = 15L;
  char* comment_data;

  // assume the next argument is number of minutes
  if (rest) {
    if ((comment_data = strchr(rest, ' '))) {
      *(comment_data++) = 0;
    }

    minutes = strtoul(rest, NULL, 10);
  }

  minutes = (minutes > 1 ? minutes : 15L);

  host*    hst;
  service* svc;
  nez_find_host_or_service(object, &hst, &svc);

  if (svc) {
    schedule_downtime_for_obj(sd, NULL, svc, minutes, comment_data);
    return 200;
  }

  if (hst) {
    schedule_downtime_for_obj(sd, hst, NULL, minutes, comment_data);

    for (servicesmember* svcmem = hst->services; svcmem; svcmem = svcmem->next) {
      schedule_downtime_for_obj(sd, NULL, svcmem->service_ptr, minutes, comment_data);
    }

    return 200;
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR: %s", object);
  return 404;
}
