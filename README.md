# Nagios Easier

Once upon a time, some pretty all right folks wrote a tool [nagiosharder](https://github.com/railsmachine/nagiosharder). It allowed you to interact with Nagios without having to load the web interface. It was pretty all right too, but due to the nature of screen scraping, it tended to break whenever Nagios versions changed (that's no fun!) and meant more screen scraping had to be done for all sorts of nested tables.

Nagios Easier attempts to provide a similarly useful interface, but using the new Query Handler support in Nagios 4. This allows us to interact with Nagios using a Unix domain socket. Since this API is standardized, it's a lot less brittle than worrying about screen scraping 5x nested tables.

## Installation

### Dependencies

* nagios
* libltdl-dev

### Building

Then you should just be able to build this with:

```
make
```

And then you'll get a `nagioseasier.so` file. You can put it wherever, really, but I recommend `/usr/lib/nagioseasier/nagioseasier.so`.

From there you can load it in your Nagios configuration as an Event Broker Module with a config line like so:

```
broker_module=/usr/lib/nagioseasier/nagioseasier.so
```

## Using

Enabling this module in Nagios adds a new `nagioseasier` query handler to the Nagios query handler socket. This allows you to interact with Nagios for common administrative tasks in a convenient way. Some examples of what you can do with nagioseasier:

```
#nagioseasier status app1
#nagioseasier status app1/nginx

#nagioseasier downtime app1/nginx 15 @wfarr testing changes

#nagioseasier disable_notifications busted1

#nagioseasier problems puppet-environment
#nagioseasier problems puppet-environment unknown
```

### Commands

```
Available commands:
  status <host|service>                          Display the status of a host or service
  check <host|service>                           Schedule a re-check of the host or service

  enable_notifications <host|service>            Enable notifications for a host or host-service
  disable_notifications <host|service>           Disable notifications for a host or host-service

  acknowledge <host|service> [<comment>]         Acknowledge a host/service problem (opt. comment)
  unacknowledge <host|service>                   Unacknowledge a host/service problem

  downtime <host|service> [<minutes> <comment>]  Schedule downtime for a host/service (opt. num minutes, comment)

  problems [<svcgroup|hstgroup> <state>]         Display all services in a non-OK state
```
