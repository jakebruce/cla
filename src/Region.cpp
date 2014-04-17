#include "CLA.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdio>

using namespace std;

namespace CLA
{
    Region::Region(int column_count, vector<char*>& input_bits,
            int column_depth,
            float fraction_columns_active)
    {
        input = input_bits;
        classifier = new Classifier();
        _last_prediction = -1;

        _precision = 0;
        _recall = 0;
        _actual_input_class = 0;
        _predicted_input_class = 0;

        active_column_count = int(fraction_columns_active * column_count);

        for (int i = 0; i < column_count; ++i)
        {
            int input_space_location = cla_rand(0,input_bits.size()-1);
            columns.push_back(new Column(input_bits, 
                        column_depth, 
                        input_space_location));

            axons.push_back(&(columns[columns.size()-1]->axon));
        }

        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            for (unsigned int j = 0; j < columns[i]->get_cells().size(); ++j)
            {
                // predictions only operate on the active state of the cells,
                // although upper regions take as input the union of predictions
                // and actives.
                neighborhood_cells.push_back(columns[i]->get_cells()[j]);
            }
        }

        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            cla_log("%d---", i);
            if (i%100==0) cla_log("\n");
            columns[i]->hook_up_distal_segments(neighborhood_cells, 
                    active_column_count,
                    -1); // no topology on distal segments
        }
        cla_log("\n");
    }

    Region::~Region() 
    {
        delete classifier;
    }

    vector<char*>& Region::get_axons()
    {
        return axons;
    }

    typedef struct
    {
        Column* column;
        int activation;
        int idx;
    } column_activation_t;

    bool compare_column_activation(column_activation_t a, column_activation_t b)
    {
        // sort in descending order
        return a.activation > b.activation;
    }   

    void Region::update_input()
    {
        vector<column_activation_t> col_act;

        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            column_activation_t ca;
            ca.column = columns[i];
            ca.activation = columns[i]->update_input();
            ca.idx = i;

            if (ca.activation > 0)
            {
                col_act.push_back(ca);
            }
        }

        // find strongest columns
        sort(col_act.begin(), col_act.end(), compare_column_activation);

        // do inhibition
        active_column_indices.clear();
        for (int j = 0; j < active_column_count and j < int(col_act.size()); ++j)
        {
            col_act[j].column->set_active_state(true);
            active_column_indices.push_back(col_act[j].idx);
        }
    }

    vector<int>& Region::get_active_columns()
    {
        return active_column_indices;
    }

    float Region::compute_anomaly_score()
    {
        float anomaly = 0;
        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            if (columns[i]->active_state and not columns[i]->predictive_state)
                anomaly += 1.0;
        }
        return anomaly;
    }

    void Region::visualize_active()
    {
        cla_log("=====Active Columns=====\n");
        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            const char* color = ANSI_COLOR_RESET"%d";
            if (int(i)==CFG::COLUMN_VIZ)
            {
                color = ANSI_COLOR_YELLOW"%d";
            }
            else if (columns[i]->active_state and columns[i]->predictive_state){
                color = ANSI_COLOR_GREEN"%d";
            }
            else if (columns[i]->active_state and not columns[i]->predictive_state){
                color = ANSI_COLOR_GREEN"%d";
            }
            else if (not columns[i]->active_state and columns[i]->predictive_state){
                color = ANSI_COLOR_BLUE"%d";
            }
            else if (not columns[i]->active_state and not columns[i]->predictive_state){
                color = ANSI_COLOR_BLUE"%d";
            }
            cla_log(color,int(columns[i]->active_state));
            if ((i+1) % int(sqrt(columns.size())*1.5) == 0)
                cla_log("\n");
        }
        cla_log(ANSI_COLOR_RESET"\n=================\n");

    }
    void Region::visualize_region()
    {
        cla_log("=======Region=======\n");
        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            const char* color = ANSI_COLOR_RESET"%d";
            if (int(i)==CFG::COLUMN_VIZ)
            {
                color = ANSI_COLOR_YELLOW"%d";
            }
            else if (columns[i]->active_state and columns[i]->predictive_state){
                color = ANSI_COLOR_GREEN"%d";
            }
            else if (columns[i]->active_state and not columns[i]->predictive_state){
                color = ANSI_COLOR_RED"%d";
            }
            else if (not columns[i]->active_state and columns[i]->predictive_state){
                color = ANSI_COLOR_CYAN"%d";
            }
            else if (not columns[i]->active_state and not columns[i]->predictive_state){
                color = ANSI_COLOR_BLUE"%d";
            }
            cla_log(color,int(columns[i]->active_state));
            if ((i+1) % int(sqrt(columns.size())*1.5) == 0)
                cla_log("\n");
        }
        cla_log(ANSI_COLOR_RESET"\n=================\n");
    }

    void Region::visualize_output()
    {
        cla_log("======Region Output======\n");
        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            const char* color = ANSI_COLOR_RESET"%d";
            if (int(i)==CFG::COLUMN_VIZ)
            {
                color = ANSI_COLOR_YELLOW"%d";
            }
            else if (columns[i]->axon)
                color = ANSI_COLOR_GREEN"%d";
            else
                color = ANSI_COLOR_BLUE"%d";
            cla_log(color,int(columns[i]->axon));
            if ((i+1)%int(sqrt(columns.size())*1.5) == 0)
                cla_log("\n");
        }
        cla_log(ANSI_COLOR_RESET"\n=================\n");
    }

    void Region::visualize_prediction(vector<char>& prediction)
    {
        cla_log("======Region Prediction======\n");
        for (unsigned int i = 0; i < prediction.size(); ++i)
        {
            const char* color = ANSI_COLOR_RESET"%d";
            if (prediction[i])
                color = ANSI_COLOR_CYAN"%d";
            else
                color = ANSI_COLOR_MAGENTA"%d";
            cla_log(color,int(prediction[i]));
            if ((i+1)%int(sqrt(prediction.size())*1.5) == 0)
                cla_log("\n");
        }
        cla_log(ANSI_COLOR_RESET"\n================\n");
    }

    // the ratio of active_predicted to predicted_total
    float Region::calculate_precision()
    {
        float active = 0;
        float predicted = 1;
        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            if (columns[i]->predictive_state)
            {
                predicted += 1.0;
                if (columns[i]->active_state)
                    active += 1.0;
            }
        }

        return active / predicted;
    }

    // the ratio of active_predicted to active_total
    float Region::calculate_recall()
    {
        float predicted = 0;
        for (unsigned int i = 0; i < active_column_indices.size(); ++i)
        {
            if (columns[active_column_indices[i]]->predictive_state)
                predicted += 1.0;
        }

        return predicted / (active_column_indices.size()+1);
    }

    float Region::update_state()
    {
        float max_duty_cycle = 0;
        float anomaly = compute_anomaly_score();

        _precision = calculate_precision();
        _recall = calculate_recall();

        int pattern_index = classifier->classify_input(input);
        _actual_input_class = pattern_index;
        _predicted_input_class = _last_prediction;

        cla_log("Current pattern classified as: %d\n", pattern_index);

        if (CFG::REGION_VIZ)
            visualize_region();
        if (CFG::ACTIVE_VIZ)
            visualize_active();
        if (CFG::OUTPUT_VIZ)
            visualize_output();

        // gathering up list of active cells
        vector<Cell*> active_cells;
        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            for (unsigned int j = 0; j < columns[i]->get_cells().size(); ++j)
            {
                if (columns[i]->get_cells()[j]->active_state)
                {
                    active_cells.push_back(columns[i]->get_cells()[j]);
                }
            }
        }

        // update temporal prediction state for next step
        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            columns[i]->update_state(
                    active_cells, 
                    pattern_index);

            if (columns[i]->get_active_duty() > max_duty_cycle)
            {
                max_duty_cycle = columns[i]->get_active_duty();
            }
        }

        // update boost level
        float avg_boost = 0;
        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            columns[i]->update_boost(CFG::MIN_DUTY_PERCENT * max_duty_cycle);
            avg_boost += columns[i]->boost;
        }
        avg_boost /= columns.size();

        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            columns[i]->update_prediction(active_cells, neighborhood_cells);
        }

        // Make prediction
        vector<int> votes;
        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            if (columns[i]->predictive_state)
                votes.push_back(columns[i]->most_likely_pattern);
        }
        sort(votes.begin(), votes.end());

        int count = 0;
        int current = -1;
        int best_count = 0;
        int best_vote = -1;

        for (unsigned int i = 0; i < votes.size(); ++i)
        {
            if (votes[i] != current and votes[i] != -1)
            {
                count = 0;
                current = votes[i];
            }
        
            count++;
            if (count > best_count)
            {
                best_vote = current;
                best_count = count;
            }
        }

        cla_log("The columns have voted on the best prediction class: %d\n", best_vote);
        _last_prediction = best_vote;

        vector<char> dense_input;
        if (best_vote != -1)
        {
            vector<int>* likely_input = classifier->reverse_class_index(best_vote);
            for (unsigned int i = 0; i < input.size(); ++i)
                dense_input.push_back(false);
            for (unsigned int i = 0; i < likely_input->size(); ++i)
                dense_input[likely_input->at(i)] = true;
        }
        else
        {
            for (unsigned int i = 0; i < input.size(); ++i)
                dense_input.push_back(0);
        }

        if (CFG::COLUMN_VIZ >= 0)
            columns[CFG::COLUMN_VIZ]->print_string();

        if (CFG::PREDICTION_VIZ)
            visualize_prediction(dense_input);

        return float(anomaly) / float(columns.size()) / float(CFG::DESIRED_SDR_DENSITY);
    }

    float Region::get_precision()
    {
        return _precision;
    }

    float Region::get_recall()
    {
        return _recall;
    }

    int Region::get_actual_input_class()
    {
        return _actual_input_class;
    }

    int Region::get_predicted_input_class()
    {
        return _predicted_input_class;
    }

    vector<int> Region::get_representation()
    {
        vector<int> active_columns;
        vector<column_activation_t> col_act;

        for (unsigned int i = 0; i < columns.size(); ++i)
        {
            column_activation_t ca;
            ca.column = columns[i];
            ca.activation = columns[i]->get_potential();
            ca.idx = i;

            if (ca.activation > 0)
            {
                col_act.push_back(ca);
            }
        }

        // find strongest columns
        sort(col_act.begin(), col_act.end(), compare_column_activation);

        // do inhibition
        for (int j = 0; j < active_column_count and j < int(col_act.size()); ++j)
        {
            active_columns.push_back(col_act[j].idx);
        }

        return active_columns;
    }
}
