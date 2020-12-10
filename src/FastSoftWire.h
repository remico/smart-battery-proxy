#pragma once

#include <SoftWire.h>

class FastSoftWire : public SoftWire
{
public:
    FastSoftWire(uint8_t sdaPin, uint8_t sclPin)
        : SoftWire(sdaPin, sclPin)
    {
    }
};
