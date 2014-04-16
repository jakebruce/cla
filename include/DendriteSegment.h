#ifndef CLA_DENDRITE_SEGMENT_H
#define CLA_DENDRITE_SEGMENT_H

#include <vector>

namespace CLA
{
    class Synapse;
    class CFG;
    class ProximalSegment;
    class DistalSegment;
    class Cell;
    class Column;
    class Region;
    class Cortex;


    class DendriteSegment
    {
        public:
            DendriteSegment();
            virtual ~DendriteSegment();

        protected:
            std::vector<Synapse*> synapses;
    };

}

#endif
