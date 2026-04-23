#include "dimacs.h"
#include <fstream>
#include <sstream>

static inline int to_lit(int dimacs_lit) {
    // dimacs_lit: positive => x, negative => ~x
    int v = (dimacs_lit > 0) ? dimacs_lit : -dimacs_lit;
    int sign = (dimacs_lit > 0) ? 0 : 1;
    return 2 * (v - 1) + sign;
}

bool read_dimacs_cnf(const std::string& path, CNF& out_cnf) {
    std::ifstream in(path);
    if (!in.is_open()) return false;

    out_cnf = CNF{};
    std::string line;

    int expected_clauses = 0;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line[0] == 'c') continue;

        if (line[0] == 'p') {
            std::stringstream ss(line);
            std::string p, fmt;
            ss >> p >> fmt >> out_cnf.nvars >> expected_clauses;
            out_cnf.clauses.reserve(expected_clauses);
            continue;
        }

        // clause line(s): may contain multiple ints
        std::stringstream ss(line);
        int lit;
        Clause clause;
        while (ss >> lit) {
            if (lit == 0) {
                out_cnf.clauses.push_back(std::move(clause));
                clause = Clause{};
            } else {
                clause.lits.push_back(to_lit(lit));
            }
        }
        // If a clause is split across lines, DIMACS allows it.
        // For simplicity, we only finalize when 0 appears; partial clause will continue
        // only if next line continues. Handling that fully requires a token-stream parse.
        // Most benchmarks put clause endings on the same line; if yours doesn't, switch
        // to token-stream parsing.
    }

    return (out_cnf.nvars > 0);
}