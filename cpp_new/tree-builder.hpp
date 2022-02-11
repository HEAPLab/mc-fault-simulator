#ifndef TREE_BUILDER_HPP_
#define TREE_BUILDER_HPP_

#include "entities.hpp"
#include <unordered_set>
#include <set>
#define LIMIT_PROB_EVENTS 1e-12


typedef std::array<std::array<float, NR_TASKS>, MAX_REEXEC>  fault_rate_t;

class TreeBuilder {
public:
    TreeBuilder(const task_array_t &base_taskset) : base_taskset(base_taskset) {
    	for(auto i=0; i < base_taskset.size(); i++) {
	    	for(auto j=0; j < base_taskset[i][0].how_many_reexec+1; j++) {
		    	fault_rate_real[i][j] = base_taskset[i][0].fault_rate;
			}
    	}
    }
    
    bool next_level(const Path &path, const task_bool_t &activables) noexcept;
    bool build_node(const Path &path, const Task &t, const task_bool_t &activables) noexcept;
    
private:
	const task_array_t & base_taskset;
    uint_fast16_t current_level;

	void do_update_probability(const Task* t, float new_p_tot) noexcept;
	void restore_probability(const Task* t, const fault_rate_t &saved_fault_rate_real) noexcept;
	void search_feasible_droppings(std::set<std::set<const Task*>> &fds, const Path &path, const fault_rate_t &saved_fault_rate_real) noexcept;
    fault_rate_t fault_rate_real;
    void powerset(std::set<std::set<const Task*>> &fds, const std::unordered_set<const Task*> droppable) noexcept;
    bool verify_feasibility(const Task* t) const noexcept;
};

#endif
