cmake_minimum_required(VERSION 3.15)
set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
project(fyro C CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_library(edit_and_continue INTERFACE)
if(MSVC)
    # ZI = add debug information with edit and continue support to the target
    # Zi = same with edit and continue disabled
    target_compile_options(edit_and_continue INTERFACE "/ZI")
    target_link_options(edit_and_continue INTERFACE "/INCREMENTAL")
endif()

include(cmake/StandardProjectSettings.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

add_library(project_options INTERFACE)
add_library(project_warnings INTERFACE)

target_link_libraries(project_options INTERFACE edit_and_continue)

target_compile_features(project_options INTERFACE cxx_std_17)
set_project_warnings(project_warnings)
enable_sanitizers(project_options)

add_subdirectory(external)
add_subdirectory(data/sprites)
add_subdirectory(src)
add_subdirectory(examples)

add_library(sprites INTERFACE)
target_include_directories(sprites INTERFACE data)
