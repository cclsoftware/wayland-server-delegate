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
// Filename    : seatdelegates.cpp
// Description : Wayland Seat Delegates
//
//************************************************************************************************

#define DEBUG_LOG 0

#include "seatdelegates.h"
#include "surfacedelegate.h"
#include "waylandserver.h"

#include <unistd.h>

using namespace WaylandServerDelegate;

//************************************************************************************************
// PointerDelegate
//************************************************************************************************

PointerDelegate::PointerDelegate (wl_seat* seat)
: WaylandResource (&::wl_pointer_interface, static_cast<wl_pointer_interface*> (this)),
  pointer (nullptr)
{
	enter = onPointerEnter;
	leave = onPointerLeave;
	motion = onPointerMotion;
	button = onPointerButton;
	axis = onPointerAxis;
	axis_source = onPointerAxisSource;
	axis_stop = onPointerAxisStop;
	axis_discrete = onPointerAxisDiscrete;
	#ifdef WL_POINTER_AXIS_VALUE120_SINCE_VERSION
	axis_value120 = onPointerAxis120;
	#endif
	#ifdef WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION
	axis_relative_direction = onPointerAxisRelativeDirection;
	#endif
	frame = onPointerFrame;

	set_cursor = setCursor;
	wl_pointer_interface::release = onRelease;

	pointer = wl_seat_get_pointer (seat);
	if(pointer)
		wl_pointer_add_listener (pointer, this, this);

	setProxy (reinterpret_cast<wl_proxy*> (pointer));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

PointerDelegate::~PointerDelegate ()
{
	if(pointer)
		wl_pointer_release (pointer);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onRelease (wl_client* client, wl_resource* resource)
{
	wl_resource_destroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::setCursor (wl_client* client, wl_resource *resource, uint32_t serial, wl_resource* surface, int32_t x, int32_t y)
{
	PointerDelegate* This = cast<PointerDelegate> (resource);
	wl_surface* waylandSurface = castProxy<wl_surface> (surface);
	if(This->pointer && waylandSurface)
		wl_pointer_set_cursor (This->pointer, serial, waylandSurface, x, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerEnter (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t x, wl_fixed_t y)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	WaylandServer& server = WaylandServer::instance ();
	WaylandResource* resource = server.findClientResource (This->clientHandle, reinterpret_cast<wl_proxy*> (surface));
	if(resource)
		wl_pointer_send_enter (This->getResourceHandle (), serial, resource->getResourceHandle (), x, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerLeave (void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	WaylandServer& server = WaylandServer::instance ();
	WaylandResource* resource = server.findClientResource (This->clientHandle, reinterpret_cast<wl_proxy*> (surface));
	if(resource)
		wl_pointer_send_leave (This->getResourceHandle (), serial, resource->getResourceHandle ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerMotion (void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	wl_pointer_send_motion (This->getResourceHandle (), time, x, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerButton (void* data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	wl_pointer_send_button (This->getResourceHandle (), serial, time, button, state);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerAxis (void* data, wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	wl_pointer_send_axis (This->getResourceHandle (), time, axis, value);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerAxisSource (void* data, wl_pointer* pointer, uint32_t axisSource)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	wl_pointer_send_axis_source (This->getResourceHandle (), axisSource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerAxisStop (void* data, wl_pointer* pointer, uint32_t time, uint32_t axis)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	wl_pointer_send_axis_stop (This->getResourceHandle (), time, axis);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerAxisDiscrete (void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	wl_pointer_send_axis_discrete (This->getResourceHandle (), axis, discrete);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerAxis120 (void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	#ifdef WL_POINTER_AXIS_VALUE120_SINCE_VERSION
	if(wl_resource_get_version (This->getResourceHandle ()) >= WL_POINTER_AXIS_VALUE120_SINCE_VERSION)
		wl_pointer_send_axis_value120 (This->getResourceHandle (), axis, discrete);
	else
	#endif
	{
		wl_pointer_send_axis_discrete (This->getResourceHandle (), axis, discrete / 120);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerAxisRelativeDirection (void* data, wl_pointer* pointer, uint32_t axis, uint32_t direction)
{
	#ifdef WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	if(wl_resource_get_version (This->getResourceHandle ()) >= WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION)
		wl_pointer_send_axis_relative_direction (This->getResourceHandle (), axis, direction);
	#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void PointerDelegate::onPointerFrame (void* data, wl_pointer* pointer)
{
	PointerDelegate* This = static_cast<PointerDelegate*> (data);
	wl_pointer_send_frame (This->getResourceHandle ());
}

//************************************************************************************************
// KeyboardDelegate
//************************************************************************************************

KeyboardDelegate::KeyboardDelegate (wl_seat* seat)
: WaylandResource (&::wl_keyboard_interface, static_cast<wl_keyboard_interface*> (this)),
  keyboard (nullptr)
{
	enter = onKeyboardFocusEnter;
	leave = onKeyboardFocusLeave;
	keymap = onKeymapReceived;
	key = onKey;
	modifiers = onModifiers;
	repeat_info = onRepeatInfo;

	release = onRelease;

	keyboard = wl_seat_get_keyboard (seat);
	if(keyboard)
		wl_keyboard_add_listener (keyboard, this, this);

	setProxy (reinterpret_cast<wl_proxy*> (keyboard));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

KeyboardDelegate::~KeyboardDelegate ()
{
	if(keyboard)
		wl_keyboard_release (keyboard);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardDelegate::onRelease (wl_client* client, wl_resource* resource)
{
	wl_resource_destroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardDelegate::onKeymapReceived (void* data, wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size)
{
	KeyboardDelegate* This = static_cast<KeyboardDelegate*> (data);
	wl_keyboard_send_keymap (This->getResourceHandle (), format, fd, size);
	::close (fd);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardDelegate::onKeyboardFocusEnter (void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface, struct wl_array* keys)
{
	KeyboardDelegate* This = static_cast<KeyboardDelegate*> (data);
	WaylandServer& server = WaylandServer::instance ();
	WaylandResource* resource = server.findClientResource (This->clientHandle, reinterpret_cast<wl_proxy*> (surface));
	if(resource)
		wl_keyboard_send_enter (This->getResourceHandle (), serial, resource->getResourceHandle (), keys);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardDelegate::onKeyboardFocusLeave (void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface)
{
	KeyboardDelegate* This = static_cast<KeyboardDelegate*> (data);
	WaylandServer& server = WaylandServer::instance ();
	WaylandResource* resource = server.findClientResource (This->clientHandle, reinterpret_cast<wl_proxy*> (surface));
	if(resource)
		wl_keyboard_send_leave (This->getResourceHandle (), serial, resource->getResourceHandle ());	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardDelegate::onKey (void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	KeyboardDelegate* This = static_cast<KeyboardDelegate*> (data);
	#ifdef WL_KEYBOARD_KEY_STATE_REPEATED_SINCE_VERSION
	if(wl_resource_get_version (This->getResourceHandle ()) < WL_KEYBOARD_KEY_STATE_REPEATED_SINCE_VERSION)
	{
		if(state == WL_KEYBOARD_KEY_STATE_REPEATED)
			state = WL_KEYBOARD_KEY_STATE_PRESSED;
	}
	#endif
	wl_keyboard_send_key (This->getResourceHandle (), serial, time, key, state);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardDelegate::onModifiers (void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t depressedModifiers, uint32_t latchedModifiers, uint32_t lockedModifiers, uint32_t group)
{
	KeyboardDelegate* This = static_cast<KeyboardDelegate*> (data);
	wl_keyboard_send_modifiers (This->getResourceHandle (), serial, depressedModifiers, latchedModifiers, lockedModifiers, group);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardDelegate::onRepeatInfo (void* data, wl_keyboard* keyboard, int32_t rate, int32_t delay)
{
	KeyboardDelegate* This = static_cast<KeyboardDelegate*> (data);
	wl_keyboard_send_repeat_info (This->getResourceHandle (), rate, delay);
}

//************************************************************************************************
// TouchDelegate
//************************************************************************************************

TouchDelegate::TouchDelegate (wl_seat* seat)
: WaylandResource (&::wl_touch_interface, static_cast<wl_touch_interface*> (this)),
  touch (nullptr)
{
	down = onTouchDown;
	up = onTouchUp;
	motion = onTouchMotion;
	cancel = onTouchCancel;
	shape = onTouchShape;
	orientation = onTouchOrientation;
	frame = onTouchFrame;

	release = onRelease;

	touch = wl_seat_get_touch (seat);
	if(touch)
		wl_touch_add_listener (touch, this, this);

	setProxy (reinterpret_cast<wl_proxy*> (touch));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

TouchDelegate::~TouchDelegate ()
{
	if(touch)
		wl_touch_release (touch);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void TouchDelegate::onRelease (wl_client* client, wl_resource* resource)
{
	wl_resource_destroy (resource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void TouchDelegate::onTouchDown (void* data, wl_touch* touch, uint32_t serial, uint32_t time, wl_surface* surface, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	TouchDelegate* This = static_cast<TouchDelegate*> (data);
	WaylandServer& server = WaylandServer::instance ();
	WaylandResource* resource = server.findClientResource (This->clientHandle, reinterpret_cast<wl_proxy*> (surface));
	if(resource)
		wl_touch_send_down (This->getResourceHandle (), serial, time, resource->getResourceHandle (), id, x, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void TouchDelegate::onTouchUp (void* data, wl_touch* touch, uint32_t serial, uint32_t time, int32_t id)
{
	TouchDelegate* This = static_cast<TouchDelegate*> (data);
	wl_touch_send_up (This->getResourceHandle (), serial, time, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void TouchDelegate::onTouchMotion (void* data, wl_touch* touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	TouchDelegate* This = static_cast<TouchDelegate*> (data);
	wl_touch_send_motion (This->getResourceHandle (), time, id, x, y);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void TouchDelegate::onTouchCancel (void* data, wl_touch* touch)
{
	TouchDelegate* This = static_cast<TouchDelegate*> (data);
	wl_touch_send_cancel (This->getResourceHandle ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void TouchDelegate::onTouchShape (void* data, wl_touch* touch, int32_t id, wl_fixed_t major, wl_fixed_t minor)
{
	TouchDelegate* This = static_cast<TouchDelegate*> (data);
	wl_touch_send_shape (This->getResourceHandle (), id, major, minor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void TouchDelegate::onTouchOrientation (void* data, wl_touch* touch, int32_t id, wl_fixed_t orientation)
{
	TouchDelegate* This = static_cast<TouchDelegate*> (data);
	wl_touch_send_orientation (This->getResourceHandle (), id, orientation);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void TouchDelegate::onTouchFrame (void* data, wl_touch* touch)
{
	TouchDelegate* This = static_cast<TouchDelegate*> (data);
	wl_touch_send_frame (This->getResourceHandle ());
}
