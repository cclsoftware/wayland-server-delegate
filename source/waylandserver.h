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
// Filename    : waylandserver.h
// Description : Wayland Server
//
//************************************************************************************************

#ifndef _waylandserver_h
#define _waylandserver_h

#include "wayland-server-delegate/iwaylandserver.h"
#include "wayland-server-delegate/waylandresource.h"

#include <vector>

namespace WaylandServerDelegate {

struct IWaylandClientContext;

//************************************************************************************************
// WaylandServer
//************************************************************************************************

class WaylandServer: public IWaylandServer
{
public:
	static WaylandServer& instance ();

	struct ClientConnection
	{
		int fds[2];
		wl_client* clientHandle;
		wl_display* clientDisplay;
		std::vector<WaylandResource*> resources;

		ClientConnection ();
		bool operator == (const ClientConnection& other);

		void addResource (WaylandResource* implementation, uint32_t id);
		void addResource (WaylandResource* implementation, uint32_t version, uint32_t id);
		void removeResource (WaylandResource* implementation);

		WaylandResource* findResource (wl_resource* resourceHandle);
		WaylandResource* findResource (wl_proxy* proxy);
	};

	IWaylandClientContext* getContext () const { return context; }
	wl_display* getDisplay () const { return display; }

	wl_event_loop* getEventLoop () const { return serverEventLoop; }
	void setEventLoop (wl_event_loop* eventLoop) { serverEventLoop = eventLoop; }

	ClientConnection* findClientConnection (wl_client* client);
	ClientConnection* findClientConnection (wl_display* display);
	WaylandResource* findClientResource (wl_client* client, wl_resource* resource);
	WaylandResource* findClientResource (wl_client* client, wl_proxy* proxy);

	int openClientConnectionFd ();
	void closClientConnectionFd (int fd);
	const std::vector<ClientConnection>& getConnections () const { return connections; }

	// IWaylandServer
	int startup (IWaylandClientContext* context) override;
	void shutdown () override;
	bool isStarted () const override { return initialized; }
	void dispatch () override;
	void flush () override;
	wl_display* openClientConnection () override;
	bool closeClientConnection (wl_display* display) override;
	int countActiveClients () const override;
	wl_proxy* createProxy (wl_display* display, wl_proxy* object, WaylandResource* implementation) override;
	void destroyProxy (wl_proxy* proxy) override;

private:
	IWaylandClientContext* context;
	wl_display* display;
	wl_event_loop* serverEventLoop;
	std::vector<ClientConnection> connections;
	bool initialized;

	WaylandServer ();
};

} // namespace WaylandServerDelegate

#endif // _waylandserver_h
