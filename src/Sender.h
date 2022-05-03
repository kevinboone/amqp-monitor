/*=====================================================================

  amqp-monitor

  Sender.h

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

#include <map>

#include "SenderList.h"

class Sender;
class Queue;
class ConnectionHandler;

/**
 Class Sender
 */
class Sender : public proton::messaging_handler 
  {
  //friend class ConnectionHandler;

  /** A reference to the underlying proton::sender object encapsulated
      by this object. */
  proton::sender sender;

  /** A reference to the list of senders, maintained by the 
      ConnectionManager. */
  SenderList& senders;

  /** My private work queue. */
  proton::work_queue& work_queue;

  std::string queue_name;

  /* The Queue to which this Sender is attached. */
  Queue* queue;

  void on_sender_close (proton::sender &sender) override;

  public:

  Sender (proton::sender s, SenderList& ss);

  /** get_queue() is called by ConnectionManager, to determine the
      Queue assigned to a specific Sender. */
  Queue *get_queue() { return queue; }

  /** Add a method call to my private work queue. */
  bool add_work (proton::work f) 
    {
    return work_queue.add(f);
    }

  /** Send a specific message immediately, to all subscribers on this 
      Queue. */
  void sendMsg (proton::message m);

  /** Called by the Queue which a client unsubscribed. This object can
      delete itself at this point. */
  void unsubscribed();

  /** Called by the QueueManager when a client subscribes to a Queue. 
      This instance registers itself with Proton as the handler for 
      sender events. */
  void bind_to_queue (Queue* q, std::string qn);
  };

