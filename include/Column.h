#ifndef CLA_COLUMN_H
#define CLA_COLUMN_H

#include <vector>

namespace CLA
{
    class Synapse;
    class DendriteSegment;
    class ProximalSegment;
    class DistalSegment;
    class Cell;
    class Region;
    class Cortex;
    class CFG;

    // The primary unit of spatial pooling

    class Column
    {
        public:
            Column(std::vector<char*>& proximal_bits,
                    std::vector<Cell*>& neighborhood_cells,
                    int cell_count, 
                    int input_space_location,
                    int active_column_count);

            // doesn't hook up distal segments yet
            Column(std::vector<char*>& proximal_bits, 
                    int cell_count, 
                    int input_space_location);
            
            virtual ~Column();

            // call this if you used the second constructor above,
            // once you've got a bunch of input bits to choose from
            void hook_up_distal_segments(std::vector<Cell*>& neighborhood_cells,
                    int active_column_count, int input_space_location);

            std::vector<Cell*>& get_cells() {return cells;}

            // get the potential feedforward activation of this column
            int get_potential();

            // process the input and return the feedforward activation of this column
            int update_input();

            // update the boost level based on the max
            // duty cycle of columns in the region
            void update_boost(float min_active_duty);

            // update the column's state after we choose winners and stuff
            void update_state(std::vector<Cell*>& active_cells,
                    int input_pattern_index);

            // do temporal prediction after all states are set
            void update_prediction(std::vector<Cell*>& active_cells,
                    std::vector<Cell*>& neighborhood_cells);

            // get the current active duty cycle
            float get_active_duty() { return active_duty_cycle; }

            char is_bursting() { return _bursting; }

            // collective output of the column.
            // It's not actually an axon, but represents the output
            // of a column to the temporal prediction step
            char axon;
            char active_state;
            char predictive_state;

            void set_active_state(char s);
            void set_predictive_state(char s);

            float boost;

            void print_string();

            // for voting on predicted input
            int most_likely_pattern;
        protected:
            ProximalSegment* proximal_segment;
            std::vector<Cell*> cells;
            float boost_function(float min_active_duty);

            char _bursting;

            int cycles;
            int active_cycles;

            float active_duty_cycle;
            float overlap_duty_cycle;

            char* _active_duty_history;
            char* _overlap_duty_history;
            int* _input_history;

            int _active_duty_cursor;
            int _overlap_duty_cursor;
            int _input_history_cursor;

            void update_overlap_duty(char did_overlap);
            void update_active_duty(char was_active);
            void update_input_history(int input_pattern_index);
            void calculate_input_vote();

            void _init_fields();
    };

}

#endif
