#pragma once

#ifndef NET_H
#define NET_H
#include <map>
#include "PacketWrapper.h"
#include "Hunts.h"
#include "enet/enet.h"

bool hasHostPlayer = false;
ENetHost* server;
std::vector<Puppet> puppets;

struct PacketWrapper {
    PType Type = TypeServer::Undefined;
    size_t DataLength = 0;
    void* PacketData = nullptr;

    PacketWrapper() {}
    ~PacketWrapper() {
        printf("PacketWrapper destroyed\n");
        if (PacketData != nullptr) free(PacketData);
    }
    PacketWrapper(ENetEvent* event) {
        printf("Wrapping packet... ");
        DataLength = event->packet->dataLength - sizeof(PType);
        printf("sizeof: %d, ", DataLength);
        Type = *(PType*)event->packet->data;
        printf(" typeof: %d, ", Type);
        PacketData = malloc(DataLength);
        printf(" malloc'd data, ");
        memcpy_s(PacketData, DataLength, event->packet->data + sizeof(PType), DataLength);
        printf(" copied data!\n");
    }

    void InterpretData(ENetHost* host, ENetPacket* packet, ENetEvent* event) {
        void* response = nullptr;
        void* data = nullptr;
        char peerHostName[17];
        enet_packet_destroy(packet); //Make sure this is dead
        packet = nullptr;

#pragma warning (disable : 6001)
#pragma warning (disable : 6011)
#pragma warning (disable : 6054)

        enet_address_get_host_ip(&event->peer->address, peerHostName, strlen(peerHostName));
        
        switch (Type)
        {
        case TypeServer::Ping:
            printf("Recieved ping from %s\n", peerHostName);
            packet = enet_packet_create(&TypeClient::Ping, sizeof(PType), ENET_PACKET_FLAG_RELIABLE);
            break;
        case TypeServer::CheckHost:
            printf("%s has asked if they are the host player\n", peerHostName);
            
            response = malloc(sizeof(PType) + sizeof(int));
            
            ((PType*)(response))[0] = (PType)TypeClient::RecieveHost;
            ((int*)(response))[1] = 0;

            for (size_t i = 0; i < puppets.size(); i++) {
                Puppet* p = &puppets.at(i);
                if (p->isHost && p->host == event->peer->host->address.host) {
                    ((PType*)(response))[1] = 1;
                    break;
                }
            }
            packet = enet_packet_create(response, sizeof(PType) + sizeof(int), ENET_PACKET_FLAG_RELIABLE);
            break;
        case TypeServer::UpdatePlayerPosition:
            if (event->packet->dataLength >= sizeof(PType) + sizeof(Vector3f)) {
                printf("%s has sent position data: %s\n", peerHostName, ((Vector3f*)PacketData)->toString().c_str());
                
                for (size_t i = 0; i < puppets.size(); i++) {
                    Puppet* p = &puppets.at(i);
                    if (p->host == event->peer->host->address.host) {
                        p->position = *(Vector3f*)response;
                        break;
                    }
                }
            }
            else printf("%s has sent incomplete position data!\n", peerHostName);
            break;
        case TypeServer::UpdateQuestPS:
            printf("Update Quest PS\n");
            
            if (CheckForChange((uint8_t*)PacketData, questProgressStage, s0)) {
                packet = WritePacketData(TypeClient::RecieveQuestPS, (uint8_t*)response, questProgressStage, s0);
                BouncePacket(event->peer, packet, server);
            }
            break;
        case TypeServer::UpdateQuestStatus:
            printf("Update QS\n");

            if (CheckForChange((uint8_t*)PacketData, questProgressStage, s1)) {
                packet = WritePacketData(TypeClient::RecieveQuestStatus, (uint8_t*)response, questStatus, s1);
                BouncePacket(event->peer, packet, server);
            }
            break;
        case TypeServer::UpdateMarkStates:
            printf("Update MS\n");

            if (CheckForChange((uint8_t*)PacketData, questProgressStage, s2)) {
                packet = WritePacketData(TypeClient::RecieveMarkStates, (uint8_t*)response, markStates, s2);
                BouncePacket(event->peer, packet, server);
            }
            break;
        case TypeServer::UpdateStoryProgress:
            printf("Update Story Progress\n");
            
            if (((uint16_t*)PacketData)[2] > storyProgress) {
                packet = WritePacketData(TypeClient::RecieveStoryProgress, (uint8_t*)response, (uint8_t*)PacketData + sizeof(PType), sizeof(uint16_t));
                BouncePacket(event->peer, packet, server);
            }

            printf("New story progress: %d\n", storyProgress);
            break;
        }

        if (packet != nullptr) enet_peer_send(event->peer, 0, packet);
        if (response != nullptr) free(response);
        if (data != nullptr) free(data);
    }
};

#endif // !NET_H

