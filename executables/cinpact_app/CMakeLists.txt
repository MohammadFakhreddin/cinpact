########################################

set(EXECUTABLE "cinpact")

set(EXECUTABLE_RESOURCES)

list(
    APPEND EXECUTABLE_RESOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/CinpactMain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CinpactApp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CinpactApp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CinpactCurve.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CinpactCurve.hpp"
)

add_executable(${EXECUTABLE} ${EXECUTABLE_RESOURCES})

if (WINDOWS)
    if (DLLS_COMMON)
        add_custom_command(
            TARGET ${EXECUTABLE} POST_BUILD                
            COMMAND ${CMAKE_COMMAND} -E copy_if_different       
            ${DLLS_COMMON}                                     
            "${CMAKE_BINARY_DIR}"
        )
    endif()
    set_property(TARGET ${EXECUTABLE} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/assets")
endif()


########################################