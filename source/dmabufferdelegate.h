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
// Filename    : dmabufferdelegate.h
// Description : Wayland DMA Buffer Delegate
//
//************************************************************************************************

#ifndef _dmabufferdelegate_h
#define _dmabufferdelegate_h

#include "wayland-server-delegate/waylandresource.h"

#include "linux-dmabuf-v1-server-protocol.h"
#include "linux-dmabuf-v1-client-protocol.h"

#ifndef ZWP_LINUX_DMABUF_V1_DESTROY_SINCE_VERSION
#include "linux-dmabuf-unstable-v1-server-protocol.h"
#include "linux-dmabuf-unstable-v1-client-protocol.h"
#endif

namespace WaylandServerDelegate {

//************************************************************************************************
// DmaBufferDelegate
//************************************************************************************************

class DmaBufferDelegate: public WaylandResource,
						 public zwp_linux_dmabuf_v1_interface
{
public:
	DmaBufferDelegate ();

	static const int kMinVersion = 4;
	static const int kMaxVersion = 5;

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void createParams (wl_client* client, wl_resource* resource, uint32_t id);
	static void getDefaultFeedback (wl_client* client, wl_resource* resource, uint32_t id);
	static void getSurfaceFeedback (wl_client* client, wl_resource* resource, uint32_t id, wl_resource* surface);

	// listener
	// not implemented: all events are deprecated in version 4

private:
	zwp_linux_dmabuf_v1* dmaBuf;
};

//************************************************************************************************
// DmaBufferParamsDelegate
//************************************************************************************************

class DmaBufferParamsDelegate: public WaylandResource,
							   public zwp_linux_buffer_params_v1_interface,
							   public zwp_linux_buffer_params_v1_listener
{
public:
	DmaBufferParamsDelegate (zwp_linux_buffer_params_v1* bufferParams);
	~DmaBufferParamsDelegate ();

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void onAdd (wl_client* client, wl_resource* resource, int32_t fd, uint32_t planeIndex, uint32_t offset, uint32_t stride, uint32_t high, uint32_t low);
	static void onCreate (wl_client* client, wl_resource* resource, int32_t width, int32_t height, uint32_t format, uint32_t flags);
	static void onCreateImmediate (wl_client* client, wl_resource* resource, uint32_t id, int32_t width, int32_t height, uint32_t format, uint32_t flags);

	// listener
	static void onCreated (void* data, zwp_linux_buffer_params_v1* bufferParams, wl_buffer* buffer);
	static void onFailed (void* data, zwp_linux_buffer_params_v1* bufferParams);

private:
	zwp_linux_buffer_params_v1* bufferParams;
};

//************************************************************************************************
// DmaBufferFeedbackDelegate
//************************************************************************************************

class DmaBufferFeedbackDelegate: public WaylandResource,
								 public zwp_linux_dmabuf_feedback_v1_interface,
								 public zwp_linux_dmabuf_feedback_v1_listener
{
public:
	DmaBufferFeedbackDelegate (zwp_linux_dmabuf_feedback_v1* feedback);
	~DmaBufferFeedbackDelegate ();

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);

	// listener
	static void onDone (void* data, zwp_linux_dmabuf_feedback_v1* feedback);
	static void onFormatTable (void* data, zwp_linux_dmabuf_feedback_v1* feedback, int32_t fd, uint32_t size);
	static void onMainDevice (void* data, zwp_linux_dmabuf_feedback_v1* feedback, wl_array* device);
	static void onTrancheDone (void* data, zwp_linux_dmabuf_feedback_v1* feedback);
	static void onTrancheTargetDevice (void* data, zwp_linux_dmabuf_feedback_v1* feedback, wl_array* device);
	static void onTrancheFormats (void* data, zwp_linux_dmabuf_feedback_v1* feedback, wl_array* indices);
	static void onTrancheFlags (void* data, zwp_linux_dmabuf_feedback_v1* feedback, uint32_t flags);

private:
	zwp_linux_dmabuf_feedback_v1* feedback;
};

} // namespace WaylandServerDelegate

#endif // _dmabufferdelegate_h
