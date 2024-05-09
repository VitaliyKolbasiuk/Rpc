# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/ClientExample_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/ClientExample_autogen.dir/ParseCache.txt"
  "CMakeFiles/ServerExample_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/ServerExample_autogen.dir/ParseCache.txt"
  "CMakeFiles/SimpleClient_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/SimpleClient_autogen.dir/ParseCache.txt"
  "CMakeFiles/SimpleServer_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/SimpleServer_autogen.dir/ParseCache.txt"
  "ClientExample_autogen"
  "ServerExample_autogen"
  "SimpleClient_autogen"
  "SimpleServer_autogen"
  )
endif()
