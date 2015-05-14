
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.4 )

project (VisuGL)

#######################################
# Get the libs/inc for the dependencies
#######################################
find_package(PkgConfig REQUIRED)

pkg_search_module(GL REQUIRED gl)
pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(FTGL REQUIRED ftgl)
pkg_search_module(GSL REQUIRED gsl)

#######################################
# Setting the compilation flags
#######################################

# cflags added by the package
SET(PROJECT_CFLAGS "-Wall -std=c++11")

# cflags added by the pkg-config dependencies contains ';' as separator. This is a fix.
string(REPLACE ";" " " FTGL_CFLAGS "${FTGL_CFLAGS}")

# ldflasgs added by the pkg-config dependencies contains ';' as separator. This is a fix.
string(REPLACE ";" " " GSL_LDFLAGS "${GSL_LDFLAGS}")

# Gathering of all flags
# (e.g. for compiling examples)
SET(PROJECT_ALL_CFLAGS  "${PROJECT_CFLAGS} ${FTGL_CFLAGS} ${GLFW_CFLAGS} ${GL_CFLAGS}")
SET(PROJECT_ALL_LDFLAGS "${PROJECT_LIBS} ${PROJECT_LDFLAGS} -L${CMAKE_BINARY_DIR}/src ${GSL_LDFLAGS} ${FTGL_LDFLAGS} ${GLFW_LDFLAGS} ${GL_LDFLAGS}")

###################################
#  Subdirectories
###################################
add_subdirectory(src)
add_subdirectory(test)


