#include "tree-builder.hpp"

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>




int main(int argc, char* argv[]) {
	task_array_t base_taskset;

    std::ifstream input_file(argv[1]);
	
    float wcet, T, n_reexec, FR, FP;
    int i=0;
    
    // Input file reading...
    while (input_file.good()) {
        input_file >> wcet >> T >> n_reexec >> FR >> FP;
        if (input_file.fail() || input_file.eof())   
            break;

		base_taskset[i][0].id = i;
		base_taskset[i][0].reexec_id = 0;
		base_taskset[i][0].how_many_reexec = n_reexec;
		base_taskset[i][0].U = wcet/T;
		base_taskset[i][0].fault_rate = FP;
		base_taskset[i][0].failure_req = FR;

        if (i >= NR_TASKS) {
        	std::cerr << "OOB." << std::endl;
        }

        for (int j=0; j<n_reexec; j++) {
            base_taskset[i][j+1] = base_taskset[i][0];
            base_taskset[i][j+1].reexec_id = j+1;
        }

        i++;
    }

    TreeBuilder tb(base_taskset);
    Path initial(base_taskset);

	task_bool_t activables;
	for (auto i=0; i<base_taskset.size(); i++) {
		activables[i][0] = false;
		for (auto j=1; j<base_taskset[i][0].how_many_reexec+1; j++) {
			activables[i][j] = true;
		}
	}
    return tb.next_level(initial, activables);
}


