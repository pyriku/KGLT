CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(kglt)

#SET (CMAKE_C_COMPILER             "/usr/bin/clang")
#SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(KAZMATH REQUIRED)

ADD_DEFINITIONS("-Wall -std=c++0x -g")
ADD_DEFINITIONS("-DBOOST_NO_RVALUE_REFERENCES") #https://svn.boost.org/trac/boost/ticket/4521

PKG_CHECK_MODULES(SDL sdl)
PKG_CHECK_MODULES(GL gl)
PKG_CHECK_MODULES(SIGC sigc++-2.0)

FIND_PACKAGE(Boost COMPONENTS system filesystem thread date_time REQUIRED)
FIND_PACKAGE(Freetype REQUIRED)

MACRO (ADD_PCH_RULE  _header_filename _src_list)
        SET(_gch_filename "${_header_filename}.gch")
        LIST(APPEND ${_src_list} ${_gch_filename})
        SET (_args ${CMAKE_CXX_FLAGS})
        LIST(APPEND _args -c ${_header_filename} -o ${_gch_filename})
        GET_DIRECTORY_PROPERTY(DIRINC INCLUDE_DIRECTORIES)
        foreach (_inc ${DIRINC})
                LIST(APPEND _args "-I" ${_inc})
        endforeach(_inc ${DIRINC})
        SEPARATE_ARGUMENTS(_args)
        add_custom_command(OUTPUT ${_gch_filename}
                   COMMAND rm -f ${_gch_filename}
                   COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_args}
                            DEPENDS ${_header_filename})
ENDMACRO(ADD_PCH_RULE _header_filename _src_list)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${KAZMATH_INCLUDE_DIRS}
    ${SDL_INCLUDE_DIRS}
    ${GL_INCLUDE_DIRS}
    ${GLU_INCLUDE_DIRS}
    ${SIGC_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

LINK_LIBRARIES(
    ${SDL_LIBRARIES}
    ${GL_LIBRARIES}
    ${GLU_LIBRARIES}
    ${SIGC_LIBRARIES}
    ${KAZMATH_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${FREETYPE_LIBRARY}
)

ADD_SUBDIRECTORY(kglt)
ADD_SUBDIRECTORY(samples)

ENABLE_TESTING()

PKG_CHECK_MODULES(UNITTEST unittest++)
INCLUDE_DIRECTORIES(
    ${UNITTEST_INCLUDE_DIRS}
)

LINK_LIBRARIES(
    ${UNITTEST_LIBRARIES}
    kglt
)

IF(UNITTEST_FOUND)
    ADD_SUBDIRECTORY(tests)
ENDIF(UNITTEST_FOUND)



