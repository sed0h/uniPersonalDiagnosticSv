# Copyright (C) 2015-2017 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required (VERSION 2.8.7)
project (udppipe)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -O2")
# -std=c++11 

# add_definitions(-DUDS_ON_DOIP)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

file(GLOB file_SRC
    "*.c"
)

add_library(${PROJECT_NAME} SHARED ${file_SRC})

target_link_libraries(${PROJECT_NAME} 
    # ${PROJECT_SOURCE_DIR}/lib/libdoip_core.so
    -lpthread
)
