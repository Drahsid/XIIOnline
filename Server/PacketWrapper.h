#pragma once

#ifndef PACKETWRAPPER_H
#define PACKETWRAPPER_H

#include <inttypes.h>
#include "Net.h"
#include "Hunts.h"
#include "enet/enet.h"
#include "StoryProgress.h"

ENetHost* server;
Puppet puppets[64];

struct ActorDataStatic {
    Vector3 pos;
    Vector3 dir;
    float yaw;
};

struct ActorDataDynamic {
    Vector3 pos;
    float yaw;
};

struct ActorNetData {
    uint32_t* base;
    ActorDataStatic s;
    //ActorDataDynamic d;
};


struct PacketWrapper {
    PType Type = TypeServer::Undefined;
    size_t DataLength = 0;
    void* PacketData = nullptr;

    PacketWrapper() {}
    ~PacketWrapper() {
        //printf("PacketWrapper destroyed\n");
        if (PacketData != nullptr) free(PacketData);
    }
    PacketWrapper(ENetEvent* event) {
        //printf("Wrapping packet... ");
        DataLength = event->packet->dataLength - sizeof(PType);
        //printf("sizeof: %d, ", DataLength);
        Type = *(PType*)event->packet->data;
        //printf(" typeof: %d, ", Type);
        PacketData = malloc(DataLength);
        //printf(" malloc'd data, ");
        memcpy_s(PacketData, DataLength, event->packet->data + sizeof(PType), DataLength);
        //printf(" copied data!\n");
    }

    void InterpretData(ENetHost* host, ENetPacket* packet, ENetEvent* event) {
        void* response = nullptr;
        void* data = nullptr;
        //Puppet* plrPuppet = nullptr;
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
            printf("%s has asked if they are the host of their scene\n", peerHostName);

            response = malloc(sizeof(PType) + sizeof(int));

            ((PType*)(response))[0] = (PType)TypeClient::RecieveHost;
            ((int*)(response))[1] = 0;

            for (size_t i = 0; i < maxPups; i++) {
                if (!puppets[i].isCache) {
                    Puppet* p = &puppets[i];
                    if (p->isHost && p->host == event->peer->host->address.host) {
                        ((PType*)(response))[1] = 1;
                        break;
                    }
                }
            }
            packet = enet_packet_create(response, sizeof(PType) + sizeof(int), ENET_PACKET_FLAG_RELIABLE);
            break;
        case TypeServer::UpdateActorTransform:
            printf("Update Actor Transforms\n");

            packet = WritePacketData(TypeClient::RecieveActorTransform, (uint8_t*)response, (uint8_t*)PacketData, (sizeof(ActorNetData) * 24) + sizeof(PType) + sizeof(int));
            BouncePacket(event->peer, packet, server);
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

            printf("Recieved story progress: %d\n", *((uint16_t*)PacketData));
            if (*((uint16_t*)PacketData) > storyProgress) {
                storyProgress = *((uint16_t*)PacketData);
                packet = WritePacketData(TypeClient::RecieveStoryProgress, (uint8_t*)response, (uint8_t*)&storyProgress, sizeof(uint16_t));
                BouncePacket(event->peer, packet, server);
            }

            printf("New story progress: %d %s\n", storyProgress, eStoryProgress.at(storyProgress).c_str());
            break;
        case TypeServer::UpdateSceneId:
            printf("Player %s has updated their scene Id\n", peerHostName);
            /*data = malloc(sizeof(int));
            
            for (size_t i = 0; i < maxPups; i++) {
                if (!puppets[i].isCache) {
                    Puppet* p = &puppets[i];
                    if (p->host == event->peer->host->address.host) {
                        p->scene = *(int*)PacketData;
                        plrPuppet = p;
                    }
                    else if (p->scene == *(int*)PacketData && p->host != event->peer->host->address.host) {
                        (*(int*)data)++;
                    }
                }
            }

            //TODO: Automate this so host transfers when a player leaves a scene
            if (plrPuppet != nullptr) {
                if (*(int*)data == 0) plrPuppet->isHost = true;
                else if (*(int*)data > 0) plrPuppet->isHost = false;
                plrPuppet->activeSlot = *(int*)data + 1;
            }*/

            break;
        }

        if (packet != nullptr) enet_peer_send(event->peer, 0, packet);
        if (response != nullptr) free(response);
        if (data != nullptr) free(data);
    }
};


#endif // !PACKETWRAPPER_H

