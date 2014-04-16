#include "CLA.h"

namespace CLA
{
    class Classifier
    {
        public:
            Classifier();
            ~Classifier();

            int classify_input(std::vector<char*>& input);
            std::vector<int>* reverse_class_index(int class_index);
        private:
            std::vector<std::vector<int>* > classes;
    };
}
