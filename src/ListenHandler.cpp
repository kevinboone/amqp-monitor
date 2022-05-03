/*=====================================================================

  amqp-monitor

  ListenHandler.pp

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

#include <iostream>

#include "ListenHandler.h"
#include "ConnectionHandler.h"
#include "logging.h"

ListenHandler::ListenHandler (QueueManager& c) : queue_manager(c) 
  {
  }

/** on_accept()  is called in response to a new client connection. Its
    purpose is to return a connection_options object to Proton. 
    This object will specify a handler -- an instance of messaging_handler
    -- that will be associated with the new connection. */
proton::connection_options ListenHandler::on_accept (proton::listener&)
  {
  DDBG (std::cout << "Connection accepted" << std::endl;)
  proton::connection_options co;
  co.handler (*(new ConnectionHandler (queue_manager)));
  return co;
  }

void ListenHandler::on_open (proton::listener& l) 
  {
  DINFO (std::cout << "Server listening on port " << l.port() << std::endl;)
  }

void ListenHandler::on_error (proton::listener&, const std::string& s) 
  {
  DERR (std::cerr << "Listener error: " << s << std::endl;)
  exit (1); // The listener is running on a separate thread so, if
            //   an exception is thrown from here, it won't be caught.
  }


