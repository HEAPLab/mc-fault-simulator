#include "entities.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

float Path::get_leaf_probability() const noexcept {
    float p_tot = 1.;
    for(const auto&t : this->input_tasks) {
        p_tot *= t->fault_rate;
    }
    return p_tot;
}

void Path::print() const {
    std::cout << "PATH: ";
    for(int i = 0; i <input_tasks.size(); i++) { 
        std::cout << " -> " << (int)input_tasks[i]->id << '(' << (int)input_tasks[i]->reexec_id << ") DROP[";
        if(dropped_tasks[i] == nullptr) {
            std::cout << "NONE";    
        } else {
            for(int j = 0; j < dropped_tasks[i]->size(); j++) {
                std::cout << (int)(*dropped_tasks[i])[j]->id << '(' << (int)(*dropped_tasks[i])[j]->reexec_id << "),";
            }
        }
        std::cout << "]";
    }
    std::cout << " U(1,1)=" << get_utilization(1,1) << " U(4,1)=" << get_utilization(4,1) << " U(4,4)=" << get_utilization(4,4) << std::endl;
}

std::vector<const Task*> Path::get_droppables() const noexcept {

    std::vector<const Task*> to_ret;

    for (const auto &t : *full_taskset) {
        bool to_add = true;
        for(int i = 0; i <input_tasks.size(); i++) {
            if(input_tasks[i]->id == t->id && input_tasks[i]->reexec_id >= t->reexec_id ) {
                to_add = false;
                break;
            }
        
            if(dropped_tasks[i] != nullptr) {
                if(std::any_of(dropped_tasks[i]->cbegin(), dropped_tasks[i]->cend(),
                   [t](const Task* t2) { return t->id == t2->id && t->reexec_id == t2->reexec_id; })) {
                    to_add = false;
                    break; 
                }
            }
        }
        if(to_add) {
            to_ret.push_back(t);
        }
    }
    
    return to_ret;
}

void Path::compute_tasks_utilization() const noexcept {
    for (const auto &t : *base_taskset) {
        uL_L[t->id] = 1;
        uL_U[t->id].clear(); 
        uL_U[t->id].push_back(0);
        uL_U[t->id].push_back(t->wcet / t->T);
    }
    
    std::vector<const Task*> to_ignore;
    
    for(int i = 0; i <input_tasks.size(); i++) {
        int l = i+1;
        
        auto switch_task_id = input_tasks[i]->id;
        
        uL_L[switch_task_id]++;
        uL_U[switch_task_id].push_back(uL_U[switch_task_id][l] + uL_U[switch_task_id][1]);
        
        for (const auto &t : *base_taskset) {
            if(t->id == switch_task_id) {
                continue;
            }
            if(std::find(to_ignore.cbegin(), to_ignore.cend(), t) != to_ignore.cend()) {
                continue;
            }
            if(dropped_tasks[i] != nullptr) {
                if(std::find(dropped_tasks[i]->cbegin(), dropped_tasks[i]->cend(), t) != dropped_tasks[i]->cend()) {
                    to_ignore.push_back(t);
                    continue;
                }
            }
            uL_L[t->id]++;
            uL_U[t->id].push_back(uL_U[t->id][l]);
            
        }
   
    }
    
}

float Path::get_utilization(int l, int k) const noexcept {

    this->compute_tasks_utilization();
    
    float U=0.;
    for (const auto &t : *base_taskset) {
        if(uL_L[t->id] == l) {
            U += uL_U[t->id][k];
        }
    }
    
    return U;
    
}

bool Path::is_schedulable() const noexcept {

    int n_levels = input_tasks.size()+1;
    
    if(n_levels == 1) {
        // This is for a basic schedulability when we consider EDF.
        // This is the only case that we have a total schedulability and not partial.
        float U = 0.;
        for(const auto &t : *full_taskset) {
            U += (float)t->wcet / (float)t->T;
        }
        
        return U <= 1. ? true : false;
    }
    
    std::vector<std::vector<float>> U_vec;
    U_vec.resize(n_levels+1);
    for (int l=1; l<n_levels+1; l++) {
        U_vec[l].resize(l+1);
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

/*        std::cout << "k=" << k;        
        std::cout << " num1=" << num1;
        std::cout << " num2=" << num2;
        std::cout << " frac1=" << frac1;
        std::cout << " frac2=" << frac2;
        std::cout << std::endl;
*/
        
        if (frac1 <= frac2) {
            return true;
        }
    }
        
    return false;
}
