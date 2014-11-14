##Remus: A Guide to the Worker Code##

### Thread Safety ###

Remus workers are not thread safe. Applications can not use a worker from multiple threads unless they use their own full memory
barrier locking mechanisms.

A Remus worker creates and starts threads on construction, so take that into consideration when designing your system.

### Polling ###
See /Remus/remus/server/Readme.md for information related to polling.
