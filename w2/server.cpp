#include <enet/enet.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>

struct Client
{
    ENetPeer* peer;
    std::string name;
    int id;
};

int main()
{
    std::map<int, Client> Clients;
    std::map<ENetPeer*, int> idTable;
    std::vector<std::string> names = { "Alex", "Dave", "Alice", "Doctor", "James", "Spock", "Sarah", "Jean-Luc", "Anakin", "Kathryn"};
    int free_id = 0;

    if (enet_initialize() != 0)
    {
        printf("Cannot init ENet");
        return 1;
    }
    ENetAddress address;

    address.host = ENET_HOST_ANY;
    address.port = 10888;

    ENetHost* server = enet_host_create(&address, 32, 2, 0, 0);

    if (!server)
    {
        printf("Cannot create ENet server\n");
        return 1;
    }

    while(true)
    {
        ENetEvent ev;
        while(enet_host_service(server, &ev, 10) > 0)
        {
            switch(ev.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("Connection with %x:%u established\n", ev.peer->address.host, ev.peer->address.port);

                    {
                        Client newClient;
                        newClient.name = names[free_id];
                        newClient.id = free_id;
                        newClient.peer = ev.peer;

                        std::string str = newClient.name.c_str() + ' ' + std::to_string(free_id);
                        const char* msg = str.c_str();
                        ENetPacket* packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(ev.peer, 1, packet);

                        if (!Clients.empty())
                        {
                            std::string playerList = "";
                            for (int i = 0; i < free_id; ++i)
                            {
                                playerList += Clients[i].name + '\n';
                            }

                            ENetPacket* packet = enet_packet_create(playerList.c_str(), 
                                                                    strlen(playerList.c_str()) + 1, 
                                                                    ENET_PACKET_FLAG_RELIABLE);
                            enet_peer_send(ev.peer, 0, packet);
                        }

                        Clients[free_id] = newClient;
                        idTable[ev.peer] = free_id;
                        free_id++;
                    }
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    printf("Packet received: '%s' from %s\n", ev.packet->data, 
                                                            Clients[idTable[ev.peer]].name);
                    enet_packet_destroy(ev.packet);
                    break;
                default:
                    break;
            };
        }

        if (!Clients.empty())
        {
            std::string serverTime = std::to_string(time(0)) + " server time\n"; //server time
            for (int i = 0; i < free_id; ++i)
            {
                ENetPacket* packet = enet_packet_create(serverTime.c_str(), 
                                                        strlen(serverTime.c_str()) + 1, 
                                                        ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(Clients[i].peer, 0, packet);

                std::string strPing = std::to_string(Clients[i].peer->roundTripTime) //player ping
                                        + Clients[i].name; 

                for (int j = 0; j < free_id; ++j)
                {
                    if (i == j)
                        continue;

                    ENetPacket* packetPing = enet_packet_create(strPing.c_str(),
                        strlen(strPing.c_str()) + 1,
                        ENET_PACKET_FLAG_UNSEQUENCED);
                    enet_peer_send(Clients[j].peer, 0, packetPing);
                }
            }

        }
    }
    enet_host_destroy(server);

    atexit(enet_deinitialize);
    return 0;
}