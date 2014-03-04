## Todo: ##

1. Add a server test where we verify the throughput of the server
   using very basic client / worker calls, we should verify that we can
   handle a certain number of client worker calls no matter what to
   verify we don't regress performance

   This should really be a collection of tests that try to tax the server
   as much as possible, so that we never lose performance.

2.  JobRequest and JobContent need to be extended to fully support the file
    source type.

    This means that when the source type is a file, we will treat the input data
    as a file path, that will be evaluated based on the current working directory.

    When we determine that the server is not local we have to read the file
    and send it along the wire.
    Do we need to have a threaded client for this to be responsive?

    A jobContent can hold the file, while a second job content can hold
    the arguments for the command line.

3. We need to add real logging to  the server, so that it is easier to enable a
   verbose server that will help us figure out concurrency and queueing issues.
   This should be done before threading the server so that it is easier to debug
   issues when moving to a threaded server
