//************************************************************************************************
//
// Wayland Server Delegate
//
// Copyright (c) 2024 CCL Software Licensing GmbH. All Rights Reserved.
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
// Filename    : dmabufferdelegate.cpp
// Description : Wayland DMA Buffer Delegate
//
//************************************************************************************************

#include "dmabufferdelegate.h"
#include "bufferdelegate.h"
#include "waylandserver.h"

#include "wayland-server-delegate/iwaylandclientcontext.h"

#include <unistd.h>

using namespace WaylandServerDelegate;

//************************************************************************************************
// DmaBufferDelegate
//************************************************************************************************

DmaBufferDelegate::DmaBufferDelegate ()
: WaylandResource (&::zwp_linux_dmabuf_v1_interface, static_cast<zwp_linux_dmabuf_v1_interface*> (this)),
  dmaBuf (nullptr)
{
	destroy = onDestroy;
	create_params = createParams;
	get_default_feedback = getDefaultFeedback;
	get_surface_feedback = getSurfaceFeedback;

	IWaylandClientContext* context = WaylandServer::instance ().getContext ();
	setProxy (reinterpret_cast<wl_proxy*> (context ? context->getDmaBuffer () : nullptr));
	wrapProxy ();
	dmaBuf = reinterpret_cast<zwp_linux_dmabuf_v1*> (proxyWrapper);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	wl_resource_destroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferDelegate::createParams (wl_client* client, wl_resource* resource, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	DmaBufferDelegate* This = cast<DmaBufferDelegate> (resource);
	zwp_linux_buffer_params_v1* bufferParams = zwp_linux_dmabuf_v1_create_params (This->dmaBuf);
	WaylandResource* implementation = new DmaBufferParamsDelegate (bufferParams);
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferDelegate::getDefaultFeedback (wl_client* client, wl_resource* resource, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	DmaBufferDelegate* This = cast<DmaBufferDelegate> (resource);
	zwp_linux_dmabuf_feedback_v1* feedback = zwp_linux_dmabuf_v1_get_default_feedback (This->dmaBuf);
	WaylandResource* implementation = new DmaBufferFeedbackDelegate (feedback);
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferDelegate::getSurfaceFeedback (wl_client* client, wl_resource* resource, uint32_t id, wl_resource* surface)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	DmaBufferDelegate* This = cast<DmaBufferDelegate> (resource);
	wl_surface* waylandSurface = castProxy<wl_surface> (surface);
	zwp_linux_dmabuf_feedback_v1* feedback = zwp_linux_dmabuf_v1_get_surface_feedback (This->dmaBuf, waylandSurface);
	WaylandResource* implementation = new DmaBufferFeedbackDelegate (feedback);
	connection->addResource (implementation, id);
}

//************************************************************************************************
// DmaBufferParamsDelegate
//************************************************************************************************

DmaBufferParamsDelegate::DmaBufferParamsDelegate (zwp_linux_buffer_params_v1* bufferParams)
: WaylandResource (&::zwp_linux_buffer_params_v1_interface, static_cast<zwp_linux_buffer_params_v1_interface*> (this)),
  bufferParams (bufferParams)
{
	destroy = onDestroy;
	add = onAdd;
	create = onCreate;
	create_immed = onCreateImmediate;

	created = onCreated;
	failed = onFailed;

	if(bufferParams)
		zwp_linux_buffer_params_v1_add_listener (bufferParams, this, this);
	setProxy (reinterpret_cast<wl_proxy*> (bufferParams));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

DmaBufferParamsDelegate::~DmaBufferParamsDelegate ()
{
	if(bufferParams)
		zwp_linux_buffer_params_v1_destroy (bufferParams);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferParamsDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	wl_resource_destroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferParamsDelegate::onAdd (wl_client* client, wl_resource* resource, int32_t fd, uint32_t planeIndex, uint32_t offset, uint32_t stride, uint32_t high, uint32_t low)
{
	DmaBufferParamsDelegate* This = cast<DmaBufferParamsDelegate> (resource);
	if(This->bufferParams)
		zwp_linux_buffer_params_v1_add (This->bufferParams, fd, planeIndex, offset, stride, high, low);
	::close (fd);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferParamsDelegate::onCreate (wl_client* client, wl_resource* resource, int32_t width, int32_t height, uint32_t format, uint32_t flags)
{
	DmaBufferParamsDelegate* This = cast<DmaBufferParamsDelegate> (resource);
	if(This->bufferParams)
		zwp_linux_buffer_params_v1_create (This->bufferParams, width, height, format, flags);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferParamsDelegate::onCreateImmediate (wl_client* client, wl_resource* resource, uint32_t id, int32_t width, int32_t height, uint32_t format, uint32_t flags)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	DmaBufferParamsDelegate* This = cast<DmaBufferParamsDelegate> (resource);
	wl_buffer* buffer = zwp_linux_buffer_params_v1_create_immed (This->bufferParams, width, height, format, flags);
	WaylandResource* implementation = new BufferDelegate (buffer);
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferParamsDelegate::onCreated (void* data, zwp_linux_buffer_params_v1* bufferParams, wl_buffer* buffer)
{
	DmaBufferParamsDelegate* This = static_cast<DmaBufferParamsDelegate*> (data);
	wl_client* client = This->getClientHandle ();
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	WaylandResource* implementation = new BufferDelegate (buffer);
	connection->addResource (implementation, 0);
	zwp_linux_buffer_params_v1_send_created (This->resourceHandle, implementation->getResourceHandle ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferParamsDelegate::onFailed (void* data, zwp_linux_buffer_params_v1* bufferParams)
{
	DmaBufferParamsDelegate* This = static_cast<DmaBufferParamsDelegate*> (data);
	zwp_linux_buffer_params_v1_send_failed (This->resourceHandle);
}

//************************************************************************************************
// DmaBufferFeedbackDelegate
//************************************************************************************************

DmaBufferFeedbackDelegate::DmaBufferFeedbackDelegate (zwp_linux_dmabuf_feedback_v1* feedback)
: WaylandResource (&::zwp_linux_dmabuf_feedback_v1_interface, static_cast<zwp_linux_dmabuf_feedback_v1_interface*> (this)),
  feedback (feedback)
{
	destroy = onDestroy;

	done = onDone;
	format_table = onFormatTable;
	main_device = onMainDevice;
	tranche_done = onTrancheDone;
	tranche_target_device = onTrancheTargetDevice;
	tranche_formats = onTrancheFormats;
	tranche_flags = onTrancheFlags;

	if(feedback)
		zwp_linux_dmabuf_feedback_v1_add_listener (feedback, this, this);
	setProxy (reinterpret_cast<wl_proxy*> (feedback));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

DmaBufferFeedbackDelegate::~DmaBufferFeedbackDelegate ()
{
	if(feedback)
		zwp_linux_dmabuf_feedback_v1_destroy (feedback);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferFeedbackDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	wl_resource_destroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferFeedbackDelegate::onDone (void* data, zwp_linux_dmabuf_feedback_v1* feedback)
{
	DmaBufferFeedbackDelegate* This = static_cast<DmaBufferFeedbackDelegate*> (data);
	zwp_linux_dmabuf_feedback_v1_send_done (This->resourceHandle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferFeedbackDelegate::onFormatTable (void* data, zwp_linux_dmabuf_feedback_v1* feedback, int32_t fd, uint32_t size)
{
	DmaBufferFeedbackDelegate* This = static_cast<DmaBufferFeedbackDelegate*> (data);
	zwp_linux_dmabuf_feedback_v1_send_format_table (This->resourceHandle, fd, size);
	::close (fd);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferFeedbackDelegate::onMainDevice (void* data, zwp_linux_dmabuf_feedback_v1* feedback, wl_array* device)
{
	DmaBufferFeedbackDelegate* This = static_cast<DmaBufferFeedbackDelegate*> (data);
	zwp_linux_dmabuf_feedback_v1_send_main_device (This->resourceHandle, device);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferFeedbackDelegate::onTrancheDone (void* data, zwp_linux_dmabuf_feedback_v1* feedback)
{
	DmaBufferFeedbackDelegate* This = static_cast<DmaBufferFeedbackDelegate*> (data);
	zwp_linux_dmabuf_feedback_v1_send_tranche_done (This->resourceHandle);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferFeedbackDelegate::onTrancheTargetDevice (void* data, zwp_linux_dmabuf_feedback_v1* feedback, wl_array* device)
{
	DmaBufferFeedbackDelegate* This = static_cast<DmaBufferFeedbackDelegate*> (data);
	zwp_linux_dmabuf_feedback_v1_send_tranche_target_device (This->resourceHandle, device);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferFeedbackDelegate::onTrancheFormats (void* data, zwp_linux_dmabuf_feedback_v1* feedback, wl_array* indices)
{
	DmaBufferFeedbackDelegate* This = static_cast<DmaBufferFeedbackDelegate*> (data);
	zwp_linux_dmabuf_feedback_v1_send_tranche_formats (This->resourceHandle, indices);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void DmaBufferFeedbackDelegate::onTrancheFlags (void* data, zwp_linux_dmabuf_feedback_v1* feedback, uint32_t flags)
{
	DmaBufferFeedbackDelegate* This = static_cast<DmaBufferFeedbackDelegate*> (data);
	zwp_linux_dmabuf_feedback_v1_send_tranche_flags (This->resourceHandle, flags);	
}
