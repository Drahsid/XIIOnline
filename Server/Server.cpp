#include <iostream>
#include <cstdio>
#include <vector>
#include <ctime>
#include "Vector3.h"
#include "Config.h"
#include "Puppet.h"
#include "Net.h"
#include "PacketWrapper.h"
#include "Hunts.h"
#include "enet/enet.h"


#define WIN32_LEAN_AND_MEAN

UserConfig config;
uint32_t uidPos = 0;

int main()
{
    for (int i = 0; i < maxPups; i++) {
        puppets[i] = Puppet();
    }

    config.UpdateUserConfig();

    enet_initialize();
    printf("enet_initialize\n");

    ENetAddress address;
    

    address.host = ENET_HOST_ANY;
    address.port = config.serverPort;
    server = enet_host_create(&address, 16, 2, 0, 0);
    printf("enet_host_create\n");

    if (server == NULL)
    {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    initQuest();

    ENetEvent event;
    Puppet newPuppet;
    clock_t lastFullSync = 0;
    
    for (;;) {
        ENetPacket* packet = nullptr;

        if ((clock() - lastFullSync) / CLOCKS_PER_SEC > 5)
        {
            printf("Sending full sync to players\n");
            FullQuestUpdate(server);
            lastFullSync = clock();
        }

        if (server->connectedPeers <= 0) initQuest(); //Reset server data

        while (enet_host_service(server, &event, 0) > 0)
        {
            char peerHostName[17];
            #pragma warning (disable : 6054)
            enet_address_get_host_ip(&event.peer->address, peerHostName, strlen(peerHostName));
            
            PacketWrapper* wPacket = nullptr;

            bool foundSlot = false;

            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %s:%u.\n", peerHostName, event.peer->address.port);
                uidPos++;
                newPuppet = Puppet();
                newPuppet.host = event.peer->address.host;
                newPuppet.uid = uidPos;


                for (int i = 0; i < maxPups; i++) {
                    if (puppets[i].isCache) {
                        puppets[i] = newPuppet;
                        puppets[i].isCache = false;
                        foundSlot = true;
                        break;
                    }
                }

                if (!foundSlot) printf("Could not add a puppet for %s\n", peerHostName);
                else foundSlot = false;
                

                FullQuestUpdate(server);
                
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                if (event.packet->dataLength >= sizeof(PType)) {
                    wPacket = new PacketWrapper(&event);
                    wPacket->InterpretData(server, packet, &event);
                    wPacket->~PacketWrapper();
                }

                enet_packet_destroy(event.packet);

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%s disconnected.\n", peerHostName);
                
                for (size_t i = 0; i < maxPups; i++) {
                    Puppet* p = &puppets[i];
                    if (p->host == event.peer->address.host) {
                        p->isCache = true;
                    }
                }

                event.peer->data = NULL;
            }
        }
        Sleep(1);
    }

    enet_host_destroy(server);
    enet_deinitialize();

    return 1;
}



