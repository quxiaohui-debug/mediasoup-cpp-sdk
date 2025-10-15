cmake_minimum_required(VERSION 3.14)

project (RTC_SDK LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    # 设置编译类型
    set(CMAKE_BUILD_TYPE "Debug")
endif()
# 添加编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # 设置编译器优化级别
    add_compile_options(-O2)
endif()
message(STATUS "CMAKE_BUILD_TYPE:${CMAKE_BUILD_TYPE}")

#开启C++14，不要开启C++17
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
		message(FATAL_ERROR "GCC version must be at least 4.9!")
	endif()
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)

set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/out/${SYSTEM_NAME_LOWER}/${BUILD_TYPE_LOWER})

#设置库文件的输出路径，包括静态库和动态库
set(LIBRARY_OUTPUT_PATH    ${OUTPUT_PATH})

#设置可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_PATH})

#设置第三方库的搜索路径
set(THIRD_PARTY_PATH "${CMAKE_SOURCE_DIR}/third_party")

find_package(PkgConfig)

# 添加 git 版本信息
set(COMMIT_HASH "Git_Unkown_commit")
set(COMMIT_TIME "Git_Unkown_time")
set(BRANCH_NAME "Git_Unkown_branch")
set(BUILD_TIME "")

string(TIMESTAMP BUILD_TIME "%Y-%m-%dT%H:%M:%S")

find_package(Git QUIET)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD
    OUTPUT_VARIABLE COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  execute_process(
    COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD
    OUTPUT_VARIABLE BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  execute_process(
    COMMAND ${GIT_EXECUTABLE} log --format=format:%aI -1
    OUTPUT_VARIABLE COMMIT_TIME
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/version.h.ini
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
  @ONLY)

message(STATUS "Git version is ${BRANCH_NAME} ${COMMIT_HASH}/${COMMIT_TIME} ${BUILD_TIME}")

#添加宏定义
add_definitions(-DWEBRTC_LINUX -DWEBRTC_POSIX -DWEBRTC_APM_DEBUG_DUMP=0)

#添加编译选项
add_compile_options(-Wno-unused-function -Wno-unused-local-typedefs -Wno-unused-variable)

add_definitions(
    -DGL_SILENCE_DEPRECATION
    -DUNICODE
    -D_UNICODE
    -D_ENABLE_EXTENDED_ALIGNED_STORAGE
    -DBUILD_STATIC
    -DUSE_AURA=1
    -DNO_TCMALLOC
    -DFULL_SAFE_BROWSING
    -DSAFE_BROWSING_CSD
    -DSAFE_BROWSING_DB_LOCAL
    -DCHROMIUM_BUILD
    -D_HAS_EXCEPTIONS=0
    -D__STD_C
    -D_CRT_RAND_S
    -D_CRT_SECURE_NO_DEPRECATE
    -D_SCL_SECURE_NO_DEPRECATE
    -D_ATL_NO_OPENGL
    -DCERT_CHAIN_PARA_HAS_EXTRA_FIELDS
    -DPSAPI_VERSION=2
    -D_SECURE_ATL
    -D_USING_V110_SDK71_
    -DDYNAMIC_ANNOTATIONS_ENABLED=1
    -DWTF_USE_DYNAMIC_ANNOTATIONS=1
    -DWEBRTC_ENABLE_PROTOBUF=1
    -DWEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE
    -DHAVE_SCTP
    -DWEBRTC_USE_H264
    -DWEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=0
    -DABSL_ALLOCATOR_NOTHROW=1
    -DHAVE_WEBRTC_VIDEO
    -DHAVE_WEBRTC_VOICE
    -DASIO_STANDALONE
    -DMSC_LOG_TRACE
)

#设置WEBRTC头文件的搜索路径
if(NOT LIBWEBRTC_INCLUDE_PATH)
	set(LIBWEBRTC_INCLUDE_PATH "/home/quxiaohui/workspace/webrtc/src" CACHE STRING "libwebrtc include path")
endif()

#设置WEBRTC库文件的搜索路径
if(NOT LIBWEBRTC_BINARY_PATH)
	set(LIBWEBRTC_BINARY_PATH "/home/quxiaohui/workspace/webrtc/src/out/m94/obj" CACHE STRING "libwebrtc binary path")
endif()

if(NOT LIBWEBRTC_INCLUDE_PATH)
	message(FATAL_ERROR "LIBWEBRTC_INCLUDE_PATH not provided")
else()
  message(STATUS "LIBWEBRTC_INCLUDE_PATH:${LIBWEBRTC_INCLUDE_PATH}")
endif()

if(NOT LIBWEBRTC_BINARY_PATH)
	message(FATAL_ERROR "LIBWEBRTC_BINARY_PATH not provided")
else()
  message(STATUS "LIBWEBRTC_BINARY_PATH:${LIBWEBRTC_BINARY_PATH}")
endif()

#定义头文件的搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sdk)
include_directories(${THIRD_PARTY_PATH}/libmediasoupclient/include)
include_directories(${THIRD_PARTY_PATH}/libsdptransform/include)
include_directories(${THIRD_PARTY_PATH}/rapidjson/include)
include_directories(${THIRD_PARTY_PATH}/spdlog/include)
include_directories(${THIRD_PARTY_PATH}/asio/asio/include)
include_directories(${THIRD_PARTY_PATH}/websocketpp)
include_directories(${LIBWEBRTC_INCLUDE_PATH})
include_directories(${LIBWEBRTC_INCLUDE_PATH}/third_party)
include_directories(${LIBWEBRTC_INCLUDE_PATH}/third_party/abseil-cpp)
include_directories(${LIBWEBRTC_INCLUDE_PATH}/third_party/boringssl/src/include)
include_directories(${LIBWEBRTC_INCLUDE_PATH}/sdk/objc)
include_directories(${LIBWEBRTC_INCLUDE_PATH}/sdk/objc/base)
include_directories(${LIBWEBRTC_INCLUDE_PATH}/third_party/libyuv/include)

#定义库文件的搜索路径
link_directories(${OUTPUT_PATH})
link_directories(${LIBWEBRTC_BINARY_PATH})

#查找指定目录下所有的源文件
file(GLOB ALL_SOURCE_FILES
  ${THIRD_PARTY_PATH}/libmediasoupclient/src/*.cpp
  ${THIRD_PARTY_PATH}/libmediasoupclient/src/sdp/*.cpp
  ${THIRD_PARTY_PATH}/libsdptransform/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/core/*/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/core/*/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/sdk/*.cpp
  )

message(STATUS "ALL_SOURCE_FILES:${ALL_SOURCE_FILES}")

#生成库文件
add_library(rtc_sdk STATIC ${ALL_SOURCE_FILES})

#生成可执行文件
add_executable(test_pusher test/test_pusher.cpp test/reader.cpp)
add_executable(test_puller test/test_puller.cpp)

target_link_libraries(test_pusher rtc_sdk webrtc pthread dl)
target_link_libraries(test_puller rtc_sdk webrtc pthread dl)