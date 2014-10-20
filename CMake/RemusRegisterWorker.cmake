#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================


# Generate a remus worker file. The generated worker file will both
# be copied to the build directory to beside the worker, but also
# will be installed.
# Usage:
#
# remus_register_worker(target
#   INPUT_TYPE <MeshInputType>
#   OUTPUT_TYPE <MeshOuputType>
#   [ EXECUTABLE_NAME <name of the executable> ]
#   [ INSTALL_PATH <path to install to> ]
#   [ WORKER_FILE_EXT <> ]
#   [ NO_INSTALL ]
#   [ FILE_TYPE  <type of requirements file> FILE_PATH  <path to requirements file> ]
#   )
#
#
# Example:
# Here is how to register a mesh worker that doesn't requirementsfile a file.
# In this example "triangle_executable" is the name of the target that generated the
# executable
# remus_register_worker(triangle_executable
#                       INPUT_TYPE "Edges"
#                       OUTPUT_TYPE "Mesh2D"
#                       )
#
# Here is how to register a mesh worker that has a requirementsfile.
# remus_register_worker(triangle_executable
#                       INPUT_TYPE "Edges"
#                       OUTPUT_TYPE "Mesh2D"
#                       FILE_TYPE "JSON"
#                       FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/triangle_reqs.json"
#                       )
#
#For the file example the remus_register_mesh_worker call will setup install rules
#so that the rw file and the file referenced by the FILE_PATH are installed
#in the bin directory. If you need to overwrite were the files are installed
#set the INSTALL_PATH optional variable.
#
#If an explicit EXECUTABLE_NAME is not given, we will presume the name
#of the executable is identical to the target's OUTPUT_NAME if set,
#otherwise we defer to the name of the target itself
#
#If WORKER_FILE_EXT is set we use the user specified file extension
#instead of using the default "rw" extension. The passed in extension
#should not start with "."
#
#NO_INSTALL allows you to generate build directory remus rw files
#
function(remus_register_mesh_worker workerTarget )
  #enable only the new parser for this function. Policies are scoped to the
  #function so we don't have to worry about this affecting the calling project
  if(POLICY CMP0053)
    cmake_policy(SET CMP0053 NEW)
  endif()

  set(options NO_INSTALL)
  set(oneValueArgs INPUT_TYPE OUTPUT_TYPE EXECUTABLE_NAME INSTALL_PATH WORKER_FILE_EXT FILE_TYPE FILE_PATH)
  set(multiValueArgs )
  cmake_parse_arguments(R "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  if(NOT R_INPUT_TYPE OR NOT R_OUTPUT_TYPE)
    message(FATAL_ERROR "Need to set INPUT_TYPE and OUTPUT_TYPE when registering ${workerTarget}")
  endif()

  #setup the files that this worker needs to be installed. By default it is
  #just the .rw file, but if the worker has file based requirements we need
  #to properly install that xml/json file too

  #determine the output directory of the target
  get_target_property(build_dir ${workerTarget} RUNTIME_OUTPUT_DIRECTORY)
  if(NOT build_dir)
    #if no property level build directory, use the global one
    set(build_dir "${EXECUTABLE_OUTPUT_PATH}")
  endif()

  #determine the name of the executable that the target is generating
  #this could be fairly. We defer this to generation time, when
  #we actually write out the rw file
  set(worker_name ${R_EXECUTABLE_NAME})
  if(NOT R_EXECUTABLE_NAME) #no user specified executable name, look for one
    get_target_property(worker_name ${workerTarget} OUTPUT_NAME)
    if(NOT worker_name)
      get_target_property(worker_name ${workerTarget} NAME)
    endif()
  endif()

  #determine the extension for the rw file
  set(worker_file_ext "${R_WORKER_FILE_EXT}")
  if(NOT R_WORKER_FILE_EXT)
    set(worker_file_ext "rw")
  endif()


  #determine where we want to install the file(s)
  set(destinations_to_install "${R_INSTALL_PATH}")
  if(NOT R_INSTALL_PATH)
    set(destinations_to_install "bin")
  endif()

  set(files_to_install "${build_dir}/${worker_name}.${worker_file_ext}")

  #configure the correct file(s) based on if the worker has file based
  #requirements
  if(R_FILE_TYPE AND R_FILE_PATH)
    get_filename_component(R_FILE_NAME "${R_FILE_PATH}" NAME)

    #need to generate the file into the build directory
    #from the string we have inside this function
    set(rw_file_content
    "
    {
    \"ExecutableName\": \"@worker_name@\",
    \"InputType\": \"@R_INPUT_TYPE@\",
    \"OutputType\": \"@R_OUTPUT_TYPE@\",
    \"File\" : \"@R_FILE_NAME@\",
    \"FileFormat\" : \"@R_FILE_TYPE@\"
    }
    "
    )
    string(CONFIGURE "${rw_file_content}" file_contents @ONLY)
    file(WRITE "${build_dir}/${worker_name}.${worker_file_ext}" "${file_contents}")
    file(COPY "${R_FILE_PATH}" DESTINATION ${build_dir})

    #add the file that holds the requirements as an item that needs to be
    #installed
    list(APPEND files_to_install "${build_dir}/${R_FILE_NAME}")
  else()
    #need to generate the file into the build directory
    #from the string we have inside this function
    set(rw_file_content
    "
    {
    \"ExecutableName\": \"@worker_name@\",
    \"InputType\": \"@R_INPUT_TYPE@\",
    \"OutputType\": \"@R_OUTPUT_TYPE@\"
    }
    "
    )
    string(CONFIGURE "${rw_file_content}" file_contents @ONLY)
    file(WRITE "${build_dir}/${worker_name}.${worker_file_ext}" "${file_contents}")
  endif()


  if(WIN32)
    set(target_depends )
    foreach(f ${files_to_install})
      get_filename_component(fname "${f}" NAME)

      #copy the worker files to install to the Debug/Release/etc build dirs
      set(output_path "${build_dir}/${CMAKE_CFG_INTDIR}/${fname}")
      add_custom_command(OUTPUT "${build_dir}/${CMAKE_CFG_INTDIR}/${fname}"
        COMMAND ${CMAKE_COMMAND} -E copy "${f}" "${output_path}")

      #properly save all the files we copied so that we can state that the
      #custom target depends on them
      list(APPEND target_depends "${output_path}")
    endforeach()

    add_custom_target(Copy${worker_name}WorkerRegFile
                      ALL DEPENDS "${target_depends}" )
  endif()

  #install all the files needed for this worker in every of the bin directories
  #that need meshing workers
  if(NOT R_NO_INSTALL)
    foreach(dest ${destinations_to_install})
      foreach(f ${files_to_install})
        install (FILES "${f}" DESTINATION "${dest}")
      endforeach()
    endforeach()
  endif()

endfunction()
