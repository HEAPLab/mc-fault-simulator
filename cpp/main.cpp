#include "tree-builder.hpp"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {

    std::ifstream input_file(argv[1]);

    std::vector<const Task*> base_taskset;
    std::vector<const Task*> reexec_taskset;
    std::vector<const Task*> full_taskset;

    float wcet, T, n_reexec, FR, FP;
    int i=0;
    while (input_file.good()) {
        input_file >> wcet >> T >> n_reexec >> FR >> FP;
        if (input_file.fail() || input_file.eof())   
            break;

        Task *new_task = new Task;
        new_task->id         = i;
        new_task->reexec_id  = 0;
        new_task->wcet       = wcet;
        new_task->T          = T;
        new_task->fault_rate = FP;
        new_task->failure_req= FR;
        new_task->how_many_reexec = n_reexec;
        
        base_taskset.push_back(new_task);
        
        for (int j=0; j<n_reexec; j++) {
            Task *new_r_task = new Task(*new_task);
            new_r_task->reexec_id = j+1;
            reexec_taskset.push_back(new_r_task);
        }

        i++;
    }
    
    for(const auto &task : base_taskset) {
        full_taskset.push_back(task);
    }
    for(const auto &task : reexec_taskset) {
        full_taskset.push_back(task);
    }


    TreeBuilder tb(&base_taskset,&reexec_taskset,&full_taskset);
    Path initial(&base_taskset,&reexec_taskset,&full_taskset);

    return tb.next_level(initial, reexec_taskset);
}
