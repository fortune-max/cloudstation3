cmake_minimum_required(VERSION 3.28)
project(frame_streamer)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
include(FindPkgConfig)

FetchContent_Declare(
        libsrtp
        GIT_REPOSITORY https://github.com/cisco/libsrtp.git
        GIT_TAG        v2.5.0 # Specify the version of libsrtp you want to use
)

FetchContent_Declare(
        libdatachannel
        GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
        GIT_TAG v0.21.2 # Specify the version of libdatachannel you want to use
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC" )
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC" )

FetchContent_MakeAvailable(libsrtp)
FetchContent_MakeAvailable(libdatachannel)

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

pkg_check_modules(LIBEVDEV REQUIRED libevdev)

#find_path(LIBEVDEV_INCLUDE_DIR libevdev/libevdev.h)
#message(STATUS "LIBEVDEV_INCLUDE_DIR: ${LIBEVDEV_INCLUDE_DIR} ${LIBEVDEV_INCLUDE_DIRS}")
#find_library(LIBEVDEV_LIBRARY evdev)

# Include FFmpeg headers
include_directories(${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR} ${LIBEVDEV_INCLUDE_DIRS})

add_executable(frame_streamer main.cpp
        joystick.cpp)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json srtp2 datachannel ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY} ${LIBEVDEV_LIBRARIES})
