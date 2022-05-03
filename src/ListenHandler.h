/*=====================================================================

  amqp-monitor

  ListenHandler.h

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#pragma once

#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/listen_handler.hpp>
#include <proton/listener.hpp>

#include "QueueManager.h"

/** ListenHandler, a singleton class, handles incoming connections
    signaled by Proton. */
class ListenHandler: public proton::listen_handler 
  {
  private:

  /** A reference to the main QueueManager, which is owned by the
      Server instance. */
  QueueManager& queue_manager;

  public:

  ListenHandler (QueueManager& c);

  private:

  /** Called when an incoming connection request from a client
      is accepted. We create a new ConnectionManager for the 
      connection, and assign it to be the connection's handler
      at the Proton level. */
  proton::connection_options on_accept (proton::listener&) override;

  /** Called when the listener has started. All we do here is log this
      fact. */
  void on_open (proton::listener& l) override;

  /** Handle listener errors, which are likely to be fatal. */
  void on_error (proton::listener&, const std::string& s) override;
  };




