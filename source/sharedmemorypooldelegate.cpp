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
// Filename    : sharedmemorypooldelegate.cpp
// Description : Wayland Shared Memory Pool Delegate
//
//************************************************************************************************

#include "sharedmemorypooldelegate.h"
#include "bufferdelegate.h"
#include "waylandserver.h"

#include "wayland-server-delegate/iwaylandclientcontext.h"

using namespace WaylandServerDelegate;

//************************************************************************************************
// SharedMemoryPoolDelegate
//************************************************************************************************

SharedMemoryPoolDelegate::SharedMemoryPoolDelegate (wl_shm_pool* pool)
: WaylandResource (&::wl_shm_pool_interface, static_cast<wl_shm_pool_interface*> (this)),
  pool (pool)
{
	create_buffer = createBuffer;
	destroy = onDestroy;
	resize = onResize;

	setProxy (reinterpret_cast<wl_proxy*> (pool));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SharedMemoryPoolDelegate::~SharedMemoryPoolDelegate ()
{
	if(pool)
		wl_shm_pool_destroy (pool);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SharedMemoryPoolDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	WaylandResource::onDestroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SharedMemoryPoolDelegate::createBuffer (wl_client* client, wl_resource* poolResource, uint32_t id, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format)
{
	SharedMemoryPoolDelegate* This = cast<SharedMemoryPoolDelegate> (poolResource);
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	wl_buffer* buffer = wl_shm_pool_create_buffer (This->pool, offset, width, height, stride, format);
	BufferDelegate* delegate = new BufferDelegate (buffer);
	connection->addResource (delegate, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SharedMemoryPoolDelegate::onResize (wl_client* client, wl_resource* resource, int32_t size)
{
	SharedMemoryPoolDelegate* This = cast<SharedMemoryPoolDelegate> (resource);
	if(This->pool)
		wl_shm_pool_resize (This->pool, size);
}
