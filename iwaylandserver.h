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
// Filename    : iwaylandserver.h
// Description : Wayland Server Interface
//
//************************************************************************************************

#ifndef _iwaylandserver_h
#define _iwaylandserver_h

struct wl_display;
struct wl_surface;
struct xdg_surface;
struct xdg_toplevel;

struct wl_proxy;

namespace WaylandServerDelegate {

class WaylandResource;
class IWaylandClientContext;

//************************************************************************************************
// IWaylandServer
//************************************************************************************************

struct IWaylandServer
{
public:
	static IWaylandServer& instance ();

	/** Startup the Wayland server.
	 * @param context a context instance representing the application's session compositor connection and related resources.
	 * @return a file descriptor which should be added to the main event loop or -1 on failure.
	 * The caller should poll for events using the returned file descriptor 
	 * and call IWaylandServer::dispatch when events are available.
	 * \a context must stay valid until the server is shut down.
	 */
	virtual int startup (IWaylandClientContext* context) = 0;

	/** Shutdown the Wayland server. */
	virtual void shutdown () = 0;

	/** Check if the Wayland server started successfully. */
	virtual bool isStarted () const = 0;

	/** Dispatch incoming events. */
	virtual void dispatch () = 0;

	/** Send all pending outgoing events to clients. */
	virtual void flush () = 0;

	/** Open a new client connection. */
	virtual wl_display* openClientConnection () = 0;
	
	/** Close a previously opened client connection. */
	virtual bool closeClientConnection (wl_display* display) = 0;

	/** Get the number of active client connections. */
	virtual int countActiveClients () const = 0;

	/** Create a proxy for a client connection, wrapping an existing server-side Wayland object.
	 * @param display a client display which has been created with \a openClientConnection.
	 * @param object an existing Wayland object which has been created using the session compositor connection.
	 * @param implementation a Wayland resource implementation to be used with the new proxy.
	 * Takes ownership of \a implementation.
	 */
	virtual wl_proxy* createProxy (wl_display* display, wl_proxy* object, WaylandResource* implementation) = 0;

	/** Destroy a previously created proxy. */
	virtual void destroyProxy (wl_proxy* proxy) = 0;
};

} // namespace WaylandServerDelegate

#endif // _iwaylandserver_h
