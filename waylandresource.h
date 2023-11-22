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
// Filename    : waylandresource.h
// Description : Wayland Resource
//
//************************************************************************************************

#ifndef _waylandresource_h
#define _waylandresource_h

#include <wayland-server.h>

struct wl_proxy;

namespace WaylandServerDelegate {

//************************************************************************************************
// WaylandResource
//************************************************************************************************

class WaylandResource
{
public:
	WaylandResource (const wl_interface* waylandInterface, void* implementation);
	virtual ~WaylandResource ();

	const wl_interface* getWaylandInterface () const { return waylandInterface; }
	void* getImplementation () const { return implementation; }
	wl_resource* getResourceHandle () const { return resourceHandle; }
	void setResourceHandle (wl_resource* resource) { resourceHandle = resource; }
	wl_client* getClientHandle () const { return clientHandle; }
	void setClientHandle (wl_client* client) { clientHandle = client; }
	wl_proxy* getProxy () const { return proxy; }
	void setProxy (wl_proxy* object) { proxy = object; }

	static void onDestroy (wl_resource* resource);

	template<class T>
	static T* cast (wl_resource* resource)
	{
		return static_cast<T*> (static_cast<WaylandResource*> (wl_resource_get_user_data (resource)));
	}

	template<class T>
	static T* castProxy (wl_resource* resource)
	{
		WaylandResource* delegate = cast<WaylandResource> (resource);
		if(delegate == nullptr)
			return nullptr;
		return reinterpret_cast<T*> (delegate->getProxy ());
	}

protected:
	const wl_interface* waylandInterface;
	void* implementation;
	wl_resource* resourceHandle;
	wl_client* clientHandle;
	wl_proxy* proxy;
};

} // namespace WaylandServerDelegate

#endif // _waylandresource_h
