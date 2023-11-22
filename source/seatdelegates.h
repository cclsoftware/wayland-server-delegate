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
// Filename    : seatdelegates.h
// Description : Wayland Seat Delegates
//
//************************************************************************************************

#ifndef _seatdelegates_h
#define _seatdelegates_h

#include "wayland-server-delegate/waylandresource.h"

#include <wayland-client.h>

namespace WaylandServerDelegate {

//************************************************************************************************
// PointerDelegate
//************************************************************************************************

class PointerDelegate: public WaylandResource,
					   public wl_pointer_interface,
					   public wl_pointer_listener
{
public:
	PointerDelegate (wl_seat* seat);
	~PointerDelegate ();

	// interface
	static void onRelease (wl_client* client, wl_resource* resource);
	static void setCursor (wl_client* client, wl_resource *resource, uint32_t serial, wl_resource* surface, int32_t x, int32_t y);

	// listener
	static void onPointerEnter (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t x, wl_fixed_t y);
	static void onPointerLeave (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface);
	static void onPointerMotion (void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y);
	static void onPointerButton (void* data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	static void onPointerAxis (void* data, wl_pointer* wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
	static void onPointerAxisSource (void* data, wl_pointer* pointer, uint32_t axisSource);
	static void onPointerAxisStop (void* data, wl_pointer* pointer, uint32_t time, uint32_t axis);
	static void onPointerAxisDiscrete (void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete);
	static void onPointerAxis120 (void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete);
	static void onPointerFrame (void* data, wl_pointer* pointer);

private:
	wl_pointer* pointer;
};

//************************************************************************************************
// KeyboardDelegate
//************************************************************************************************

class KeyboardDelegate: public WaylandResource,
						public wl_keyboard_interface,
						public wl_keyboard_listener
{
public:
	KeyboardDelegate (wl_seat* seat);
	~KeyboardDelegate ();

	// interface
	static void onRelease (wl_client* client, wl_resource* resource);

	// listener
	static void onKeymapReceived (void* data, wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size);
	static void onKeyboardFocusEnter (void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface, struct wl_array* keys);
	static void onKeyboardFocusLeave (void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface);
	static void onKey (void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
	static void onModifiers (void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t depressedModifiers, uint32_t latchedModifiers, uint32_t lockedModifiers, uint32_t group);
	static void onRepeatInfo (void* data, wl_keyboard* keyboard, int32_t rate, int32_t delay);
	
private:
	wl_keyboard* keyboard;
};

//************************************************************************************************
// TouchDelegate
//************************************************************************************************

class TouchDelegate: public WaylandResource,
					 public wl_touch_interface,
					 public wl_touch_listener
{
public:
	TouchDelegate (wl_seat* seat);
	~TouchDelegate ();

	// interface
	static void onRelease (wl_client* client, wl_resource* resource);

	// listener
	static void onTouchDown (void* data, wl_touch* touch, uint32_t serial, uint32_t time, wl_surface* surface, int32_t id, wl_fixed_t x, wl_fixed_t y);
	static void onTouchUp (void* data, wl_touch* touch, uint32_t serial, uint32_t time, int32_t id);
	static void onTouchMotion (void* data, wl_touch* touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y);
	static void onTouchCancel (void* data, wl_touch* touch);
	static void onTouchShape (void* data, wl_touch* touch, int32_t id, wl_fixed_t major, wl_fixed_t minor);
	static void onTouchOrientation (void* data, wl_touch* touch, int32_t id, wl_fixed_t orientation);
	static void onTouchFrame (void* data, wl_touch* touch);

private:
	wl_touch* touch;
};

} // namespace WaylandServerDelegate

#endif // _seatdelegates_h
