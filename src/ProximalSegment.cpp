#include "CLA.h"
#include <vector>
#include <cmath>
#include <cstdio>

using namespace std;

namespace CLA
{
    ProximalSegment::ProximalSegment(vector<char*>& input_bits,
            int num_synapses,
            int input_space_location)

    {
        float avg_loc = 0;
        for (int i = 0; i < num_synapses; ++i)
        {
            // connect to a random input_bit that we have not yet connected to
            // by creating a Synapse and adding it to synapses
            //
            // we're doing simple topology here

            
            int random_bit_idx = -1;
            random_bit_idx = cla_rand(0,input_bits.size()-1);

            char* target_bit = input_bits[random_bit_idx];

            int dist = abs(input_space_location - random_bit_idx)+1;
            int location_boost = (input_bits.size()/100)/dist;

            int rand_boost = cla_rand(0,63) - 32;

            cla_register_hookup(random_bit_idx, input_bits.size());

            synapses.push_back(new Synapse(target_bit, 
                        CFG::CONNECTION_THRESHOLD+rand_boost+location_boost));
        }

        avg_loc /= num_synapses;

        activation_level = 0;
    }
    
    ProximalSegment::~ProximalSegment() {}

    int ProximalSegment::get_potential()
    {
        int act = 0;
        for (unsigned int i = 0; i < synapses.size(); ++i)
            if (synapses[i]->get_potential_state())
                act++;
        return act;
    }

    int ProximalSegment::get_sum()
    {
        return activation_level;
    }

    void ProximalSegment::update_input()
    {
        activation_level = 0;
        for (unsigned int i = 0; i < synapses.size(); ++i)
            if (synapses[i]->get_state())
                activation_level++;
    }

    // ignore column_state for now, but we might want to do something
    // with segments on inactive columns later
    void ProximalSegment::learn(char column_state)
    {
        if (column_state)
        {
            for (unsigned int i = 0; i < synapses.size(); ++i)
            {
                if (synapses[i]->target_firing())
                {
                    synapses[i]->strengthen();
                }
                else
                {
                    synapses[i]->weaken();
                }
            }
        }
    }

    void ProximalSegment::strengthen_by_scale_factor()
    {
        for (unsigned int i = 0; i < synapses.size(); ++i)
        {
            //synapses[i]->strengthen_by_scale_factor(CFG::CONNECTION_SCALE_DELTA);
            synapses[i]->strengthen();
        }
    }

    void ProximalSegment::print_string()
    {
        cla_log("[");
        for (unsigned int i = 0; i < synapses.size(); ++i)
            if (synapses[i]->is_connected())
                cla_log(ANSI_COLOR_GREEN"%d", synapses[i]->get_state());
            else
                cla_log(ANSI_COLOR_BLUE"%d", synapses[i]->get_state());
        cla_log(ANSI_COLOR_RESET);
        cla_log("]");
    }
}
