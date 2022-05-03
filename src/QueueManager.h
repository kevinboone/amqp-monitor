/*=====================================================================

  amqp-monitor

  QueueManager.h

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

#include <atomic>

#include "Queue.h"

/** It's convenient to define a new type to represent the
    queue map -- particular when used with an iterator. */
typedef std::map<std::string, Queue*> QueueList;

/** QueueManager is a singleton class that maintains a map
    linking queue names to Queue objects. */
class QueueManager 
  {
  private:

  /** QueueManager needs a reference to the container, because the
      Queues it create need a reference to the container, because
      the container manages the scheduling of work_queue instances. */
  proton::container& container;

  /** My private work_queue. */
  proton::work_queue work_queue;

  /** The set of queues being managed */
  QueueList queues;

  /** Count of messages sent. This is used to generate the message ID.
      Making it atomic reduces the likelihood that multiple messages 
      will end up with the same ID, in a multi-threaded context. */
  std::atomic<int> message_count;

public:

  QueueManager (proton::container& c);

  /** Add a method call to my work queue. */
  bool add (proton::work f) 
    {
    return work_queue.add(f);
    }

  /** Publish a text message to the named queue. This method is
      (I hope) thread safe. If there are subscribers on the queue,
      they each get the message immediately. If there are none, the message
      is lost. */
  void publish (const std::string &name, const std::string &text);

  /** Called from the ConnectionManager when a client creates a new
      link by which messages can be sent to it. This method either
      finds the Queue object for the clients queue name, or creates
      it. It then calls bindToQueue() on the Sender, so that the 
      Sender can assign itself to be the proton::messaging_handler
      for the link. */
  void find_queue_for_sender (Sender* s, std::string qn);
  };

