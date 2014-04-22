#ifndef CLA_CELL_H
#define CLA_CELL_H

#include <vector>

namespace CLA
{   
    class Synapse;
    class DendriteSegment;
    class ProximalSegment;
    class DistalSegment;
    class Column;
    class Region;
    class Cortex;
    class CFG;

    // The primary unit of temporal prediction

    class Cell
    {
        public:
            Cell(std::vector<Cell*>& neighborhood_cells,
                    int distal_segment_count,
                    int segment_size,
                    int active_column_count,
                    int input_space_location);
            // hook up distal segments later
            Cell();
            virtual ~Cell();

            // hook up distal segments to bits
            void hook_up_distal_segments(std::vector<Cell*>& neighborhood_cells,
                    int distal_segment_count,
                    int segment_size,
                    int active_column_count,
                    int input_space_location);

            // this is called to reset the cell on each time step
            void update_input();

            void update_input_history(int pattern_index);

            // can't update on input for a cell (they share a column's prox_seg)
            // but we still have to do temporal prediction step after feedforward
            void update_state(char fire, char learn, std::vector<Cell*>& active_cells, int pattern_index);

            // make the prediction for next step. This should be done last.
            void update_prediction(std::vector<Cell*>& active_cells, std::vector<Cell*>& neighborhood_cells);

            int get_prediction_potential();

            // takes on the union of active_state and predictive_state 
            char axon;

            // this cell is active from predicted (or bursting) feed-forward input
            char active_state;
            // this cell predicts its activity in the next time step
            char predictive_state;
            // this cell was the strongest predictor of the input, so we'll use
            // it as the input to learning stages of distal dendrites connected
            // to it, if (learn_state)
            char learn_state;

            void print_string();

            int most_likely_input;
        protected:
            ProximalSegment* proximal_segment;
            std::vector<DistalSegment*> distal_segments;

            int calculate_prediction_potential();
            char any_segments_active();

            DistalSegment* get_best_segment();

            void _init_fields();

            int _prediction_level;
            int _active_column_count;

            int* _input_history;
            int _input_history_cursor;

            void calculate_input_vote();

            std::vector<int> _active_segments;
    };

}

#endif
