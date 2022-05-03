/*=====================================================================

  amqp-monitor

  main.cpp

  Entry point for amqp-monitor utility. Just sets up the operating
  environment and instantiates a Server, which does all the real
  work.

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#include <iostream>
#include <thread>
#include <getopt.h>

#include "monitor_thread.h"
#include "Server.h"
#include "logging.h"

int log_level = 2; 

/*=====================================================================

  show_help

=====================================================================*/
void show_help (void)
  {
  std::cout << NAME << " [options]" << std::endl;
  std::cout << "   -c, --cpu-load  load average trigger point (0.9)" 
    << std::endl;
  std::cout << "   -p, --port      listen port number (5672)" << std::endl;
  std::cout << "   -v, --version   show version" << std::endl;
  }

/*=====================================================================

  show_version

=====================================================================*/
void show_version (void)
  {
  std::cout << NAME << " version " << VERSION << std::endl;
  std::cout << "Copyright (c)2022 Kevin Boone, GPL v3.0" << std::endl;
  }

/*=====================================================================

  main 

=====================================================================*/
int main (int argc, char **argv) 
  {
  static struct option long_options[] =
    {
      {"help", no_argument, NULL, 'h'},
      {"version", no_argument, NULL, 'v'},
      {"log-level", required_argument, NULL, 'l'},
      {"cpu-load", required_argument, NULL, 'c'},
      {"port", required_argument, NULL, 'p'},
      {0, 0, 0, 0}
    };

  bool flag_version = false;
  bool flag_help = false;
  std::string port = "5672"; 
  double cpu_load_threshold = 0.9;

  int opt = 0;
  int ret = 0;

  while (ret == 0)
    {
    int option_index = 0;
    opt = getopt_long (argc, argv, "hvl:p:c:", long_options, &option_index);

    if (opt == -1) break;

    switch (opt)
      {
      case 'c':
        cpu_load_threshold = atof (optarg);
        break;
      case 'h':
        flag_help = true;
        break;
      case 'v':
        flag_version = true;
        break;
      case 'l':
        log_level = atoi (optarg);
        break;
      case 'p':
        port = optarg;
        break;
      default:
        ret = 1;
      }
    }

  if (flag_version)
    {
    show_version();
    ret = -1;
    }

  if (flag_help)
    {
    show_help();
    ret = -1;
    }

  if (ret == 0)
    {
    try 
      {
      std::string address((std::string) "0.0.0.0" + ":" + port);

      Server b (address);
      std::thread t (monitor_thread, &b, cpu_load_threshold); 
      b.run();
      } 
    catch (const std::exception& e) 
      {
      DERR (std::cout << "Server shutdown on exception: " 
         << e.what() << std::endl;)
      }
    }
  if (ret == -1) ret = 0;
  return ret;
  }

