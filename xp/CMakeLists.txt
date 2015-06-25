# Make sure the compiler can find include files from our library.
include_directories (${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIR})

# Define the libs used to compile the examples
# set(LIBS_FOR_EXAMPLE visugl)

# Define our examples to compile in variable ${xp}
file(
        GLOB 
        xp
        xp*.cpp
)

# loop over the list to compile them
# Prefer to use 'targer_link' over  set_target_properties'
foreach(f ${xp})
    get_filename_component(xpName ${f} NAME_WE) 
    add_executable (${xpName} ${f})
    target_link_libraries (${xpName} ${XP_LIBS})
    set_target_properties(${xpName} PROPERTIES LINKER_LANGUAGE CXX)
    #set_target_properties(${exampleName} PROPERTIES COMPILE_FLAGS "${PROJECT_ALL_CFLAGS} ${SHARE_PATH_CFLAGS}" LINK_FLAGS "${PROJECT_ALL_LDFLAGS}")
    set_target_properties(${xpName} PROPERTIES COMPILE_FLAGS "${PROJECT_ALL_CFLAGS} ${SHARE_PATH_CFLAGS}")
endforeach(f)
