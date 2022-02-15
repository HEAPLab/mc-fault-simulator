#include "tree-builder.hpp"

#include <algorithm>
#include <iostream>

#define PWR_LIMIT 50

bool TreeBuilder::next_level(const Path &path, const task_bool_t &activables) noexcept {
	//std::cout << "TreeBuilder::next_level " << current_level << " " << path.get_leaf_probability() << std::endl;
    current_level++;
    

    if (path.get_leaf_probability() < LIMIT_PROB_EVENTS) {
    	//std::cout << "LEAF LIMIT" << std::endl;
    	current_level--;
        return true;    // Success
    }

	for (auto i=0; i<base_taskset.size(); i++) {
		bool prev_not_exists = true;

		for (auto j=1; j<base_taskset[i][0].how_many_reexec+1; j++) {
			//std::cout << i << " " << j << " ACT? " << activables[i][j] << std::endl; 
			if (!activables[i][j]) {
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
                	current_level--;
               		return false; // Failure
           		}
			}
			prev_not_exists = false;
		}
	}
    
    current_level--;
    return true;    // Success
}

bool TreeBuilder::build_node(const Path &path, const Task &t, const task_bool_t &activables) noexcept {
	//std::cout << "TreeBuilder::build_node" << std::endl;

	Path new_path(path);
	

	auto new_node = new_path.new_node(&t);

	//std::cout << " old path prob " << path.get_leaf_probability() << " new path prob " << new_path.get_leaf_probability() << std::endl;

	
	task_bool_t new_activables = activables;
	new_activables[t.id][t.reexec_id] = false;
	
	new_path.compute_tasks_utilization();
	if(new_path.is_schedulable()) {
		if(this->next_level(new_path, new_activables)) {
            // No dropping necessary simple schedulability
            //std::cout << "Simple schedulability" << std::endl;
            return true;
        }
	}
	
	auto saved_fault_rate_real = fault_rate_real;

	std::set<std::set<const Task*>> fds;
	search_feasible_droppings(fds, path, saved_fault_rate_real);
	
	for (auto it = fds.cbegin(); it != fds.cend(); it++) {
		if(it->size() == 0) continue;
		
		new_path.reset_dropped(new_node);
		
		for (auto it2 = it->cbegin(); it2 != it->cend(); it2++)
			new_path.add_dropped(new_node, *it2);
		
		for (const auto&f : *it) {
            // We selected a fds, so we update the fault probability of all tasks
            // dropped here
            do_update_probability(f, new_path.get_leaf_probability());
        }
        
        new_path.compute_tasks_utilization();
        if(new_path.is_schedulable()) {
			task_bool_t temp_new_activables = new_activables;
			for (const auto&f : *it) {
				temp_new_activables[f->id][f->reexec_id] = false;
			}
			
			 bool result_next_level = this->next_level(new_path, temp_new_activables);
             if(result_next_level) {
                return true;
             }
        }
        
        for (const auto&f : *it) {
            restore_probability(f, saved_fault_rate_real);
        }
		
	}
    
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

void TreeBuilder::powerset(std::set<std::set<const Task*>> &fds, const std::unordered_set<const Task*> droppable) noexcept {
	int i=0;
	
	// First loop is single task drop
	for (const auto d : droppable) {
		if (i >= PWR_LIMIT) break;
		
		auto task_id = d->id;
		auto reexec_id = d->reexec_id;
		if ( std::none_of(droppable.begin(), droppable.end(), [task_id, reexec_id](const Task* t) { return t->id == task_id && t->reexec_id < reexec_id; })  ) {
			fds.insert(std::set<const Task*>{d}); 
			i++;
		}
	}
	
	std::set<std::set<const Task*>> fds_2nd;
	
	for (const auto d : droppable) {
		for (const auto fd : fds) {
			if (i >= PWR_LIMIT) break;
			
			if(fd.find(d) == fd.end()) {
				fds_2nd.insert(std::set<const Task*>{d, *fd.begin()}); 
				i++;
			}
			
		}
		if (i >= PWR_LIMIT) break;	
	}
	
	fds.merge(fds_2nd);
	
	std::set<std::set<const Task*>> fds_3rd;
	
	for (const auto d : droppable) {
		for (const auto fd : fds_2nd) {
			if (i >= PWR_LIMIT) break;
			
			if(fd.find(d) == fd.end()) {
				fds_3rd.insert(std::set<const Task*>{d, *fd.begin(), *std::next(fd.begin())}); 
				i++;
			}
			
		}
		if (i >= PWR_LIMIT) break;	
	}
	
	fds.merge(fds_3rd);
}

