#ifndef CLA_REGION_H
#define CLA_REGION_H

#include <vector>

namespace CLA
{
    class Synapse;
    class DendriteSegment;
    class ProximalSegment;
    class DistalSegment;
    class Cell;
    class Column;
    class CFG;
    class Cortex;
    class Classifier;

    class Region
    {
        public:
            Region(int columns,
                    std::vector<char*>& input_bits,
                    int column_depth,
                    float fraction_columns_active);
            virtual ~Region();

            std::vector<char*>& get_axons();
            std::vector<int>& get_active_columns();
            void update_input();
            float update_state();

            float get_precision();
            float get_recall();

            int get_actual_input_class();
            int get_predicted_input_class();

            std::vector<int> get_representation();
        protected:
            void visualize_region();
            void visualize_active();
            void visualize_output();
            void visualize_prediction(std::vector<char>& prediction);

            float compute_anomaly_score();
            float calculate_precision();
            float calculate_recall();

            std::vector<Column*> columns;
            std::vector<char*> input;
            std::vector<char*> axons;
            std::vector<Cell*> neighborhood_cells;
            std::vector<int> active_column_indices;
            int active_column_count;

            float _precision;
            float _recall;
            float _actual_input_class;
            float _predicted_input_class;

            int _last_prediction;

            Classifier* classifier;
    };

}

#endif
