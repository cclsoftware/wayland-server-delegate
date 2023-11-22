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
// Filename    : registrydelegate.cpp
// Description : Registry Delegate
//
//************************************************************************************************

#include "registrydelegate.h"
#include "regiondelegate.h"
#include "sharedmemorypooldelegate.h"
#include "surfacedelegate.h"
#include "seatdelegates.h"
#include "xdgsurfacedelegate.h"
#include "waylandserver.h"

#include "wayland-server-delegate/iwaylandclientcontext.h"

#include <algorithm>

using namespace WaylandServerDelegate;

//************************************************************************************************
// RegistryDelegate
//************************************************************************************************

RegistryDelegate::RegistryDelegate ()
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

RegistryDelegate& RegistryDelegate::instance ()
{
	static RegistryDelegate theInstance;
	return theInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::startup ()
{
	const WaylandServer& server = WaylandServer::instance ();
	IWaylandClientContext* context = server.getContext ();

	registerGlobal (reinterpret_cast<wl_proxy*> (context->getCompositor ()), &wl_compositor_interface, WAYLAND_COMPOSITOR_VERSION, nullptr, bindWaylandCompositor);
	registerGlobal (reinterpret_cast<wl_proxy*> (context->getSubCompositor ()), &wl_subcompositor_interface, 1, nullptr, bindSubCompositor);
	registerGlobal (reinterpret_cast<wl_proxy*> (context->getSharedMemory ()), &wl_shm_interface, 1, nullptr, bindSharedMemory);
	registerGlobal (reinterpret_cast<wl_proxy*> (context->getWindowManager ()), &xdg_wm_base_interface, 5, nullptr, bindXdgWindowManager);
	registerGlobal (reinterpret_cast<wl_proxy*> (context->getSeat ()), &wl_seat_interface, 8, nullptr, bindSeat);
	
	updateOutputs ();

	context->addListener (this);

	// maybe add: wl_data_device_manager, zxdg_decoration_manager_v1, xdg_activation_v1, wp_viewporter, zwp_keyboard_shortcuts_inhibit_manager_v1, zwp_tablet_manager_v2, zwp_text_input_manager_v3
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::shutdown ()
{
	const WaylandServer& server = WaylandServer::instance ();
	IWaylandClientContext* context = server.getContext ();
	context->removeListener (this);

	for(const auto& global : globals)
	{
		if(global.second)
			wl_global_destroy (global.second);
	}
	globals.clear ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::contextChanged (ChangeType type)
{
	switch(type)
	{
	case kSeatCapabilitiesChanged :
		updateSeatCapabilities ();
		break;
	
	case kOutputsChanged :
		updateOutputs ();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::updateSeatCapabilities ()
{
	WaylandServer& server = WaylandServer::instance ();
	IWaylandClientContext* context = server.getContext ();

	for(auto& connection : server.getConnections ())
	{
		WaylandResource* resource = server.findClientResource (connection.clientHandle, reinterpret_cast<wl_proxy*> (context->getSeat ()));
		if(resource == nullptr)
			continue;

		static_cast<SeatDelegate*> (resource)->sendCapabilities ();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::updateOutputs ()
{
	WaylandServer& server = WaylandServer::instance ();
	IWaylandClientContext* context = server.getContext ();

	std::vector<uint32_t> newOutputs;
	std::vector<int> addedOutputs;
	for(int i = 0; i < context->countOutputs (); i++)
	{
		wl_proxy* proxy = reinterpret_cast<wl_proxy*> (context->getOutput (i).handle);
		uint32_t id = wl_proxy_get_id (proxy);
		newOutputs.push_back (id);
		if(std::find (outputs.begin (), outputs.end (), id) == outputs.end ())
			addedOutputs.push_back (i);
	}
	std::sort (newOutputs.begin (), newOutputs.end ());

	std::vector<uint32_t> removedOutputs;
	std::set_difference (outputs.begin (), outputs.end (), newOutputs.begin (), newOutputs.end (), std::inserter (removedOutputs, removedOutputs.begin ()));
	
	for(uint32_t removedOutput : removedOutputs)
	{
		auto global = globals.find (removedOutput);
		if(global == globals.end ())
			continue;

		unregisterGlobal (global->second);
	}

	for(int addedOutputIndex : addedOutputs)
	{
		wl_proxy* proxy = reinterpret_cast<wl_proxy*> (context->getOutput (addedOutputIndex).handle);
		uint32_t id = wl_proxy_get_id (proxy);
		registerGlobal (id, &wl_output_interface, 3, reinterpret_cast<void*> (int64_t(addedOutputIndex)), bindOutput);
	}

	outputs = newOutputs;

	for(int i = 0; i < context->countOutputs (); i++)
	{
		if(std::find (addedOutputs.begin (), addedOutputs.end (), i) != addedOutputs.end ())
			continue;

		for(auto& connection : server.getConnections ())
		{
			WaylandResource* resource = server.findClientResource (connection.clientHandle, reinterpret_cast<wl_proxy*> (context->getOutput (i).handle));
			if(resource == nullptr)
				continue;

			static_cast<OutputDelegate*> (resource)->sendProperties ();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

wl_global* RegistryDelegate::registerGlobal (wl_proxy* proxy, const wl_interface* interface, int version, void* data, wl_global_bind_func_t bindFunction)
{
	if(proxy == nullptr)
		return nullptr;

	return registerGlobal (wl_proxy_get_id (proxy), interface, version, data, bindFunction);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

wl_global* RegistryDelegate::registerGlobal (uint32_t proxyId, const wl_interface* interface, int version, void* data, wl_global_bind_func_t bindFunction)
{
	const WaylandServer& server = WaylandServer::instance ();
	wl_display* display = server.getDisplay ();

	wl_global* global = wl_global_create (display, interface, version, data, bindFunction);
	if(global == nullptr)
		return nullptr;
	
	globals.insert ({ proxyId, global });
	return global;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::unregisterGlobal (wl_global* handle)
{
	for(auto global = globals.begin (); global != globals.end (); global++)
	{
		if(global->second == handle)
		{
			wl_global_destroy (global->second);
			global = globals.erase (global);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::sendInvalidVersion (wl_client* client, const char* interfaceName, uint32_t minVersion)
{
	wl_client_post_implementation_error (client, "%s implementation requires binding version %d or later.\n", interfaceName, minVersion);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::sendResourceNotFound (wl_client* client, wl_resource* resource)
{
	wl_client_post_implementation_error (client, "Resource %p (%s) not found.\n", resource, resource ? wl_resource_get_class (resource) : "unknown");
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::bind (WaylandResource* implementation, wl_client* client, uint32_t version, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	connection->addResource (implementation, version, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::bindWaylandCompositor (wl_client* client, void* data, uint32_t version, uint32_t id)
{
	uint32_t selectedVersion = std::min<uint32_t> (CompositorDelegate::kMinVersion, version);

	if(selectedVersion < CompositorDelegate::kMinVersion)
	{
		sendInvalidVersion (client, wl_compositor_interface.name, CompositorDelegate::kMinVersion);
		return;
	}
	
	WaylandResource* implementation = new CompositorDelegate;
	instance ().bind (implementation, client, selectedVersion, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::bindSubCompositor (wl_client* client, void* data, uint32_t version, uint32_t id)
{
	uint32_t selectedVersion = std::min<uint32_t> (SubCompositorDelegate::kMinVersion, version);

	if(selectedVersion < SubCompositorDelegate::kMinVersion)
	{
		sendInvalidVersion (client, wl_subcompositor_interface.name, SubCompositorDelegate::kMinVersion);
		return;
	}
	
	WaylandResource* implementation = new SubCompositorDelegate;
	instance ().bind (implementation, client, selectedVersion, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::bindSharedMemory (wl_client* client, void* data, uint32_t version, uint32_t id)
{
	uint32_t selectedVersion = std::min<uint32_t> (SharedMemoryDelegate::kMinVersion, version);

	if(selectedVersion < SharedMemoryDelegate::kMinVersion)
	{
		sendInvalidVersion (client, wl_shm_interface.name, SharedMemoryDelegate::kMinVersion);
		return;
	}
	
	WaylandResource* implementation = new SharedMemoryDelegate;
	instance ().bind (implementation, client, selectedVersion, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::bindSeat (wl_client* client, void* data, uint32_t version, uint32_t id)
{
	uint32_t selectedVersion = std::min<uint32_t> (SeatDelegate::kMinVersion, version);

	if(selectedVersion < SeatDelegate::kMinVersion)
	{
		sendInvalidVersion (client, wl_seat_interface.name, SeatDelegate::kMinVersion);
		return;
	}
	
	SeatDelegate* implementation = new SeatDelegate;
	instance ().bind (implementation, client, selectedVersion, id);
	implementation->sendCapabilities ();
	implementation->sendName ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::bindOutput (wl_client* client, void* data, uint32_t version, uint32_t id)
{
	uint32_t selectedVersion = std::min<uint32_t> (3, version);

	if(selectedVersion < 3)
	{
		sendInvalidVersion (client, wl_output_interface.name, 3);
		return;
	}
	
	int64_t index = reinterpret_cast<int64_t> (data);
	OutputDelegate* implementation = new OutputDelegate (int (index));
	instance ().bind (implementation, client, selectedVersion, id);
	implementation->sendProperties ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void RegistryDelegate::bindXdgWindowManager (wl_client* client, void* data, uint32_t version, uint32_t id)
{
	uint32_t selectedVersion = std::min<uint32_t> (XdgWindowManagerDelegate::kMinVersion, version);

	if(selectedVersion < XdgWindowManagerDelegate::kMinVersion)
	{
		sendInvalidVersion (client, xdg_wm_base_interface.name, XdgWindowManagerDelegate::kMinVersion);
		return;
	}
	
	WaylandResource* implementation = new XdgWindowManagerDelegate;
	instance ().bind (implementation, client, selectedVersion, id);
}

//************************************************************************************************
// CompositorDelegate
//************************************************************************************************

CompositorDelegate::CompositorDelegate ()
: WaylandResource (&::wl_compositor_interface, static_cast<wl_compositor_interface*> (this))
{
	create_surface = onCreateSurface;
	create_region = onCreateRegion;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CompositorDelegate::onCreateSurface (wl_client* client, wl_resource* resource, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	WaylandResource* implementation = new SurfaceDelegate;
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CompositorDelegate::onCreateRegion (wl_client* client, wl_resource* resource, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	WaylandResource* implementation = new RegionDelegate;
	connection->addResource (implementation, id);
}

//************************************************************************************************
// SubCompositorDelegate
//************************************************************************************************

SubCompositorDelegate::SubCompositorDelegate ()
: WaylandResource (&::wl_subcompositor_interface, static_cast<wl_subcompositor_interface*> (this))
{
	destroy = onDestroy;
	get_subsurface = getSubsurface;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SubCompositorDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	WaylandResource::onDestroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SubCompositorDelegate::getSubsurface (wl_client* client, wl_resource* resource, uint32_t id, wl_resource* surface, wl_resource* parent)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	WaylandResource* waylandSurfaceResource = connection->findResource (surface);
	if(waylandSurfaceResource == nullptr)
	{
		RegistryDelegate::sendResourceNotFound (client, surface);
		return;
	}

	WaylandResource* parentSurfaceResource = connection->findResource (parent);
	if(parentSurfaceResource == nullptr)
	{
		RegistryDelegate::sendResourceNotFound (client, parent);
		return;
	}

	WaylandResource* implementation = new SubSurfaceDelegate (waylandSurfaceResource, parentSurfaceResource);
	connection->addResource (implementation, id);
}

//************************************************************************************************
// SharedMemoryDelegate
//************************************************************************************************

SharedMemoryDelegate::SharedMemoryDelegate ()
: WaylandResource (&::wl_shm_interface, static_cast<wl_shm_interface*> (this))
{
	create_pool = createPool;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SharedMemoryDelegate::createPool (wl_client* client, wl_resource* resource,  uint32_t id, int32_t fd, int32_t size)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	WaylandResource* implementation = new SharedMemoryPoolDelegate (fd, size);
	connection->addResource (implementation, id);
}

//************************************************************************************************
// SeatDelegate
//************************************************************************************************

SeatDelegate::SeatDelegate ()
: WaylandResource (&::wl_seat_interface, static_cast<wl_seat_interface*> (this)),
  seat (nullptr)
{
	wl_seat_interface::release = onRelease;
	get_pointer = getPointer;
	get_keyboard = getKeyboard;
	get_touch = getTouch;

	IWaylandClientContext* context = WaylandServer::instance ().getContext ();
	seat = context->getSeat ();

	setProxy (reinterpret_cast<wl_proxy*> (seat));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SeatDelegate::sendCapabilities () const
{
	IWaylandClientContext* context = WaylandServer::instance ().getContext ();
	wl_seat_send_capabilities (getResourceHandle (), context->getSeatCapabilities ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SeatDelegate::sendName () const
{
	IWaylandClientContext* context = WaylandServer::instance ().getContext ();
	wl_seat_send_name (getResourceHandle (), context->getSeatName ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SeatDelegate::onRelease (wl_client* client, wl_resource* resource)
{
	WaylandResource::onDestroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SeatDelegate::getPointer (wl_client* client, wl_resource* resource, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	SeatDelegate* This = cast<SeatDelegate> (resource);
	WaylandResource* implementation = new PointerDelegate (This->seat);
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SeatDelegate::getKeyboard (wl_client* client, wl_resource* resource, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	SeatDelegate* This = cast<SeatDelegate> (resource);
	WaylandResource* implementation = new KeyboardDelegate (This->seat);
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SeatDelegate::getTouch (wl_client* client, wl_resource* resource, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	SeatDelegate* This = cast<SeatDelegate> (resource);
	WaylandResource* implementation = new TouchDelegate (This->seat);
	connection->addResource (implementation, id);
}

//************************************************************************************************
// OutputDelegate
//************************************************************************************************

OutputDelegate::OutputDelegate (int index)
: WaylandResource (&::wl_output_interface, static_cast<wl_output_interface*> (this)),
  index (index),
  outputHandle (nullptr)
{
	wl_output_interface::release = onRelease;

	IWaylandClientContext* context = WaylandServer::instance ().getContext ();
	const WaylandOutput& output = context->getOutput (index);
	outputHandle = output.handle;

	setProxy (reinterpret_cast<wl_proxy*> (outputHandle));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void OutputDelegate::sendProperties () const
{
	IWaylandClientContext* context = WaylandServer::instance ().getContext ();
	const WaylandOutput& output = context->getOutput (index);

	wl_output_send_geometry (getResourceHandle (), output.x, output.y, output.physicalWidth, output.physicalHeight, 
		output.subPixelOrientation, output.manufacturer, output.model, output.transformType);
	wl_output_send_mode (getResourceHandle (), WL_OUTPUT_MODE_CURRENT, output.width, output.height, output.refreshRate);
	wl_output_send_scale (getResourceHandle (), output.scaleFactor);
	wl_output_send_done (getResourceHandle ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void OutputDelegate::onRelease (wl_client* client, wl_resource* resource)
{ 
	WaylandResource::onDestroy (resource);
}

//************************************************************************************************
// XdgWindowManagerDelegate
//************************************************************************************************

XdgWindowManagerDelegate::XdgWindowManagerDelegate ()
: WaylandResource (&::xdg_wm_base_interface, static_cast<xdg_wm_base_interface*> (this))
{
	destroy = onDestroy;
	create_positioner = createPositioner;
	get_xdg_surface = getXdgSurface;
	pong = onPong;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgWindowManagerDelegate::sendPing ()
{
	wl_display* display = WaylandServer::instance ().getDisplay ();
	xdg_wm_base_send_ping (resourceHandle, wl_display_next_serial (display));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgWindowManagerDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	WaylandResource::onDestroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgWindowManagerDelegate::createPositioner (wl_client* client, wl_resource* resource, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	WaylandResource* implementation = new XdgPositionerDelegate;
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgWindowManagerDelegate::getXdgSurface (wl_client* client, wl_resource* resource, uint32_t id, wl_resource* surface)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	WaylandResource* waylandSurfaceResource = connection->findResource (surface);
	if(waylandSurfaceResource == nullptr)
	{
		RegistryDelegate::sendResourceNotFound (client, surface);
		return;
	}

	SurfaceDelegate* surfaceDelegate = cast<SurfaceDelegate> (waylandSurfaceResource->getResourceHandle ());

	XdgWindowManagerDelegate* This = cast<XdgWindowManagerDelegate> (resource);
	WaylandResource* implementation = new XdgSurfaceDelegate (This, surfaceDelegate);
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgWindowManagerDelegate::onPong (wl_client* client, wl_resource* resource, uint32_t serial)
{
	// UNSURE do we need to handle this?
}
