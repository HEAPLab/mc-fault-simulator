#include "tree-builder.hpp"

#include <algorithm>
#ifdef ENABLE_DEBUG
	#include <iostream>
#endif

#define PWR_LIMIT 50 // Number of dropping elements tries per node

bool TreeBuilder::next_level(const Path &path, const task_bool_t &activables) noexcept {

#ifdef ENABLE_DEBUG
    std::cout << "TreeBuilder::next_level " << current_level << " " << path.get_leaf_probability() << std::endl;
    current_level++;
#endif

    if (path.get_leaf_probability() < LIMIT_PROB_EVENTS) {
        // In this case, the probability of the path reached the minimum probability
        // allowed (like 10^-12), so it's not necessary to continue to check the path
        // because other faults are too rare to be considered.
#ifdef ENABLE_DEBUG
        std::cout << "-> LEAF LIMIT" << std::endl;
        current_level--;
#endif
        return true;    // Success
    }

    for (auto i=0; i<base_taskset.size(); i++) {
        bool prev_not_exists = true;

        for (auto j=1; j<base_taskset[i][0].how_many_reexec+1; j++) {   // For each re-execution task...
            if (!activables[i][j]) {
                // We cannot activate the j-th reexecution if the j-1 reexecution is not yet triggered.
                // Therefore, we use prev_not_exists to mark the fact that the previous re-execution is
                // not triggered.
                // We set this variable to true in two scenarios: the task has been executed or dropped.
                // But if dropped, all the re-execution tasks will enter this `if` and therefore
                // ignored.
                prev_not_exists = true;
                continue;
            }
            uint_fast8_t reexec_id = base_taskset[i][j].reexec_id;
            uint_fast8_t task_id   = base_taskset[i][j].id;

            // If the previous re-execution task has not been activated yet, we can't activate this
            // one
            if(prev_not_exists) {
                // Ok let's create the new node for the current arc
                bool res = this->build_node(path, base_taskset[i][j], activables);
                if (!res) {
#ifdef ENABLE_DEBUG
                    current_level--;
#endif
                    return false; // Failure
                }
            }
            prev_not_exists = false;
        }
    }

#ifdef ENABLE_DEBUG
    current_level--;
#endif
    return true;    // Success
}

bool TreeBuilder::build_node(const Path &path, const Task &t, const task_bool_t &activables) noexcept {
#ifdef ENABLE_DEBUG
    std::cout << "TreeBuilder::build_node" << std::endl;
#endif

    Path new_path(path);    // Copy the path to create a new one	

    // Add a new node with label the task currently activated due to fault
    auto new_node = new_path.new_node(&t);

    // Define a new activables matrix and removes the task activated at this
    // level from the list of activable tasks
    task_bool_t new_activables = activables;
	new_activables[t.id][t.reexec_id] = false;
	
	// Recompute the path utilization
	new_path.compute_tasks_utilization();

    if(new_path.is_schedulable()) {
        // If the new path is schedulable...
        if(new_path.get_leaf_probability() < LIMIT_PROB_EVENTS) {
            // ... and we hit the probability limit, then we consider
            // it schedulable (see comment at the beginning of TreeBuilder::next_level)
#ifdef ENABLE_DEBUG
        std::cout << "-> LEAF LIMIT (2)" << std::endl;
#endif
            return true;
        }

        if(this->next_level(new_path, new_activables)) {
            // ... and no dropping necessary: simple schedulability without
            // dropping any task at this level
#ifdef ENABLE_DEBUG
            std::cout << "-> SIMPLE SCHEDULABILITY" << std::endl;
#endif
            return true;
        }
    }

    // Otherwise, we need to drop some tasks...

    // Let's save the current fault rates (we need to restore this later)
    auto saved_fault_rate_real = fault_rate_real;

    std::set<std::set<const Task*>> fds;    // This will contain the set of possible sets of droppable tasks
    search_feasible_droppings(fds, path, saved_fault_rate_real);
	
    for (auto it = fds.cbegin(); it != fds.cend(); it++) {  // For each possible set of droppable tasks...
        if(it->size() == 0) continue;

        // Configure the new list of the dropped tasks
        new_path.reset_dropped(new_node);

        for (auto it2 = it->cbegin(); it2 != it->cend(); it2++)
            new_path.add_dropped(new_node, *it2);

        for (const auto&f : *it) {
            // We selected a fds, so we update the fault probability of all tasks
            // dropped here
            do_update_probability(f, new_path.get_leaf_probability());
        }
		
    // Recompute the utilization and check if schedulable
    new_path.compute_tasks_utilization();
    if(new_path.is_schedulable()) {
        if(path.get_leaf_probability() < LIMIT_PROB_EVENTS) {
            return true;
        }
        
        task_bool_t temp_new_activables = new_activables;
        for (const auto&f : *it) {
            // Set all dropped tasks as non-activables
            temp_new_activables[f->id][f->reexec_id] = false;
        }

        // Recursive call...
        bool result_next_level = this->next_level(new_path, temp_new_activables);
        if(result_next_level) {
            return true;
       }
    }
		
		// Restore the previous probability for the next try
		for (const auto&f : *it) {
		    restore_probability(f, saved_fault_rate_real);
		}
			
	}
    
    // We didn't find any droppable set which generates a schedulable and compliant path
    return false;
}

void TreeBuilder::do_update_probability(const Task* t, float new_p_tot) noexcept {

    float current = fault_rate_real[t->id][t->reexec_id] ;
    float new_prob = 1. - (1.-current) * (1.-new_p_tot);
    fault_rate_real[t->id][t->reexec_id] = new_prob;

}

void TreeBuilder::restore_probability(const Task* t, const fault_rate_t &saved_fault_rate_real) noexcept {
    fault_rate_real[t->id][t->reexec_id] = saved_fault_rate_real[t->id][t->reexec_id];
}

bool TreeBuilder::verify_feasibility(const Task* t) const noexcept {
    // This function check the compliance with the failure requirement
    float p_tot = 1.;

    for(const auto &p : fault_rate_real[t->id]) {
        p_tot *= p;
    }
    
    return p_tot <= t->failure_req;
}

void TreeBuilder::search_feasible_droppings(std::set<std::set<const Task*>> &fds, const Path &path, const fault_rate_t &saved_fault_rate_real) noexcept {

	task_bool_t droppable;
    path.get_droppables(droppable);

    std::unordered_set<const Task*> droppable_tasks_and_feasible;
    droppable_tasks_and_feasible.reserve(NR_TASKS * MAX_REEXEC);

    float new_p_tot = path.get_leaf_probability();
    
    for(auto i=0; i<droppable.size(); i++) {
		for(auto j=0; j<base_taskset[i][0].how_many_reexec+1; j++) {
			if (droppable[i][j]) {
				auto *dt = &base_taskset[i][j];
				do_update_probability(dt, new_p_tot);
				if(verify_feasibility(dt)) {
				    droppable_tasks_and_feasible.insert(dt);
				}

        		restore_probability(dt, saved_fault_rate_real);
			}
		}
    }
    
    powerset(fds, droppable_tasks_and_feasible);

	auto is_valid = [=](const std::set<const Task*> & fd ){
		for(const auto &t : fd) {
             do_update_probability(t, new_p_tot);
        }
        bool ok=true;
        for(const auto &t : fd) {
           if(! verify_feasibility(t)) {
                ok=false;
                break;
           }
        }
        for(const auto &t : fd) {
             restore_probability(t, saved_fault_rate_real);
        }
        
        return ok;
	};
	

    for(auto it = fds.begin(); it != fds.end(); ) {
        if(!is_valid(*it))
            it = fds.erase(it);
        else
            ++it;
    }


}

void TreeBuilder::powerset(std::set<std::set<const Task*>> &fds, const std::unordered_set<const Task*> &droppable) noexcept {
    int i=0;

    // This is not an exact powerset but only a subset of it. This approximation reduces the 
    // analysis abaility to find a schedulable+compliant taskset, but improves the performance
    // by reduxing the computation time. Future works may explore new algorithms.

    // First loop is single task drop
    // We can drop all the tasks that have the last re-execution id
    for (const auto &d : droppable) {
        if (i >= PWR_LIMIT) break;

        auto task_id = d->id;
        auto reexec_id = d->reexec_id;
        if ( reexec_id == d->how_many_reexec ) {
            fds.insert(std::set<const Task*>{d});
            i++;
        }
    }

    // Now we take the second level, i.e., 2 tasks per dropping
	
    std::set<std::set<const Task*>> fds_2nd;
	
    for (const auto &d : droppable) {
        for (const auto &fd : fds) {
            if (i >= PWR_LIMIT) break;
            
            auto task_id = d->id;
            auto reexec_id = d->reexec_id;
            
            if((*fd.begin())->id != task_id || (*fd.begin())->reexec_id == reexec_id-1) {
                fds_2nd.insert(std::set<const Task*>{d, *fd.begin()}); 
                i++;
            }
        }
        if (i >= PWR_LIMIT) break;	
    }
	
	fds.merge(fds_2nd);
	
    // Now we take the second level, i.e., 3 tasks per dropping
	
	std::set<std::set<const Task*>> fds_3rd;
	
	for (const auto &d : droppable) {
		for (const auto &fd : fds_2nd) {
			if (i >= PWR_LIMIT) break;
			
            auto task_id = d->id;
            auto reexec_id = d->reexec_id;
            
			bool ok = true;

			for (const auto &fd_item : fd) { 
			    if (fd_item->id == task_id) {
    			    if (fd_item->reexec_id == reexec_id - 1 ) {
    			        ok = true;
    			        break;
    			    }
			        ok = false;
			    } 
			}
			if(ok) {
				fds_3rd.insert(std::set<const Task*>{d, *fd.begin(), *std::next(fd.begin())}); 
				i++;
			}
			
		}
		if (i >= PWR_LIMIT) break;	
	}
	
	fds.merge(fds_3rd);
}

