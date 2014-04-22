#ifndef HTM_CLA_H
#define HTM_CLA_H

#include "Cell.h"
#include "Cortex.h"
#include "Column.h"
#include "Region.h"
#include "Synapse.h"
#include "DendriteSegment.h"
#include "ProximalSegment.h"
#include "DistalSegment.h"
#include "Classifier.h"

#define ANSI_COLOR_BLACK   "\x1b[30m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#include <cstdarg>
#include <vector>

namespace CLA
{
    class CFG
    {
        public:
          CFG();
          ~CFG();

          static int REGIONS;

          static float BRANCHING_FACTOR;

          static int COLUMNS;
          static int COLUMN_DEPTH;

          static int INITIAL_STRENGTH;
          static int CONNECTION_THRESHOLD;
          static int CONNECTION_DELTA_NEG;
          static int CONNECTION_DELTA_POS;
          static float CONNECTION_SCALE_DELTA;

          static int PROXIMAL_SEGMENT_SIZE;
          static int MINIMUM_PROXIMAL_ACTIVATION;

          static int DISTAL_SEGMENT_SIZE;
          static int DISTAL_SEGMENTS;

          static float DESIRED_SDR_DENSITY;

          static int BEGIN_BOOST;
          static float BOOST_AMOUNT;
          static float MIN_DUTY_PERCENT;

          static int DUTY_HISTORY;
          static int INPUT_HISTORY;

          static char DEBUG;

          /* 0 column prediction
           * 1 cell prediction
           * 2 distalsegment prediction
           */
          static int PREDICTION_MODE;

          static int COLUMN_VIZ;
          static bool REGION_VIZ;
          static bool ACTIVE_VIZ;
          static bool OUTPUT_VIZ;
          static bool PREDICTION_VIZ;
    };

    // random generator seeded once on demand
    int cla_rand(int min, int max);

    // logging debug info
    void cla_log(const char* message, ...);

    // for debugging, to make sure we are evenly distributed across the inputs
    void cla_register_hookup(int bit_index, int input_size);
    void cla_print_hookups();
}

#endif
