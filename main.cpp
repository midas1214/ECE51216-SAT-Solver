#include "dimacs.h"
#include "sat_solver.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    CNF cnf;
    if (!read_dimacs_cnf(argv[1], cnf)) {
        return 1;
    }

    SatSolver solver(cnf);
    std::vector<int8_t> assignment;

    bool sat = solver.solve(assignment);

    if (sat) {
        std::cout << "RESULT:SAT\n";
        std::cout << "ASSIGNMENT:";

        for (int v = 1; v <= cnf.nvars; ++v) {
            int8_t val = assignment[v];
            if (val == -1) val = 0;

            if (v > 1) std::cout << " ";
            std::cout << v << "=" << int(val);
        }

        std::cout << "\n";
    } else {
        std::cout << "RESULT:UNSAT\n";
    }

    return 0;
}