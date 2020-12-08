#pragma once

#ifndef PUPPET_H
#define PUPPET_H

#include "Vector3.h"
#include "enet/enet.h"
#include <inttypes.h>

struct Puppet {
    Vector3f position = Vector3f(0, 0, 0);
    Vector3f rotation = Vector3f(0, 0, 0);
    uint32_t uid = 0;
    enet_uint32 host = 0;
    int scene = -1;
    bool isHost = false;
    bool isCache = true;
    uint16_t activeSlot = 0;
    uint16_t character = 0;
    //uint8_t animationData[];
};

#endif


