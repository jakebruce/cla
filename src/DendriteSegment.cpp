#include "CLA.h"

namespace CLA
{
    DendriteSegment::DendriteSegment() {}

    DendriteSegment::~DendriteSegment() 
    {
        for (unsigned int i = 0; i < synapses.size(); ++i)
            delete synapses[i];
    }
}
