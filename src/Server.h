/*=====================================================================

  amqp-monitor

  Server.h

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#pragma once

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

#include "QueueManager.h"
#include "ConnectionHandler.h"
#include "ListenHandler.h"

/** Server is the main class for this application. Its run() 
    method defines the program's lifetime. An instance of
    Server encapsulates a proton::container, from which all
    subsequent Proton entities are created. */
class Server 
  {
  public:

  /** Create a Server, specifying the listen address
      (which could be 0.0.0.0 or a real IP). The constructor
      sets up the QueueManeger and main listener. */
  Server (const std::string addr);

  /** Publish the specified text message to the queue with the specified
      name. The queue will be created if it doesn't exist but, in that
      case, no message will be sent -- if there were subscribers, the
      queue would exist already. */
  void publish (const std::string &name, const std::string &text);

  /** Run this server. In practice, this method does not
      exit, except in a catastrophic failure. */
  void run();

  private:

  /** This instance of Server will own the main proton::container. */
  proton::container container;
 
  // The main QueueManager and ListenHandler are owned by this
  //   object, and have the same lifetime.
  QueueManager queue_manager;
  ListenHandler listen_handler;
  };


