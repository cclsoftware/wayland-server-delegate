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
// Filename    : waylandserver.cpp
// Description : Wayland Server
//
//************************************************************************************************

#include "waylandserver.h"
#include "registrydelegate.h"

#include <iostream>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace WaylandServerDelegate;

//************************************************************************************************
// IWaylandServer
//************************************************************************************************

IWaylandServer& IWaylandServer::instance ()
{
	return WaylandServer::instance ();
}

//************************************************************************************************
// WaylandServer
//************************************************************************************************

WaylandServer::WaylandServer ()
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

WaylandServer& WaylandServer::instance ()
{
	static WaylandServer theInstance;
	return theInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int WaylandServer::startup (IWaylandClientContext* clientContext)
{
	context = clientContext;
	if(context == nullptr)
		return -1;

	display = wl_display_create ();
	if(display == nullptr)
	{
		std::cerr << "Failed to create a Wayland display." << std::endl;
		return -1;
	}
	
	serverEventLoop = wl_display_get_event_loop (display);

	RegistryDelegate::instance ().startup ();

	initialized = true;
	
	return wl_event_loop_get_fd (serverEventLoop);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandServer::shutdown ()
{
	if(!initialized)
		return;

	if(display)
	{
		wl_display_destroy_clients (display);
		RegistryDelegate::instance ().shutdown ();
		wl_display_destroy (display);
	}
	display = nullptr;

	connections.clear ();
	
	initialized = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandServer::dispatch ()
{
	wl_event_loop_dispatch (serverEventLoop, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandServer::flush ()
{
	if(display)
		wl_display_flush_clients (display);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

WaylandServer::ClientConnection* WaylandServer::findClientConnection (wl_client* client)
{
	for(ClientConnection& connection : connections)
	{
		if(connection.clientHandle == client)
			return &connection;
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

WaylandServer::ClientConnection* WaylandServer::findClientConnection (wl_display* display)
{
	for(ClientConnection& connection : connections)
	{
		if(connection.clientDisplay == display)
			return &connection;
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

WaylandResource* WaylandServer::findClientResource (wl_client* client, wl_resource* resourceHandle)
{
	ClientConnection* connection = findClientConnection (client);
	if(connection == nullptr)
		return nullptr;

	return connection->findResource (resourceHandle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

WaylandResource* WaylandServer::findClientResource (wl_client* client, wl_proxy* proxy)
{
	ClientConnection* connection = findClientConnection (client);
	if(connection == nullptr)
		return nullptr;

	return connection->findResource (proxy);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

wl_display* WaylandServer::openClientConnection ()
{
	if(display == nullptr)
		return nullptr;

	ClientConnection connection;
	if(::socketpair (AF_UNIX, SOCK_STREAM, 0, connection.fds) == -1)
		return nullptr;

	::fcntl (connection.fds[0], F_SETFD, FD_CLOEXEC);

	connection.clientHandle = wl_client_create (display, connection.fds[0]);
	if(connection.clientHandle == nullptr)
	{
		::close (connection.fds[0]);
		::close (connection.fds[1]);
		return nullptr;
	}

	connection.clientDisplay = wl_display_connect_to_fd (connection.fds[1]);
	if(connection.clientDisplay == nullptr)
	{
		::close (connection.fds[0]);
		::close (connection.fds[1]);
		return nullptr;
	}

	//TODO wl_client_add_destroy_listener (connection.clientHandle, ...);

	connections.push_back (connection);

	flush ();

	return connection.clientDisplay;
}


//////////////////////////////////////////////////////////////////////////////////////////////////

bool WaylandServer::closeClientConnection (wl_display* display)
{
	if(display == nullptr)
		return false;

	for(auto connection = connections.begin (); connection != connections.end (); connection++)
	{
		if(connection->clientDisplay == display)
		{
			wl_client_destroy (connection->clientHandle);
			::close (connection->fds[0]);
			::close (connection->fds[1]);
			connections.erase (connection);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int WaylandServer::countActiveClients () const
{
	return int(connections.size ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

wl_proxy* WaylandServer::createProxy (wl_display* display, wl_proxy* object, WaylandResource* implementation)
{
	if(display == nullptr || object == nullptr || implementation == nullptr)
	{
		std::cerr << "Failed to create a proxy object: invalid arguments." << std::endl;
		delete implementation;
		return nullptr;
	}

	ClientConnection* connection = findClientConnection (display);
	if(connection == nullptr)
	{
		std::cerr << "Failed to create a proxy object: invalid display." << std::endl;
		delete implementation;
		return nullptr;
	}

	wl_proxy* result = wl_proxy_create (reinterpret_cast<wl_proxy*> (display), implementation->getWaylandInterface ());
	uint32_t id = wl_proxy_get_id (result);

	wl_display_flush (display);
	wl_event_loop_dispatch (serverEventLoop, 0);
	
	connection->addResource (implementation, wl_proxy_get_version (object), id);

	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandServer::destroyProxy (wl_proxy* proxy)
{
	wl_proxy_destroy (proxy);
}

//************************************************************************************************
// WaylandServer::ClientConnection
//************************************************************************************************

WaylandServer::ClientConnection::ClientConnection ()
: fds {0},
  clientHandle (nullptr),
  clientDisplay (nullptr)
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool WaylandServer::ClientConnection::operator == (const ClientConnection& other)
{
	return clientHandle == other.clientHandle;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandServer::ClientConnection::addResource (WaylandResource* implementation, uint32_t id)
{
	if(implementation == nullptr || implementation->getProxy () == nullptr)
		return;

	uint32_t version = wl_proxy_get_version (implementation->getProxy ());

	addResource (implementation, version, id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandServer::ClientConnection::addResource (WaylandResource* implementation, uint32_t version, uint32_t id)
{
	wl_resource* resource = wl_resource_create (clientHandle, implementation->getWaylandInterface (), version, id);
	if(resource == nullptr)
	{
		wl_client_post_no_memory (clientHandle);
		return;
	}

	implementation->setResourceHandle (resource);
	implementation->setClientHandle (clientHandle);
	resources.push_back (implementation);
	wl_resource_set_implementation (resource, implementation->getImplementation (), implementation, WaylandResource::onDestroy);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void WaylandServer::ClientConnection::removeResource (WaylandResource* implementation)
{
	for(auto resource = resources.begin (); resource != resources.end (); resource++)
	{
		if((*resource)->getResourceHandle () == implementation->getResourceHandle ())
		{
			resources.erase (resource);
			delete implementation;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

WaylandResource* WaylandServer::ClientConnection::findResource (wl_resource* resourceHandle)
{
	if(resourceHandle == nullptr)
		return nullptr;
	for(WaylandResource* resource : resources)
	{	
		if(resource->getResourceHandle () == resourceHandle)
			return resource;
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

WaylandResource* WaylandServer::ClientConnection::findResource (wl_proxy* proxy)
{
	if(proxy == nullptr)
		return nullptr;
	for(WaylandResource* resource : resources)
	{	
		if(resource->getProxy () == proxy)
			return resource;
	}
	return nullptr;
}
