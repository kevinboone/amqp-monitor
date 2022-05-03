/*=====================================================================

  amqp-monitor

  monitor_thread.cpp

  The monitor_thread() function is started as a new thread by
  main(). It's job is to monitor whatever needs to be monitored,
  and call Server.publish() to publish whatever messages are
  required, to create the appropriate notifications.

  I've used CPU load here as an example, because it's easy to
  measure. However, all kinds of things could be monitored, and
  turned into AMQP messages. For example, the program could subscribe
  to DBUS, and publish messages indicated that removeable disks 
  have been plugged or unplugged. 

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#include <unistd.h>
#include <stdlib.h>

#include <iostream>

#include "Server.h"
#include "config.h"
#include "logging.h"

/*=====================================================================

  check_load 

  Returns true if the load average is above the set threshold

=====================================================================*/
bool check_load (double load_threshold)
  {
  double load = 0;
  getloadavg (&load, 1);
  DDBG (std::cout << "Load average is " << load << std::endl;)
  return load > load_threshold;
  }

/*=====================================================================

 monitor_thread 

=====================================================================*/

void monitor_thread (Server *b, double load_threshold)
  {
  // load_trip will be set true when the load avg has increased above
  //  threshold, until it falls below threshold.
  bool load_trip = false;
  while (true)
    {
    b->publish (TICK_QUEUE, "tick");
    if (load_trip)
      {
      // We are already above threshold
      if (!check_load (load_threshold))
        {
        // Fallen below threshold
        load_trip = false;
        DINFO (std::cout << "Load average has fallen below theshold" 
           << std::endl;)
        }
      }
    else
      {
      // We are presently below threshold
      if (check_load (load_threshold))
        {
        // Risen above threshold
        load_trip = true;
        DINFO (std::cout << "Load average has risen above theshold" 
           << std::endl;)
        b->publish (LOAD_QUEUE, "CPU load alert");
        }
      }
    usleep (TICK_INTERVAL);
    }
  }

