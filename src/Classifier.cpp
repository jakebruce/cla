#include "CLA.h"
#include <vector>

using namespace std;

namespace CLA
{
    Classifier::Classifier()
    {

    }

    Classifier::~Classifier()
    {
        for (unsigned int i = 0; i < classes.size(); ++i)
            delete classes[i];
    }

    int Classifier::classify_input(vector<char*>& input)
    {
        vector<int>* sparse_input = new vector<int>();

        for (unsigned int i = 0; i < input.size(); ++i)
            if (*(input[i]))
                sparse_input->push_back(i);

        int best_overlap = 0;
        int best_idx = -1;

        for (unsigned int i = 0; i < classes.size(); ++i)
        {
            int overlap = 0;
            for (unsigned int j = 0; j < sparse_input->size(); ++j)
            {
                for (unsigned int k = 0; k < classes[i]->size(); ++k)
                {
                    if (sparse_input->at(j) == classes[i]->at(k))
                    {
                        overlap++;
                    }
                }
            }

            if (overlap > best_overlap)
            {
                best_overlap = overlap;
                best_idx = i;
            }
        }

        // if we matched most of the bits it's probably this input
        if (best_overlap > sparse_input->size()/1.5)
        {
            delete sparse_input;
            return best_idx;
        }
        else
        {
            classes.push_back(sparse_input);
            return classes.size()-1;
        }
    }

    vector<int>* Classifier::reverse_class_index(int class_index)
    {
        if (class_index > int(classes.size()-1) or class_index < 0)
        {
            cla_log("WARNING: requested an invalid class index\n");
            return 0;
        }
        
        return classes[class_index];
    }
}
