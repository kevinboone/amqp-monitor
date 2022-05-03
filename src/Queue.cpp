/*=====================================================================

  amqp-monitor

  Queue.cpp

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

#include "Queue.h"
#include "logging.h"

Queue::Queue (proton::container& c, const std::string& n) :
        work_queue(c), name(n)
  {
  }

void Queue::queueMsg (proton::message m) 
  { 
  DDBG (std::cout << "Adding message to queue " << name << std::endl;)
  int added = 0;
  for (Subscriptions::iterator i = subscriptions.begin(); 
        i != subscriptions.end(); i++)
    {
    // Put a sendMsg() call into the Sender's work queue.
    // (*i).first is the Sender instance. Note that it is passed
    //   to make_work in the argument list, as it is the implicit
    //   'this' in the method call sendMsg() 
    (*i).first->add_work (make_work (&Sender::sendMsg, (*i).first, m));
    added++;
    }
  DDBG(std::cout << "Added message for " << added 
    << " subscriber(s)" << std::endl;)
  }

void Queue::subscribe (Sender* s) 
  {
  DINFO (std::cout << "Client subscribed to queue " << name << std::endl;)
  subscriptions[s] = 0;
  }

void Queue::unsubscribe (Sender* s) 
  {
  DINFO (std::cout << "Client unsubscribed from queue " << name << std::endl;)
  subscriptions.erase(s);
  // Tell the Sender it has been unsubscribed -- schedule a call to
  //   Sender::unsubscribed
  s->add_work (make_work (&Sender::unsubscribed, s));
  }


