# Derived from https://alexreinking.com/blog/building-a-dual-shared-and-static-library-with-cmake.html

if(CMAKE_VERSION VERSION_LESS 3.21)
  set(REINKING_PACKAGING_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
endif()

function(reinking_package name)
  set(config_file "${CMAKE_CURRENT_BINARY_DIR}/${name}-config.cmake")
  set(config_version_file "${CMAKE_CURRENT_BINARY_DIR}/${name}-config-version.cmake")

  if(NOT DEFINED REINKING_PACKAGING_BASE_DIR)
    set(REINKING_PACKAGING_BASE_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}")
  endif()

  if(NOT DEFINED ${name}_INSTALL_CMAKEDIR)
     set(${name}_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${name}"
         CACHE STRING "Path to ${name} CMake files")
  endif()

  install(TARGETS ${name}
    EXPORT ${name}_Targets
    RUNTIME COMPONENT ${name}_Runtime
    LIBRARY COMPONENT ${name}_Runtime
    NAMELINK_COMPONENT ${name}_Development
    ARCHIVE COMPONENT ${name}_Development
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )

  if(${name}_BUILD_SHARED_LIBS)
      set(type shared)
  else()
      set(type static)
  endif()

  # We are only setting the library name, so use configure_file(), not configure_package_config_file(). 
  configure_file("${REINKING_PACKAGING_BASE_DIR}/shared-static-config.cmake.in" "${config_file}" @ONLY)

  install(EXPORT ${name}_Targets
    DESTINATION "${${name}_INSTALL_CMAKEDIR}"
    NAMESPACE ${name}::
    FILE ${name}-${type}-targets.cmake
    COMPONENT ${name}_Development
  )

  write_basic_package_version_file(
    ${name}-config-version.cmake
    COMPATIBILITY SameMajorVersion
  )

  install(FILES
    "${config_file}"
    "${config_version_file}"
    DESTINATION "${${name}_INSTALL_CMAKEDIR}"
    COMPONENT ${name}_Development
  )

endfunction()
