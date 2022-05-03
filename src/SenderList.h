/*=====================================================================

  amqp-monitor

  SenderList.h

  Define the SenderList type as a map containing mappings between
  proton::sender and Sender objects. This list is maintained by
  the ConnectionHandler, and is shared with all Sender objects so
  they can maintain their own mappings.

  The reason this type exists is just to simplify iterator
  definitions, which would be very ugly otherwise.

  Copyright (c)2022 Kevin Boone, GPL v3.0

=====================================================================*/

#pragma once

class Sender;

// Senders is a mapping between proton::sender objects and
//   internal Sender objects
typedef std::map <proton::sender, Sender*> SenderList;


