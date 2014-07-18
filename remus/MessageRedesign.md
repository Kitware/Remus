

1. Server should send all messages and response in NO_BLOCK mode.
   and attempt at failing to send the message should.

   We can also think about using zmq3. in that case the server
   would set ZMQ_SNDTIMEO to something like 100ms


2. Response has to have an invalid state, it needs to make sure it did actually
   fetch items each time.

3. Client has to query the response and see if it is valid.

4. Client needs a query to see if the server is alive. Maybe we can overload
  heartbeat

5. Can we merge Message.h and Response.h?