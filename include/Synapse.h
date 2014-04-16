#ifndef CLA_SYNAPSE_H
#define CLA_SYNAPSE_H

#include <vector>

namespace CLA
{
    class CFG;
    class DendriteSegment;
    class ProximalSegment;
    class DistalSegment;
    class Cell;
    class Column;
    class Region;
    class Cortex;


    class Synapse
    {
        public:
            Synapse(char* target_bit, 
                    int initial_strength);
            virtual ~Synapse();
            char get_state();
            char get_potential_state();
            void strengthen();
            void strengthen_by_scale_factor(float scale_factor);
            void weaken();
            bool is_connected();
            bool target_firing();
            bool fired_last_step();
            bool target_fired_last_step();
        protected:
            char* target;
            char _last_step;
            char _target_last_step;
            int strength;
    };

}

#endif
