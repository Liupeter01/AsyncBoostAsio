file(GLOB_RECURSE ASYNCSERVER_SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.h include/*.hpp)
add_executable(async_server ${ASYNCSERVER_SOURCES})

set(Boost_DIR "/usr/local/lib/cmake/Boost-1.85.0")
find_package(Boost COMPONENTS system thread REQUIRED)
target_include_directories(async_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(async_server PRIVATE Boost::system Boost::thread)

find_package(jsoncpp)
target_link_libraries(async_server PRIVATE jsoncpp_object)