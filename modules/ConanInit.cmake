find_program(CONAN_BIN conan REQUIRED)
find_program(BASH_BIN bash)
find_program(GREP_BIN grep)

function (add_conan_remote NAME URL)
	find_program(CONAN_BIN conan REQUIRED)

	execute_process(
		COMMAND ${CONAN_BIN} remote add ${NAME} ${URL}
		ERROR_VARIABLE CONAN_ADD_REMOTE_ERR
		OUTPUT_VARIABLE CONAN_ADD_REMOTE_OUT
		RESULT_VARIABLE CONAN_ADD_REMOTE_RES
	)
	message(STATUS "Conan: ${CONAN_BIN} remote add ${NAME} ${URL}")

	if (CONAN_ADD_REMOTE_ERR)
		message(STATUS "Conan: Add remote error: ${CONAN_ADD_REMOTE_ERR}")
	endif ()
	if (CONAN_ADD_REMOTE_OUT)
		if (NOT "${CONAN_ADD_REMOTE_OUT}" STREQUAL "ERROR: Remote '${NAME}' already exists in remotes (use update to modify)")
			message(STATUS "Conan: Add remote output: ${CONAN_ADD_REMOTE_OUT}")
		endif ()
	endif ()
	if (CONAN_ADD_REMOTE_RES)
		message(STATUS "Conan: Add remote res: ${CONAN_ADD_REMOTE_RES}")
	endif ()
endfunction ()

macro (conan_init)
	include(ConanBuild)
	conan_cmake_run(
		CONANFILE conanfile.py
		BUILD missing
		BASIC_SETUP
		OPTIONS "${CONAN_OPTS}"
	)

	if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
		include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
		conan_basic_setup(TARGETS)
	else ()
		message(WARNING "The file ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake doesn't exist, you have to run conan install first")
	endif ()
endmacro ()