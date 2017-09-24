# =====================================================
#  Public Domain: Inspired by github.com/KjellKod/g3log/CMakeLists.cmake
#  2015, April 30, @author Kjell.Hedstrom
# =====================================================

CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
ENABLE_LANGUAGE(CXX)
SET(CMAKE_BUILD_TYPE Release)

PROJECT (g3log_example)
SET(PROJECT_SRC src)

MESSAGE(" PROJECT_SRC = : ${PROJECT_SRC}")
INCLUDE_DIRECTORIES(${PROJECT_SRC})
SET(ACTIVE_CPP0xx_DIR "Release")

find_library(G3LOG g3logger PATHS ${G3_LIBRARY_PATH})
find_library(G3LOGROTATE g3logrotate PATHS ${G3SINK_LIBRARY_PATH})

SET(CMAKE_CXX_FLAGS " -Wall -Werror -rdynamic -Wunused -std=c++11 -pthread -D_GLIBCXX_USE_NANOSLEEP")


# GENERIC STEPS
FILE(GLOB SRC_FILES ${PROJECT_SRC}/*.h ${PROJECT_SRC}/*.h ${PROJECT_SRC}/*.ipp ${PROJECT_SRC}/*.cpp) 


add_executable(g3log_example ${PROJECT_SRC}/main.cpp)
SET_TARGET_PROPERTIES(g3log_example PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(g3log_example  ${G3LOG} ${G3LOGROTATE})
