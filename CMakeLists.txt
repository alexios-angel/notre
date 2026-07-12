cmake_minimum_required(VERSION 3.14...4.3)


if(NOT CMAKE_EXPERIMENTAL_CXX_STD AND NOT CMAKE_CXX_MODULE_STD)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30" AND CMAKE_VERSION VERSION_LESS_EQUAL "4.0")
      set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
    elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0" AND CMAKE_VERSION VERSION_LESS "4.0.3")
        set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")

    elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "4.0.3" AND CMAKE_VERSION VERSION_LESS "4.3")
        set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
        set(CMAKE_CXX_MODULE_STD 1)
    elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "4.3")
        set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
        set(CMAKE_CXX_MODULE_STD 1)
    endif()
endif()

# When updating to a newer version of CMake, see if we can use the following
project(notre
  HOMEPAGE_URL "https://compile-time.re"
  VERSION 3.9.0
  LANGUAGES CXX)
set(PROJECT_DESCRIPTION "Fast compile-time regular expressions with support for matching/searching/capturing during compile-time or runtime.")

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(GNUInstallDirs)
include(CTest)

find_program(NOTRE_DPKG_BUILDPACKAGE_FOUND dpkg-buildpackage)
find_program(NOTRE_RPMBUILD_FOUND rpmbuild)

cmake_dependent_option(NOTRE_BUILD_TESTS "Build notre Tests" ON
  "BUILD_TESTING;CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
cmake_dependent_option(NOTRE_BUILD_PACKAGE "Build notre Packages" ON
  "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
cmake_dependent_option(NOTRE_BUILD_PACKAGE_DEB
  "Create DEB Package (${PROJECT_NAME})" ON
  "NOTRE_BUILD_PACKAGE;NOTRE_DPKG_BUILDPACKAGE_FOUND" OFF)
cmake_dependent_option(NOTRE_BUILD_PACKAGE_RPM
  "Create RPM Package (${PROJECT_NAME})" ON
  "NOTRE_BUILD_PACKAGE;NOTRE_RPMBUILD_FOUND" OFF)

option(NOTRE_MODULE "build C++ module" OFF)

if(NOTRE_MODULE)
	if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.29.20240416") 
		add_library(${PROJECT_NAME})
		
		target_sources(${PROJECT_NAME} PUBLIC FILE_SET CXX_MODULES TYPE CXX_MODULES FILES notre.cppm)
		target_sources(${PROJECT_NAME} PUBLIC FILE_SET HEADERS TYPE HEADERS
		    BASE_DIRS
			"${CMAKE_CURRENT_SOURCE_DIR}/include"
		    FILES 
			include/ctll.hpp
			include/notre/functions.hpp
			include/notre/utility.hpp
			include/notre/utf8.hpp
			include/notre/evaluation.hpp
			include/notre/starts_with_anchor.hpp
			include/notre/pcre_actions.hpp
			include/notre/resolve_subroutines.hpp
			include/notre/rotate.hpp
			include/notre/iterators.hpp
			include/notre/literals.hpp
			include/notre/return_type.hpp
			include/notre/find_captures.hpp
			include/notre/id.hpp
			include/notre/atoms_characters.hpp
			include/notre/actions/mode.inc.hpp
			include/notre/actions/characters.inc.hpp
			include/notre/actions/class.inc.hpp
			include/notre/actions/conditional.inc.hpp
			include/notre/actions/look.inc.hpp
			include/notre/actions/sequence.inc.hpp
			include/notre/actions/fusion.inc.hpp
			include/notre/actions/asserts.inc.hpp
			include/notre/actions/capture.inc.hpp
			include/notre/actions/comment.inc.hpp
			include/notre/actions/named_class.inc.hpp
			include/notre/actions/backreference.inc.hpp
			include/notre/actions/callout.inc.hpp
			include/notre/actions/options.inc.hpp
			include/notre/actions/atomic_group.inc.hpp
			include/notre/actions/set.inc.hpp
			include/notre/actions/subroutine.inc.hpp
			include/notre/actions/hexdec.inc.hpp
			include/notre/actions/repeat.inc.hpp
			include/notre/actions/properties.inc.hpp
			include/notre/actions/boundaries.inc.hpp
			include/notre/operators.hpp
			include/notre/pcre.hpp
			include/notre/atoms_unicode.hpp
			include/notre/callouts.hpp
			include/notre/range.hpp
			include/notre/wrapper.hpp
			include/notre/first.hpp
			include/notre/flags_and_modes.hpp
			include/notre/atoms.hpp
			include/unicode-db.hpp
			include/unicode-db/unicode_interface.hpp
			include/unicode-db/unicode-db.hpp
			include/ctll/parser.hpp
			include/ctll/actions.hpp
			include/ctll/callout.hpp
			include/ctll/fixed_string.hpp
			include/ctll/function.hpp
			include/ctll/map.hpp
			include/ctll/list.hpp
			include/ctll/utilities.hpp
			include/ctll/grammars.hpp
			include/notre.hpp
			include/notre-unicode.hpp
		)
		
		# we are using `import std;`
		if (NOT DEFINED NOTRE_CXX_STANDARD OR NOTRE_CXX_STANDARD LESS 23)
			set(NOTRE_CXX_STANDARD 23)
		endif()

		target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${NOTRE_CXX_STANDARD})

		install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
			FILE_SET CXX_MODULES DESTINATION "${CMAKE_INSTALL_LIBDIR}/cxx/${PROJECT_NAME}"
			FILE_SET HEADERS DESTINATION "include")
	else()
		message(FATAL_ERROR "unsupported cmake for c++ modules")
	endif()
else()
	add_library(${PROJECT_NAME} INTERFACE)
	
	target_include_directories(${PROJECT_NAME} INTERFACE
	  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	  $<INSTALL_INTERFACE:include>)

	if (NOT NOTRE_CXX_STANDARD)
	  set(NOTRE_CXX_STANDARD 20)
	endif()

	target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_${NOTRE_CXX_STANDARD})
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_SCAN_FOR_MODULES 0)
	
	install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets)
	install(DIRECTORY include/ DESTINATION include
	    FILES_MATCHING PATTERN *.hpp)
endif()

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (NOT EXISTS "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in")
  file(WRITE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in [[
    @PACKAGE_INIT@
    include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake")
  ]])
endif()

configure_package_config_file(
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(notre-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(EXPORT ${PROJECT_NAME}-targets 
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  NAMESPACE ${PROJECT_NAME}::)
install(
  FILES
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

if(NOTRE_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if (NOT NOTRE_BUILD_PACKAGE)
  return()
endif()

list(APPEND source-generators TBZ2 TGZ TXZ ZIP)

if (NOTRE_BUILD_PACKAGE_DEB)
  list(APPEND binary-generators "DEB")
endif()

if (NOTRE_BUILD_PACKAGE_RPM)
  list(APPEND binary-generators "RPM")
endif()

set(CPACK_SOURCE_GENERATOR ${source-generators})
set(CPACK_GENERATOR ${binary-generators})

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Hana Dusíková")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_DEBIAN_PACKAGE_NAME "lib${PROJECT_NAME}-dev")

set(CPACK_RPM_PACKAGE_NAME "lib${PROJECT_NAME}-devel")

set(PKG_CONFIG_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/pkgconfig.pc.in" "${PKG_CONFIG_FILE_NAME}" @ONLY)
install(FILES "${PKG_CONFIG_FILE_NAME}"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

list(APPEND CPACK_SOURCE_IGNORE_FILES /.git/ /build/ .gitignore .DS_Store)

include(CPack)
