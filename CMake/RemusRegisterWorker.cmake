#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================


#we create a text file that list this mesh worker and its type
function(Register_Mesh_Worker workerExecutableName meshType)
configure_file(${Remus_SOURCE_DIR}/CMake/MeshWorker.msw.in
			   ${EXECUTABLE_OUTPUT_PATH}/${workerExecutableName}.msw
			   @ONLY)
endfunction()


