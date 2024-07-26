//************************************************************************************************
//
// Wayland Server Delegate
//
// Copyright (c) 2023 CCL Software Licensing GmbH. All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// - Neither the name of the wayland-server-delegate project nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS",
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Filename    : surfacedelegate.h
// Description : Wayland Surface Delegate
//
//************************************************************************************************

#ifndef _surfacedelegate_h
#define _surfacedelegate_h

#include "wayland-server-delegate/waylandresource.h"

#include "xdg-shell-client-protocol.h"

namespace WaylandServerDelegate {

//************************************************************************************************
// SurfaceDelegate
//************************************************************************************************

class SurfaceDelegate: public WaylandResource,
					   public wl_surface_interface,
					   public wl_surface_listener
{
public:
	SurfaceDelegate (wl_surface* surface);
	~SurfaceDelegate ();

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void onAttach (wl_client* client, wl_resource* resource, wl_resource* buffer, int32_t x, int32_t y);
	static void onDamage (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height);
	static void requestFrame (wl_client* client, wl_resource* resource, uint32_t callback);
	static void setOpaqueRegion (wl_client* client, wl_resource* resource, wl_resource* region);
	static void setInputRegion (wl_client* client, wl_resource* resource, wl_resource* region);
	static void onCommit (wl_client* client, wl_resource* resource);
	static void setBufferTransform (wl_client* client, wl_resource* resource, int32_t transform);
	static void setBufferScale (wl_client* client, wl_resource* resource, int32_t scale);
	static void onDamageBuffer (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height);
	static void setOffset (wl_client* client, wl_resource* resource, int32_t x, int32_t y);

	// listener
	static void onEnter (void* data, wl_surface* surface, wl_output* output);
	static void onLeave (void* data, wl_surface* surface, wl_output* output);

private:
	wl_surface* surface;
};

//************************************************************************************************
// SurfaceDelegate
//************************************************************************************************

class SubSurfaceDelegate: public WaylandResource,
						  public wl_subsurface_interface
{
public:
	SubSurfaceDelegate (wl_subsurface* subSurface);
	~SubSurfaceDelegate ();

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void setPosition (wl_client* client, wl_resource* resource, int32_t x, int32_t y);
	static void placeAbove (wl_client* client, wl_resource* resource, wl_resource* sibling);
	static void placeBelow (wl_client* client, wl_resource* resource, wl_resource* sibling);
	static void setSync (wl_client* client, wl_resource* resource);
	static void setDesync (wl_client* client, wl_resource* resource);

private:
	wl_subsurface* subSurface;
};

} // namespace WaylandServerDelegate

#endif // _surfacedelegate_h
