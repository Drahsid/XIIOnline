#pragma once

#ifndef NET_H
#define NET_H

constexpr uint32_t maxPups = 32;

typedef int PType;

struct PacketType {
    static const int Undefined = -1,
        Ping = 0,
        Count = 10;
};

struct TypeServer : public PacketType {
    static const int CheckHost = 1,
        UpdateActorTransform = 2,
        UpdateQuestPS = 3,
        UpdateQuestStatus = 4, //Marks
        UpdateMarkStates = 5,
        UpdateStoryProgress = 6,
        UpdateSceneId = 7;
};

struct TypeClient : public PacketType {
    static const int RecieveHost = 1,
        RecieveActorTransform = 2,
        RecieveQuestPS = 3,
        RecieveQuestStatus = 4, //Marks
        RecieveMarkStates = 5,
        RecieveStoryProgress = 6;
};

void BouncePacket(ENetPeer* inPlayer, ENetPacket* packet, ENetHost* server) {
    for (int i = 0; i < server->connectedPeers; i++) {
        if (inPlayer == nullptr || server->peers[i].connectID != inPlayer->connectID) {
            enet_peer_send(&server->peers[i], 0, packet);
        }
    }
}

//Check for changes, set new data, and return. lhs is new data, rhs is old data; writes new to old
bool CheckForChange(uint8_t* lhs, uint8_t* rhs, size_t length) {
    bool didChange = false;
    printf("i: ");
    for (size_t i = 0; i < length; i++) {
        printf("%d", i);
        if (lhs[i] > rhs[i]) {
            rhs[i] = lhs[i];
            didChange++;
            printf("++");
        }
        if (i < length - 1) printf(", ");
    }

    printf(" Changed? %d\n", didChange);

    return didChange;
}

//rhs is the data to write
ENetPacket* WritePacketData(PType type, uint8_t* response, uint8_t* rhs, size_t length) {
    response = (uint8_t*)malloc(length + sizeof(PType));
    ((PType*)response)[0] = type;
    memcpy_s(response + sizeof(PType), length, rhs, length);
    return enet_packet_create(response, length + sizeof(PType), ENET_PACKET_FLAG_RELIABLE);
}

#endif

