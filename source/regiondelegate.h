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
// Filename    : regiondelegate.h
// Description : Wayland Region Delegate
//
//************************************************************************************************

#ifndef _regiondelegate_h
#define _regiondelegate_h

#include "wayland-server-delegate/waylandresource.h"

namespace WaylandServerDelegate {

//************************************************************************************************
// RegionDelegate
//************************************************************************************************

class RegionDelegate: public WaylandResource,
					  public wl_region_interface
{
public:
	RegionDelegate (wl_region* region);
	~RegionDelegate ();

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void onAdd (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height);
	static void onSubtract (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height);

private:
	wl_region* region;
};

} // namespace WaylandServerDelegate

#endif // _regiondelegate_h
