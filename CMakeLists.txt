cmake_minimum_required(VERSION 3.0.0)
project(inference VERSION 0.0.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(MSGPACK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/msgpack-c-master/include")
include_directories(${MSGPACK_INCLUDE_DIR})
find_package(Boost COMPONENTS system REQUIRED)

# definition of requests that are passed b/w threads
add_library(request_defs request_defs.cc)

add_library(serverless_pipeline serverless_pipeline.cc)
target_link_libraries(serverless_pipeline request_defs pthread )

# Connection Class
add_library(tcp_connection tcp_connection.cc)
target_link_libraries(tcp_connection
    pthread
    ${Boost_SYSTEM_LIBRARY}
)
# The server
add_executable(tcp_server tcp_server.cc)
target_link_libraries(tcp_server
    tcp_connection
    inference_pipeline
    pthread
    ${Boost_SYSTEM_LIBRARY}
)

# Client
add_library(rpc_client rpc_client.cc)
target_link_libraries(rpc_client
    ${Boost_SYSTEM_LIBRARY}
)
add_executable(work_load_generator work_load_generator.cc)
target_link_libraries(work_load_generator
    rpc_client
    pthread
)
