project(Remus_Client)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ZeroMQ_INCLUDE_DIR})

set(headers
    Client.h
    Job.h
    JobRequest.h
    JobResult.h
    JobStatus.h
    ServerConnection.h
    )

remus_public_headers(${headers})
