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
// Filename    : xdgsurfacedelegate.h
// Description : Wayland XDG Surface Delegate
//
//************************************************************************************************

#ifndef _xdgsurfacedelegate_h
#define _xdgsurfacedelegate_h

#include "wayland-server-delegate/waylandresource.h"

#include "xdg-shell-server-protocol.h"
#include "xdg-shell-client-protocol.h"

namespace WaylandServerDelegate {

class XdgPopupDelegate;
class XdgToplevelDelegate;
class XdgWindowManagerDelegate;

//************************************************************************************************
// XdgSurfaceDelegate
//************************************************************************************************

class XdgSurfaceDelegate: public WaylandResource,
						  public xdg_surface_interface,
						  public xdg_surface_listener
{
public:
	XdgSurfaceDelegate (XdgWindowManagerDelegate* windowManager, WaylandResource* waylandSurface);

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void getToplevel (wl_client* client, wl_resource* resource, uint32_t id);
	static void getPopup (wl_client* client, wl_resource* resource, uint32_t id, wl_resource* parent, wl_resource* positioner);
	static void setWindowGeometry (wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height);
	static void ackConfigure (wl_client* client, wl_resource* resource, uint32_t serial);

	// listener
	static void onConfigure (void* data, xdg_surface* xdg_surface, uint32_t serial);

private:
	xdg_surface* surface;
	XdgPopupDelegate* popup;
	XdgToplevelDelegate* toplevel;
	XdgWindowManagerDelegate* windowManager;
};

//************************************************************************************************
// XdgPopupDelegate
//************************************************************************************************

class XdgPopupDelegate: public WaylandResource,
						public xdg_popup_interface,
						public xdg_popup_listener
{
public:
	XdgPopupDelegate (xdg_surface* surface, xdg_surface* parent, xdg_positioner* positioner);

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void onGrab (wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial);
	static void onReposition (wl_client* client, wl_resource* resource, wl_resource* positioner, uint32_t token);

	// listener
	static void onConfigure (void* data, xdg_popup* xdg_popup, int32_t x, int32_t y, int32_t width, int32_t height);
	static void onPopupDone (void* data, xdg_popup* xdg_popup);
	static void onRepositioned (void* data, xdg_popup* xdg_popup, uint32_t token);

private:
	xdg_popup* popup;
};

//************************************************************************************************
// XdgToplevelDelegate
//************************************************************************************************

class XdgToplevelDelegate: public WaylandResource,
						   public xdg_toplevel_interface,
						   public xdg_toplevel_listener
{
public:
	XdgToplevelDelegate (xdg_surface* surface);

	// interface
	static void onDestroy (wl_client* client, wl_resource* resource);
	static void setTitle (wl_client* client, wl_resource* resource, const char* title);
	static void setApplicationID (wl_client* client, wl_resource* resource, const char* applicationId);
	static void showWindowMenu (wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial, int32_t x, int32_t y);
	static void onMove (wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial);
	static void onResize (wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial, uint32_t edges);
	static void setMaxSize (wl_client* client, wl_resource* resource, int32_t width, int32_t height);
	static void setMinSize (wl_client* client, wl_resource *resource, int32_t width, int32_t height);
	static void setMaximized (wl_client* client, wl_resource *resource);
	static void unsetMaximized (wl_client* client, wl_resource *resource);
	static void setFullscreen (wl_client* client, wl_resource *resource, wl_resource* output);
	static void unsetFullscreen (wl_client* client, wl_resource *resource);
	static void setMinimized (wl_client* client, wl_resource *resource);

	// listener
	static void onConfigure (void* data, xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, wl_array* states);
	static void onClose (void* data, xdg_toplevel* xdg_toplevel);
	static void onConfigureBounds (void* data, xdg_toplevel* xdg_toplevel, int32_t width, int32_t height);
	static void onWindowManagerCapabilities (void* data, xdg_toplevel* xdg_toplevel, wl_array* capabilities);

private:
	xdg_toplevel* toplevel;
};

//************************************************************************************************
// XdgPositionerDelegate
//************************************************************************************************

class XdgPositionerDelegate: public WaylandResource,
							 public xdg_positioner_interface
{
public:
	XdgPositionerDelegate ();

	static void onDestroy (wl_client* client, wl_resource* resource);
	static void setSize (wl_client* client, wl_resource* resource, int32_t width, int32_t height);
	static void setAnchorRect (wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height);
	static void setAnchor (wl_client* client, wl_resource* resource, uint32_t anchor);
	static void setGravity (wl_client* client, wl_resource* resource, uint32_t gravity);
	static void setConstraintAdjustment (wl_client* client, wl_resource* resource, uint32_t constraintAdjustment);
	static void setOffset (wl_client* client, wl_resource* resource, int32_t x, int32_t y);
	static void setReactive (wl_client* client, wl_resource* resource);
	static void setParentSize (wl_client* client, wl_resource* resource, int32_t parentWidth, int32_t parentHeight);
	static void setParentConfigure (wl_client* client, wl_resource* resource, uint32_t serial);

private:
	xdg_positioner* positioner;
};

} // namespace WaylandServerDelegate

#endif // _xdgsurfacedelegate_h
