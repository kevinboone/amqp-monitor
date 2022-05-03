/*=====================================================================

  amqp-monitor

  Queue.h

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#pragma once

#include <proton/connection.hpp>
#include <proton/connection_options.hpp>
#include <proton/listen_handler.hpp>
#include <proton/listener.hpp>
#include <proton/message.hpp>
#include <proton/message_id.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/sender_options.hpp>
#include <proton/source_options.hpp>
#include <proton/transport.hpp>
#include <proton/work_queue.hpp>

#include "Sender.h"

/** Subscriptions is a type that defines a 
    list of subscriptions, that is,
    Sender objects associated with this Queue. Actually,
    it's a map, because Senders are unique within a particular
    queue. Moreover, it's a map from a Sender to an int.
    I had in mind to use the int to store the amount of 
    link credit associated with the Sender, but I decided
    not to handle credit in this simple application at
    all (for now, at least). Note also that the map
    is based on Sender* objects. The Queue does not
    own these objects -- they are instantiated by the
    ConnectionManager instance that belongs to a specific
    client connection. */
typedef std::map<Sender*, int> Subscriptions;

/** Queue represents a queue, that is, a name that clients
    create links to, to receive messages. In this simple
    application, a Queue is really nothing more than a 
    link between a name, and a set of Sender objects that
    represent the subscribers to the queue. No storage is
    associated with a Queue. */
class Queue
  {
  private:

  /** Define a private work_queue */
  proton::work_queue work_queue;

  /** The name of this queue. */
  const std::string name;

  /** The list of subscribers (Sender objects) assigned to this
     queue. */
  Subscriptions subscriptions;

  public:

  /** Note that the Queue class needs a reference to the container, because
      the container manages the work queue. */
  Queue (proton::container &c, const std::string& n);

  /** Add a function call to my work queue. */
  bool add_work (proton::work f) 
    {
    return work_queue.add(f);
    }

  /** Add a message to this queue. Since there is no storage 
      associaeted with queues in this simple application, and we
      aren't handling credit, all we do is send the message 
      directly to every subscriber associated with the Queue. */
  void queueMsg (proton::message m);

  /** Register a Sender as being a subscriber to this queue. This 
      process is triggered by the ConnectionHandler's on_sender_open
      method being called in response to the client opening a
      new link. */ 
  void subscribe (Sender* s);

  /** Remove the sender as a subscriber to this Queue. This process
      is triggered by closing the session or the transport. */
  void unsubscribe (Sender* s); 
  };

