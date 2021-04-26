#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include <cstdint>
#include <vector>

class Task {


public:
    uint_fast8_t id;
    uint_fast8_t reexec_id;
    uint_fast8_t how_many_reexec;
    
    float wcet;
    float T;
    
    float fault_rate;
    float failure_req;
};

class Path {

public:

    Path(const std::vector<const Task*>* base_taskset,
         const std::vector<const Task*>* reexec_taskset,
         const std::vector<const Task*>* full_taskset) :
         base_taskset(base_taskset), reexec_taskset(reexec_taskset), full_taskset(full_taskset) {
    
        uL_L.resize(base_taskset->size());
        uL_U.resize(base_taskset->size());

    }

    void print() const;

    float get_leaf_probability() const noexcept;

    bool is_schedulable() const noexcept;
    
    inline int new_node(const Task *t) {
        int size = this->input_tasks.size();
        this->input_tasks.push_back(t);
        this->dropped_tasks.push_back(nullptr);
        return size;
    }
    
    inline void add_dropped(int i, const std::vector<const Task*> * d_t) {
        this->dropped_tasks[i] = d_t;
    }
    
    std::vector<const Task*> get_droppables() const noexcept;

private:
    const std::vector<const Task*>* base_taskset;
    const std::vector<const Task*>* reexec_taskset;
    const std::vector<const Task*>* full_taskset;

    std::vector<const Task*> input_tasks;
    std::vector<const std::vector<const Task*> *> dropped_tasks;

    mutable std::vector<int> uL_L;
    mutable std::vector<std::vector<float>> uL_U;
    
    float get_utilization(int l, int k) const noexcept;
    void compute_tasks_utilization() const noexcept;
    
};

#endif
