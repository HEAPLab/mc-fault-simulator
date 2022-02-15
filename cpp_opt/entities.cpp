#include "entities.hpp"

#include <cassert>
#include <iostream>
#include <set>

void Path::print() const {
    std::cout << "PATH: ";
    for(int i = 0; i < lbl_arcs_len; i++) { 
        std::cout << " -> " << (int)lbl_arcs.at(i)->id << '(' << (int)lbl_arcs.at(i)->reexec_id << ") DROP[";
        if(nodes_labels_len[i] == 0) {
            std::cout << "NONE";    
        } else {
            for(int j = 0; j < nodes_labels_len[i]; j++) {
                std::cout << ((int)nodes_labels.at(i).at(j)->id) << '(' << ((int)nodes_labels.at(i).at(j)->reexec_id) << "),";
            }
        }
        std::cout << "]";
    }
    if (util_is_ready) {
	    int n_levels = lbl_arcs_len+1;
    	for (int l=1; l<n_levels+1; l++) {
        	for(int k=1; k<l+1; k++) {
			    std::cout << " U(" << l << "," << k << ")=" << get_utilization(l,k) << ", ";
       		}
        }
        std::cout  << std::endl;
	} else {
		std::cout << " U(x,x)=no info" << std::endl;
	}
}

/**
 * Get the utilization U(l,k). You must call compute_tasks_utilization() before invoking this
 * function.
 */
float Path::get_utilization(uint_fast8_t l, uint_fast8_t k) const noexcept {
    assert(this->util_is_ready);
    
    float U=0.;
    for (const auto &t : base_taskset) {
        if(uL_L[t[0].id] == l) {
            U += uL_U[t[0].id][k];
        }
    }
    
    return U;
}

/**
 * Compute the utilization matrix. Use get_utilization(l,k) to access the data.
 */
void Path::compute_tasks_utilization() noexcept {

	if(this->util_is_ready) {
		return;
	}

	// Compute the basic utilization without re-execution
    for (const auto &t : base_taskset) {
        uL_L[t[0].id] = 1;
        uL_U[t[0].id][0] = 0;
        uL_U[t[0].id][1] = t[0].U;
        uL_U_len[t[0].id] = 2;
        
        //std::cout << "uL_U " << (int)t[0].id << " " << t[0].U << std::endl;
    }
    
    // Now we have to check which task is dropped and which not
    std::set<uint_fast8_t> to_ignore;
    
    for(int i = 0; i < lbl_arcs_len; i++) {
    	int l = i+1;
    	auto switch_task_id = lbl_arcs.at(i)->id;
    	
    	// This is the task we have to re-execute
    	uL_L[switch_task_id]++;
    	uL_U[switch_task_id][uL_U_len[switch_task_id]++] = uL_U[switch_task_id][l] + uL_U[switch_task_id][1];

        //std::cout << "uL_U[switch_task_id][uL_U_len[switch_task_id]++] " << (int)switch_task_id << " " << (int)uL_U_len[switch_task_id]-1 << std::endl;

    	for (const auto &t : base_taskset) {	// Now let's check the others if they have to run or not
    		auto t_id = t[0].id;
    		if(t_id == switch_task_id) {
                continue;	// Same task, already added
            }
            
            if(to_ignore.find(t_id) != to_ignore.end()) {
            	continue;
            }
            
            // Check if the dropped task is in the current node or not
            for (int j=0; j<nodes_labels_len[i]; j++) {
				if(nodes_labels.at(i).at(j)->id == t_id) {
					to_ignore.insert(t_id);
					continue;
				}
			}
			
			// Otherwise the task is still active and must be counted in the utilization
			uL_L[t_id]++;
			uL_U[t_id][uL_U_len[t_id]++] = uL_U[t_id][l];    
        	//std::cout << "uL_U[t_id][uL_U_len[t_id]++]" << (int)t_id << " " << (int)uL_U_len[t_id]-1 << std::endl;	
    	}
    	
    	
    }
    
    this->util_is_ready = true;
    
}

/**
 * Return if the current path is schedulable or not. It uses the EDF-VD algorithm. 
 */
bool Path::is_schedulable() const noexcept {

    int n_levels = lbl_arcs_len+1;
    
    if(n_levels == 1) {
        // This is for a basic schedulability when we consider EDF.
        // This is the only case that we have a total schedulability and not partial.
        float U = 0.;
        for(const auto &t : base_taskset) {
        	for (int i=0; i< t[0].how_many_reexec+1; i++) {
	            U += t[i].U;
			}
        }
        
        return U <= 1. ? true : false;
    }
    
    std::array<std::array<float, NR_TASKS*MAX_REEXEC>, NR_TASKS*MAX_REEXEC> U_vec;

    for (int l=1; l<n_levels+1; l++) {
        for(int k=1; k<l+1; k++) {
            U_vec[l][k] = get_utilization(l, k);
        }
    }

    // Condition 1    
    float sum_Ull = 0.;
    for (int l=1; l<n_levels+1; l++) {
        sum_Ull += U_vec[l][l];
    }
    
    if(sum_Ull <= 1.) {
        return true;
    }
    
    // Condition 2
    for (int k=1; k<n_levels; k++) {
        float sum_Ull = 0.;
        for (int l=1; l<k+1; l++) {
            sum_Ull += U_vec[l][l];
        }
        float cond1 = 1. - sum_Ull;
        
        if(cond1 <= 0.) continue;   // not ok
        
        float num1 = 0.;
        for(int l=k+1; l<n_levels+1; l++) {
            num1 += U_vec[l][k];
        }
        
        float num2 = 0.;
        for(int l=k+1; l<n_levels+1; l++) {
            num2 += U_vec[l][l];
        }
        
        if(sum_Ull == 0.) {
            sum_Ull = 1e-50;
        }
        if(sum_Ull == 1.) {
            sum_Ull = 1-1e-50;
        }
        
        float frac1 = num1 / (1.-sum_Ull);
        float frac2 = (1.-num2) / sum_Ull;

        
        if (frac1 <= frac2) {
            return true;
        }
    }
        
    return false;
}

/**
 * Return the list of tasks that can be still dropped at the end of the path, i.e., the task not yet
 * dropped and not yet failed or activated due to failure. 
 */
void Path::get_droppables(task_bool_t &to_ret) const noexcept {

	for (auto i=0; i<base_taskset.size(); i++) {
		for (auto j=0; j<base_taskset[i][0].how_many_reexec+1; j++) {
			assert(i == base_taskset[i][j].id);
			to_ret[i][j] = true;
		}
	}
	
	for(int i = 0; i <lbl_arcs_len; i++) {
		auto start_reexec_id = lbl_arcs.at(i)->reexec_id;
		
		// All the tasks with a lower re-exec id have been already executed, so they can't be dropped
		for (auto j=0; j<start_reexec_id; j++) {
			to_ret[lbl_arcs.at(i)->id][j] = false;
		}

		// All the tasks already dropped, cannot be dropped again
		for (auto j=0; j<nodes_labels_len[i]; j++) {
			auto i_el = nodes_labels.at(i).at(j)->id;
			auto j_el = nodes_labels.at(i).at(j)->reexec_id ;
			to_ret[i_el][j_el] = false;
		}

	}

}

