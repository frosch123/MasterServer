/* $Id$ */

/*
 * This file is part of OpenTTD's master server.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MASTERSERVER_H
#define MASTERSERVER_H

#include "shared/udp_server.h"

/**
 * @file masterserver/masterserver.h Configuration and classes used by the master server
 */

/**
 * Some configuration constants
 */
enum {
	GAME_SERVER_LIST_AGE  = 10, ///< Maximum age of the serverlist packet in frames

	SERVER_QUERY_TIMEOUT  =  5, ///< How many frames it takes for a server to time out
	SERVER_QUERY_ATTEMPTS =  3, ///< How many times do we try to query?

	SAFE_MTU = 1360, ///< Safe threshold for MTUs, some networks don't like big ones.
};

class MSQueriedServer : public QueriedServer {
protected:
	friend class MasterNetworkUDPSocketHandler;
	NetworkAddress reply_address; ///< Address of the reply UDP packet
	uint64 session_key;           ///< The unique identifier of the server

public:
	/**
	 * Creates a new queried server for the gameserver with the given address
	 * @param query_address the address of the gameserver
	 * @param reply_address the address of the requester
	 * @param frame         time of the last attempt
	 */
	MSQueriedServer(const NetworkAddress &query_address, const NetworkAddress &reply_address, uint64 session_key, uint frame);

	void DoAttempt(UDPServer *server);

	/**
	 * Gets the address this game server has used to query us.
	 * @return the address the game server used to query us
	 */
	NetworkAddress *GetReplyAddress() { return &this->reply_address; }

	/* virtual */ uint64 GetSessionKey() const { return this->session_key; }
};

/**
 * Code specific to the master server
 */
class MasterServer : public UDPServer {
private:
	bool update_serverlist_packet[SLT_END]; ///< Whether to update the cached server list packet
	Packet *serverlist_packet[SLT_END];     ///< The cached server list packet
	uint next_serverlist_frame[SLT_END];    ///< Frame where to make a new server list packet
	uint64 session_key;                     ///< New session key to give out

protected:
	NetworkUDPSocketHandler *master_socket; ///< Socket to listen for registration, unregistration and queries for the server list

public:
	/**
	 * Create a new masterserver using a given SQL server and bind to the
	 * given hostname and port.
	 * @param sql         the SQL server used as persistent storage
	 * @param addresses   the addresses to bind on
	 */
	MasterServer(SQL *sql, NetworkAddressList *addresses);

	/** The obvious destructor */
	~MasterServer();

	void ReceivePackets();

	MSQueriedServer *GetQueriedServer(NetworkAddress *client_addr) { return (MSQueriedServer*)UDPServer::GetQueriedServer(client_addr); }

	void ServerStateChange()
	{
		for (uint i = 0; i < SLT_END; i++) {
			this->update_serverlist_packet[i] = true;
		}
	}

	/**
	 * Send a registration ack to the server.
	 * @param qs the server to ack.
	 */
	void SendAck(MSQueriedServer *qs);

	Packet *GetServerListPacket(ServerListType type); ///< Gets an (relatively) up-to-date packet with all game servers

	/**
	 * Get the next, semi-random, session key
	 * @return the session key
	 */
	uint64 NextSessionKey();

	/**
	 * Get the current session key.
	 * @return the session key
	 */
	uint64 GetSessionKey() { return this->session_key; }
};

/** Handler for the query socket of the masterserver */
class QueryNetworkUDPSocketHandler : public NetworkUDPSocketHandler {
protected:
	MasterServer *ms; ///< The masterserver we are related to
	virtual void Receive_SERVER_RESPONSE(Packet *p, NetworkAddress *client_addr); ///< Handle a PACKET_UDP_SERVER_RESPONSE packet
public:
	/**
	 * Create a new query socket handler for a given masterserver
	 * @param ms the masterserver this socket is related to
	 * @param addresses the host to bind on
	 */
	QueryNetworkUDPSocketHandler(MasterServer *ms, NetworkAddressList *addresses) :
		NetworkUDPSocketHandler(addresses),
		ms(ms)
	{}

	/** The obvious destructor */
	virtual ~QueryNetworkUDPSocketHandler() {}
};

/** Handler for the master socket of the masterserver */
class MasterNetworkUDPSocketHandler : public NetworkUDPSocketHandler {
protected:
	MasterServer *ms; ///< The masterserver we are related to
	virtual void Receive_SERVER_REGISTER(Packet *p, NetworkAddress *client_addr);   ///< Handle a PACKET_UDP_SERVER_REGISTER packet
	virtual void Receive_CLIENT_GET_LIST(Packet *p, NetworkAddress *client_addr);   ///< Handle a PACKET_UDP_CLIENT_GET_LIST packet
	virtual void Receive_SERVER_UNREGISTER(Packet *p, NetworkAddress *client_addr); ///< Handle a PACKET_UDP_SERVER_UNREGISTER packet
public:
	/**
	 * Create a new masterserver socket handler for a given masterserver
	 * @param ms the masterserver this socket is related to
	 * @param addresses the addresses to bind on
	 */
	MasterNetworkUDPSocketHandler(MasterServer *ms, NetworkAddressList *addresses) :
		NetworkUDPSocketHandler(addresses),
		ms(ms)
	{}

	/** The obvious destructor */
	virtual ~MasterNetworkUDPSocketHandler() {}
};

#endif /* MASTERSERVER_H */
