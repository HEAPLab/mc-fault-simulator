#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <array>

#ifndef NR_TASKS
	#error "NR_TASKS not defined"
#endif

#define MAX_REEXEC 5

class Task {

public:

	Task() {}

    uint_fast8_t id;
    uint_fast8_t reexec_id;
    uint_fast8_t how_many_reexec;
    
    float U;
    
    float fault_rate;
    float failure_req;

	~Task() {
	}
    
private:
	friend int main(int argc, char* argv[]);
    Task & operator=(const Task&) = default;
    Task(const Task&) = delete;


};


typedef std::array<std::array<Task, MAX_REEXEC>, NR_TASKS> task_array_t; // [i][0] is always valid
typedef std::array<std::array<bool, MAX_REEXEC>, NR_TASKS> task_bool_t;

class Path {

public:

    Path(const task_array_t &base_taskset) : base_taskset(base_taskset) {

    }
    
    Path(const Path &p) : base_taskset(p.base_taskset),
                         util_is_ready(false),
                         leaf_probability(p.leaf_probability),
                         lbl_arcs(p.lbl_arcs),
                         lbl_arcs_len(p.lbl_arcs_len),
                         nodes_labels(p.nodes_labels),
                         nodes_labels_len(p.nodes_labels_len)
    {
        // We define a new copy costructor because we don't need to copy some variables
        // so we can save some execution time
        
    }

	float get_leaf_probability() const noexcept {
		return leaf_probability;
	}
	
	// Add a node to the path, and return the path length
	int new_node(const Task *label_task) noexcept {
	
		// Add the new arc to the list of arcs
		int size  = this->lbl_arcs_len;
		lbl_arcs[size] = label_task;
		nodes_labels_len[size] = 0;	// The node contains no data
		this->lbl_arcs_len++;
		
		// Update the leaf probability for future use
		leaf_probability = leaf_probability * label_task->fault_rate;
	
	    this->util_is_ready = false;
		return size;
    }

    void add_dropped(int i, const Task* t) noexcept {

    	assert(nodes_labels_len[i] < NR_TASKS*MAX_REEXEC);
        nodes_labels[i][nodes_labels_len[i]++] = t;
        this->util_is_ready = false;
    }
    
    void reset_dropped(int i) noexcept {
    	nodes_labels_len[i] = 0;
    }

    void print() const;
    
	float get_utilization(uint_fast8_t l, uint_fast8_t k) const noexcept;

	void get_droppables(task_bool_t &to_ret) const noexcept;

    bool is_schedulable() const noexcept;

    void compute_tasks_utilization() noexcept;

private:
    const task_array_t &base_taskset;
    
    bool util_is_ready=false;
    float leaf_probability = 1.;

	std::array<const Task *, NR_TASKS*MAX_REEXEC> lbl_arcs;
	uint_fast8_t lbl_arcs_len = 0;
	std::array<std::array<const Task *,NR_TASKS*MAX_REEXEC>, NR_TASKS*MAX_REEXEC> nodes_labels;
    std::array<uint_fast8_t, NR_TASKS*MAX_REEXEC> nodes_labels_len;

    std::array<uint_fast8_t, NR_TASKS> uL_L;	// Number of levels
    std::array<std::array<float, NR_TASKS*MAX_REEXEC>, NR_TASKS> uL_U;
    std::array<uint_fast8_t, NR_TASKS> uL_U_len;
    

};

#endif
