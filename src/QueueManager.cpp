/*=====================================================================

  amqp-monitor

  QueueManager.cpp

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
#include <ostream>
#include <sstream>

#include "Queue.h"
#include "QueueManager.h"
#include "logging.h"


QueueManager::QueueManager (proton::container& c) :
        container(c), work_queue(c), message_count(1)
  {
  }

void QueueManager::publish (const std::string &name, const std::string &text)
  {
  DDBG (std::cout << "Publishing to queue " << name << std::endl;)
  // See if the queue exists -- there is no storage in this utility so,
  //  if there is no queue, no point trying to publish a message.
  QueueList::iterator i = queues.find (name);
  if (i != queues.end())
    {
    // Format the message, including a message ID based on the
    //   message count (which increments atomically, making this 
    //   method thread-safe. Not that it really needs to be.)
    proton::message msg (text);
    char s[20];
    // Just a litte gotcha -- the number of digits required to format
    //  an "int" could be platform dependent. I'm not worrying about
    //  it in this simple example. 
    snprintf (s, sizeof (s) - 1, "ID:%08X", (int)(message_count++));
    proton::message_id id (s); 
    msg.id (id);
    (*i).second->queueMsg (msg); 
    }
  else
    DDBG (std::cout << "Queue " << name << 
       " has not been subscribed -- message lost" << std::endl;)
  }

void QueueManager::find_queue_for_sender (Sender* s, std::string qn) 
  {
  // We don't support dynamic queue creation. TODO -- can we reject the 
  //   client connection if it does not provide a link address ?
  if (qn.empty()) 
    {
    qn = "__NONAME__"; // This will lead to a client that gets no
                       //   messages. Not sure what else to do.
    }
  Queue* q = 0;
  QueueList::iterator i = queues.find(qn);
  if (i == queues.end()) 
    {
    q = new Queue (container, qn);
    queues[qn] = q;
    } 
  else 
    {
    q = i->second;
    }
  s->add_work (make_work (&Sender::bind_to_queue, s, q, qn));
  }


