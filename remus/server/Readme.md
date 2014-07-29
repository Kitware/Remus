##Remus: A Guide to the Server Code##

### Understanding the RW file format ###

### Creating a New Worker Factory ###

### Extend the Server ###

### Polling ###

App Nap is a 'feature' that ships with OS X Mavericks and later.  It slows down processes that OS X determines are completely hidden and not doing anything for you.  Unfortunately, OS X's definition of inactive is rather broad -- it's been observed that Remus workers and servers can be napped and not be given enough CPU time to properly heartbeat.  This could cause Remus server to think that a worker has crashed, unreachable, or otherwise unresponsive.

To solve this problem, Remus uses a dynamic polling system.  When a worker heartbeats, it will specify the duration until it's next heartbeat.  It computes this value internally based on its own event loop, but is biased towards sleeping.  We also put caps on this duration in mins and maxes.  The headers for Remus exopse this duration range, which is the primary means of controlling the loop from the API.

However, this only addresses App Napping or sleeping on the worker side of things.  What happens if the server is napped or goes to sleep?  The server keeps track of its event loop too, and is able to tell if an 'abnormal event' occured, where polling time was much longer in the past.  For such cases, the server gives a 'freebie' to the worker until the system has normalized it, and will not classify it as unresponsive