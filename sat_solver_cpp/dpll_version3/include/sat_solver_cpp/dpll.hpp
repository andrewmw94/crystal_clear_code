#ifndef sat_solver_cpp_dpll_hpp
#define sat_solver_cpp_dpll_hpp

#include <vector>
#include <string>
#include <memory>

class DPLL {

    size_t num_vars;
    std::vector<std::vector<int>> clauses;

    public:
    DPLL(const std::string & filename);

    size_t get_num_vars() { return num_vars; }
    size_t get_num_clauses() { return clauses.size(); }

    std::pair<bool,std::vector<int>> solve();

    int get_num_assigns() {
        return num_assigns;
    }

    private:
    int recursion_level = 0;

    int num_assigns = 0;

    bool solve_recurse_helper(std::shared_ptr<std::vector<std::vector<int>>> curr_clauses, std::shared_ptr<std::vector<int>> &curr_assignment);

    bool unit_propagate(std::shared_ptr<std::vector<std::vector<int>>> curr_clauses, const std::shared_ptr<std::vector<int>> curr_assignment);

    int choose_variable_heuristically(const std::shared_ptr<std::vector<std::vector<int>>> curr_clauses, const std::shared_ptr<std::vector<int>> curr_assignment);

    void remove_clause(std::shared_ptr<std::vector<std::vector<int>>> curr_clauses, size_t index);

    void rewrite_clause_for_unit_assgn(std::shared_ptr<std::vector<std::vector<int>>> curr_clauses, size_t index, int unit_assgn);

    void cleanup_problem();

    int get_sign(int n) {
        return (n > 0) - (n < 0);
    }

};

#endif
