/*=====================================================================

  amqp-monitor

  config.h

  This file contains settings that can usefully be changed, but
  probably don't need to be changed often enough to have 
  command-line switches

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#pragma once

// Time in usec between status checks
#define TICK_INTERVAL 1000000

// The name of the queue that will publish "tick" messages
#define TICK_QUEUE "tick"

// The name of the queue that will publish CPU load alerts
#define LOAD_QUEUE "load"


