#include "CLA.h"
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

namespace CLA
{
    void Column::_init_fields()
    {
        _active_duty_history = new char[CFG::DUTY_HISTORY];
        _overlap_duty_history = new char[CFG::DUTY_HISTORY];
        _input_history = new int[CFG::INPUT_HISTORY];

        for (int i = 0; i < CFG::DUTY_HISTORY; ++i)
            _active_duty_history[i] = false;
        for (int i = 0; i < CFG::DUTY_HISTORY; ++i)
            _overlap_duty_history[i] = false;
        for (int i = 0; i < CFG::INPUT_HISTORY; ++i)
            _input_history[i] = -1;

        active_state = false;
        predictive_state = false;
        axon = predictive_state or active_state;

        boost = 1;

        cycles = 0;
        active_cycles = 0;

        active_duty_cycle = 1;
        overlap_duty_cycle = 1;
        most_likely_pattern = -1;

        _active_duty_cursor = 0;
        _overlap_duty_cursor = 0;
        _input_history_cursor = 0;

        _bursting = false;

    }

    Column::Column(vector<char*>& proximal_bits, vector<Cell*>& neighborhood_cells,
            int cell_count, int input_space_location, int active_column_count)
    {
        proximal_segment = new ProximalSegment(
                proximal_bits, 
                CFG::PROXIMAL_SEGMENT_SIZE, 
                input_space_location);
        for (int i = 0; i < cell_count; ++i)
        {
            cells.push_back(new Cell(
                    neighborhood_cells, 
                    CFG::DISTAL_SEGMENTS, 
                    CFG::DISTAL_SEGMENT_SIZE,
                    active_column_count,
                    -1)); // no topology
                    //input_space_location*cell_count));
        }

        _init_fields();
    }

    Column::Column(vector<char*>& proximal_bits,
            int cell_count,
            int input_space_location)
    {
        proximal_segment = new ProximalSegment(
                proximal_bits, 
                CFG::PROXIMAL_SEGMENT_SIZE,
                input_space_location);
        for (int i = 0; i < cell_count; ++i)
        {
            cells.push_back(new Cell());
        }

        _init_fields();
    }

    Column::~Column() 
    { 
        delete proximal_segment; 
        delete _active_duty_history;
        delete _overlap_duty_history;
        for (unsigned int i = 0; i < cells.size(); ++i)
        {
            delete cells[i];
        }
    }

    void Column::hook_up_distal_segments(vector<Cell*>& neighborhood_cells, 
            int active_column_count, int input_space_location)
    {
        for (unsigned int i = 0; i < cells.size(); ++i)
        {
            cells[i]->hook_up_distal_segments(neighborhood_cells, 
                    CFG::DISTAL_SEGMENTS, 
                    CFG::DISTAL_SEGMENT_SIZE,
                    active_column_count,
                    input_space_location);
        }
    }

    void Column::update_active_duty(char was_active)
    {
        float total = 0.0;
        _active_duty_history[_active_duty_cursor] = was_active;
        _active_duty_cursor = (_active_duty_cursor+1) % CFG::DUTY_HISTORY;
        for (int i = 0; i < min(CFG::DUTY_HISTORY, cycles); ++i)
            total += _active_duty_history[i] ? 1.0 : 0.0;
        active_duty_cycle = total / min(CFG::DUTY_HISTORY, cycles);
    }

    void Column::update_overlap_duty(char did_overlap)
    {
        float total = 0.0;
        _overlap_duty_history[_overlap_duty_cursor] = did_overlap;
        _overlap_duty_cursor = (_overlap_duty_cursor+1) % CFG::DUTY_HISTORY;

        for (int i = 0; i < min(CFG::DUTY_HISTORY, cycles); ++i)
            total += _overlap_duty_history[i] ? 1.0 : 0.0;
        overlap_duty_cycle = total / min(CFG::DUTY_HISTORY, cycles);
    }

    void Column::update_input_history(int input_pattern_index)
    {
        _input_history[_input_history_cursor] = input_pattern_index;
        _input_history_cursor = (_input_history_cursor+1) % CFG::INPUT_HISTORY;

        vector<int> tmp_history;
        for (int i = 0; i < min(CFG::INPUT_HISTORY, active_cycles); ++i)
        {
            tmp_history.push_back(_input_history[i]);
        }
        sort(tmp_history.begin(), tmp_history.end());

        int count = 0;
        int current = -1;
        int best = -1;
        int best_count = 0;

        for (unsigned int j = 0; j < tmp_history.size(); ++j)
        {
            if (tmp_history[j] != current and tmp_history[j] != -1)
            {
                count = 0;
                current = tmp_history[j];
            }

            count++;
            if (count > best_count)
            {
                best = current;
                best_count = count;
            }
        }

        most_likely_pattern = best;
    }

    int Column::get_potential()
    {
        int act = proximal_segment->get_potential();
        if (act < CFG::MINIMUM_PROXIMAL_ACTIVATION)
            act = 0;
        return act;
    }

    int Column::update_input()
    {
        cycles++;

        proximal_segment->update_input();
        int activation = proximal_segment->get_sum();

        if (activation < CFG::MINIMUM_PROXIMAL_ACTIVATION)
        {
            activation = 0;
        }

        update_overlap_duty(activation > 0);

        set_active_state(false);

        return int(activation * boost);
    }

    float Column::boost_function(float min_active_duty)
    {
        if (cycles < CFG::BEGIN_BOOST)
            return 1.0;

        if (active_duty_cycle > min_active_duty)
            return 1.0;
        else
        {
            // increases linearly with distance to min_active_duty
            float clearance = abs(min_active_duty-active_duty_cycle);
            return 1.0 + clearance * CFG::BOOST_AMOUNT;
        }
    }

    void Column::update_boost(float min_active_duty)
    {
        // update boost value
        boost = boost_function(min_active_duty);

        // update synapse strengths
        if (overlap_duty_cycle < min_active_duty)
        {
            proximal_segment->strengthen_by_scale_factor();
        }
    }

    void Column::update_state(
            vector<Cell*>& active_cells, 
            int input_pattern_index)
    {
        update_active_duty(active_state);
        proximal_segment->learn(active_state);

        Cell* strongest_predictor = 0;
        if (active_state)
        {
            active_cycles++;
            update_input_history(input_pattern_index);

            // find the best predictor
            if (not predictive_state)
            {
                _bursting = true;
                int best_prediction_level = -1;
                vector<Cell*> tmp = cells;
                random_shuffle(tmp.begin(), tmp.end());
                for (unsigned int i = 0; i < tmp.size(); ++i)
                {
                    if (tmp[i]->get_prediction_potential() > best_prediction_level
                            and tmp[i]->get_prediction_potential() > 0)
                    {
                        strongest_predictor = tmp[i];
                        best_prediction_level = tmp[i]->get_prediction_potential();
                    }
                }

                // no good predictors, just pick a random cell
                if (strongest_predictor == 0)
                {
                    strongest_predictor = cells[cla_rand(0,cells.size()-1)];
                }
            }
        }

        for (unsigned int i = 0; i < cells.size(); ++i)
        {
            if (active_state)
            {
                // if some cells were predicted
                if (predictive_state)
                {
                    char cell_predicted = cells[i]->predictive_state;
                    cells[i]->update_state(cell_predicted, cell_predicted, active_cells);
                }
                // bursting
                else
                {
                    cells[i]->update_state((char)true, (char)(cells[i]==strongest_predictor), active_cells);
                }
            }
            // just set all cells inactive
            else
            {
                cells[i]->update_state((char)false, (char)false, active_cells);
            }
        }
    }

    void Column::update_prediction(vector<Cell*>& active_cells, vector<Cell*>& neighborhood_cells)
    {
        // do temporal prediction in the cells in this column
        set_predictive_state(false);
        for (unsigned int i = 0; i < cells.size(); ++i)
        {
            cells[i]->update_prediction(active_cells, neighborhood_cells);

            if (cells[i]->predictive_state)
            {
                set_predictive_state(true);
            }
        }
    }

    void Column::set_active_state(char s)
    {
        active_state = s;
        axon = active_state or predictive_state;
    }

    void Column::set_predictive_state(char s)
    {
        predictive_state = s;
        axon = active_state or predictive_state;
    }

    void Column::print_string()
    {
        cla_log("=====Column=====\n");
        for (unsigned int i = 0; i < cells.size(); ++i)
        {
            cells[i]->print_string();
            cla_log("\n");
        }
        cla_log("----------------\n");
        cla_log("active_duty_cycle: %f\n", active_duty_cycle);
        cla_log("overlap_duty_cycle: %f\n", overlap_duty_cycle);
        cla_log("active_state: ");
        if (active_state)
            cla_log(ANSI_COLOR_GREEN"active\n"ANSI_COLOR_RESET);
        else
            cla_log(ANSI_COLOR_BLUE"inactive\n"ANSI_COLOR_RESET);
        cla_log("predictive_state: ");
        if (predictive_state)
            cla_log(ANSI_COLOR_CYAN"active\n"ANSI_COLOR_RESET);
        else
            cla_log(ANSI_COLOR_BLUE"inactive\n"ANSI_COLOR_RESET);
        cla_log("boost: %f\n", boost);
        cla_log("proximal_segment: ");
        proximal_segment->print_string();
        cla_log("\n================\n");
    }
}
