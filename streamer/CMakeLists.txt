cmake_minimum_required(VERSION 3.28)
project(frame_streamer)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
        libsrtp
        GIT_REPOSITORY https://github.com/cisco/libsrtp.git
        GIT_TAG        v2.6.0 # Specify the version of libsrtp you want to use
)

FetchContent_Declare(
        libdatachannel
        GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
        GIT_TAG v0.21.2 # Specify the version of libdatachannel you want to use
)

FetchContent_MakeAvailable(libsrtp)
FetchContent_MakeAvailable(libdatachannel)

add_executable(frame_streamer main.cpp)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json srtp2 datachannel)
