#include "CLA.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cstdarg>
#include <cstdio>

using namespace std;

namespace CLA
{
    CFG::CFG() {}
    CFG::~CFG() {}

    int CFG::INITIAL_STRENGTH = 100;
    int CFG::CONNECTION_THRESHOLD = 150;
    int CFG::CONNECTION_DELTA_POS = 19;
    int CFG::CONNECTION_DELTA_NEG = 20;
    float CFG::CONNECTION_SCALE_DELTA = 0.1;

    int CFG::PROXIMAL_SEGMENT_SIZE = 128;
    int CFG::MINIMUM_PROXIMAL_ACTIVATION = 4;

    int CFG::DISTAL_SEGMENT_SIZE = 32;
    int CFG::DISTAL_SEGMENTS = 128;

    float CFG::DESIRED_SDR_DENSITY = 0.02;

    int CFG::BEGIN_BOOST = 500;
    float CFG::BOOST_AMOUNT = 10000.0;
    float CFG::MIN_DUTY_PERCENT = 0.01;

    int CFG::DUTY_HISTORY = 1000;
    int CFG::INPUT_HISTORY = 1000;

    char CFG::DEBUG = true;

    int CFG::COLUMN_VIZ = -1;
    bool CFG::REGION_VIZ = false;
    bool CFG::OUTPUT_VIZ = false;
    bool CFG::ACTIVE_VIZ = false;
    bool CFG::PREDICTION_VIZ = false;

    bool _rand_is_init = false;

    int cla_rand(int min, int max)
    {
        if (not _rand_is_init)
        {
            cla_log("cla_rand: initializing seed with %ld\n", time(0));
            srand(time(0));
            _rand_is_init = true;
        }

        return rand() % (max-min+1) + min;
    }

    void cla_log(const char *message, ...)
    {
        // only print stuff if we're in debug mode
        if (not CFG::DEBUG) return;

        va_list argptr;
        va_start(argptr, message);

        vprintf(message, argptr);

        va_end(argptr);    
    }
    
    bool _hookup_init = false;
    int* _hookup_counts;
    int _input_size;
    void cla_register_hookup(int bit_index, int input_size)
    {
        if (not _hookup_init)
        {
            _hookup_init = true;
            _hookup_counts = new int[input_size];
            for (int i = 0; i < input_size; ++i)
                _hookup_counts[i] = 0;
        }

        _hookup_counts[bit_index]++;
        _input_size = input_size;
    }

    void cla_print_hookups()
    {
        if (not _hookup_init)
            cla_log("WARNING: trying to print hookups before hooking up any synapses\n");

        for (int i = 0; i < _input_size; ++i)
            cla_log("%d ", _hookup_counts[i]);
    }
}
