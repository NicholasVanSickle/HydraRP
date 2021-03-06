cmake_minimum_required(VERSION 2.8.11)

project(Hydra2)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Boost 1.46.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(QTCOMPONENTS Widgets Xml Test)
foreach(QTCOMPONENT ${QTCOMPONENTS})
    find_package(Qt5${QTCOMPONENT} REQUIRED)
endforeach(QTCOMPONENT)

cmake_policy(SET CMP0043 OLD)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
endforeach( OUTPUTCONFIG )

add_subdirectory(lib)
add_subdirectory(hydra2)
add_subdirectory(test)
