# Make sure the compiler can find include files from our library.
include_directories (${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include)
# Define the libs used to compile the examples
# set(LIBS_FOR_EXAMPLE visugl)

# Define our examples to compile in variable ${usage_examples}
file(
        GLOB 
        usage_examples
        *.cpp
)

# loop over the list to compile them
# Prefer to use 'targer_link' over  set_target_properties'
foreach(f ${usage_examples})
    get_filename_component(exampleName ${f} NAME_WE) 
    add_executable (${exampleName} ${f})
    target_link_libraries (${exampleName} ${EXAMPLE_LIBS})
    set_target_properties(${exampleName} PROPERTIES LINKER_LANGUAGE CXX)
    #set_target_properties(${exampleName} PROPERTIES COMPILE_FLAGS "${PROJECT_ALL_CFLAGS} ${SHARE_PATH_CFLAGS}" LINK_FLAGS "${PROJECT_ALL_LDFLAGS}")
    set_target_properties(${exampleName} PROPERTIES COMPILE_FLAGS "${PROJECT_ALL_CFLAGS} ${SHARE_PATH_CFLAGS}")
endforeach(f)
  