# crystal_clear_code

Pedagogical implementations of fundamental algorithms.

## sat_solver_cpp

Various implementations to solve the boolean satisfiablity problem.

Each program expects input in the [DIMACS file format](https://logic.pdmi.ras.ru/~basolver/dimacs.html#:~:text=DIMACS%20CNF%20format.%20This%20format%20is%20widely%20accepted,defined%20by%20the%20line%20p%20cnf%20variables%20clauses.)

The hope is to show the pros and cons of different design choices.

### dpll_version1 
DPLL in a single file.

compile:

```c++ main.cpp -o dpll_version1```

### dpll_version2
DPLL using vectors of ints in a fairly standard cmake project layout.

compile:

```mkdir build```
```cd build```
```cmake ..```
```make```

### dpll_version3
DPLL using OOP practices in the same cmake project layout.

compile:

```mkdir build```
```cd build```
```cmake ..```
```make```

