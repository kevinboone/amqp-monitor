/*=====================================================================

  amqp-monitor

  ConnectionHandler.h

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
#include "Sender.h"
#include "SenderList.h"

/** There is one instance of ConnectionHandler for each connection
    created by a client. Each instance is created by the ListenHandler
    when the container detects an incoming connection. When a 
    client disconnects, the instance of ConnectionHandler deletes
    itself. 

    ConnectionHandler is a subclass of proton::messaging_handler, 
    so it can be assigned as the handler for a proton::connection. */

class ConnectionHandler : public proton::messaging_handler 
  {
  private:

  /** A reference to the main queue manager, set when this 
      object is constructed. */
  QueueManager& queue_manager;

  /** A mapping between proton::sender objects, and our
      Sender objects. This is maintained by the connection
      manager, and updated as senders (links) are opened
      and closed. */
  SenderList senders;

  public:

  /** Constructor takes a reference to the singleton QueueManager,
      which is owner by the Server object. */
  ConnectionHandler (QueueManager& qm);

  private:

  /** Called when the ListenManager detects a new 
      connection request. All we do is open the connection. */
  void on_connection_open (proton::connection& c) override; 

  /** When a new sender (link) is opened by Proton, create
      a new Sender wrapper object for it, then add it to
      the list of known senders. Then ask the QueueManager
      to create the new queue for the address specified
      in the link. Then hander the Sender assign itself to be
      the messaging_handler for the Proton sender object */
  void on_sender_open (proton::sender &sender) override;

  /** Called when a session is closed on a specific client 
      connection. Remove all senders associated with the
      session from our internal state. */
  void on_session_close(proton::session &session) override;

  /** Called in response to a transport-level error. Just
      log the error message. */
  void on_error(const proton::error_condition& e) override;

  /** Called when client connection closed. Remove any
      senders associated with the connection from
      out internal state, then delete
      this instance, as it is no longer required. */
  void on_transport_close(proton::transport& t) override;
  };


