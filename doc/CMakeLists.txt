option(libadplug_BUILD_DOCUMENTATION "Build documentation" "${libadplug_IS_TOP_LEVEL}")
if(NOT libadplug_BUILD_DOCUMENTATION)
  return()
endif()

find_program(MAKEINFO_EXECUTABLE
  NAMES makeinfo
  DOC "makeinfo executable location"
)

include(GNUInstallDirs)

set(VERSION "${PROJECT_VERSION}")
set(sharedstatedir "${SHAREDSTATEDIR}")
configure_file(adplugdb.1.in "${CMAKE_CURRENT_BINARY_DIR}/doc/adplugdb.1")

if(NOT MAKEINFO_EXECUTABLE)
  if(NOT WIN32)
    message(STATUS "makeinfo not found")
  endif()
  return()
endif()

if(CMAKE_VERSION VERSION_LESS 3.20)
  function(convert_to_tex result path)
    get_filename_component(base_name "${file}" NAME_WLE)
    set(tex_name "${base_name}.tex")
    set(${result} "${tex_name}" PARENT_SCOPE)
  endfunction()

  function(get_input_path result path)
    get_filename_component(output_path "${path}" ABSOLUTE)
    set(${result} "${output_path}" PARENT_SCOPE)
  endfunction()
else()
  function(convert_to_tex result path)
    cmake_path(GET path STEM LAST_ONLY base_name)
    set(tex_name "${base_name}.tex")
    set(${result} "${tex_name}" PARENT_SCOPE)
  endfunction()

  function(get_input_path result path)
    cmake_path(ABSOLUTE_PATH path OUTPUT_VARIABLE output_path)
    set(${result} "${output_path}" PARENT_SCOPE)
  endfunction()
endif()


set(LIBADPLUG_INFO_FILE "${CMAKE_CURRENT_BINARY_DIR}/libadplug.info")
set(LIBADPLUG_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/version.texi")

file(TIMESTAMP "${CMAKE_CURRENT_SOURCE_DIR}/libadplug.texi" LIBADPLUG_TEXI_UPDATED "%d %B %Y" UTC)
file(TIMESTAMP "${CMAKE_CURRENT_SOURCE_DIR}/libadplug.texi" LIBADPLUG_TEXI_MONTH_UPDATED "%B %Y" UTC)

set(at @)

configure_file(version.texi.in "${LIBADPLUG_VERSION_FILE}")

set(SOURCES libadplug.texi fdl.texi "${LIBADPLUG_VERSION_FILE}")

get_input_path(input_f libadplug.texi)

add_custom_command(
  COMMAND
    "${MAKEINFO_EXECUTABLE}" -o "${LIBADPLUG_INFO_FILE}" "${input_f}"
  OUTPUT "${LIBADPLUG_INFO_FILE}"
  DEPENDS ${SOURCES}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "Building documentation"
  VERBATIM
)

add_custom_target(libadplug_doc ALL DEPENDS "${LIBADPLUG_INFO_FILE}")

