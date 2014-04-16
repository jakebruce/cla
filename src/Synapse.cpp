#include "CLA.h"
#include <cmath>

using namespace std;

namespace CLA
{
    Synapse::Synapse(char* target_bit,
            int initial_strength)
    {
        target = target_bit;
        if (initial_strength < 0 or initial_strength > 255)
            cla_log("WARNING: initial_strength = %d\n", initial_strength);
        strength = initial_strength;
        _last_step = false;
        _target_last_step = false;
    }

    Synapse::~Synapse() {}

    void Synapse::strengthen()
    {
        strength += CFG::CONNECTION_DELTA_POS;
        if (strength > 255)
            strength = 255;
    }

    void Synapse::strengthen_by_scale_factor(float scale_factor)
    {
        strength = int((1.0+scale_factor)*strength);
        if (strength > 255)
            strength = 255;
    }

    void Synapse::weaken()
    {
        strength -= CFG::CONNECTION_DELTA_NEG;
        if (strength < 0)
            strength = 0;
    }

    bool Synapse::is_connected()
    {
        return strength >= CFG::CONNECTION_THRESHOLD;
    }

    bool Synapse::target_firing()
    {
        return *target;
    }

    char Synapse::get_potential_state()
    {
        return target_firing() and is_connected();
    }

    char Synapse::get_state()
    {
        _target_last_step = target_firing();
        _last_step = get_potential_state();
        return _last_step;
    }

    bool Synapse::fired_last_step()
    {
        return _last_step;
    }

    bool Synapse::target_fired_last_step()
    {
        return _target_last_step;
    }
}
