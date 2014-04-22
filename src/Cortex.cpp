#include "CLA.h"
#include <vector>
#include <cmath>

using namespace std;

namespace CLA
{
    // fixed depth, hierarchy generated via branching factor
    Cortex::Cortex(int input_size,
            int region_count,
            int bottom_region_size,
            float branching_factor,
            int column_depth,
            int learning_positive,
            int learning_negative,
            int proximal_segment_size,
            int distal_segment_size,
            int distal_segments,
            float sdr_density,
            int prediction_mode,
            int column_viz,
            bool region_viz,
            bool output_viz,
            bool prediction_viz,
            bool active_viz)
    {
        input_bits = new char[input_size];
        vector<char*> region_input;

        CFG::CONNECTION_DELTA_POS = learning_positive;
        CFG::CONNECTION_DELTA_NEG = learning_negative;

        CFG::PROXIMAL_SEGMENT_SIZE = proximal_segment_size;
        CFG::DISTAL_SEGMENT_SIZE = distal_segment_size;
        CFG::DISTAL_SEGMENTS = distal_segments;

        CFG::DESIRED_SDR_DENSITY = sdr_density;

        CFG::PREDICTION_MODE = prediction_mode;

        CFG::COLUMN_VIZ = column_viz;
        CFG::REGION_VIZ = region_viz;
        CFG::OUTPUT_VIZ = output_viz;
        CFG::PREDICTION_VIZ = prediction_viz;
        CFG::ACTIVE_VIZ = active_viz;

        cla_log("Cortex::Cortex: setting up %d bits of input to Cortex\n", input_size);
        for (int i = 0; i < input_size; ++i)
        {
            input_bits[i] = false;
            region_input.push_back(&(input_bits[i]));
        }
        
        cla_log("Cortex::Cortex: starting at bottom level Region of %d Columns\n", bottom_region_size);
        int region_size = bottom_region_size;
        for (int i = 0; i < region_count; ++i)
        {
            cla_log("Cortex::Cortex: setting up Region of %d Columns with %d feedforward inputs\n", region_size, region_input.size());
            regions.push_back(new Region(region_size, region_input,
                    column_depth, CFG::DESIRED_SDR_DENSITY));

            region_input = regions[regions.size()-1]->get_axons();

            // keep at least 10 columns in each region
            region_size = max(int(region_size/branching_factor), 10);
        }

        input_count = input_size;

        cla_log("Cortex::Cortex: synapse hookup distribution over input bits:\n");
        cla_print_hookups();
        cla_log("\n");
    }

    // specified hierarchy
    Cortex::Cortex(vector<int> region_sizes,
            int input_size,
            int column_depth)
    {
        vector<char*> region_input;
        input_bits = new char[input_size];

        cla_log("Cortex::Cortex: setting up %d bits of input to Cortex\n", input_size);
        for (int i = 0; i < input_size; ++i)
        {
            input_bits[i] = false;
            region_input.push_back(&(input_bits[i]));
        }

        for (unsigned int i = 0; i < region_sizes.size(); ++i)
        {
            cla_log("Cortex::Cortex: setting up Region of %d Columns with %d feedforward inputs\n", 
                    region_sizes[i], region_input.size());
            regions.push_back(new Region(region_sizes[i], region_input,
                        column_depth, CFG::DESIRED_SDR_DENSITY));
            region_input = regions[regions.size()-1]->get_axons();
        }

        input_count = input_size;
    }

    Cortex::~Cortex() 
    {
        for (unsigned int i = 0; i < regions.size(); ++i)
            delete regions[i];
        delete input_bits;
    }

    float Cortex::update(std::vector<char>& input)
    {
        //cla_log("Cortex::update: feeding %d bits into Cortex\n", input.size());
        update_input(input);
        return update_state();
    }

    void Cortex::update_input(std::vector<char>& input)
    {
        for (unsigned int i = 0; i < input.size() and i < input_count; ++i)
        {
            input_bits[i] = input[i];
        }

        for (unsigned int i = 0; i < regions.size(); ++i)
        {
            regions[i]->update_input();
        }
    }

    float Cortex::update_state()
    {
        float anomaly = 0;
        for (unsigned int i = 0; i < regions.size(); ++i)
        {
            anomaly += regions[i]->update_state();
        }

        return float(anomaly)/float(regions.size());
    }

    float Cortex::get_precision()
    {
        float precision = 0;

        for (unsigned int i = 0; i < regions.size(); ++i)
            precision += regions[i]->get_precision();

        return precision / regions.size();
    }

    float Cortex::get_recall()
    {
        float recall = 0;

        for (unsigned int i = 0; i < regions.size(); ++i)
            recall += regions[i]->get_recall();

        return recall / regions.size();
    }

    int Cortex::get_actual_input_class()
    {
        return regions[0]->get_actual_input_class();
    }

    int Cortex::get_predicted_input_class()
    {
        return regions[0]->get_predicted_input_class();
    }

    vector<int> Cortex::get_representation(vector<char>& input)
    {
        char temp_bits[input_count];
        for (unsigned int i = 0; i < input_count; ++i)
            temp_bits[i] = input_bits[i];

        for (unsigned int i = 0; i < input_count; ++i)
            input_bits[i] = input[i];

        vector<int> rep = regions[0]->get_representation();

        for (unsigned int i = 0; i < input_count; ++i)
            input_bits[i] = temp_bits[i];

        return rep; 
    }
}
