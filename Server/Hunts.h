#pragma once

#ifndef QUEST_H
#define QUEST_H

#include <inttypes.h>
#include "enet/enet.h"


size_t s0 = sizeof(uint8_t) * 257;
size_t s1 = sizeof(uint8_t) * 8;
size_t s2 = sizeof(uint8_t) * 368;

uint8_t questProgressStage[257];
uint8_t questStatus[8];
uint8_t markStates[368];
uint16_t storyProgress = 0;

void initQuest() {
    
    memset(questProgressStage, 0, s0);
    memset(questStatus, 0, s1);
    memset(markStates, 0, s2);
    storyProgress = 0;
}

void FullQuestUpdate(ENetPacket* packet, ENetHost* host) {
    uint8_t* data = (uint8_t*)malloc((sizeof(uint8_t) * 368) + sizeof(int));

    ((int*)data)[0] = (PType)TypeClient::RecieveQuestPS;
    memcpy_s(data + sizeof(PType), s0, questProgressStage, s0);
    packet = enet_packet_create(data, s0, ENET_PACKET_FLAG_RELIABLE);
    BouncePacket(nullptr, packet, host);

    ((int*)data)[0] = (PType)TypeClient::RecieveQuestStatus;
    memcpy_s(data + sizeof(PType), s1, questStatus, s1);
    packet = enet_packet_create(data, s1, ENET_PACKET_FLAG_RELIABLE);
    BouncePacket(nullptr, packet, host);

    ((int*)data)[0] = (PType)TypeClient::RecieveMarkStates;
    memcpy_s(data + sizeof(PType), s2, markStates, s2);
    packet = enet_packet_create(data, s2, ENET_PACKET_FLAG_RELIABLE);
    BouncePacket(nullptr, packet, host);
}


#endif

