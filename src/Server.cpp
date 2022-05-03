/*=====================================================================

  amqp-monitor

  Server.cpp

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#include <proton/connection.hpp>
#include <proton/connection_options.hpp>
#include <proton/container.hpp>
#include <proton/listen_handler.hpp>
#include <proton/listener.hpp>
#include <proton/message.hpp>
#include <proton/message_id.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/sender_options.hpp>
#include <proton/source_options.hpp>
#include <proton/transport.hpp>
#include <proton/work_queue.hpp>

#include <ostream>
#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "Server.h"
#include "QueueManager.h"
#include "ConnectionHandler.h"
#include "logging.h" 

Server::Server (const std::string addr) :
        container (NAME), queue_manager (container), 
        listen_handler (queue_manager)
 {
 DDBG (std::cout << "Starting listener" << std::endl;)
 container.listen (addr, listen_handler);
 }

void Server::publish (const std::string &name, const std::string &text)
  {
  queue_manager.publish (name, text);
  }

void Server::run() 
  {
  DDBG (std::cout << "Running container" << std::endl;)
  container.run (std::thread::hardware_concurrency());
  }

