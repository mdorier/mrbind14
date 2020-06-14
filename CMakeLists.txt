cmake_minimum_required (VERSION 3.1)
project (mrbind14 CXX)

set (CMAKE_CXX_STANDARD 14)

option(ENABLE_TESTS "Build tests. May require CppUnit_ROOT" OFF)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package (CppUnit)
if (CPPUNIT_FOUND)
    message(STATUS "CppUnit found, unit tests will be compiled")
    include_directories(${CPPUNIT_INCLUDE_DIR})
    enable_testing()
    if(${ENABLE_TESTS})
        add_subdirectory (test)
    endif(${ENABLE_TESTS})
else (CPPUNIT_FOUND)
    message(STATUS "CppUnit not found, unit tests will not be compiled")
endif (CPPUNIT_FOUND)

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/mrbind14
         DESTINATION include
         FILES_MATCHING PATTERN "*.hpp")
