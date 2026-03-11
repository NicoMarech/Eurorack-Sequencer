#pragma once

#include <cstdint>

class Sequencer
{
public:
    Sequencer();
    void update();

private:
    float mTempo;
};