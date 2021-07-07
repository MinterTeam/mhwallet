cmake_minimum_required(VERSION 3.5)
project(mhwallet
        LANGUAGES CXX C
        VERSION 0.3.0
        DESCRIPTION "Minter Ledger Wallet CLI and C++ library to communicate with wallet")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE OFF)
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0091 NEW)

option(ENABLE_SHARED "Build shared library" Off)
option(ENABLE_CLI "Build cli interface" Off)
option(ENABLE_TEST "Enable tests (now not implemented yet)" Off)

if (ENABLE_CLI)
	list(APPEND CONAN_OPTS "cli=True")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cfg/version.in ${CMAKE_CURRENT_SOURCE_DIR}/version @ONLY NEWLINE_STYLE UNIX)

include(modules/compiler_flags.cmake)

set(MINTER_MH_EXPORTING 1)
if (ENABLE_SHARED)
	set(MINTER_EXPORT_SHARED 1)
endif ()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cfg/mhwallet_core.in.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/minter/ledger/mhwallet_core.h" NEWLINE_STYLE UNIX)

include(ConanInit)
conan_init()
add_conan_remote(edwardstock https://edwardstock.jfrog.io/artifactory/api/conan/conan)
add_conan_remote(minter https://minter.jfrog.io/artifactory/api/conan/conan)


set(HID_SOURCES)
set(HID_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/hid)
set(HID_HEADERS include/minter/ledger/hid/hidapi.h)

if (APPLE)
	message(STATUS "hidapi: MacOS")
	set(HID_SOURCES ${HID_SOURCES} ${HID_SRC_ROOT}/mac/hid.c)
elseif (MSVC)
	message(STATUS "hidapi: Windows ${HID_SRC_ROOT}/windows/hid.c")
	set(HID_SOURCES ${HID_SOURCES} ${HID_SRC_ROOT}/windows/hid.c)
elseif (MINGW)
	message(STATUS "hidapi: MinGW")
	message(FATAL_ERROR "Unimplemented functional for MinGW")
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
    include/minter/ledger/hidpp_device.h
    include/minter/ledger/hidpp_base.h
    include/minter/ledger/hidpp_device_info.h
    include/minter/ledger/hidpp.h
    include/minter/ledger/frame_io.h
    include/minter/ledger/frame_io_apdu.h
    include/minter/ledger/nanos_wallet.h
    include/minter/ledger/errors.h
    include/minter/ledger/utils.h)

set(SOURCES
    ${HEADERS}
    ${HID_SOURCES}
    src/hidpp_device.cpp
    src/hidpp_device_info.cpp
    src/hidpp_base.cpp
    src/frame_io.cpp
    src/frame_io_apdu.cpp
    src/nanos_wallet.cpp
    src/utils.cpp)

# Main LIB
if (ENABLE_SHARED)
	message(STATUS "Build shared library")
	add_library(${PROJECT_NAME} SHARED ${SOURCES})
	set_property(TARGET ${PROJECT_NAME} PROPERTY
	             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
else ()
	add_library(${PROJECT_NAME} STATIC ${SOURCES})
	set_property(TARGET ${PROJECT_NAME} PROPERTY
	             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES LANGUAGE CXX)


target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::fmt)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::minter_tx)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::toolbox)


target_include_directories(${PROJECT_NAME} PUBLIC include/minter/ledger/hid)

if (UNIX AND NOT APPLE)
	include(FindPkgConfig)
	# required: libusb-1.0-0-dev
	# required: libudev-dev
	# libgl1-mesa-dev
	find_library(UDEV_LIB udev NAME libudev.a libudev.so udev)
	if (UDEV_LIB-NOT_FOUND)
		message(FATAL_ERROR "libudev not found! Please install it (for debian: apt install libudev-dev)")
	endif ()

	find_package(LibUsb 1.0)
	if (LibUsb-NOT_FOUND)
		message(FATAL_ERROR "libusb not found! Please install it (for debian: apt install libusb-1.0-0-dev")
	endif ()

	message(STATUS "libudev: ${UDEV_LIB}")
	message(STATUS "libusb: ${LibUsb_LIBRARIES} ${LibUsb_INCLUDE_DIRS}")
	target_include_directories(${PROJECT_NAME} PUBLIC ${LibUsb_INCLUDE_DIRS})

	target_link_libraries(${PROJECT_NAME} ${UDEV_LIB})
	target_link_libraries(${PROJECT_NAME} ${LibUsb_LIBRARIES})

elseif (APPLE)
	find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
	find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
	find_library(USB_LIB usb REQUIRED NAMES libusb-1.0.a libusb.a libusb.dylib libusb-1.0-dylib libusb-1.0.0.dylib)
	message(STATUS "libusb: ${USB_LIB}")
	message(STATUS "Framework IOKit: ${IOKIT_FRAMEWORK}")
	message(STATUS "Framework CoreFoundation: ${CORE_FOUNDATION_FRAMEWORK}")
	target_link_libraries(${PROJECT_NAME} ${USB_LIB})
	target_link_libraries(${PROJECT_NAME} ${IOKIT_FRAMEWORK})
	target_link_libraries(${PROJECT_NAME} ${CORE_FOUNDATION_FRAMEWORK})

elseif (MSVC)
	include(FindWinHID)
	target_link_libraries(${PROJECT_NAME} ${WINHID_LIBRARY} ${WINHID_SETUPAPI_LIBRARY})
	target_include_directories(${PROJECT_NAME} PUBLIC ${WINHID_INCLUDE_DIR})
	target_include_directories(${PROJECT_NAME} PUBLIC ${WINHID_CRT_INCLUDE_DIR})

	message(STATUS "WINHID LIBS: ${WINHID_SETUPAPI_LIBRARY}")

elseif (MINGW)
	include(FindWinHID)
	target_link_libraries(${PROJECT_NAME} ${WINHID_LIBRARY})
endif ()

# CLI
if (ENABLE_CLI)
	add_executable(${PROJECT_NAME}-cli src/main.cpp)

	target_include_directories(${PROJECT_NAME}-cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_link_libraries(${PROJECT_NAME}-cli ${PROJECT_NAME})
	target_link_libraries(${PROJECT_NAME}-cli CONAN_PKG::boost)
endif ()

if (ENABLE_TEST)
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
	    tests/main.cpp)

	add_executable(${PROJECT_NAME_TEST} ${TEST_SOURCES})
	target_link_libraries(${PROJECT_NAME_TEST} ${PROJECT_NAME})
	target_link_libraries(${PROJECT_NAME_TEST} CONAN_PKG::gtest)

	if (NOT MSVC)
		target_compile_options(${PROJECT_NAME_TEST} PUBLIC -Wno-unused-parameter)
	endif ()

endif ()
