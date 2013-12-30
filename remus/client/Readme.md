##Remus: A Guide to the Client Code##


The client API resolves

How do we formulate the names??

where each JobRequirements holds the worker id that the reqs apply to
std::vector< JobRequirements > DetermineRequirements( JobType )

We have JobSubmissionInfo
  - SubmissionInfo holds
    -JobData ( file based, or blob) ( handle multiple files )
    -JobRequrements
    -Filled out Requirements structure?

How does the JobRequirements and so forth deal with templating?
we have so requirements that are going to require multiple files be sent
to the worker?

Are we just using key value pairs?

Job SubmitJob(JobSubmissionInfo)
