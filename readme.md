# Wayland Server Delegate

A nested delegating Wayland server implementation.

Wayland applications can use `wayland-server-delegate` to allow plug-ins or child processes to connect to the main application via standard Wayland interfaces.

The main application acts as both a Wayland client and a Wayland server. Plug-ins or child processes don't connect to the session compositor, but connect to the main application instead.

By using `wayland-server-delegate`, Wayland requests issued by the plug-in are forwarded to the main application's session compositor connection. Likewise, session compositor events are forwarded to the plug-in's Wayland connection.

# Building wayland-server-delegate

`wayland-server-delegate` can either be used as a CMake package or built as a standalone static library.

## CMake Package

Clone this repository, add the `wayland-server-delegate/cmake` directory to your CMAKE_PREFIX_PATH and call `find_package` to import a new static library target.

Example:

```
cmake_minimum_required (VERSION 3.20)

project ("Wayland Server Delegate Test")

add_executable (test main.cpp)

list (APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../wayland-server-delegate/cmake")

find_package (wayland-server-delegate)

link_libraries (test PRIVATE wayland-server-delegate)
```

## Standalone Static Library

Clone this repository and run the following commands to build `libwayland-server-delegate.a`:

```
cd cmake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

# Usage

In order to use `wayland-server-delegate`, an application needs to provide an implementation of `WaylandServerDelegate::IWaylandClientContext`.

Using this implementation, the application must call `WaylandServerDelegate::IWaylandServer::instance ().startup (context)` and incorporate the returned server file descriptor into the main event loop.

The application must call `WaylandServerDelegate::IWaylandServer::instance ().dispatch ()` whenever data is available at the server file descriptor.

The application must also call `WaylandServerDelegate::IWaylandServer::instance ().flush ()` regularly.

To connect a plug-in to the main application, the application may call `WaylandServerDelegate::IWaylandServer::instance ().openClientConnection ()` and pass the returned `wl_display*` handle to the plug-in.
The plug-in may then use this display handle in standard Wayland calls like `wl_display_get_fd` or `wl_display_read_events`.

In order to share Wayland objects with a plug-in, the application may call `WaylandServerDelegate::IWaylandServer::instance ().createProxy (...)`, providing the plug-in's display handle, an existing Wayland object, and an implementation class derived from `WaylandServerDelegate::WaylandResource`. The plug-in can then use the returned `wl_proxy*` in standard Wayland calls.
One use case for this would be to embed a plug-in user interface into an existing application window. The application could create a proxy for an existing `wl_surface`, which the plug-in could then use as a parent in a call to `wl_subcompositor_get_subsurface`.

Upon termination, the application should call `WaylandServerDelegate::IWaylandServer::instance ().openClientConnection (display)` to close the previously opened client connection and `WaylandServerDelegate::IWaylandServer::instance ().shutdown ()` to release all server resources.
