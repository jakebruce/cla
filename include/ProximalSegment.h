#ifndef CLA_PROXIMAL_SEGMENT_H
#define CLA_PROXIMAL_SEGMENT_H

#include <vector>

namespace CLA
{

    class ProximalSegment : public DendriteSegment
    {
        public:
            ProximalSegment(std::vector<char*>& input_bits,
                    int num_synapses,
                    int input_space_location = -1);
            virtual ~ProximalSegment();

            // compute the activation of the segment
            void update_input();
            // return the precomputed activation level
            int get_sum();
            // strengthen active synapses, weaken inactive ones
            void learn(char column_state);
            // strengthen by CONNECTION_SCALE_DELTA factor
            void strengthen_by_scale_factor();
            void print_string();

            // nondestructive
            int get_potential();
        protected:
            int activation_level;
    };

}


#endif
