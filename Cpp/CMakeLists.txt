cmake_minimum_required(VERSION 3.12)
project(Rpc)

#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_PREFIX_PATH "/home/vitaliykolbasiuk/Qt/6.6.0/gcc_64/lib/cmake")

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# Include Protocol Buffers
find_package(Protobuf REQUIRED)

# Add the directory containing your .proto files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

 add_executable(SimpleServer
         Server/SimpleServer.cpp
 )

 add_executable(SimpleClient
         Client/SimpleClient.cpp
 )

 add_executable(ClientExample
         Client/ClientExample.cpp
         Client/RpcClient.h
         Client/RpcClient.cpp
         Client/ClientInterfaces.h
         Client/TcpClient.h
         Client/ClientMain.cpp
         Operations.h
 )

add_executable(ServerExample
        Server/ServerMain.cpp
        Server/ServerExample.cpp
        Server/ModernRpcTcpServer.h
        Server/TcpServer.cpp
        Server/ServerSession.h
        Server/Interfaces.h
        Server/RpcModel.h
)

add_executable(ClientServerTest
        clientServerTest.cpp
        Operations.h
        Server/Interfaces.h
        Server/RpcModel.h
        Client/RpcClient.h
        Client/RpcClient.cpp
        Client/ClientInterfaces.h
        Client/TcpClient.h
        Server/ModernRpcTcpServer.h
        Server/TcpServer.cpp
        Server/ServerSession.h
        ModernRpc.h
)

add_executable(exampleModernRpcClient
        ModernRpc.h
        Client/exampleModernClient.cpp
        Client/TcpClient.h
        Client/RpcClient.cpp
        Client/RpcClient.h
        Client/ClientInterfaces.h
)

add_executable(exampleModernRpcServer
        ModernRpc.h
        Server/exampleModernServer.cpp
        Server/TcpServer.cpp
        Server/ModernRpcTcpServer.h
        Server/ServerSession.h
        Server/Interfaces.h
        Server/RpcModel.h
)

target_include_directories(ClientServerTest PUBLIC . cmake-build-debug/ Client/ Server/)

#target_link_libraries(ServerExample protobuf::libprotobuf Qt${QT_VERSION_MAJOR}::Core)
#target_link_libraries(ClientExample protobuf::libprotobuf Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(ClientServerTest protobuf::libprotobuf)

#protobuf_generate(
#        LANGUAGE cpp  # specify the desired language (e.g., cpp, python)
#        TARGET ServerExample  # specify the target name for the generated code
#        PROTOS Protobuf/AddressBook.proto  # specify your .proto file(s)
#        #OUT_VAR Protobuf # specify the variable to store the generated source files
#)

protobuf_generate(
        LANGUAGE cpp  # specify the desired language (e.g., cpp, python)
        TARGET ClientServerTest  # specify the target name for the generated code
        PROTOS Protobuf/AddressBook.proto  # specify your .proto file(s)
        #OUT_VAR Protobuf # specify the variable to store the generated source files
)


include(GNUInstallDirs)
install(TARGETS ServerExample
    # install(TARGETS SimpleServer SimpleClient ClientExample ServerExample
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
