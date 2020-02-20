#pragma once

#ifndef PUPPET_H
#define PUPPET_H

#include "Vector3.h"
#include "enet/enet.h"

struct Puppet {
    enet_uint32 host = 0;
    Vector3f position = Vector3f();
    bool isHost = false;
};

#endif


