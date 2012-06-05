project(Remus_Client)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ZeroMQ_INCLUDE_DIR})


set(headers
    Client.h
    ServerConnection.h
    )

remus_public_headers(${headers})
