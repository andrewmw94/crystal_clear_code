#include <sat_solver_cpp/dpll.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

DPLL::DPLL(const std::string &filename) : num_vars(0), clauses()
{
    std::ifstream input_file(filename);
    if (!input_file.is_open())
    {
        std::cout << "ERROR: could not open file: " << filename << std::endl;
    }

    std::string line;

    while (std::getline(input_file, line))
    {
        if (line.size() < 1)
        {
            continue;
        }
        else if (line[0] == 'c')
        {
            continue;
        }
        else if (line[0] == 'p')
        {
            std::istringstream is(line);

            char p;
            std::string format;
            unsigned int num_vs;
            unsigned int num_cls;

            is >> p >> format >> num_vs >> num_cls;

            if (format != "cnf")
            {
                std::cout << "ERROR: we only support CNF files: " << line.substr(2, 3) << std::endl;
            }
            else
            {
                num_vars = num_vs;
                clauses.reserve(num_cls);
            }
        }
        else
        {
            std::vector<int> clause;
            int next_int = 0;
            std::istringstream is(line);
            while (is >> next_int)
            {
                if (next_int == 0)
                {
                    break;
                }
                if ((size_t)std::abs(next_int) > num_vars)
                {
                    std::cout << "ERROR: variable: " << next_int << " is greater than num_vars: " << num_vars
                              << std::endl;
                }
                clause.push_back(next_int);
            }
            clauses.push_back(clause);
        }
    }
    cleanup_problem();
}

std::pair<bool, std::vector<int>> DPLL::solve()
{
    std::shared_ptr<std::vector<int>> assignments = std::make_shared<std::vector<int>>();

    for (size_t i = 0; i < num_vars + 1; i++)
    {
        assignments->push_back(0);
    }

    bool res = solve_recurse_helper(std::make_shared<std::vector<std::vector<int>>>(clauses), assignments);

    return std::make_pair(res, *assignments);
}

// Return true if curr_assignment is a satisfying assignment; Return false if no
// extension of curr_assignment is a satisfying assignment
bool DPLL::solve_recurse_helper(std::shared_ptr<std::vector<std::vector<int>>> curr_clauses,
                                std::shared_ptr<std::vector<int>> &curr_assignment)
{
    recursion_level++;
    bool res = unit_propagate(curr_clauses, curr_assignment);
    if (!res)
    {
        recursion_level--;
        return false;
    }

    if (curr_clauses->size() > 0)
    {
        int assign = choose_variable_heuristically(curr_clauses, curr_assignment);
        if (assign == 0)
        {
            recursion_level--;
            return false;
        }

        std::vector<std::vector<int>> cls_cpy = *curr_clauses;
        std::vector<int> new_cls;
        new_cls.push_back(assign);
        cls_cpy.push_back(new_cls);
        std::shared_ptr<std::vector<int>> assgn_cpy = std::make_shared<std::vector<int>>(*curr_assignment);
        (*assgn_cpy)[std::abs(assign)] = get_sign(assign);
        if (solve_recurse_helper(std::make_shared<std::vector<std::vector<int>>>(cls_cpy), assgn_cpy))
        {
            curr_assignment = assgn_cpy;
            recursion_level--;
            return true;
        }

        cls_cpy = *curr_clauses;
        new_cls.clear();
        new_cls.push_back(-assign);
        cls_cpy.push_back(new_cls);
        assgn_cpy = std::make_shared<std::vector<int>>(*curr_assignment);
        (*assgn_cpy)[std::abs(assign)] = get_sign(-assign);
        if (solve_recurse_helper(std::make_shared<std::vector<std::vector<int>>>(cls_cpy), assgn_cpy))
        {
            curr_assignment = assgn_cpy;
            recursion_level--;
            return true;
        }
    }
    else
    {
        recursion_level--;
        return true;
    }

    recursion_level--;
    return false;
}

// Make all unit-prop deductions available. Return false if we derive a
// contradiction; true otherwise.
bool DPLL::unit_propagate(std::shared_ptr<std::vector<std::vector<int>>> curr_clauses,
                          const std::shared_ptr<std::vector<int>> curr_assignment)
{
    if (curr_clauses->size() == 0)
    {
        return true;
    }
    int next_unit_assignment = 0;
    for (size_t i = curr_clauses->size(); i > 0; i--)
    {
        if ((*curr_clauses)[i - 1].size() == 1)
        {
            next_unit_assignment = (*curr_clauses)[i - 1][0];
            remove_clause(curr_clauses, i - 1);
            break;
        }
    }

    if (next_unit_assignment == 0)
    {
        return true;
    }

    while (next_unit_assignment != 0)
    {
        int unit_assignment = next_unit_assignment;
        next_unit_assignment = 0;

        if ((*curr_assignment)[std::abs(unit_assignment)] != 0)
        {
            if ((*curr_assignment)[std::abs(unit_assignment)] == -unit_assignment)
            {
                std::cout << "ERROR: The assignment doesn't agree with the unit_assignment. \n";

                std::cout << "clauses: \n";
                for (std::vector<int> v : *curr_clauses)
                {
                    for (int i : v)
                    {
                        std::cout << i << ", ";
                    }
                    std::cout << "\n";
                }

                std::cout << "assignment: \n";
                for (size_t i = 0; i < curr_assignment->size(); i++)
                {
                    std::cout << i << " = " << curr_assignment->at(i) << "\n";
                }

                return false;
            }
        }
        else
        {
            (*curr_assignment)[std::abs(next_unit_assignment)] = get_sign(next_unit_assignment);
        }

        for (size_t i = 0; i < curr_clauses->size(); i++)
        {
            std::vector<int> cls = (*curr_clauses)[i];

            for (int j : cls)
            {
                if (j == unit_assignment)
                {
                    remove_clause(curr_clauses, i);
                    i--;
                    break;
                }
                else if (j == -unit_assignment)
                {
                    rewrite_clause_for_unit_assgn(curr_clauses, i, unit_assignment);
                    // We derived a contradiction
                    if ((*curr_clauses)[i].size() == 0)
                    {
                        return false;
                    }
                    break;
                }
            }
            if (next_unit_assignment == 0 && (*curr_clauses)[i].size() == 1)
            {
                next_unit_assignment = (*curr_clauses)[i][0];
                remove_clause(curr_clauses, i);
                i--;
            }
        }
    }
    return true;
}

// return 0 if no variable can be chosen heuristically
int DPLL::choose_variable_heuristically(const std::shared_ptr<std::vector<std::vector<int>>> curr_clauses,
                                        const std::shared_ptr<std::vector<int>> curr_assignment)
{
    num_assigns++;
    std::vector<unsigned int> positive_counts;
    std::vector<unsigned int> negative_counts;

    for (size_t i = 0; i < num_vars + 1; i++)
    {
        positive_counts.push_back(0);
        negative_counts.push_back(0);
    }

    bool found_two_clause = false;
    for (const std::vector<int> &cls : *curr_clauses)
    {
        if (cls.size() == 2)
        {
            found_two_clause = true;
            if (cls[0] > 0)
            {
                positive_counts[cls[0]]++;
            }
            else
            {
                negative_counts[-cls[0]]++;
            }
            if (cls[1] > 0)
            {
                positive_counts[cls[1]]++;
            }
            else
            {
                negative_counts[-cls[1]]++;
            }
        }
    }

    // We there are no 2-clauses
    if (!found_two_clause)
    {
        // positive_counts and negative_counts are all zeros
        for (const std::vector<int> &cls : *curr_clauses)
        {
            for (int i : cls)
            {
                if (i > 0)
                {
                    positive_counts[i]++;
                }
                else if (i < 0)
                {
                    negative_counts[-i]++;
                }
            }
        }
    }

    int max_index = 0;
    for (size_t i = 1; i < positive_counts.size(); i++)
    {
        if ((*curr_assignment)[i] != 0)
        {  // already assigned
            continue;
        }
        if (positive_counts[i] + negative_counts[i] > positive_counts[max_index] + negative_counts[max_index])
        {
            max_index = i;
        }
    }

    if (max_index == 0)
    {
        std::cout << "ERROR: we chose var 0\n";
        std::cout << curr_clauses->size() << "\n";
        for (std::vector<int> clause : *curr_clauses)
        {
            for (int i : clause)
            {
                std::cout << i << ", ";
            }
            std::cout << "\n";
        }
    }

    if (positive_counts[max_index] > negative_counts[max_index])
    {
        return max_index;
    }
    else
    {
        return -max_index;
    }
}

void DPLL::remove_clause(std::shared_ptr<std::vector<std::vector<int>>> curr_clauses, size_t index)
{
    if (index > curr_clauses->size() - 1)
    {
        std::cout << "ERROR: Trying to remove non-existent clause.\n";
    }
    else if (index < curr_clauses->size() - 1)
    {
        (*curr_clauses)[index] = (*curr_clauses)[curr_clauses->size() - 1];
    }
    curr_clauses->pop_back();
}

void DPLL::rewrite_clause_for_unit_assgn(std::shared_ptr<std::vector<std::vector<int>>> curr_clauses,
                                         size_t index, int unit_assgn)
{
    std::vector<int> tmp_cls;
    for (int v : (*curr_clauses)[index])
    {
        if (v != -unit_assgn)
        {
            tmp_cls.push_back(v);
        }
    }
    (*curr_clauses)[index] = tmp_cls;
}

// Ensure no clauses have the same literal twice. If (a v ~a v b), then remove the clause. If (a v a v b),
// then (a v b)
void DPLL::cleanup_problem()
{
    for (size_t i = 0; i < clauses.size(); i++)
    {
        bool removed_clause = false;
        for (size_t j = 0; j < clauses[i].size() - 1 && !removed_clause; j++)
        {
            for (size_t k = j + 1; k < clauses[i].size(); k++)
            {
                if (clauses[i][j] == -clauses[i][k])
                {
                    std::shared_ptr<std::vector<std::vector<int>>> ptr_clauses =
                        std::make_shared<std::vector<std::vector<int>>>(clauses);
                    remove_clause(ptr_clauses, i);
                    i--;
                    clauses = *ptr_clauses;
                    removed_clause = true;
                    break;
                }
                else if (clauses[i][j] == clauses[i][k])
                {
                    clauses[i][k] = clauses[i][clauses[i].size() - 1];
                    clauses[i].pop_back();
                    k--;
                }
            }
        }
    }
}
