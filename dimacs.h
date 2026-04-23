#pragma once
#include "sat_solver.h"
#include <string>

bool read_dimacs_cnf(const std::string& path, CNF& out_cnf);