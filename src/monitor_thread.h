/*=====================================================================

  amqp-monitor

  monitor_thread.h

  The function monitor_thread runs in a separate thread from the
  message server, and collects the information that is to 
  be published.

  In this simple example, we just collect CPU load average figures.

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#pragma once

class Server;

void monitor_thread (Server *b, double load_threshold);

