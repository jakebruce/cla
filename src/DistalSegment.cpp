#include "CLA.h"
#include <vector>
#include <algorithm>

using namespace std;

namespace CLA
{
    DistalSegment::DistalSegment(vector<Cell*>& input_bits, 
            int num_synapses, 
            int thresh,
            int input_space_location)
    {
        for (int i = 0; i < num_synapses; ++i)
        {
            int random_bit;
            if (input_space_location >= 0)
            {
                random_bit = input_space_location + cla_rand(0,input_bits.size()/2)
                    - input_bits.size()/4;
                while (random_bit < 0)
                    random_bit += input_bits.size();
                while (random_bit >= (int) input_bits.size())
                    random_bit %= input_bits.size();
            }
            else
            {
                random_bit = cla_rand(0,input_bits.size()-1);
            }

            potential_synapses.push_back(input_bits[random_bit]);
        }

        dead = false;
        learned = false;
        threshold = thresh;

        _input_history_cursor = 0;
        _input_history = new int[CFG::INPUT_HISTORY];

        for (int i = 0; i < CFG::INPUT_HISTORY; ++i)
            _input_history[i] = -1;
    }
    
    DistalSegment::~DistalSegment() {}

    void DistalSegment::update_input_history(int pattern_index)
    {
        _input_history[_input_history_cursor] = pattern_index;
        _input_history_cursor = (_input_history_cursor+1) % CFG::INPUT_HISTORY;
    }

    int _segment_winning_vote(vector<int>& v)
    {
        int count = 0;
        int current = -1;
        int best = -1;
        int best_count = 0;

        sort(v.begin(), v.end());

        for (unsigned int j = 0; j < v.size(); ++j)
        {
            if (v[j] != current and v[j] != -1)
            {
                count = 0;
                current = v[j];
            }

            count++;
            if (count > best_count)
            {
                best = current;
                best_count = count;
            }
        }

        return best;
    }

    int DistalSegment::calculate_input_vote()
    {
        vector<int> v;
        for (int i = 0; i < CFG::INPUT_HISTORY; ++i)
            v.push_back(_input_history[i]);

        return _segment_winning_vote(v);
    }

    char DistalSegment::get_state()
    {
        if (not learned)
            return false; // can't activate a non-learned segment

        int count = 0;
        for (unsigned int i = 0; i < synapses.size(); ++i)
        {
            if (synapses[i]->get_state())
            {
                count++;
                if (count >= threshold)
                {
                    return true;
                }
            }
        }

        return false;
    }

    char DistalSegment::get_potential_state()
    {
        if (not learned)
            return false;

        int count = 0;
        for (unsigned int i = 0; i < synapses.size(); ++i)
        {
            if (synapses[i]->get_potential_state())
            {
                count++;
                if (count > threshold)
                {
                    return true;
                }
            }
        }

        return false;
    }

    void DistalSegment::grow(vector<Cell*>& active_cells)
    {
        /*
        for (unsigned int i = 0; i < synapses.size(); ++i)
        {
            // prune dead connections to make room for the new ones
            if (not synapses[i]->is_connected())
            {
                delete synapses[i];
                synapses.erase(synapses.begin()+i);
                i--;
            }
        }
        */

        // add all the active columns
        for (unsigned int i = 0; i < active_cells.size(); ++i)
            if (active_cells[i]->learn_state)
                synapses.push_back(new Synapse(&(active_cells[i]->active_state), 
                            CFG::CONNECTION_THRESHOLD+CFG::CONNECTION_DELTA_POS*2));

        learned = true;
        potential_synapses.clear();
    }

    void DistalSegment::learn(bool correct_prediction)
    {
        int connected_synapses = 0;
        for (unsigned int i = 0; i < synapses.size(); ++i)
        {
            if (correct_prediction)
            {
                if (synapses[i]->target_fired_last_step())
                {
                    synapses[i]->strengthen();
                }
                else
                {
                    synapses[i]->weaken();
                }
            }
            else
            {
                if (synapses[i]->fired_last_step())
                {
                    synapses[i]->weaken();
                }
            }

            if (synapses[i]->is_connected())
                connected_synapses++;
        }

        if (connected_synapses < threshold)
        {
            // throw out these synapses and get a new random set
            dead = true;
        }
    }

    int DistalSegment::get_potential()
    {
        int count = 0;
        if (not learned)
        {
            for (unsigned int i = 0; i < potential_synapses.size(); ++i)
            {
                if (potential_synapses[i]->active_state)
                    count++;
            }
        }
        else
        {
            for (unsigned int i = 0; i < synapses.size(); ++i)
            {
                if (synapses[i]->get_potential_state())
                    count++;
            }
        }
        return count;
    }

    bool DistalSegment::is_learned()
    {
        return learned;
    }
}
