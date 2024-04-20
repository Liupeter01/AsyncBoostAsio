file(GLOB_RECURSE ASYNCCLIENT_SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.h include/*.hpp)
add_executable(async_client ${ASYNCCLIENT_SOURCES})

set(Boost_DIR "/usr/local/lib/cmake/Boost-1.85.0")
find_package(Boost COMPONENTS system thread REQUIRED)
target_include_directories(async_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(async_client PRIVATE Boost::system Boost::system)

find_package(jsoncpp)
target_link_libraries(async_client PRIVATE jsoncpp_object)