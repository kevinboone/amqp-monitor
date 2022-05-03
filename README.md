# amqp-monitor

Version 0.1a 
Kevin Boone, May 2022

## What is this?

`amqp-monitor` demonstrates how to create a simple "message server" for Linux
using the Apache Qpid Proton library, that AMQP clients can connect to and
receive messages about system status or other events.  `amqp-monitor`
is implemented in C++, because it's relatively easy to get low-level status
information, or interface to sensors, using C/C++; but almost any
programming language has a library for handling AMQP messaging. Java, for
example, has Qpid JMS. The basic purpose of `amqp-monitor` is to distribute
monitoring or telemetry information, either within a host or across a
network, that is independent of programming language.

The principle of operation is that AMQP clients will connect to a queue that
publishes information of the required type. Then, the client will
receive updates or alerts on that queue. Any number of clients can connect to
the same "Server" -- all will receive a copy of the message.  If a client is
not connected, messages are not queued, but discarded.  In JMS terms, the queue
is actually a "topic". In other technologies, it might be called a "multicast
queue".

In this example, the only measure published is the system load average.
Clients can subscribe to the queue "load", and receive an alert when the load
exceeds a set threshold. There's also a queue called "tick", that publishes a
message once a second, primarily for testing purpose. Other applications
might include, for example, distributing information from an external
sensor (temperature, pressure, orientation...)

Please note that _this is not a complete application_. It is an educational
example, that could be extended to perform more complex monitoring operations. 

## Usage

Just start the server using the --cpu-load property to set the point at which a
load average alert will be triggered. The load average is a decimal number that
is usually less than 1.0 in a healthy system.

For example:

    $ amqp-monitor --cpu-load 20

The server port can be changed using the `--port` switch; the default is 5672.
To see a lot of diagnostic information, use `--log-level 3`.  To see nothing at
all, use `--log-level 0`.

Clients should subscribe to the queue `load` to get load average notifications.
For testing purposes, feel free to use my Java `amqutil` utility, available
here:

https://github.com/kevinboone/amqutil

For example:

    $ amqutil subscribe 100 --qpid --destination load --format text

Here is a Python (3) example, that uses the Qpid Proton Python library
(`dnf install python3-qpid-proton`):

    #!/usr/bin/env python3

    from proton.handlers import MessagingHandler
    from proton.reactor import Container

    address="localhost:5672/load"

    class Recv (MessagingHandler):
	def __init__ (self, url):
	    super (Recv, self).__init__()
	    self.url = url

	def on_start (self, event):
	    event.container.create_receiver(self.url)

	def on_message (self, event):
		print (event.message.body)

    Container (Recv (address)).run()

For debugging purposes, subscribe to the queue "tick"; this publishes
one message every second, regardless of conditions.

## Building

You'll need the Proton library with development headers. On 
Ubuntu, install using 

    $ sudo apt-get install libqpid-proton-cpp12-dev 

On Fedora/RHEL, use

   $ sudo dnf install qpid-proton-cpp-devel

Then, just run `make`.

## Internals

The monitoring work is done in the function `monitor_thread`, in the file
`monitor_thread.cpp`. This function runs until the server is stopped. It takes
a `Server` instance as an argument. The `Server` class exposes only one useful
method to the monitor thread: `Server.publish()`.  The `publish()` method takes
two `std::string` arguments: the first is the name of the queue to which to
publish; the second is the text to publish. Any number of queues can be
published to; they are created if they do not already exist. More on the
`Server` class later.

`monitor_thread.cpp` is the only file that is not concerned with Proton, and
the management of AMQP connections.

The main work of the AMQP engine is encapsulated in the `proton::container`
class. An instance of this class is initialized along with the `Server`
instance, as is the `QueueManager` instance, which maintains the mapping
between queue names and `Queue` objects. A `Queue` object holds the list of
subscribers (clients) for a specific queue.

When the `Server` instance is initialized, as well as instantiating the
`QueueManager` and `proton::container`, it sets the container to listening for
incoming connections, passing an instance of `ListenHandler` to manage these
connections. 

`ListenHandler` is a subclass of `proton::listen_handler`. Proton calls the
method `on_accept()` on a `listen_handler`  when a new connection is made by a
client. The purpose of this method is to populate a
`proton::connection_options` instance, which will define -- among other things
-- a handler for the new connection. This, also, is an instance of
`messaging_handler`.

Proton calls the handler's `on_connection_open()` method, passing a
`proton::connection` object. The application calls `open()` on this object to
complete the opening of the connection.

When the client asks to subscribe to a queue, Proton calls the connection
handler's `on_sender_open()` method. This terminology is, perhaps, a little
confusing -- it is a _sender_ that is being opened, because it is a channel for
sending messages to the client. From the client's perspective, it is opening a
receiver, or subscriber; the server's end of the link is therefore a sender. I
have used the name `Sender` for the class that manages this client
subscriptions.

The name of the queue specified by the client is obtained using a call to
`proton::sender().source().address()`. In principle, the client need not
specify an address, and the server can assign one dynamically.  However, it
makes no sense to do this in this application and, if the client does not
specify a queue name, strange things will happen.

The `ConnectionManager` maintains a list of all senders. This list is updated
by the `ConnectionManager` when a new sender is allocated, but the `Sender`
instance itself removes its own entry from the list when it is closed by the
client. Therefore, the ConnectionManager must pass its list of senders to each
`Sender` instance it creates. The `ConnectionManager` also has the
`QueueManager` create a new `Queue` object, which will hold the associations
between the queue and the senders that are subscribed to it. The complexity of
the relationship between `Queue`/`Sender`/`ConnectionHandler` comes about
because multiple client connections from different clients can subscribe to the
same queue. 

As part of the operation of `on_sender_open()`, the newly-created `Sender`
object is set to be the Proton handler for the new sender, in the call to
`proton::sender::open()`. Subsequent sender-level events will be handled by the
`Sender`, not by the `ConnectionManager`.

The client may close a specific sender, in which case proton calls the
`Sender`'s `on_sender_close()` method. Alternatively, the client may close its
session, or its connection. Both these events are handled by the
`ConnectionManager`, which must take care of removing all the associated
`Sender` objects it created. If it is the connection that is being closed, the
`ConnectionManager` must delete itself.  

The `Server`'s `publish()` method just delegates to a method of the same name
on the `QueueManager`. This is because the `publish()` method, for convenience,
takes the name of a queue, not a `Queue` object. The `QueueManager` knows how
to get the relevant `Queue` object, given the queue name. 

The `publish()` method assigns a message ID to each new message.  I use the
count of published messages for this ID. The count is a `std::atomic<int>`, so
that multiple threads won't get the same ID for their messages. In practice,
there's no way in this application, as it is currently implemented, for the
`publish()` method to be entered on multiple threads. Still, it's good practice
to be careful here.

Having created the message and identified the relevant `Queue` object, the
`QueueManager` calls `queueMsg()` on the `Queue` to send the message to the
client.

In this application, the message is not actually queued -- it's either sent to
the subscribers, if there are any, or lost. In a more sophisticated application
we might actually have to queue messages, that is, to accumulate them so they
can be sent to subscribers later.

The `Queue` instance maintains a list of its subscribers, that is, a list of
`Sender` objects that are assigned to that queue. Note that these `Sender`
objects will be associated with different connections in practice, so care has
to be taken to keep the `ConnectionHandler`'s sender list and each queue's
subscriber list in sync.

The actual message send operation is simply a call to `proton::sender::send()`.

It might be useful to touch on the notion of a "work queue" in Proton.  Broadly
speaking, Proton does not offer thread-safety between connections. That is,
objects associated with one connection cannot safely call methods on objects
associated with other connections. Proton has the notion of a "work queue" to
simplify this situation. A work queue is a queue of function calls, that will
be serialized by the Proton container, in such a way that the same function
call will not be entered on concurrent threads.  

All the classes in this application that might be entered concurrently
(`Queue`, `QueueManager`, `Sender`) have a private work queue. When the methods
in these objects are called from other classes, the calls are made not
directly, but by putting the call into the object's work queue.

The Proton function `make_work` generates a work instance from a method call
template, that can be added to a work queue. So, for example, When `Sender`
calls `Queue::unsubscribe()`, it does it like this:

    queue->add_work (make_work (&Queue::unsubscribe, queue, this)) 

The `add_work()` method in `Queue` simply adds the result of `make_work()` to
its work queue. The syntax is not particularly easy to read. In this case, the
direct method call would be

    queue->unsubscribe (this)

where `this` refers to the current `Sender` object. However, the `make_work`
call takes three arguments, not one: the first is the method to be called. The
second is the implicit `this` pointer that always has to be passed to C++
methods. All subsequent arguments are the arguments to the function itself.

It seems to me that proper use of work queues is one of the fiddliest aspects
of Proton programming. It isn't entirely clear when they need to be used, and
they can't _always_ be used because they are inappropriate for methods that
return results. They are also inappropriate for methods whose object arguments
might change between the time the work is added, and the actual method is
called. So far as I can tell, the only safe approach is to use work queues for
every method call that can actually be queued. The fact that a Proton-based
application appears to be single-threaded from the programmer's point of view
does not mean that it is really single-threaded, when there are concurrent
connections to or from the application. 

## Limitations

`This is not a complete application`. Leaving that aside, there are a number of
limitations.

There is no accounting for link credit. The server assumes that it always has
sufficient AMQP credit to send messages to the clients.  Since the message flow
rate is expected to be low in an application of this kind, this seems a
reasonable assumption, particularly considering the amount by which is
simplifies the implementation.
 
There is no authentication or security of any kind: the application should not
be extended to publish sensitive information without authentication and
encryption. Note, however, that the server does not _receive_ anything from
clients, and cannot take any action as a result of what clients do. Message
flow is strictly one-way.

The server runs in the foreground, and logs (if it logs at all) to standard
out. It would be easy enough to make it run as a daemon, and log to the system
logger, if required. 

`ampq-monitor` uses Proton work queues to schedule almost all operations
asynchronously. I think it is reasonably thread-safe. However, I can't be
_certain_ that there isn't some potential race condition between different
interactions with the server.

## Author and legal

`amqp-monitor` is maintained by Kevin Boone, as an educational example, and
distributed under the terms of the GNU Public Licence, v3.0.  There is no
warranty of any kind.

