#include "CLA.h"
#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>
#include <fstream>

using namespace std;
using namespace CLA;

int INPUT_SIZE = 512;

int REGIONS = 1;
int BRANCHING_FACTOR = 2;
int COLUMNS_BOTTOM = 512;

int COLUMN_DEPTH = 8;

int BLOB = 16;
int SLIDING_RATE = 5;

int MOVING_AVERAGE_WINDOW = 50;

int LEARNING_POSITIVE = 10;
int LEARNING_NEGATIVE = 15;

int DISTAL_SEGMENTS = 128;
int DISTAL_SEGMENT_SIZE = 32;
int PROXIMAL_SEGMENT_SIZE = 128;

float SDR_DENSITY = 0.02;

int PREDICTION_MODE = 2;

int COLUMN_VIZ = -1;
bool INPUT_VIZ = false;
bool REGION_VIZ = false;
bool OUTPUT_VIZ = false;
bool PREDICTION_VIZ = false;
bool ACTIVE_VIZ = false;
bool STABLE_INPUT_VIZ = false;

bool ALTERNATING = false;

bool STEP = false;

typedef unsigned long long timestamp_t;
static timestamp_t get_timestamp()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_usec + (timestamp_t) now.tv_sec * 1000000;
}

int main(int argc, char* argv[])
{
    char* savefile = 0;
    char* loadfile = 0;

    for (int i = 0; i < argc; ++i)
    {
        if (string("--prediction_mode").compare(argv[i])==0)
        {
            PREDICTION_MODE = atoi(argv[i+1]);
        }

        if (string("--load").compare(argv[i])==0)
        {
            loadfile = argv[i+1];
        }

        if (string("--input_size").compare(argv[i])==0)
        {
            INPUT_SIZE = atoi(argv[i+1]);
        }

        if (string("--column_viz").compare(argv[i])==0)
        {
            COLUMN_VIZ = atoi(argv[i+1]);
        }

        if (string("--regions").compare(argv[i])==0)
        {
            REGIONS = atoi(argv[i+1]);
        }

        if (string("--branching_factor").compare(argv[i])==0)
        {
            BRANCHING_FACTOR = atoi(argv[i+1]);
        }

        if (string("--columns").compare(argv[i])==0)
        {
            COLUMNS_BOTTOM = atoi(argv[i+1]);
        }
        
        if (string("--column_depth").compare(argv[i])==0)
        {
            COLUMN_DEPTH = atoi(argv[i+1]);
        }
        
        if (string("--blob").compare(argv[i])==0)
        {
            BLOB = atoi(argv[i+1]);
        }
        
        if (string("--sliding_rate").compare(argv[i])==0)
        {
            SLIDING_RATE = atoi(argv[i+1]);
        }
        
        if (string("--window_size").compare(argv[i])==0)
        {
            MOVING_AVERAGE_WINDOW = atoi(argv[i+1]);
        }
        
        if (string("--learning_positive").compare(argv[i])==0)
        {
            LEARNING_POSITIVE = atoi(argv[i+1]);
        }

        if (string("--learning_negative").compare(argv[i])==0)
        {
            LEARNING_NEGATIVE = atoi(argv[i+1]);
        }

        if (string("--distal_segments").compare(argv[i])==0)
        {
            DISTAL_SEGMENTS = atoi(argv[i+1]);
        }

        if (string("--distal_size").compare(argv[i])==0)
        {
            DISTAL_SEGMENT_SIZE = atoi(argv[i+1]);
        }

        if (string("--proximal_size").compare(argv[i])==0)
        {
            PROXIMAL_SEGMENT_SIZE = atoi(argv[i+1]);
        }

        if (string("--sdr_density").compare(argv[i])==0)
        {
            SDR_DENSITY = atof(argv[i+1]);
        }

        if (string("--save").compare(argv[i])==0)
        {
            savefile = argv[i+1];
        }

        if (string("--region_viz").compare(argv[i])==0)
        {
            REGION_VIZ = true;
        }

        if (string("--output_viz").compare(argv[i])==0)
        {
            OUTPUT_VIZ = true;
        }

        if (string("--prediction_viz").compare(argv[i])==0)
        {
            PREDICTION_VIZ = true;
        }

        if (string("--active_viz").compare(argv[i])==0)
        {
            ACTIVE_VIZ = true;
        }

        if (string("--stable_viz").compare(argv[i])==0)
        {
            STABLE_INPUT_VIZ = true;
        }

        if (string("--input_viz").compare(argv[i])==0)
        {
            INPUT_VIZ = true;
        }

        if (string("--step").compare(argv[i])==0)
        {
            STEP = true;
        }
 
        if (string("--alternating").compare(argv[i])==0)
        {
            ALTERNATING = true;
        }

        if (string("--help").compare(argv[i])==0
                or string("-h").compare(argv[i])==0)
        {
            cout << "usage: " << argv[0] << " [options]" << endl
                << "options: " << endl
                << "--load <file: one integer from 0 to 255 per line>" << endl
                << "--columns <number of cortical columns> [def: 512]" << endl
                << "--columns_depth <cells per column> [def: 8]" << endl
                << "--learning_positive <pos reinforce rate> [def: 10]" << endl
                << "--learning_negative <neg reinforce rate> [def: 15]" << endl
                << "--distal_segments <num> [def: 128]" << endl
                << "--distal_size <synapses per segment> [def: 32]" << endl
                << "--proximal_size <synapses per segment> [def: 128]" << endl
                << "--sdr_density <percentage of columns to activate> [def: 0.02]" << endl
                << "--save <file: save running stats to this prefix>" << endl
                << "--prediction_mode < 0:Column | 1:Cell | 2:DistalSegment >" << endl
                << "--region_viz [show the region state]" << endl
                << "--output_viz [show the output to a higher region]" << endl
                << "--prediction_viz [show the expected next input" << endl
                << "--active_viz [show just the columns that are active]" << endl
                << "--stable_viz [show the network's response to a stable input]" << endl
                << "--input_viz [show the SDR of each input]" << endl
                << "--step [wait for enter key between steps]" << endl
                << "--column_viz <column index, be careful>" << endl;
            return 0;
        }
    }

    ofstream anomalyfile;
    ofstream precisionfile;
    ofstream recallfile;
    ofstream actualfile;
    ofstream predictedfile;
    ofstream timefile;

    ifstream infile;

    printf("Testing the Cortex. Here we go!\n");
    printf("==========BUILDING CORTEX==========\n");
    Cortex cortex(
        INPUT_SIZE, // size of input in bits
        REGIONS, // number of regions
        COLUMNS_BOTTOM, // number of columns in bottom level
        BRANCHING_FACTOR, // the next region up will be this much smaller
        COLUMN_DEPTH, // number of cells per column
        LEARNING_POSITIVE,
        LEARNING_NEGATIVE,
        PROXIMAL_SEGMENT_SIZE,
        DISTAL_SEGMENT_SIZE,
        DISTAL_SEGMENTS,
        SDR_DENSITY,
        PREDICTION_MODE,
        COLUMN_VIZ,
        REGION_VIZ,
        OUTPUT_VIZ,
        PREDICTION_VIZ,
        ACTIVE_VIZ
        );

    printf("==========DONE BUILDING CORTEX==========\n");

    float anomaly = 0;
    float precision = 0;
    float recall = 0;
    vector<char> input;
    for (int i = 0; i < INPUT_SIZE; ++i)
    {
        input.push_back((char)(i-BLOB/2<0));
    }

    float anomalywindow[MOVING_AVERAGE_WINDOW];
    for (int i = 0; i < MOVING_AVERAGE_WINDOW; ++i)
        anomalywindow[i] = 1.0;

    float precisionwindow[MOVING_AVERAGE_WINDOW];
    for (int i = 0; i < MOVING_AVERAGE_WINDOW; ++i)
        precisionwindow[i] = 0.0;

    float recallwindow[MOVING_AVERAGE_WINDOW];
    for (int i = 0; i < MOVING_AVERAGE_WINDOW; ++i)
        recallwindow[i] = 0.0;

    if (savefile)
    {
        anomalyfile.open((string(savefile)+".anomaly").c_str());
        precisionfile.open((string(savefile)+".precision").c_str());
        recallfile.open((string(savefile)+".recall").c_str());
        actualfile.open((string(savefile)+".actual_input").c_str());
        predictedfile.open((string(savefile)+".predicted_input").c_str());
        timefile.open((string(savefile)+".time").c_str());
    }

    char stable_chars[512] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
        0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
        0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0
    };
    int stable_size = 512;

    vector<char> stable_input;
    for (int i = 0; i < INPUT_SIZE; ++i)
    {
        if (i < stable_size)
            stable_input.push_back(stable_chars[i]);
        else
            stable_input.push_back(0);
    }

    int cycle = 0;
    for (int outer = 0; outer < 1000; ++outer) 
    {
        int d = 1;
        for (int inc = 0; inc >= 0; inc+=d*SLIDING_RATE)
        {
            for (int i = 0; i < INPUT_SIZE; ++i)
            {
                input[i] = (char) ((i>inc-BLOB/2)and(i<inc+BLOB/2));
            }

            if (INPUT_VIZ)
            {
                printf("=====Input=====\n");
                for (int i = 0; i < INPUT_SIZE; ++i)
                {
                    if (input[i])
                        printf(ANSI_COLOR_YELLOW"%d"ANSI_COLOR_RESET,input[i]);
                    else
                        printf(ANSI_COLOR_MAGENTA"%d"ANSI_COLOR_RESET,input[i]);
                    if ((i+1) % int(sqrt(INPUT_SIZE)*1.5) == 0)
                        printf("\n");
                }
                printf("\n================\n");
                printf("\n");
            }

            if (STABLE_INPUT_VIZ)
            {
                vector<int> active_columns = cortex.get_representation(stable_input);

                char outprint[INPUT_SIZE];
                for (int i = 0; i < INPUT_SIZE; ++i)
                    outprint[i] = 0;

                for (unsigned int i = 0; i < active_columns.size(); ++i)
                    outprint[active_columns[i]] = 1;

                printf("===Representation of Stable Input===\n");
                for (int i = 0; i < INPUT_SIZE; ++i)
                {
                    if (outprint[i])
                        printf(ANSI_COLOR_GREEN"%d"ANSI_COLOR_RESET,outprint[i]);
                    else
                        printf(ANSI_COLOR_BLUE"%d"ANSI_COLOR_RESET,outprint[i]);
                    if ((i+1)%int(sqrt(INPUT_SIZE)*1.5)==0)
                        printf("\n");
                }
                printf("\n================\n");
                printf("\n");
            }

            printf("epoch %d, cycle %d\n", outer, cycle);

            int predicted_input_class = cortex.get_predicted_input_class();

            timestamp_t t0 = get_timestamp();
            anomalywindow[cycle%MOVING_AVERAGE_WINDOW] = cortex.update(input);
            timestamp_t t1 = get_timestamp();

            double ms = (t1-t0)/1000.0L;
            printf("Cortex.update() took %lf ms\n", ms);
            
            precisionwindow[cycle%MOVING_AVERAGE_WINDOW] = cortex.get_precision();
            recallwindow[cycle%MOVING_AVERAGE_WINDOW] = cortex.get_recall();

            anomaly = 0;
            for (int i = 0; i < MOVING_AVERAGE_WINDOW; ++i)
                anomaly += anomalywindow[i];
            anomaly /= MOVING_AVERAGE_WINDOW;

            precision = 0;
            for (int i = 0; i < MOVING_AVERAGE_WINDOW; ++i)
                precision += precisionwindow[i];
            precision /= MOVING_AVERAGE_WINDOW;

            recall = 0;
            for (int i = 0; i < MOVING_AVERAGE_WINDOW; ++i)
                recall += recallwindow[i];
            recall /= MOVING_AVERAGE_WINDOW;

            int actual_input_class = cortex.get_actual_input_class();

            if (savefile)
            {
                anomalyfile << cycle << " " << anomaly << endl;
                precisionfile << cycle << " " << precision << endl;
                recallfile << cycle << " " << recall << endl;

                actualfile << cycle 
                    << " " << actual_input_class << endl;
                predictedfile << cycle 
                    << " " << predicted_input_class << endl;

                timefile << cycle
                    << " " << ms << endl;
            }

            cycle++;

            if (inc >= INPUT_SIZE-1)
            {
                if (ALTERNATING)
                    d=-1;
                else
                    break;
            }

            if (STEP)
                cin.ignore();
        }
    }
    if (savefile)
    {
        anomalyfile.close();
        precisionfile.close();
        recallfile.close();
        actualfile.close();
        predictedfile.close();
        timefile.close();
    }
    return 0;
}
