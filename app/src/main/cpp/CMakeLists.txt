cmake_minimum_required(VERSION 3.31.3)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("rdvr" CXX)

list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/")

add_library(${PROJECT_NAME} SHARED SessionActivity.cpp tools.cpp)
target_compile_options(${PROJECT_NAME}
    PRIVATE -Wall
    PRIVATE -Wextra
    PRIVATE -Werror
    PRIVATE -Wno-deprecated-declarations
)
find_library(log-lib log)
find_package(FreeRDP-Client REQUIRED)
find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE ${log-lib}
    PRIVATE freerdp-client
    PRIVATE OpenSSL::SSL
)
target_include_directories(${PROJECT_NAME}
    PRIVATE ${FreeRDP-Client_INCLUDE_DIR}
)
