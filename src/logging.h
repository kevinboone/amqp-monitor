/*=====================================================================

  amqp-monitor

  logging.h 

  Logging functions. I've defined these as macros, so they can
  be compiled out completely, if no diagnostic output is required.

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#pragma once

#define DDBG(x) {if (log_level >= 3) {std::cout << "DEBUG "; x}}
#define DINFO(x) {if (log_level >= 2) {std::cout << "INFO "; x}}
#define DWARN(x) {if (log_level >= 1) {std::cout << "WARN "; x}}
#define DERR(x) { std::cout << "ERROR "; {x}}

// Log level: 0-3
extern int log_level;
