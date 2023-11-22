cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project ("Wayland Server Delegate")

include ("${CMAKE_CURRENT_LIST_DIR}/wayland-server-delegate-config.cmake")

install (TARGETS wayland-server-delegate wayland_protocols
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
