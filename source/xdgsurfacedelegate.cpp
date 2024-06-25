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
// Filename    : xdgsurfacedelegate.cpp
// Description : Wayland XDG Surface Delegate
//
//************************************************************************************************

#include "xdgsurfacedelegate.h"
#include "surfacedelegate.h"
#include "registrydelegate.h"
#include "waylandserver.h"

#include "wayland-server-delegate/iwaylandclientcontext.h"

using namespace WaylandServerDelegate;

//************************************************************************************************
// XdgSurfaceDelegate
//************************************************************************************************

XdgSurfaceDelegate::XdgSurfaceDelegate (XdgWindowManagerDelegate* windowManager, WaylandResource* waylandSurfaceResource)
: WaylandResource (&::xdg_surface_interface, static_cast<xdg_surface_interface*> (this)),
  windowManager (windowManager),
  surface (nullptr),
  popup (nullptr),
  toplevel (nullptr)
{
	destroy = onDestroy;
	get_toplevel = getToplevel;
	get_popup = getPopup;
	set_window_geometry = setWindowGeometry;
	ack_configure = ackConfigure;

	configure = onConfigure;

	IWaylandClientContext* context = WaylandServer::instance ().getContext ();
	xdg_wm_base* sessionWindowManager = context ? context->getWindowManager () : nullptr;
	if(sessionWindowManager == nullptr)
		return;
	
	wl_surface* waylandSurface = reinterpret_cast<wl_surface*> (waylandSurfaceResource->getProxy ());
	if(waylandSurface == nullptr)
		return;

	surface = xdg_wm_base_get_xdg_surface (sessionWindowManager, waylandSurface);
	if(surface)
		xdg_surface_add_listener (surface, this, this);

	setProxy (reinterpret_cast<wl_proxy*> (surface));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgSurfaceDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	WaylandResource::onDestroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgSurfaceDelegate::getToplevel (wl_client* client, wl_resource* resource, uint32_t id)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	XdgSurfaceDelegate* This = cast<XdgSurfaceDelegate> (resource);

	WaylandResource* implementation = new XdgToplevelDelegate (This->surface);
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgSurfaceDelegate::getPopup (wl_client* client, wl_resource* resource, uint32_t id, wl_resource* parent, wl_resource* positioner)
{
	WaylandServer::ClientConnection* connection = WaylandServer::instance ().findClientConnection (client);
	if(connection == nullptr)
	{
		wl_client_post_no_memory (client);
		return;
	}

	XdgSurfaceDelegate* This = cast<XdgSurfaceDelegate> (resource);
	xdg_surface* parentSurface = castProxy<xdg_surface> (parent);
	xdg_positioner* xdgPositioner = castProxy<xdg_positioner> (positioner);

	WaylandResource* implementation = new XdgPopupDelegate (This->surface, parentSurface, xdgPositioner);
	connection->addResource (implementation, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgSurfaceDelegate::setWindowGeometry (wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
	XdgSurfaceDelegate* This = cast<XdgSurfaceDelegate> (resource);
	if(This->surface)
		xdg_surface_set_window_geometry (This->surface, x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgSurfaceDelegate::ackConfigure (wl_client* client, wl_resource* resource, uint32_t serial)
{
	XdgSurfaceDelegate* This = cast<XdgSurfaceDelegate> (resource);
	if(This->surface)
		xdg_surface_ack_configure (This->surface, serial);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgSurfaceDelegate::onConfigure (void* data, xdg_surface* xdg_surface, uint32_t serial)
{
	XdgSurfaceDelegate* This = static_cast<XdgSurfaceDelegate*> (data);
	xdg_surface_send_configure (This->resourceHandle, serial);
	This->windowManager->sendPing ();
}

//************************************************************************************************
// XdgPopupDelegate
//************************************************************************************************

XdgPopupDelegate::XdgPopupDelegate (xdg_surface* surface, xdg_surface* parent, xdg_positioner* positioner)
: WaylandResource (&::xdg_popup_interface, static_cast<xdg_popup_interface*> (this)),
  popup (nullptr)
{
	destroy = onDestroy;
	grab = onGrab;
	reposition = onReposition;

	configure = onConfigure;
	popup_done = onPopupDone;
	repositioned = onRepositioned;

	popup = xdg_surface_get_popup (surface, parent, positioner);
	if(popup != nullptr)
		xdg_popup_add_listener (popup, this, this);

	setProxy (reinterpret_cast<wl_proxy*> (popup));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPopupDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	WaylandResource::onDestroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPopupDelegate::onGrab (wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial)
{
	XdgPopupDelegate* This = cast<XdgPopupDelegate> (resource);
	wl_seat* seatHandle = castProxy<wl_seat> (seat);
	if(This->popup && seatHandle)
		xdg_popup_grab (This->popup, seatHandle, serial);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPopupDelegate::onReposition (wl_client* client, wl_resource* resource, wl_resource* positioner, uint32_t token)
{
	XdgPopupDelegate* This = cast<XdgPopupDelegate> (resource);
	xdg_positioner* positionerHandle = castProxy<xdg_positioner> (positioner);
	if(This->popup && positionerHandle)
		xdg_popup_reposition (This->popup, positionerHandle, token);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPopupDelegate::onConfigure (void* data, xdg_popup* xdg_popup, int32_t x, int32_t y, int32_t width, int32_t height)
{
	XdgPopupDelegate* This = static_cast<XdgPopupDelegate*> (data);
	xdg_popup_send_configure (This->resourceHandle, x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPopupDelegate::onPopupDone (void* data, xdg_popup* xdg_popup)
{
	XdgPopupDelegate* This = static_cast<XdgPopupDelegate*> (data);
	xdg_popup_send_popup_done (This->resourceHandle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPopupDelegate::onRepositioned (void* data, xdg_popup* xdg_popup, uint32_t token)
{
	XdgPopupDelegate* This = static_cast<XdgPopupDelegate*> (data);
	xdg_popup_send_repositioned (This->resourceHandle, token);
}

//************************************************************************************************
// XdgToplevelDelegate
//************************************************************************************************

XdgToplevelDelegate::XdgToplevelDelegate (xdg_surface* surface)
: WaylandResource (&::xdg_toplevel_interface, static_cast<xdg_toplevel_interface*> (this)),
  toplevel (nullptr)
{
	destroy = onDestroy;
	set_title = setTitle;
	set_app_id = setApplicationID;
	show_window_menu = showWindowMenu;
	move = onMove;
	resize = onResize;
	set_max_size = setMaxSize;
	set_min_size = setMinSize;
	set_maximized = setMaximized;
	unset_maximized = unsetMaximized;
	set_fullscreen = setFullscreen;
	unset_fullscreen = unsetFullscreen;
	set_minimized = setMinimized;

	configure = onConfigure;
	close = onClose;
	configure_bounds = onConfigureBounds;

	#ifdef XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION
	wm_capabilities = onWindowManagerCapabilities;
	#endif

	toplevel = xdg_surface_get_toplevel (surface);
	if(toplevel != nullptr)
		xdg_toplevel_add_listener (toplevel, this, this);

	setProxy (reinterpret_cast<wl_proxy*> (toplevel));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	WaylandResource::onDestroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::setTitle (wl_client* client, wl_resource* resource, const char* title)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	if(This->toplevel)
		xdg_toplevel_set_title (This->toplevel, title);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::setApplicationID (wl_client* client, wl_resource* resource, const char* applicationId)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	if(This->toplevel)
		xdg_toplevel_set_app_id (This->toplevel, applicationId);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::showWindowMenu (wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial, int32_t x, int32_t y)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	wl_seat* seatHandle = castProxy<wl_seat> (seat);
	if(This->toplevel && seatHandle)
		xdg_toplevel_show_window_menu (This->toplevel, seatHandle, serial, x, y);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::onMove (wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	wl_seat* seatHandle = castProxy<wl_seat> (seat);
	if(This->toplevel && seatHandle)
		xdg_toplevel_move (This->toplevel, seatHandle, serial);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::onResize (wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial, uint32_t edges)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	wl_seat* seatHandle = castProxy<wl_seat> (seat);
	if(This->toplevel && seatHandle)
		xdg_toplevel_resize (This->toplevel, seatHandle, serial, edges);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::setMaxSize (wl_client* client, wl_resource* resource, int32_t width, int32_t height)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	if(This->toplevel)
		xdg_toplevel_set_max_size (This->toplevel, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::setMinSize (wl_client* client, wl_resource *resource, int32_t width, int32_t height)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	if(This->toplevel)
		xdg_toplevel_set_min_size (This->toplevel, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::setMaximized (wl_client* client, wl_resource *resource)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	if(This->toplevel)
		xdg_toplevel_set_maximized (This->toplevel);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::unsetMaximized (wl_client* client, wl_resource *resource)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	if(This->toplevel)
		xdg_toplevel_unset_maximized (This->toplevel);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::setFullscreen (wl_client* client, wl_resource *resource, wl_resource* output)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	wl_output* outputHandle = castProxy<wl_output> (output);
	if(This->toplevel && outputHandle)
		xdg_toplevel_set_fullscreen (This->toplevel, outputHandle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::unsetFullscreen (wl_client* client, wl_resource *resource)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	if(This->toplevel)
		xdg_toplevel_unset_fullscreen (This->toplevel);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::setMinimized (wl_client* client, wl_resource *resource)
{
	XdgToplevelDelegate* This = cast<XdgToplevelDelegate> (resource);
	if(This->toplevel)
		xdg_toplevel_unset_fullscreen (This->toplevel);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::onConfigure (void* data, xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, wl_array* states)
{
	XdgToplevelDelegate* This = static_cast<XdgToplevelDelegate*> (data);
	xdg_toplevel_send_configure (This->resourceHandle, width, height, states);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::onClose (void* data, xdg_toplevel* xdg_toplevel)
{
	XdgToplevelDelegate* This = static_cast<XdgToplevelDelegate*> (data);
	xdg_toplevel_send_close (This->resourceHandle);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::onConfigureBounds (void* data, xdg_toplevel* xdg_toplevel, int32_t width, int32_t height)
{
	XdgToplevelDelegate* This = static_cast<XdgToplevelDelegate*> (data);
	xdg_toplevel_send_configure_bounds (This->resourceHandle, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgToplevelDelegate::onWindowManagerCapabilities (void* data, xdg_toplevel* xdg_toplevel, wl_array* capabilities)
{
	#ifdef XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION
	XdgToplevelDelegate* This = static_cast<XdgToplevelDelegate*> (data);
	xdg_toplevel_send_wm_capabilities (This->resourceHandle, capabilities);
	#endif
}

//************************************************************************************************
// XdgPositionerDelegate
//************************************************************************************************

XdgPositionerDelegate::XdgPositionerDelegate ()
: WaylandResource (&::xdg_positioner_interface, static_cast<xdg_positioner_interface*> (this))
{
	destroy = onDestroy;
	set_size = setSize;
	set_anchor_rect = setAnchorRect;
	set_anchor = setAnchor;
	set_gravity = setGravity;
	set_constraint_adjustment = setConstraintAdjustment;
	set_offset = setOffset;
	set_reactive = setReactive;
	set_parent_size = setParentSize;
	set_parent_configure = setParentConfigure;

	IWaylandClientContext* context = WaylandServer::instance ().getContext ();
	xdg_wm_base* windowManager = context ? context->getWindowManager () : nullptr;
	if(windowManager == nullptr)
		return;

	positioner = xdg_wm_base_create_positioner (windowManager);

	setProxy (reinterpret_cast<wl_proxy*> (positioner));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::onDestroy (wl_client* client, wl_resource* resource)
{
	WaylandResource::onDestroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::setSize (wl_client* client, wl_resource* resource, int32_t width, int32_t height)
{
	XdgPositionerDelegate* This = cast<XdgPositionerDelegate> (resource);
	if(This->positioner)
		xdg_positioner_set_size (This->positioner, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::setAnchorRect (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
	XdgPositionerDelegate* This = cast<XdgPositionerDelegate> (resource);
	if(This->positioner)
		xdg_positioner_set_anchor_rect (This->positioner, x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::setAnchor (wl_client* client, wl_resource* resource, uint32_t anchor)
{
	XdgPositionerDelegate* This = cast<XdgPositionerDelegate> (resource);
	if(This->positioner)
		xdg_positioner_set_anchor (This->positioner, anchor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::setGravity (wl_client* client, wl_resource* resource, uint32_t gravity)
{
	XdgPositionerDelegate* This = cast<XdgPositionerDelegate> (resource);
	if(This->positioner)
		xdg_positioner_set_gravity (This->positioner, gravity);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::setConstraintAdjustment (wl_client* client, wl_resource* resource, uint32_t constraintAdjustment)
{
	XdgPositionerDelegate* This = cast<XdgPositionerDelegate> (resource);
	if(This->positioner)
		xdg_positioner_set_constraint_adjustment (This->positioner, constraintAdjustment);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::setOffset (wl_client* client, wl_resource* resource, int32_t x, int32_t y)
{
	XdgPositionerDelegate* This = cast<XdgPositionerDelegate> (resource);
	if(This->positioner)
		xdg_positioner_set_offset (This->positioner, x, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::setReactive (wl_client* client, wl_resource* resource)
{
	XdgPositionerDelegate* This = cast<XdgPositionerDelegate> (resource);
	if(This->positioner)
		xdg_positioner_set_reactive (This->positioner);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::setParentSize (wl_client* client, wl_resource* resource, int32_t parentWidth, int32_t parentHeight)
{
	XdgPositionerDelegate* This = cast<XdgPositionerDelegate> (resource);
	if(This->positioner)
		xdg_positioner_set_parent_size (This->positioner, parentWidth, parentHeight);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void XdgPositionerDelegate::setParentConfigure (wl_client* client, wl_resource* resource, uint32_t serial)
{
	XdgPositionerDelegate* This = cast<XdgPositionerDelegate> (resource);
	if(This->positioner)
		xdg_positioner_set_parent_configure (This->positioner, serial);
}
