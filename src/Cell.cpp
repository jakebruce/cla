#include "CLA.h"
#include <vector>
#include <algorithm>

using namespace std;

namespace CLA
{
    void Cell::_init_fields()
    {
        axon = false;

        active_state = false;
        predictive_state = false;
        learn_state = false;

        _prediction_level = 0;
    }

    Cell::Cell(vector<Cell*>& neighborhood_cells, 
            int distal_segment_count,
            int segment_size,
            int active_column_count,
            int input_space_location)
    {
        _active_column_count = active_column_count;
        hook_up_distal_segments(neighborhood_cells, distal_segment_count, segment_size,
                char(active_column_count/2), input_space_location);

        _init_fields();
    }

    Cell::Cell()
    {
        _init_fields();
    }
    
    Cell::~Cell() 
    {
        for (unsigned int i = 0; i < distal_segments.size(); ++i)
            delete distal_segments[i];
    }

    void Cell::hook_up_distal_segments(vector<Cell*>& neighborhood_cells,
            int distal_segment_count,
            int segment_size,
            int active_column_count,
            int input_space_location)
    {
        for (int i = 0; i < distal_segment_count; ++i)
        {
            distal_segments.push_back(
                    new DistalSegment(
                        neighborhood_cells, 
                        segment_size, 
                        active_column_count/2, 
                        input_space_location));
        }
    }

    void Cell::update_input()
    {
        axon = false;
        active_state = false;
        learn_state = false;
    }

    int Cell::calculate_prediction_potential()
    {
        DistalSegment* best = get_best_segment();
        if (best == 0) // no free segments
            return 0;

        return best->get_potential();
    }

    int Cell::get_prediction_potential()
    {
        return _prediction_level;
    }

    char Cell::any_segments_active()
    {
        char ret = false;
        _active_segments.clear();
        for (unsigned int i = 0; i < distal_segments.size(); ++i)
            if (distal_segments[i]->get_state())
            {
                _active_segments.push_back(i);
                ret = true;
            }
        return ret;
    }

    DistalSegment* Cell::get_best_segment()
    {
        int best = -1;
        DistalSegment* pbest = 0;

        vector<DistalSegment*> tmp = distal_segments;
        random_shuffle(tmp.begin(), tmp.end());

        for (unsigned int i = 0; i < tmp.size(); ++i)
        {
            int s = tmp[i]->get_potential();

            if (s > best)
            {
                best = s;
                pbest = tmp[i];
            }
        }
        return pbest;
    }

    void Cell::update_state(char fire, char learn, vector<Cell*>& active_cells)
    {
        learn_state = learn;
        active_state = fire;

        if (active_state && learn_state)
        {
            DistalSegment* pbest = get_best_segment();
            if (pbest != 0)
            {
                pbest->grow(active_cells);
            }
        }
    }

    // call this once all cell states have been updated
    void Cell::update_prediction(vector<Cell*>& active_cells, vector<Cell*>& neighborhood_cells)
    {
        for (unsigned int i = 0; i < _active_segments.size(); ++i)
        {
            if (predictive_state and active_state)
            {
                distal_segments[_active_segments[i]]->learn(true);
            }
            if (predictive_state and not active_state)
            {
                distal_segments[_active_segments[i]]->learn(false);
            }

            if (distal_segments[_active_segments[i]]->dead)
            {
                delete distal_segments[_active_segments[i]];
                distal_segments[_active_segments[i]] = new DistalSegment(
                        neighborhood_cells, 
                        CFG::DISTAL_SEGMENT_SIZE,
                        _active_column_count/2,
                        cla_rand(0,neighborhood_cells.size()-1));
            }
        }

        _prediction_level = calculate_prediction_potential();

        predictive_state = any_segments_active();

        axon = active_state;// or predictive_state;
    }

    void Cell::print_string()
    {
        cla_log("[");
        const char* color = ANSI_COLOR_RESET"%d";
        if (predictive_state)
        {
            color = ANSI_COLOR_CYAN"%d"ANSI_COLOR_RESET;
        }
        else if (active_state)
        {
            color = ANSI_COLOR_GREEN"%d"ANSI_COLOR_RESET;
        }
        else
        {
            color = ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET;
        }
        cla_log(color, active_state);
        cla_log("] {");

        for (unsigned int i = 0; i < distal_segments.size(); ++i)
        {
            color = ANSI_COLOR_RESET"%d";
            if (distal_segments[i]->is_learned())
            {
                if (distal_segments[i]->get_potential_state())
                {
                    color = ANSI_COLOR_CYAN"%d"ANSI_COLOR_RESET;
                }
                else
                {
                    color = ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET;
                }

                cla_log(color, distal_segments[i]->get_potential_state());
            }
            else
            {
                color = ANSI_COLOR_BLACK"%c"ANSI_COLOR_RESET;
                cla_log(color,'x');
            }
        }

        cla_log("}");
    }
}
