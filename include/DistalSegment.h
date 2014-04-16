#ifndef CLA_DISTAL_SEGMENT_H
#define CLA_DISTAL_SEGMENT_H

#include <vector>

namespace CLA
{ 
    class Synapse;
    class DendriteSegment;
    class ProximalSegment;
    class CFG;
    class Cell;
    class Column;
    class Region;
    class Cortex;


    class DistalSegment : public DendriteSegment
    {
        public:
            DistalSegment(std::vector<Cell*>& input_bits, 
                    int num_synapses, 
                    int threshold,
                    int input_space_location);
            virtual ~DistalSegment();

            void grow(std::vector<Cell*>& active_cells);
            void learn(bool correct_prediction);
            char get_state();
            char get_potential_state();
            int get_potential();
            bool is_learned();

            // flag indicating whether this synapse should be forgotten
            char dead;
        protected:
            std::vector<Cell*> potential_synapses;
            char learned;
            int threshold;
    };

}

#endif
