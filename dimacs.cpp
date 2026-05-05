#include "dimacs.h"
#include <fstream>
#include <sstream>
#include <string>

static inline int to_lit(int dimacs_lit) {
    int v = (dimacs_lit > 0) ? dimacs_lit : -dimacs_lit;
    int sign = (dimacs_lit > 0) ? 0 : 1;
    return 2 * (v - 1) + sign;
}

bool read_dimacs_cnf(const std::string& path, CNF& out_cnf) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    out_cnf = CNF{};

    std::string tok;
    int expected_clauses = 0;
    bool saw_header = false;

    while (in >> tok) {
        if (tok == "c") {
            std::string rest;
            std::getline(in, rest);
        } else if (tok == "p") {
            std::string fmt;
            in >> fmt >> out_cnf.nvars >> expected_clauses;

            if (fmt != "cnf" || out_cnf.nvars <= 0 || expected_clauses < 0) {
                return false;
            }

            out_cnf.clauses.reserve(expected_clauses);
            saw_header = true;
            break;
        }
    }

    if (!saw_header) {
        return false;
    }

    Clause clause;

    while (in >> tok) {
        if (tok == "c") {
            std::string rest;
            std::getline(in, rest);
            continue;
        }

        if (tok == "%") {
            break;
        }

        int dimacs_lit;

        try {
            dimacs_lit = std::stoi(tok);
        } catch (...) {
            return false;
        }

        if (dimacs_lit == 0) {
            out_cnf.clauses.push_back(std::move(clause));
            clause = Clause{};
        } else {
            clause.lits.push_back(to_lit(dimacs_lit));
        }
    }

    return out_cnf.nvars > 0;
}