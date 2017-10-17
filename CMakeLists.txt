cmake_minimum_required(VERSION 3.5)
project(node)

set(CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED )
find_package( Boost REQUIRED COMPONENTS system)
include_directories( ${OpenCV_INCLUDE_DIRS})
include_directories( ${Boost_INCLUDE_DIR})


set(SOURCE_FILES src/main.cpp src/MyTypes.h src/NodeClient.cpp src/NodeClient.h)
add_executable(node ${SOURCE_FILES})

target_link_libraries( node ${OpenCV_LIBS} ${Boost_LIBRARIES} -lpthread )