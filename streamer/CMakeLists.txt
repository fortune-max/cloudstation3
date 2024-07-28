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

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

# Include FFmpeg headers
include_directories(${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})

add_executable(frame_streamer main.cpp)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json srtp2 datachannel ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})
