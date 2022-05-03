/*=====================================================================

  amqp-monitor

  Sender.cpp

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

#include "Sender.h"
#include "Queue.h"
#include "logging.h"

Sender::Sender (proton::sender s, SenderList& ss) :
        sender(s), senders(ss), work_queue(s.work_queue()), queue(0)
  {
  }

void Sender::sendMsg (proton::message m) 
  {
  DDBG (std::cout << "Sender object " << this 
     << " sending message to client" << std::endl;);
  sender.send(m);
  }

void Sender::unsubscribed() 
  {
  DDBG (std::cout << "Deleting sender object " << this << std::endl;);
  delete this;
  }

void Sender::on_sender_close (proton::sender &sender) 
  {
  DDBG (std::cout << "Sender object " << this << " closing" <<  std::endl;);
  if (queue) 
    {
    DDBG (std::cout << "Unsubscribing Sender object " << this << 
       "from Queue object " << queue << std::endl;);
    queue->add_work (make_work (&Queue::unsubscribe, queue, this));
    } 
  // Remove this Sender from the list of Senders held by the 
  //   ConnectionManager
  senders.erase (sender);
  }

void Sender::bind_to_queue (Queue* q, std::string qn) 
  {
  DDBG (std::cout << "Sender object " << this << " bound to Queue object " 
     << q <<" (name " << qn << ")" << std::endl;);
  queue = q;
  queue_name = qn;

  q->add_work (make_work (&Queue::subscribe, q, this));
  sender.open (proton::sender_options()
        .source ((proton::source_options().address (queue_name)))
        .handler(*this));
  }


