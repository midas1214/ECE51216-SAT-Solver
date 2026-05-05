#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

// Literal encoding:
//   var in [1..n]
//   lit = 2*(var-1) + sign
//   sign=0 means positive literal x
//   sign=1 means negative literal ~x
static inline int lit_var(int lit) { return (lit / 2) + 1; }
static inline int lit_sign(int lit) { return lit & 1; }
static inline int lit_neg(int lit) { return lit ^ 1; }

struct Clause {
    std::vector<int> lits;   // encoded literals
    int w1 = -1;             // first watched literal index
    int w2 = -1;             // second watched literal index
};

struct CNF {
    int nvars = 0;
    std::vector<Clause> clauses;
};

class SatSolver {
public:
    explicit SatSolver(CNF& cnf);

    // Returns true if SAT.
    // If SAT, assignment_out[var] is 0 or 1 for var 1..nvars.
    bool solve(std::vector<int8_t>& assignment_out);

private:
    CNF& cnf_;
    int nvars_;
    int nclauses_;

    // assignment[var] in {-1, 0, 1}
    // -1 = unassigned, 0 = false, 1 = true
    std::vector<int8_t> assign_;

    // reason[var] = clause id that implied it, or -1 for decision/unassigned
    std::vector<int> reason_;

    // decision level of each variable
    std::vector<int> level_;

    // watch_list[lit] contains clause ids watching literal lit
    std::vector<std::vector<int>> watch_list_;

    // assignment trail
    std::vector<int> trail_;

    // trail_lim_[level - 1] gives trail index where that decision level starts
    std::vector<int> trail_lim_;

    // propagation queue head
    size_t qhead_ = 0;

    // VSIDS-like activity
    std::vector<double> activity_;
    double var_inc_ = 1.0;
    double var_decay_ = 0.95;

    // Phase saving: 0 means prefer positive, 1 means prefer negative
    std::vector<int8_t> saved_phase_;

private:
    int current_level() const;
    int8_t value_lit(int lit) const;

    bool enqueue(int lit, int from_clause);
    bool init_watches();
    int propagate();

    void new_decision_level();
    void undo_to(int trail_size, int num_levels);

    int pick_branch_lit();
    void bump_activity_from_conflict(int cid);

    bool search();
};