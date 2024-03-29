cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/src/oatpp/core/base/Environment.hpp" OATPP_VERSION_MACRO REGEX "#define OATPP_VERSION \"[0-9]+.[0-9]+.[0-9]+\"$")
string(REGEX REPLACE "#define OATPP_VERSION \"([0-9]+.[0-9]+.[0-9]+)\"$" "\\1" oatpp_VERSION "${OATPP_VERSION_MACRO}")

###################################################################################################
## These variables are passed to oatpp-module-install.cmake script
## use these variables to configure module installation

set(OATPP_THIS_MODULE_NAME oatpp) ## name of the module (also name of folders in installation dirs)
set(OATPP_THIS_MODULE_VERSION ${oatpp_VERSION}) ## version of the module (also sufix of folders in installation dirs)
set(OATPP_THIS_MODULE_LIBRARIES oatpp oatpp-test) ## list of libraries to find when find_package is called
set(OATPP_THIS_MODULE_TARGETS oatpp oatpp-test) ## list of targets to install
set(OATPP_THIS_MODULE_DIRECTORIES oatpp oatpp-test) ## list of directories to install

###################################################################################################

project(oatpp VERSION ${OATPP_THIS_MODULE_VERSION} LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(OATPP_INSTALL "Create installation target for oat++" ON)
option(OATPP_BUILD_TESTS "Create test target for oat++" ON)

###################################################################################################
## COMPILATION CONFIG #############################################################################
###################################################################################################

option(OATPP_DISABLE_ENV_OBJECT_COUNTERS "Disable object counting for Release builds for better performance" OFF)
option(OATPP_DISABLE_POOL_ALLOCATIONS "This will make oatpp::base::memory::MemoryPool, method obtain and free call new and delete directly" OFF)

set(OATPP_THREAD_HARDWARE_CONCURRENCY "AUTO" CACHE STRING "Predefined value for function oatpp::concurrency::Thread::getHardwareConcurrency()")
set(OATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT "10" CACHE STRING "Number of shards of ThreadDistributedMemoryPool")
set(OATPP_ASYNC_EXECUTOR_THREAD_NUM_DEFAULT "2" CACHE STRING "oatpp::async::Executor default number of threads")

option(OATPP_COMPAT_BUILD_NO_THREAD_LOCAL "Disable 'thread_local' feature" OFF)

## Print config ##################################################################################

message("\n############################################################################")
message("## oatpp module compilation config:\n")

message("OATPP_DISABLE_ENV_OBJECT_COUNTERS=${OATPP_DISABLE_ENV_OBJECT_COUNTERS}")
message("OATPP_DISABLE_POOL_ALLOCATIONS=${OATPP_DISABLE_POOL_ALLOCATIONS}")
message("OATPP_THREAD_HARDWARE_CONCURRENCY=${OATPP_THREAD_HARDWARE_CONCURRENCY}")
message("OATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT=${OATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT}")
message("OATPP_ASYNC_EXECUTOR_THREAD_NUM_DEFAULT=${OATPP_ASYNC_EXECUTOR_THREAD_NUM_DEFAULT}")

message("OATPP_COMPAT_BUILD_NO_THREAD_LOCAL=${OATPP_COMPAT_BUILD_NO_THREAD_LOCAL}")

## Set definitions ###############################################################################

if(OATPP_DISABLE_ENV_OBJECT_COUNTERS)
    add_definitions(-DOATPP_DISABLE_ENV_OBJECT_COUNTERS)
endif()

if(OATPP_DISABLE_POOL_ALLOCATIONS)
    add_definitions (-DOATPP_DISABLE_POOL_ALLOCATIONS)
endif()

set(AUTO_VALUE AUTO)
if(NOT OATPP_THREAD_HARDWARE_CONCURRENCY STREQUAL AUTO_VALUE)
    add_definitions (-DOATPP_THREAD_HARDWARE_CONCURRENCY=${OATPP_THREAD_HARDWARE_CONCURRENCY})
endif()

add_definitions (
        -DOATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT=${OATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT}
        -DOATPP_ASYNC_EXECUTOR_THREAD_NUM_DEFAULT=${OATPP_ASYNC_EXECUTOR_THREAD_NUM_DEFAULT}
)

if(OATPP_COMPAT_BUILD_NO_THREAD_LOCAL)
    add_definitions(-DOATPP_COMPAT_BUILD_NO_THREAD_LOCAL)
endif()

if (MSVC)
    # Add compilerswitch to allow pointer-to-members
    # https://docs.microsoft.com/en-us/cpp/preprocessor/pointers-to-members?view=vs-2019
    add_compile_options(/vmg)
endif(MSVC)

message("\n############################################################################\n")

###################################################################################################

message("oatpp version: '${OATPP_THIS_MODULE_VERSION}'")

add_subdirectory(src)

if(OATPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
