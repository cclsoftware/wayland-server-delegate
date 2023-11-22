#************************************************************************************************
#
# Wayland Server Delegate
#
# Copyright (c) 2023 CCL Software Licensing GmbH. All Rights Reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# - Neither the name of the wayland-server-delegate project nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS",
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Filename    : wayland-server-delegate-config.cmake
# Description : CMake target for Wayland Server Delegate
#
#************************************************************************************************

if (TARGET wayland-server-delegate)
	return ()
endif ()

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
find_package (Wayland REQUIRED COMPONENTS client server protocols)

find_path (serverdelegate_dir NAMES "iwaylandserver.h" HINTS "${CMAKE_CURRENT_LIST_DIR}/.." DOC "Wayland Server Delegate base directory")

# Add target
add_library (wayland-server-delegate STATIC)

list (APPEND serverdelegate_source_files
	${serverdelegate_dir}/source/bufferdelegate.cpp
	${serverdelegate_dir}/source/bufferdelegate.h
	${serverdelegate_dir}/source/callbackdelegate.cpp
	${serverdelegate_dir}/source/callbackdelegate.h
	${serverdelegate_dir}/source/regiondelegate.cpp
	${serverdelegate_dir}/source/regiondelegate.h
	${serverdelegate_dir}/source/registrydelegate.cpp
	${serverdelegate_dir}/source/registrydelegate.h
	${serverdelegate_dir}/source/seatdelegates.cpp
	${serverdelegate_dir}/source/seatdelegates.h
	${serverdelegate_dir}/source/sharedmemorypooldelegate.cpp
	${serverdelegate_dir}/source/sharedmemorypooldelegate.h
	${serverdelegate_dir}/source/surfacedelegate.cpp
	${serverdelegate_dir}/source/surfacedelegate.h
	${serverdelegate_dir}/source/waylandresource.cpp
	${serverdelegate_dir}/source/waylandserver.cpp
	${serverdelegate_dir}/source/xdgsurfacedelegate.cpp
	${serverdelegate_dir}/source/xdgsurfacedelegate.h
)

list (APPEND serverdelegate_public_header_files
	${serverdelegate_dir}/iwaylandclientcontext.h
	${serverdelegate_dir}/iwaylandserver.h
	${serverdelegate_dir}/waylandresource.h
)

source_group ("source" FILES ${serverdelegate_source_files} ${serverdelegate_public_header_files})

target_sources (wayland-server-delegate PRIVATE ${serverdelegate_source_files} ${serverdelegate_public_header_files})
target_link_libraries (wayland-server-delegate PUBLIC ${WAYLAND_LIBRARIES})
target_include_directories (wayland-server-delegate PRIVATE "${serverdelegate_dir}/source" PUBLIC "${serverdelegate_dir}/..")
set_target_properties (wayland-server-delegate PROPERTIES PUBLIC_HEADER "${serverdelegate_public_header_files}")
