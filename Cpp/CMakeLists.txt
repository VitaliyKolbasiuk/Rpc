cmake_minimum_required(VERSION 3.12)
project(Rpc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the directory containing your .proto files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

 add_executable(ClientExample
         Client/RpcClient.h
         Client/RpcClient.cpp
         Client/TcpClient.h
         EasyRpcFunction.h
 )

add_executable(ServerExample
        Server/EasyRpcTcpServer.h
        Server/EasyRpcTcpServer.cpp
        Server/ServerSession.h
)

add_executable(ClientServerTest
        EasyRpcFunction.h
        Client/RpcClient.h
        Client/RpcClient.cpp
        Client/TcpClient.h
        Server/EasyRpcTcpServer.h
        Server/EasyRpcTcpServer.cpp
        Server/ServerSession.h
        EasyRpcClientBase.h
)

add_executable(exampleEasyRpcClient
        EasyRpcClientBase.h
        Client/exampleEasyClient.cpp
        Client/TcpClient.h
        Client/RpcClient.cpp
        Client/RpcClient.h
)

add_executable(exampleEasyRpcServer
        EasyRpcClientBase.h
        Server/exampleEasyServer.cpp
        Server/EasyRpcTcpServer.cpp
        Server/EasyRpcTcpServer.h
        Server/ServerSession.h
        EasyRpcSessionBase.h
)

add_executable(GenerateCode
        Generator.cpp
)

target_include_directories(exampleEasyRpcServer PUBLIC . cmake-build-debug/ Client/ Server/)

include(GNUInstallDirs)
install(TARGETS ServerExample
    # install(TARGETS SimpleServer SimpleClient ClientExample ServerExample
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
