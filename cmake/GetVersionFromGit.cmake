find_program(GIT_COMMAND git)

function(get_version_from_git VERSION_VAR VERSION_FULL_VAR)
  if(GIT_COMMAND)
    execute_process(
      COMMAND ${GIT_COMMAND} describe
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      RESULT_VARIABLE GIT_DESCRIBE_RESULT
      OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(GIT_DESCRIBE_RESULT EQUAL 0)
      string(REGEX REPLACE "^v([0-9]+.[0-9]+.[0-9]+).*" "\\1" VERSION ${GIT_DESCRIBE_OUTPUT})
      string(REGEX REPLACE "^v[0-9]+.[0-9]+.[0-9]+-?([0-9]*)-?.*" "\\1" VERSION_POST_COMMITS ${GIT_DESCRIBE_OUTPUT})
      if(VERSION_POST_COMMITS)
        set(VERSION_FULL "${VERSION}.post${VERSION_POST_COMMITS}")
      else()
        set(VERSION_FULL "${VERSION}")
      endif()
    endif()
  endif()
  if(NOT VERSION AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/version.txt")
    message(STATUS "Could not read version from git and will fall back to version.txt")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" VERSION_RAW)
    string(STRIP "${VERSION_RAW}" VERSION_FULL)
    string(REGEX REPLACE "^([0-9]+.[0-9]+.[0-9]+).*" "\\1" VERSION ${VERSION_FULL})
  endif()
  if(NOT VERSION)
    message(WARNING "Could not read version from git or version.txt")
    set(VERSION "0.0.0")
    set(VERSION_FULL "Unknown")
  endif()
  set(${VERSION_VAR}
      ${VERSION}
      PARENT_SCOPE
  )
  set(${VERSION_FULL_VAR}
      ${VERSION_FULL}
      PARENT_SCOPE
  )
endfunction()
