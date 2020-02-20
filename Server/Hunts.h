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

void FullQuestUpdate(ENetHost* host) {
    ENetPacket* packet0;
    ENetPacket* packet1;
    ENetPacket* packet2;
    ENetPacket* packet3;
    
    uint8_t* data0 = (uint8_t*)malloc((sizeof(uint8_t) * 257) + sizeof(PType));
    uint8_t* data1 = (uint8_t*)malloc((sizeof(uint8_t) * 8) + sizeof(PType));
    uint8_t* data2 = (uint8_t*)malloc((sizeof(uint8_t) * 368) + sizeof(PType));
    uint8_t* data3 = (uint8_t*)malloc(sizeof(uint16_t) + sizeof(PType));

    ((PType*)data0)[0] = (PType)TypeClient::RecieveQuestPS;
    memcpy_s(data0 + sizeof(PType), s0, questProgressStage, s0);
    packet0 = enet_packet_create(data0, s0, ENET_PACKET_FLAG_RELIABLE);
    BouncePacket(nullptr, packet0, host);

    ((PType*)data1)[0] = (PType)TypeClient::RecieveQuestStatus;
    memcpy_s(data1 + sizeof(PType), s1, questStatus, s1);
    packet1 = enet_packet_create(data1, s1, ENET_PACKET_FLAG_RELIABLE);
    BouncePacket(nullptr, packet1, host);

    ((PType*)data2)[0] = (PType)TypeClient::RecieveMarkStates;
    memcpy_s(data2 + sizeof(PType), s2, markStates, s2);
    packet2 = enet_packet_create(data2, s2, ENET_PACKET_FLAG_RELIABLE);
    BouncePacket(nullptr, packet2, host);

    ((PType*)data3)[0] = (PType)TypeClient::RecieveStoryProgress;
    memcpy_s(data3 + sizeof(PType), sizeof(uint16_t), &storyProgress, sizeof(uint16_t));
    packet3 = enet_packet_create(data3, sizeof(PType) + sizeof(uint16_t), ENET_PACKET_FLAG_RELIABLE);
    BouncePacket(nullptr, packet3, host);
}


#endif

