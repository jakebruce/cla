#ifndef CLA_CORTEX_H
#define CLA_CORTEX_H

#include <vector>

namespace CLA
{
    class Synapse;
    class DendriteSegment;
    class ProximalSegment;
    class DistalSegment;
    class Cell;
    class Column;
    class Region;
    class CFG;


    class Cortex
    {
        public:
            // fixed depth, branching factor
            Cortex(int input_size,  // size of raw input
                int region_count,         // hierarchy depth
                int bottom_region_size, // size of the lowest level in the hierarchy
                float branching_factor, // ratio of child region to parent region size
                int column_depth,
                int learning_positive,
                int learning_negative,
                int proximal_segment_size,
                int distal_segment_size,
                int distal_segments,
                float sdr_density,
                int prediction_mode,
                int column_viz,
                bool output_viz,
                bool region_viz,
                bool prediction_viz,
                bool active_viz
                );

            // specific hierarchy
            Cortex(std::vector<int> region_sizes, // in order of ascending the hierarchy
                    int input_size,
                    int column_depth);

            virtual ~Cortex();

            // stream some input pattern into the cortex and returns the anomaly score
            float update(std::vector<char>& input);

            float get_precision();
            float get_recall();

            int get_actual_input_class();
            int get_predicted_input_class();

            std::vector<int> get_representation(std::vector<char>& input);
        protected:
            std::vector<Region*> regions;
            char* input_bits;
            unsigned int input_count;

            // convenience methods called by update()
            void update_input(std::vector<char>& input);
            float update_state();
    };

}

#endif
