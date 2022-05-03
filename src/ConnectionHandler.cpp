/*=====================================================================

  amqp-monitor

  ConnectionHandler.cpp

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

#include "QueueManager.h"
#include "ConnectionHandler.h"
#include "logging.h"

ConnectionHandler::ConnectionHandler (QueueManager& qm) : queue_manager(qm)
  {
  }

void ConnectionHandler::on_connection_open (proton::connection& c)
  {
  DDBG (std::cout << "ConnectionHandler open connecton " << c 
     << std::endl;)
  c.open(); 
  }

void ConnectionHandler::on_sender_open (proton::sender &sender)
  {
  DDBG (std::cout << "ConnectionHandler open sender " << sender 
     << std::endl;)
  std::string qn = sender.source().address();
  DDBG (std::cout << "Sender's address is " << qn 
     << std::endl;)
  // Note that a sender is created with reference to the connection's
  //   list of all senders. Senders can thus remove themselves from the
  //   list when they are closed by Proton
  Sender* s = new Sender (sender, senders);
  senders[sender] = s;
  // Ensure queue exists -- create it if not
  queue_manager.add (make_work (&QueueManager::find_queue_for_sender, 
     &queue_manager, s, qn));
  }

void ConnectionHandler::on_session_close (proton::session &session)
  {
  DDBG (std::cout << "ConnectionHandler close session " << session
     << std::endl;)
  // Unsubscribe all senders in this session.
  for (proton::sender_iterator i = session.senders().begin(); 
        i != session.senders().end(); ++i) 
    {
    // See if the session's sender is in out list of senders
    //   (but why should it not be?)
    SenderList::iterator j = senders.find(*i);
    if (j == senders.end()) continue;
    Sender* s = j->second;
    // If the sender has a queue, mark the sender unsubscribed
    //   from that queue
    if (s->get_queue()) 
      {
      // Schedule a call to Queue::unsubscribe()
      s->get_queue()->add_work 
          (make_work(&Queue::unsubscribe, s->get_queue(), s));
      }
    // Remove the session's sender from our list of senders
    senders.erase(j);
    }
  }

void ConnectionHandler::on_transport_close (proton::transport& t) 
  {
  DDBG (std::cout << "ConnectionHandler transport closed " 
    << t << std::endl;);
  for (proton::sender_iterator i = t.connection().senders().begin(); 
        i != t.connection().senders().end(); ++i) 
    {
    // See if the transport's sender is in out list of senders
    //   (but why should it not be?)
    SenderList::iterator j = senders.find (*i);
    if (j == senders.end()) return;
    Sender* s = j->second;
    // If the sender has a queue, mark the sender unsubscribed
    //   from that queue
    if (s->get_queue()) 
      {
      // Schedule a call to Queue::unsubscribe
      s->get_queue()->add_work 
           (make_work (&Queue::unsubscribe, s->get_queue(), s));
      }
    }
  // Delete this object, as the client connection is gone
  delete this; 
  }

void ConnectionHandler::on_error (const proton::error_condition& e)
  {
  DERR (std::cout << "Protocol error: " << e.what() << std::endl;)
  }


