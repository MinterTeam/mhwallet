cmake_minimum_required(VERSION 3.10)
project(mhwallet
        LANGUAGES CXX C
        VERSION 0.1.0
        DESCRIPTION "Minter Ledger Wallet CLI and C++ library to communicate with wallet")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

if (WIN32)
	execute_process(
		COMMAND cmd /C "echo ${CMAKE_PROJECT_VERSION} > ${CMAKE_CURRENT_SOURCE_DIR}/version"
	)
else ()
	execute_process(
		COMMAND bash -c "echo ${CMAKE_PROJECT_VERSION} > ${CMAKE_CURRENT_SOURCE_DIR}/version"
	)
endif ()


if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2 /MP")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DEBUG /Od")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
	string(REGEX REPLACE "\\/W3" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	add_compile_options(/wd4068)
	add_compile_options(/wd4319)
	add_compile_options(/wd4251)
	add_compile_options(/wd4275)

	if (NOT ENABLE_SHARED)
		foreach (flag_var
		         CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
		         CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
			if (${flag_var} MATCHES "/MD")
				string(REGEX REPLACE "MD" "MT" ${flag_var} "${${flag_var}}")
			endif (${flag_var} MATCHES "/MD")
		endforeach (flag_var)
	endif ()

	foreach (flag_var
	         CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
	         CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if (${flag_var} MATCHES "/MD ")
			string(REGEX REPLACE "MD " "MDd " ${flag_var} "${${flag_var}}")
		endif (${flag_var} MATCHES "/MD ")
		if (${flag_var} MATCHES "/MT ")
			string(REGEX REPLACE "MT " "MTd " ${flag_var} "${${flag_var}}")
		endif (${flag_var} MATCHES "/MT ")
	endforeach (flag_var)

	message(STATUS "
	               CXXFLAGS=${CMAKE_CXX_FLAGS}
	         CXXFLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
	       CXXFLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
	    CXXFLAGS_MINSIZEREL=${CMAKE_CXX_FLAGS_MINSIZEREL}
	CXXFLAGS_RELWITHDEBINFO=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
	message(STATUS "
	                 CFLAGS=${CMAKE_C_FLAGS}
	          CFLAGSS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
	         CFLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
	      CFLAGS_MINSIZEREL=${CMAKE_C_FLAGS_MINSIZEREL}
	  CFLAGS_RELWITHDEBINFO=${CMAKE_C_FLAGS_RELWITHDEBINFO}")


elseif (MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -Wall -fPIC -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -Wno-shift-count-overflow")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif ()

set(MINTER_MH_EXPORTING 1)
if (ENABLE_SHARED)
	set(MINTER_EXPORT_SHARED 1)
endif ()

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cfg/mhwallet_core.h.in
	${CMAKE_CURRENT_SOURCE_DIR}/include/minter/mhwallet_core.h
)

include(ConanInit)
conan_init()
add_conan_remote(bincrafters https://api.bintray.com/conan/bincrafters/public-conan)
add_conan_remote(edwardstock https://api.bintray.com/conan/edwardstock/conan-public)
add_conan_remote(minter https://api.bintray.com/conan/edwardstock/conan-public)


set(HID_HEADERS include/minter/hid/hidapi.h)
set(HID_SOURCES)
set(HID_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/hid)

if (APPLE)
	message(STATUS "hidapi: MacOS")
	set(HID_SOURCES ${HID_SOURCES} ${HID_SRC_ROOT}/mac/hid.c)
elseif (MSVC)
	message(STATUS "hidapi: Windows")
	set(HID_SOURCES ${HID_SOURCES} ${HID_SRC_ROOT}/windows/hid.c)
elseif (MINGW)
	message(STATUS "hidapi: MinGW")
	#TODO
elseif (CYGWIN)
	message(STATUS "hidapi: Windows Cygwin")
	set(HID_SOURCES ${HID_SOURCES} ${HID_SRC_ROOT}/libusb/hid.c)
	find_library(USB_LIB usb REQUIRED)
elseif (ANDROID_PLATFORM)
	message(STATUS "hidapi: Android")
	#TODO
else ()
	message(STATUS "hidapi: Linux")
	set(HID_SOURCES ${HID_SOURCES} ${HID_SRC_ROOT}/linux/hid.c)
endif ()

set(HEADERS
    ${HID_HEADERS}
    include/minter/hidpp_device.h
    include/minter/hidpp_base.h
    include/minter/hidpp_device_info.h
    include/minter/hidpp.h
    include/minter/frame_io.h
    include/minter/frame_io_apdu.h
    include/minter/nanos_wallet.h
    )

set(SOURCES
    ${HEADERS}
    ${HID_SOURCES}
    src/hidpp_device.cpp
    src/hidpp_device_info.cpp
    src/hidpp_base.cpp
    src/frame_io.cpp
    src/frame_io_apdu.cpp
    src/nanos_wallet.cpp)

# Main LIB
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::fmt)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::minter_tx)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::toolboxpp)
target_include_directories(${PROJECT_NAME} PUBLIC include/minter/hid/)

if (UNIX AND NOT APPLE)
	find_library(UDEV_LIB udev NAME libudev.a libudev.so udev)
	if (UDEV_LIB-NOT_FOUND)
		message(FATAL_ERROR "libudev not found!")
	endif ()

	find_package(LibUsb 1.0 REQUIRED)

	message(STATUS "libudev: ${UDEV_LIB}")
	message(STATUS "libusb: ${LibUSB_LIBRARIES} ${LibUSB_INCLUDE_DIRS}")
	target_include_directories(${PROJECT_NAME} PUBLIC ${LibUSB_INCLUDE_DIRS})

	target_link_libraries(${PROJECT_NAME} ${UDEV_LIB})
	target_link_libraries(${PROJECT_NAME} ${USB_LIB})

elseif (APPLE)
	find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
	find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
	find_library(USB_LIB usb REQUIRED)
	message(STATUS "libusb: ${USB_LIB}")
	message(STATUS "Framework IOKit: ${IOKIT_FRAMEWORK}")
	message(STATUS "Framework CoreFoundation: ${CORE_FOUNDATION_FRAMEWORK}")
	target_link_libraries(${PROJECT_NAME} ${USB_LIB})
	target_link_libraries(${PROJECT_NAME} ${IOKIT_FRAMEWORK})
	target_link_libraries(${PROJECT_NAME} ${CORE_FOUNDATION_FRAMEWORK})
elseif (MSVC)
	find_library(MS_HID_LIB hid NAMES hid.lib hid.dll)
	message(STATUS "WIN32      hid: ${MS_HID_LIB}")

	find_library(MS_KERNEL32_LIB kernel32 NAMES kernel32.lib kernel32.dll)
	message(STATUS "WIN32 kernel32: ${MS_KERNEL32_LIB}")

	find_library(MS_COMDL32_LIB comdlg32 NAMES comdlg32.lib comdlg32.dll)
	message(STATUS "WIN32 comdlg32: ${MS_COMDL32_LIB}")

	find_library(MS_SETUPAPI_LIB setupapi NAMES setupapi.lib setupapi.dll)
	message(STATUS "WIN32 setupapi: ${MS_SETUPAPI_LIB}")

	target_link_libraries(${PROJECT_NAME} ${MS_HID_LIB} ${MS_KERNEL32_LIB} ${MS_COMDL32_LIB})
	target_link_libraries(${PROJECT_NAME} ${MS_HID_LIB} ${MS_SETUPAPI_LIB})

elseif (MINGW)
	find_library(MS_HID_LIB hid NAMES hid.lib hid.dll)
	message(STATUS "WIN32      hid: ${MS_HID_LIB}")

	find_library(MS_KERNEL32_LIB kernel32 NAMES kernel32.lib kernel32.dll)
	message(STATUS "WIN32 kernel32: ${MS_KERNEL32_LIB}")

	find_library(MS_COMDL32_LIB comdlg32 NAMES comdlg32.lib comdlg32.dll)
	message(STATUS "WIN32 comdlg32: ${MS_COMDL32_LIB}")

	find_library(MS_SETUPAPI_LIB setupapi NAMES setupapi.lib setupapi.dll)
	message(STATUS "WIN32 setupapi: ${MS_SETUPAPI_LIB}")

	target_link_libraries(${PROJECT_NAME} ${MS_HID_LIB} ${MS_KERNEL32_LIB} ${MS_COMDL32_LIB})
	target_link_libraries(${PROJECT_NAME} ${MS_HID_LIB} ${MS_SETUPAPI_LIB})
endif ()

# CLI
add_executable(${PROJECT_NAME}-cli src/main.cpp)
target_include_directories(${PROJECT_NAME}-cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}-cli ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}-cli CONAN_PKG::boost)


if (TESTS)
	set(gtest_force_shared_crt ${ENABLE_SHARED} CACHE BOOL "" FORCE)
	add_definitions(-DTEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests")
	set(PROJECT_NAME_TEST ${PROJECT_NAME}-test)

	if (APPLE)
		add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
		add_definitions(-D__GLIBCXX__)
	endif ()

	if (MSVC)
		string(REGEX REPLACE "\\/W4" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
		string(REGEX REPLACE "\\/W4" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
	endif ()

	set(TEST_SOURCES
	    tests/main.cpp
	    )

	add_executable(${PROJECT_NAME_TEST} ${TEST_SOURCES})
	target_link_libraries(${PROJECT_NAME_TEST} ${PROJECT_NAME})
	target_link_libraries(${PROJECT_NAME_TEST} CONAN_PKG::gtest)

	if (NOT MSVC)
		target_compile_options(${PROJECT_NAME_TEST} PUBLIC -Wno-unused-parameter)
	endif ()

endif ()
