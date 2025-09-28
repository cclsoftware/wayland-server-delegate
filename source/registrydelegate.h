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
// Filename    : registrydelegate.h
// Description : Registry Delegate
//
//************************************************************************************************

#ifndef _registrydelegate_h
#define _registrydelegate_h

#include "wayland-server-delegate/waylandresource.h"
#include "wayland-server-delegate/iwaylandclientcontext.h"

#include "xdg-shell-server-protocol.h"

#include <unordered_map>
#include <vector>

#ifdef WL_SURFACE_PREFERRED_BUFFER_TRANSFORM_SINCE_VERSION
#define WAYLAND_COMPOSITOR_VERSION WL_SURFACE_PREFERRED_BUFFER_TRANSFORM_SINCE_VERSION
#elif defined (WL_SURFACE_OFFSET_SINCE_VERSION)
#define WAYLAND_COMPOSITOR_VERSION WL_SURFACE_OFFSET_SINCE_VERSION
#else
#define WAYLAND_COMPOSITOR_VERSION WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION
#endif

#ifdef WL_KEYBOARD_KEY_STATE_REPEATED_SINCE_VERSION
#define WAYLAND_SEAT_VERSION WL_KEYBOARD_KEY_STATE_REPEATED_SINCE_VERSION
#elif defined (WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION)
#define WAYLAND_SEAT_VERSION WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION
#elif defined (WL_POINTER_AXIS_VALUE120_SINCE_VERSION)
#define WAYLAND_SEAT_VERSION WL_POINTER_AXIS_VALUE120_SINCE_VERSION
#else
#define WAYLAND_SEAT_VERSION 7
#endif

namespace WaylandServerDelegate {

//************************************************************************************************
// RegistryDelegate
//************************************************************************************************

class RegistryDelegate: public IContextListener
{
public:
	static RegistryDelegate& instance ();

	static void sendInvalidVersion (wl_client* client, const char* interfaceName, uint32_t minVersion);
	static void sendResourceNotFound (wl_client* client, wl_resource* resource);

	void startup ();
	void shutdown ();

	void bind (WaylandResource* implementation, wl_client* client, uint32_t version, uint32_t id);
	template<class T> static void bind (wl_client* client, void* data, uint32_t version, uint32_t id);

	// IContextListener
	void contextChanged (ChangeType type) override;

private:
	std::unordered_map<uint32_t, wl_global*> globals;
	std::vector<uint32_t> outputs;

	RegistryDelegate ();

	wl_global* registerGlobal (wl_proxy* proxy, const wl_interface* interface, int maxVersion, void* data, wl_global_bind_func_t bindFunction);
	wl_global* registerGlobal (uint32_t proxyId, const wl_interface* interface, int version, void* data, wl_global_bind_func_t bindFunction);
	void unregisterGlobal (wl_global* global);

	void updateSeatCapabilities ();
	void updateOutputs ();
};

//************************************************************************************************
// CompositorDelegate
//************************************************************************************************

class CompositorDelegate: public WaylandResource,
						  public wl_compositor_interface
{
public:
	CompositorDelegate ();

	static const int kMinVersion = WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION;
	static const int kMaxVersion = WAYLAND_COMPOSITOR_VERSION;

	// interface
	static void onCreateSurface (wl_client* client, wl_resource* resource, uint32_t id);
	static void onCreateRegion (wl_client* client, wl_resource* resource, uint32_t id);

private:
	wl_compositor* compositor;
};

//************************************************************************************************
// SubCompositorDelegate
//************************************************************************************************

class SubCompositorDelegate: public WaylandResource,
							 public wl_subcompositor_interface
{
public:
	SubCompositorDelegate ();

	static const int kMinVersion = 1;
	static const int kMaxVersion = 1;

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void getSubsurface (wl_client* client, wl_resource* resource, uint32_t id, wl_resource* surface, wl_resource* parent);

private:
	wl_subcompositor* subCompositor;
};

//************************************************************************************************
// SharedMemoryDelegate
//************************************************************************************************

class SharedMemoryDelegate: public WaylandResource,
							public wl_shm_interface
{
public:
	SharedMemoryDelegate ();

	static const int kMinVersion = 1;
	static const int kMaxVersion = 1;

	// interface
	static void createPool (wl_client* client, wl_resource* resource,  uint32_t id, int32_t fd, int32_t size);

private:
	wl_shm* shm;
};

//************************************************************************************************
// SeatDelegate
//************************************************************************************************

class SeatDelegate: public WaylandResource,
					public wl_seat_interface
{
public:
	SeatDelegate ();

	static const int kMinVersion = WL_POINTER_AXIS_DISCRETE_SINCE_VERSION;
	static const int kMaxVersion = WAYLAND_SEAT_VERSION;

	void sendCapabilities () const;
	void sendName () const;

	// WaylandResource
	void initialize () override;

	// interface
	static void onRelease (wl_client* client, wl_resource* resource);
	static void getPointer (wl_client* client, wl_resource* resource, uint32_t id);
	static void getKeyboard (wl_client* client, wl_resource* resource, uint32_t id);
	static void getTouch (wl_client* client, wl_resource* resource, uint32_t id);

private:
	wl_seat* seat;
};

//************************************************************************************************
// OutputDelegate
//************************************************************************************************

class OutputDelegate: public WaylandResource,
					  public wl_output_interface
{
public:
	OutputDelegate (int index = 0);

	static const int kMinVersion = 3;
	static const int kMaxVersion = 3;

	void sendProperties () const;

	// WaylandResource
	void initialize () override;

	// interface
	static void onRelease (wl_client* client, wl_resource* resource);

private:
	wl_output* outputHandle;
	int index;
};

//************************************************************************************************
// XdgWindowManagerDelegate
//************************************************************************************************

class XdgWindowManagerDelegate: public WaylandResource,
								public xdg_wm_base_interface
{
public:
	XdgWindowManagerDelegate ();

	static const int kMinVersion = 4;
	static const int kMaxVersion = 7;

	void sendPing ();

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void createPositioner (wl_client* client, wl_resource* resource, uint32_t id);
	static void getXdgSurface (wl_client* client, wl_resource* resource, uint32_t id, wl_resource* surface);
	static void onPong (wl_client* client, wl_resource* resource, uint32_t serial);

private:
	xdg_wm_base* windowManager;
};

} // namespace WaylandServerDelegate

#endif // _registrydelegate_h
