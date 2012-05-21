project(MeshServer_Client)
include_directories(${ZeroMQ_INCLUDE_DIR})


set(headers
    Client.h
    ServerConnection.h
    )

meshserver_public_headers(${headers})
