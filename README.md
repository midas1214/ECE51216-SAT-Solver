# ECE 51216 SAT Solver Project

## Project Option

We selected **Option 1: Build a SAT Solver**.

This project implements a SAT solver based on the DPLL backtracking algorithm. The solver reads Boolean formulas in DIMACS CNF format and determines whether the formula is satisfiable. If the formula is satisfiable, the solver prints a satisfying assignment. If the formula is unsatisfiable, the solver prints `RESULT:UNSAT`.

## Implemented Features

Our solver includes the following components:

1. **DPLL Backtracking Search**
   - Branches on unassigned variables.
   - Tries one polarity first, then backtracks and tries the opposite polarity if needed.
   - Returns SAT if all variables can be assigned consistently.
   - Returns UNSAT if all branches lead to conflicts.

2. **Boolean Constraint Propagation (BCP)**
   - After each decision, the solver propagates forced assignments.
   - If a clause becomes unit, the remaining unassigned literal is forced to be true.
   - If all literals in a clause become false, the solver reports a conflict.

3. **Watched Literals**
   - Each clause keeps track of two watched literals.
   - During propagation, only clauses watching a falsified literal need to be checked.
   - This reduces unnecessary clause scans during Boolean Constraint Propagation.

4. **VSIDS-like Decision Heuristic**
   - Each variable has an activity score.
   - When a conflict occurs, variables in the conflicting clause receive an activity bump.
   - The solver chooses the unassigned variable with the highest activity score as the next decision variable.

5. **Phase Saving**
   - The solver remembers the last polarity used for each variable.
   - When the variable is selected again, the saved polarity is tried first.

## Directory Structure

```text
.
├── main.cpp
├── dimacs.h
├── dimacs.cpp
├── sat_solver.h
├── sat_solver.cpp
├── Makefile
├── mySAT
├── README.md
└── benchmarks/
    ├── simple_sat.cnf
    ├── simple_unsat.cnf
    ├── uf20-91/
    └── uf50-218/
```

## File Descriptions

### `main.cpp`

The main entry point of the program. It:

- Reads the CNF benchmark filename from the command line.
- Calls the DIMACS parser.
- Creates a `SatSolver` object.
- Runs the solver.
- Prints the result in the required format.

### `dimacs.h` and `dimacs.cpp`

These files implement the DIMACS CNF parser.

The parser reads files of the form:

```text
p cnf num_variables num_clauses
literal literal literal 0
literal literal literal 0
...
```

The parser converts DIMACS literals into the solver's internal literal encoding.

Internal literal encoding:

```text
positive literal x  -> 2 * (x - 1)
negative literal -x -> 2 * (x - 1) + 1
```

This encoding makes negation efficient:

```cpp
lit_neg(lit) = lit ^ 1
```

### `sat_solver.h` and `sat_solver.cpp`

These files contain the main SAT solving implementation.

Important data structures include:

- `Clause`: stores the literals in a clause and the indices of the two watched literals.
- `CNF`: stores the number of variables and the list of clauses.
- `assign_`: stores the current assignment of each variable.
- `reason_`: stores the reason clause for each implied assignment.
- `level_`: stores the decision level of each variable.
- `trail_`: stores the sequence of assigned literals.
- `trail_lim_`: stores the start index of each decision level.
- `watch_list_`: maps each literal to the list of clauses currently watching that literal.
- `activity_`: stores VSIDS-like activity scores for each variable.
- `saved_phase_`: stores the last used polarity for each variable.

### `Makefile`

The Makefile compiles the SAT solver into an executable named `mySAT`.

## Compilation Instructions

To compile the project, run:

```bash
make clean
make
```

This produces the executable:

```bash
mySAT
```

The project uses:

```text
g++ -O2 -std=c++17 -Wall -Wextra -pedantic
```

## Running the Solver

The solver should be run using the following format:

```bash
./mySAT benchmark.cnf
```

Example:

```bash
./mySAT benchmarks/uf20-91/uf20-01.cnf
```

If the formula is satisfiable, the output format is:

```text
RESULT:SAT
ASSIGNMENT:1=1 2=0 3=1 ...
```

If the formula is unsatisfiable, the output format is:

```text
RESULT:UNSAT
```

The program does not print extra debugging output.

## Example Runs

### Example 1: SAT benchmark

Command:

```bash
./mySAT benchmarks/uf20-91/uf20-01.cnf
```

Example output:

```text
RESULT:SAT
ASSIGNMENT:1=1 2=0 3=0 4=0 5=0 6=1 7=0 8=0 9=1 10=0 11=0 12=0 13=0 14=1 15=1 16=0 17=1 18=0 19=0 20=1
```

### Example 2: Larger SAT benchmark

Command:

```bash
./mySAT benchmarks/uf50-218/uf50-0159.cnf
```

Example output:

```text
RESULT:SAT
ASSIGNMENT:1=1 2=0 3=0 4=1 5=0 6=1 7=1 8=0 9=0 10=0 11=0 12=1 13=1 14=1 15=1 16=0 17=0 18=1 19=1 20=0 21=1 22=1 23=1 24=1 25=1 26=0 27=1 28=1 29=1 30=0 31=0 32=1 33=1 34=1 35=1 36=1 37=0 38=0 39=0 40=1 41=0 42=0 43=0 44=1 45=0 46=1 47=0 48=0 49=1 50=1
```

### Example 3: Simple UNSAT test

For the file `benchmarks/simple_unsat.cnf`:

```text
p cnf 1 2
1 0
-1 0
```

Command:

```bash
./mySAT benchmarks/simple_unsat.cnf
```

Expected output:

```text
RESULT:UNSAT
```

## Benchmarks Used

We tested the solver on:

- Small manually created SAT cases.
- Small manually created UNSAT cases.
- SATLIB-style `uf20-91` satisfiable 3-SAT benchmarks.
- SATLIB-style `uf50-218` satisfiable 3-SAT benchmarks.

The benchmark files used for testing are included in the `benchmarks/` directory.

## Cleaning Build Files

To remove object files and the executable, run:

```bash
make clean
```

## Notes

This solver implements a complete DPLL-style search with Boolean Constraint Propagation and chronological backtracking. The two main advanced heuristics are watched literals and a VSIDS-like decision heuristic. Phase saving is also implemented as an additional optimization. The solver does not implement conflict-driven clause learning, non-chronological backtracking, restarts, or clause database deletion.
