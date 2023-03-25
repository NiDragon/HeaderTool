cmake_minimum_required(VERSION 3.8)

project(HeaderTool)

# Define Source Directory
set(SRCROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Gather Sources
file(GLOB PRIVATE_INCLUDES CONFIGURE_DEPENDS 
    ${SRCROOT}/Private/*.h
    ${SRCROOT}/Private/*.hpp
)

file(GLOB PUBLIC_INCLUDES CONFIGURE_DEPENDS 
    ${SRCROOT}/Public/*.h
    ${SRCROOT}/Public/*.hpp
)

file(GLOB PRIVATE_SRCS CONFIGURE_DEPENDS
    ${SRCROOT}/Private/*.c
    ${SRCROOT}/Private/*.cpp
)

file(GLOB PUBLIC_SRCS CONFIGURE_DEPENDS
    ${SRCROOT}/Public/*.c
    ${SRCROOT}/Public/*.cpp
)

# Create Executable
add_executable(HeaderTool ${PRIVATE_INCLUDES} ${PRIVATE_SRCS} ${PUBLIC_INCLUDES} ${PUBLIC_SRCS})

# This tool needs C++17 for filesystem 
set_property(TARGET HeaderTool PROPERTY CXX_STANDARD 17)

if(WIN32)
target_sources(HeaderTool PRIVATE ${SRCROOT}/Private/windows/HeaderTool.rc)
source_group(Resources FILES ${SRCROOT}/Private/windows/HeaderTool.rc)
endif()

# Group Sources
source_group(Private FILES ${PRIVATE_INCLUDES} ${PRIVATE_SRCS})
source_group(Public FILES ${PUBLIC_INCLUDES} ${PUBLIC_SRCS})

# Get Includes
target_include_directories(HeaderTool PRIVATE ${SRCROOT}/Private)
target_include_directories(HeaderTool PUBLIC ${SRCROOT}/Public)

# Dependencies
find_package(Clang REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

target_link_libraries(HeaderTool PRIVATE clangTooling)
