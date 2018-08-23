message( "External project - Glog" )
set (Glog_VERSION 0.3.5)
option( USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)

set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

ExternalProject_Add(Glog
    GIT_REPOSITORY ${git_protocol}://github.com/google/glog.git
    GIT_TAG v${Glog_VERSION}
    CMAKE_GENERATOR ${CMAKE_GENERATOR} 
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DGflags_NAMESPACE=google
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    )

set( GLOG_ROOT_DIR ${INSTALL_DEPENDENCIES_DIR} CACHE PATH "Glog Root dir")
