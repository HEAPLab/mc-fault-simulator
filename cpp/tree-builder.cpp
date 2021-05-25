#include "tree-builder.hpp"

#include <algorithm>
#include <iostream>

TreeBuilder::TreeBuilder( const std::vector<const Task*>* base_taskset,
         const std::vector<const Task*>* reexec_taskset,
         const std::vector<const Task*>* full_taskset) : current_level(0),
         base_taskset(base_taskset), reexec_taskset(reexec_taskset), full_taskset(full_taskset) {
    
    for(const auto &bt : *base_taskset) {
        std::vector<float> rates;
        rates.reserve(bt->how_many_reexec+1);
        for (int i=0; i < bt->how_many_reexec+1; i++) {
            rates.push_back(bt->fault_rate);
        }
        fault_rate_real.push_back(rates);
    }
     
    
}

bool TreeBuilder::next_level(const Path &path, const std::vector<const Task*> &activables) noexcept {

    current_level++;

    if (path.get_leaf_probability() < LIMIT_PROB_EVENTS) {
        //path.print();
        return true;    // Success
    }

    if (activables.size() == 0) {   // Oh, this is a leaf
        current_level--;
        //path.print();
        return true;    // Success
    }
    
    for (const auto &a: activables) {   
        uint_fast8_t reexec_id = a->reexec_id;
        uint_fast8_t task_id   = a->id;

//        if(current_level == 1) {
//            std::cout << "TASK " << (int)task_id << "(" << (int)reexec_id << ")" << std::endl;
//        }

        
        // If the previous re-execution task has not been activated yet, we can't activate this
        // one
        bool prev_not_exists;
        if (reexec_id == 0) {
            prev_not_exists = true;
        } else {
            prev_not_exists = std::none_of(activables.cbegin(), activables.cend(),
               [reexec_id, task_id](const Task* t) { return t->id == task_id && t->reexec_id == reexec_id-1; });
        }
        
        if(prev_not_exists) {
            // Ok let's create the new node for the current arc
            bool res = this->build_node(path, a, activables);
            if (!res) {
                current_level--;
                return false; // Failure
            }
        }
    }
    
    current_level--;
    return true;    // Success
}


bool TreeBuilder::build_node(const Path &path, const Task* t, const std::vector<const Task*> &activables) noexcept {

    Path new_path(path);
    auto new_node = new_path.new_node(t);
    
    std::vector<const Task*> new_activables;
    new_activables.reserve(activables.size()-1);
    for(const auto &a : activables) {
        if (a != t) {
            new_activables.push_back(a);
        }
    }
    

    if(new_path.is_schedulable()) {
        if(this->next_level(new_path, new_activables)) {
            // No dropping necessary simple schedulability
            return true;
        }
    }
    
    std::vector<std::vector<float>> saved_fault_rate_real;
    saved_fault_rate_real = fault_rate_real;

    std::vector<bool> valids;
    std::vector<std::vector<const Task*>> fds;
    search_feasible_droppings(valids, fds, new_path, new_activables, saved_fault_rate_real);
    
    int n_fds = fds.size();
    
    for (int i=0; i<n_fds; i++) {
        if(fds[i].size() == 0) continue;
        if(! valids[i]) continue;
        
        new_path.add_dropped(new_node, &fds[i]);

        for (const auto&f : fds[i]) {
            // We selected a fds, so we update the fault probability of all tasks
            // dropped here
            do_update_probability(f, new_path.get_leaf_probability());
        }
        
        if(new_path.is_schedulable()) {
             std::vector<const Task*> temp_new_activables;
             temp_new_activables.reserve(new_activables.size());
             for (const auto &a : new_activables) {
                if(std::find(fds[i].cbegin(), fds[i].cend(), a) == fds[i].cend()) {
                    temp_new_activables.push_back(a);
                }
             }
             
             bool result_next_level = this->next_level(new_path, temp_new_activables);
             if(result_next_level) {
                return true;
             }
        }
        
        for (const auto&f : fds[i]) {
            restore_probability(f, saved_fault_rate_real);
        }
        
    }   
    
    return false;

}



void TreeBuilder::restore_probability(const Task* t, const std::vector<std::vector<float>> &saved_fault_rate_real) noexcept {
    fault_rate_real[t->id][t->reexec_id] = saved_fault_rate_real[t->id][t->reexec_id];
}

void TreeBuilder::do_update_probability(const Task* t, float new_p_tot) noexcept {

    float current = fault_rate_real[t->id][t->reexec_id] ;
    float new_prob = 1. - (1.-current) * (1-new_p_tot);
    fault_rate_real[t->id][t->reexec_id] = new_prob;

}

void TreeBuilder::powerset(std::vector<std::vector<const Task*>> &fds, std::vector<const Task*> seq) noexcept {
    
    fds.clear();
    
    for(const auto&s : seq) {
    
        const Task* found_greater = nullptr;
        for(const auto& s2 : seq) {
            if(s2->id == s->id) {
                if(s2->reexec_id == s->reexec_id + 1) {
                    found_greater = s2;
                }
            }
        }
    
        int fds_size = fds.size();
        for (int i=0; i<fds_size; i++) {
            if(fds[i].size() > 2) { continue; }
        
            bool is_ok = false;
            if(found_greater != nullptr) {
                if(std::find(fds[i].cbegin(), fds[i].cend(), found_greater) != fds[i].cend()) {
                    is_ok = true;
                }
            } else {
                is_ok = true;
            }
            if(is_ok) {
                auto new_set = fds[i];
                new_set.push_back(s);
                fds.push_back(std::move(new_set));
            }
        }
        
        if(found_greater == nullptr) {
            std::vector<const Task*> temp = {s};
            fds.push_back(temp);
        }
        
    }

    fds.push_back(std::vector<const Task*>());
    
}

bool TreeBuilder::verify_feasibility(const Task* t) const noexcept {
    float p_tot = 1.;

    for(const auto &p : fault_rate_real[t->id]) {
        p_tot *= p;
    }
    
    return p_tot <= t->failure_req;
}

void TreeBuilder::search_feasible_droppings(std::vector<bool> &valids, std::vector<std::vector<const Task*>> &fds, const Path &path, std::vector<const Task*> &activables, const std::vector<std::vector<float>> &saved_fault_rate_real) noexcept {

    std::vector<const Task*> droppable_tasks = path.get_droppables();
    std::vector<const Task*> droppable_tasks_and_feasible;
    droppable_tasks_and_feasible.reserve(droppable_tasks.size());
    
    float new_p_tot = path.get_leaf_probability();
    
    for(const auto&dt : droppable_tasks) {
        do_update_probability(dt, new_p_tot);

        if(verify_feasibility(dt)) {
            droppable_tasks_and_feasible.push_back(dt);
        }
        
        restore_probability(dt, saved_fault_rate_real);
    }
    
    
    powerset(fds, droppable_tasks_and_feasible);
    
    std::sort(fds.begin(), fds.end(), [](std::vector<const Task*> a, std::vector<const Task*> b) {
        return a.size() < b.size();
    });

    if(fds.size() > 25) {
        fds.resize(25);
    }
    
    valids.reserve(fds.size());
    
    for(const auto &f : fds) {
        for(const auto &t : f) {
             do_update_probability(t, new_p_tot);
        }
        bool ok=true;
        for(const auto &t : f) {
           if(! verify_feasibility(t)) {
                ok=false;
           }
        } 
        
        valids.push_back(ok);
        
        for(const auto &t : f) {
             restore_probability(t, saved_fault_rate_real);
        }
    }

}


