
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
pkg_search_module(GAML REQUIRED gaml)

#######################################
# Setting the compilation flags
#######################################

# cflags added by the package
SET(PROJECT_CFLAGS "-Wall -std=c++11")

# cflags added by the pkg-config dependencies contains ';' as separator. This is a fix.
string(REPLACE ";" " " FTGL_CFLAGS "${FTGL_CFLAGS}")
string(REPLACE ";" " " GLFW_CFLAGS "${GLFW_CFLAGS}")
string(REPLACE ";" " " GAML_CFLAGS "${GAML_CFLAGS}")

# ldflasgs added by the pkg-config dependencies contains ';' as separator. This is a fix.
string(REPLACE ";" " " GSL_LDFLAGS "${GSL_LDFLAGS}")
string(REPLACE ";" " " GLFW_LDFLAGS "${GLFW_LDFLAGS}")

# Gathering of all flags
# (e.g. for compiling examples)
SET(PROJECT_ALL_CFLAGS  "${PROJECT_CFLAGS}  ${RAPIDJSON_CFLAGS} ${GAML_CFLAGS} ${FTGL_CFLAGS} ${GLFW_CFLAGS} ${GL_CFLAGS}")
SET(PROJECT_ALL_LDFLAGS "${PROJECT_LIBS} ${PROJECT_LDFLAGS} -L${CMAKE_BINARY_DIR}/src ${GL_LDFLAGS} ${GSL_LDFLAGS} ${FTGL_LDFLAGS} ${GLFW_LDFLAGS}")
## Set of libraries for examples
SET(EXAMPLE_LIBS "${GL_LDFLAGS} ${GSL_LDFLAGS} ${FTGL_LDFLAGS} ${GLFW_LDFLAGS} ${GAML_LDFLAGS}")
###################################
#  Subdirectories
###################################
add_subdirectory(src)
add_subdirectory(test)


