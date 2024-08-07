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
// Filename    : waylandresource.cpp
// Description : Wayland Resource
//
//************************************************************************************************

#include "waylandserver.h"

#include "wayland-server-delegate/waylandresource.h"

#include <wayland-client.h>

using namespace WaylandServerDelegate;

//************************************************************************************************
// WaylandResource
//************************************************************************************************

WaylandResource::WaylandResource (const wl_interface* waylandInterface, void* implementation)
: resourceHandle (nullptr),
  waylandInterface (waylandInterface),
  clientHandle (nullptr),
  proxyWrapper (nullptr),
  originalProxy (nullptr),
  implementation (implementation)
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

WaylandResource::~WaylandResource ()
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandResource::wrapProxy ()
{
	if(proxyWrapper)
		wl_proxy_wrapper_destroy (proxyWrapper);
	proxyWrapper = nullptr;

	wl_event_queue* queue = WaylandServer::instance ().getQueue ();
	if(queue == nullptr)
		return;

	if(originalProxy)
		proxyWrapper = static_cast<wl_proxy*> (wl_proxy_create_wrapper (originalProxy));
	if(proxyWrapper)
		wl_proxy_set_queue (proxyWrapper, queue);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandResource::assignQueue ()
{
	wl_event_queue* queue = WaylandServer::instance ().getQueue ();
	if(originalProxy && queue)
		wl_proxy_set_queue (originalProxy, queue);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandResource::setProxy (wl_proxy* object)
{
	originalProxy = object;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandResource::onDestroy (wl_resource* resource)
{
	WaylandResource* This = static_cast<WaylandResource*> (wl_resource_get_user_data (resource));
	if(This)
	{
		if(This->proxyWrapper)
			wl_proxy_wrapper_destroy (This->proxyWrapper);
		This->proxyWrapper = nullptr;

		wl_resource_set_user_data (resource, nullptr);
		WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (This->clientHandle);
		if(connection)
			connection->removeResource (This);
	}
}
