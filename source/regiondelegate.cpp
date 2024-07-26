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
// Filename    : regiondelegate.cpp
// Description : Wayland Region Delegate
//
//************************************************************************************************

#include "regiondelegate.h"
#include "waylandserver.h"

#include "wayland-server-delegate/iwaylandclientcontext.h"

#include <wayland-client.h>

using namespace WaylandServerDelegate;

//************************************************************************************************
// RegionDelegate
//************************************************************************************************

RegionDelegate::RegionDelegate (wl_region* region)
: WaylandResource (&::wl_region_interface, static_cast<wl_region_interface*> (this)),
  region (region)
{
	destroy = onDestroy;
	add = onAdd;
	subtract = onSubtract;

	setProxy (reinterpret_cast<wl_proxy*> (region));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

RegionDelegate::~RegionDelegate ()
{
	if(region)
		wl_region_destroy (region);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegionDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	WaylandResource::onDestroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegionDelegate::onAdd (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
	RegionDelegate* This = cast<RegionDelegate> (resource);
	if(This->region)
		wl_region_add (This->region, x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegionDelegate::onSubtract (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
	RegionDelegate* This = cast<RegionDelegate> (resource);
	if(This->region)
		wl_region_subtract (This->region, x, y, width, height);
}
