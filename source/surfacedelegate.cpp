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
// Filename    : surfacedelegate.cpp
// Description : Wayland Surface Delegate
//
//************************************************************************************************

#define DEBUG_LOG 0

#include "surfacedelegate.h"
#include "bufferdelegate.h"
#include "regiondelegate.h"
#include "callbackdelegate.h"
#include "waylandserver.h"

#include "wayland-server-delegate/iwaylandclientcontext.h"

using namespace WaylandServerDelegate;

//************************************************************************************************
// SurfaceDelegate
//************************************************************************************************

SurfaceDelegate::SurfaceDelegate (wl_surface* surface)
: WaylandResource (&::wl_surface_interface, static_cast<wl_surface_interface*> (this)),
  surface (surface)
{
	destroy = onDestroy;
	attach = onAttach;
	damage = onDamage;
	frame = requestFrame;
	set_opaque_region = setOpaqueRegion;
	set_input_region = setInputRegion;
	commit = onCommit;
	set_buffer_transform = setBufferTransform;
	set_buffer_scale = setBufferScale;
	damage_buffer = onDamageBuffer;
	#ifdef WL_SURFACE_OFFSET_SINCE_VERSION
	offset = setOffset;
	#endif

	enter = onEnter;
	leave = onLeave;
	#ifdef WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION
	preferred_buffer_scale = onPreferredBufferScale;
	preferred_buffer_transform = onPreferredBufferTransform;
	#endif

	if(surface)
		wl_surface_add_listener (surface, this, this);

	setProxy (reinterpret_cast<wl_proxy*> (surface));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SurfaceDelegate::~SurfaceDelegate ()
{
	if(surface)
		wl_surface_destroy (surface);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	wl_resource_destroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::onAttach (wl_client* client, wl_resource* resource, wl_resource* buffer, int32_t x, int32_t y)
{
	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	wl_buffer* bufferHandle = castProxy<wl_buffer> (buffer);
	if(This->surface && bufferHandle)
	{
		#if WL_SURFACE_OFFSET_SINCE_VERSION
		if(wl_surface_get_version (This->surface) >= WL_SURFACE_OFFSET_SINCE_VERSION)
		{
			wl_surface_attach (This->surface, bufferHandle, 0, 0);
			if(wl_resource_get_version (This->getResourceHandle ()) < WL_SURFACE_OFFSET_SINCE_VERSION)
				wl_surface_offset (This->surface, x, y);
		}
		else
		#endif
		{
			wl_surface_attach (This->surface, bufferHandle, x, y);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::onDamage (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	if(This->surface)
		wl_surface_damage (This->surface, x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::requestFrame (wl_client* client, wl_resource* resource, uint32_t callback)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	if(This->surface)
	{
		wl_callback* callbackHandle = wl_surface_frame (This->surface);
		WaylandResource* implementation = new CallbackDelegate (callbackHandle);
		connection->addResource (implementation, callback);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::setOpaqueRegion (wl_client* client, wl_resource* resource, wl_resource* region)
{
	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	wl_region* regionHandle = castProxy<wl_region> (region);
	if(This->surface && regionHandle)
		wl_surface_set_opaque_region (This->surface, regionHandle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::setInputRegion (wl_client* client, wl_resource* resource, wl_resource* region)
{
	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	wl_region* regionHandle = castProxy<wl_region> (region);
	if(This->surface)
		wl_surface_set_input_region (This->surface, regionHandle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::onCommit (wl_client* client, wl_resource* resource)
{
	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	if(This->surface)
		wl_surface_commit (This->surface);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::setBufferTransform (wl_client* client, wl_resource* resource, int32_t transform)
{
	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	if(This->surface)
		wl_surface_set_buffer_transform (This->surface, transform);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::setBufferScale (wl_client* client, wl_resource* resource, int32_t scale)
{
	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	if(This->surface)
		wl_surface_set_buffer_scale (This->surface, scale);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::onDamageBuffer (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	if(This->surface)
		wl_surface_damage_buffer (This->surface, x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::setOffset (wl_client* client, wl_resource* resource, int32_t x, int32_t y)
{
	#ifdef WL_SURFACE_OFFSET_SINCE_VERSION
	SurfaceDelegate* This = cast<SurfaceDelegate> (resource);
	if(This->surface)
		wl_surface_offset (This->surface, x, y);
	#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::onEnter (void* data, wl_surface* surface, wl_output* output)
{
	SurfaceDelegate* This = static_cast<SurfaceDelegate*> (data);
	WaylandServer& server = WaylandServer::instance ();
	WaylandResource* resource = server.findClientResource (This->clientHandle, reinterpret_cast<wl_proxy*> (output));
	if(resource)
		wl_surface_send_enter (This->getResourceHandle (), resource->getResourceHandle ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::onLeave (void* data, wl_surface* surface, wl_output* output)
{
	SurfaceDelegate* This = static_cast<SurfaceDelegate*> (data);
	WaylandServer& server = WaylandServer::instance ();
	WaylandResource* resource = server.findClientResource (This->clientHandle, reinterpret_cast<wl_proxy*> (output));
	if(resource)
		wl_surface_send_leave (This->getResourceHandle (), resource->getResourceHandle ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::onPreferredBufferScale (void* data, wl_surface* surface, int32_t factor)
{
	#ifdef WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION
	SurfaceDelegate* This = static_cast<SurfaceDelegate*> (data);
	if(wl_resource_get_version (This->getResourceHandle ()) >= WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION)
		wl_surface_send_preferred_buffer_scale (This->getResourceHandle (), factor);
	#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SurfaceDelegate::onPreferredBufferTransform (void* data, wl_surface* surface, uint32_t transform)
{
	#ifdef WL_SURFACE_PREFERRED_BUFFER_TRANSFORM_SINCE_VERSION
	SurfaceDelegate* This = static_cast<SurfaceDelegate*> (data);
	if(wl_resource_get_version (This->getResourceHandle ()) >= WL_SURFACE_PREFERRED_BUFFER_TRANSFORM_SINCE_VERSION)
		wl_surface_send_preferred_buffer_transform (This->getResourceHandle (), transform);
	#endif
}

//************************************************************************************************
// SubSurfaceDelegate
//************************************************************************************************

SubSurfaceDelegate::SubSurfaceDelegate (wl_subsurface* subSurface)
: WaylandResource (&::wl_subsurface_interface, static_cast<wl_subsurface_interface*> (this)),
  subSurface (subSurface)
{
	destroy = onDestroy;
	set_position = setPosition;
	place_above = placeAbove;
	place_below = placeBelow;
	set_sync = setSync;
	set_desync = setDesync;

	setProxy (reinterpret_cast<wl_proxy*> (subSurface));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SubSurfaceDelegate::~SubSurfaceDelegate ()
{
	if(subSurface)
		wl_subsurface_destroy (subSurface);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SubSurfaceDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	wl_resource_destroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SubSurfaceDelegate::setPosition (wl_client* client, wl_resource* resource, int32_t x, int32_t y)
{
	SubSurfaceDelegate* This = cast<SubSurfaceDelegate> (resource);
	if(This->subSurface)
		wl_subsurface_set_position (This->subSurface, x, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SubSurfaceDelegate::placeAbove (wl_client* client, wl_resource* resource, wl_resource* sibling)
{
	SubSurfaceDelegate* This = cast<SubSurfaceDelegate> (resource);
	wl_surface* siblingSurface = castProxy<wl_surface> (sibling);
	if(This->subSurface && siblingSurface)
		wl_subsurface_place_above (This->subSurface, siblingSurface);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SubSurfaceDelegate::placeBelow (wl_client* client, wl_resource* resource, wl_resource* sibling)
{
	SubSurfaceDelegate* This = cast<SubSurfaceDelegate> (resource);
	wl_surface* siblingSurface = castProxy<wl_surface> (sibling);
	if(This->subSurface && siblingSurface)
		wl_subsurface_place_below (This->subSurface, siblingSurface);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SubSurfaceDelegate::setSync (wl_client* client, wl_resource* resource)
{
	SubSurfaceDelegate* This = cast<SubSurfaceDelegate> (resource);
	if(This->subSurface)
		wl_subsurface_set_sync (This->subSurface);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SubSurfaceDelegate::setDesync (wl_client* client, wl_resource* resource)
{
	SubSurfaceDelegate* This = cast<SubSurfaceDelegate> (resource);
	if(This->subSurface)
		wl_subsurface_set_desync (This->subSurface);
}
