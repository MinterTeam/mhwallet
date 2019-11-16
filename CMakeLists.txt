cmake_minimum_required(VERSION 3.13)
project(mhwallet
        LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
include(ConanInit)
conan_init()

set(HID_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libs/hid)

set(HID_HEADERS
    ${HID_ROOT}/hidapi/hidapi.h
    )

set(HID_SOURCES)

if (APPLE)
	message(STATUS "hidapi: MacOS")
	set(HID_SOURCES ${HID_SOURCES} ${HID_ROOT}/mac/hid.c)
elseif (MSVC)
	message(STATUS "hidapi: Windows")
	set(HID_SOURCES ${HID_SOURCES} ${HID_ROOT}/windows/hid.c)
elseif (MINGW)
	message(STATUS "hidapi: MinGW")
	#TODO
elseif (CYGWIN)
	message(STATUS "hidapi: Windows Cygwin")
	set(HID_SOURCES ${HID_SOURCES} ${HID_ROOT}/libusb/hid.c)
	find_library(USB_LIB usb REQUIRED)
elseif (ANDROID_PLATFORM)
	message(STATUS "hidapi: Android")
	set(HID_SOURCES ${HID_SOURCES} ${HID_ROOT}/libusb/hid.c)
	#TODO
else ()
	message(STATUS "hidapi: Linux")
	set(HID_SOURCES ${HID_SOURCES} ${HID_ROOT}/linux/hid.c)
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
target_include_directories(${PROJECT_NAME} PUBLIC ${HID_ROOT}/hidapi)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::fmt)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::minter_tx)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::toolboxpp)

if (UNIX AND NOT APPLE)
	find_library(UDEV_LIB udev REQUIRED)
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
endif ()

# CLI
add_executable(${PROJECT_NAME}-cli src/main.cpp)
target_include_directories(${PROJECT_NAME}-cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME}-cli PUBLIC ${HID_ROOT}/hidapi)
target_link_libraries(${PROJECT_NAME}-cli ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} CONAN_PKG::boost_program_options)

set(TESTS ON)
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
