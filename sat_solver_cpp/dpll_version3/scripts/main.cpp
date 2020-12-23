#include <sat_solver_cpp/dpll.hpp>

#include <string>
#include <iostream>
#include <chrono>

bool option_print_sat_assignment = false;

int main(int argc, char **argv)
{
    std::ios_base::sync_with_stdio(false);

    if (argc != 2)
    {
        std::cout << "Error, call: sat_solver filename.dimacs" << std::endl;
        return -1;
    }
    std::string file_name = argv[1];

    auto t1 = std::chrono::high_resolution_clock::now();

    DPLL my_solver(file_name);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << "Built problem with " << my_solver.get_num_vars() << " vars and "
              << my_solver.get_num_clauses() << " clauses." << std::endl;

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    std::cout << "Time to construct: " << duration << " microseconds \n";

    std::cout << "Solving..." << std::endl;

    t1 = std::chrono::high_resolution_clock::now();

    std::pair<bool, std::vector<int>> res = my_solver.solve();

    t2 = std::chrono::high_resolution_clock::now();

    if (!res.first)
    {
        std::cout << "UNSAT" << std::endl;
    }
    else
    {
        std::cout << "SAT" << std::endl;
        if (option_print_sat_assignment)
        {
            for (size_t i = 1; i < res.second.size(); i++)
            {
                if (res.second[i] > 0)
                {
                    std::cout << i << "=1" << std::endl;
                }
                else if (res.second[i] < 0)
                {
                    std::cout << i << "=0" << std::endl;
                }
                else
                {
                    std::cout << i << "=X" << std::endl;
                }
            }
        }
    }

    duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    std::cout << "Time to solve: " << duration << " microseconds \n";
    std::cout << "Num literal selections: " << my_solver.get_num_assigns() << "\n";
    return 0;
}
