#ifndef TREE_BUILDER_HPP_
#define TREE_BUILDER_HPP_

#include "entities.hpp"

#include <vector>

#define LIMIT_PROB_EVENTS 1e-12

class TreeBuilder {

public:
    TreeBuilder(const std::vector<const Task*>* base_taskset,
         const std::vector<const Task*>* reexec_taskset,
         const std::vector<const Task*>* full_taskset);
    
    bool next_level(const Path &path, const std::vector<const Task*> &activables) noexcept;
    bool build_node(const Path &path, const Task* t, const std::vector<const Task*> &activables) noexcept;

private:
    uint_fast16_t current_level;
    
    std::vector<std::vector<float>> fault_rate_real;
    
    const std::vector<const Task*>* base_taskset;
    const std::vector<const Task*>* reexec_taskset;
    const std::vector<const Task*>* full_taskset;

    void search_feasible_droppings(std::vector<bool> &valids, std::vector<std::vector<const Task*>> &fds, const Path &path, std::vector<const Task*> &activables, const std::vector<std::vector<float>> &saved_fault_rate_real) noexcept;
    void do_update_probability(const Task* t, float new_probability) noexcept;
    void restore_probability(const Task* t, const std::vector<std::vector<float>> &saved_fault_rate_real) noexcept;
    void powerset(std::vector<std::vector<const Task*>> &fds, std::vector<const Task*> seq) noexcept;
    bool verify_feasibility(const Task* task) const noexcept;
};

#endif
