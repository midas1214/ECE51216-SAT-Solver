#include "sat_solver.h"
#include <algorithm>

SatSolver::SatSolver(CNF& cnf)
    : cnf_(cnf),
      nvars_(cnf.nvars),
      nclauses_(static_cast<int>(cnf.clauses.size()))
{
    assign_.assign(nvars_ + 1, -1);
    reason_.assign(nvars_ + 1, -1);
    level_.assign(nvars_ + 1, 0);
    saved_phase_.assign(nvars_ + 1, 0);

    watch_list_.resize(2 * nvars_);
    activity_.assign(nvars_ + 1, 0.0);
}

int SatSolver::current_level() const {
    return static_cast<int>(trail_lim_.size());
}

int8_t SatSolver::value_lit(int lit) const {
    int v = lit_var(lit);
    int sign = lit_sign(lit);

    int8_t av = assign_[v];
    if (av == -1) return -1;

    bool is_true = (av == 1 && sign == 0) || (av == 0 && sign == 1);
    return is_true ? 1 : 0;
}

bool SatSolver::enqueue(int lit, int from_clause) {
    int v = lit_var(lit);
    int sign = lit_sign(lit);

    int8_t wanted_value = (sign == 0) ? 1 : 0;

    if (assign_[v] != -1) {
        return assign_[v] == wanted_value;
    }

    assign_[v] = wanted_value;
    reason_[v] = from_clause;
    level_[v] = current_level();

    // Remember the polarity that made this literal true.
    saved_phase_[v] = static_cast<int8_t>(sign);

    trail_.push_back(lit);
    return true;
}

bool SatSolver::init_watches() {
    for (auto& wl : watch_list_) {
        wl.clear();
    }

    for (int cid = 0; cid < nclauses_; ++cid) {
        Clause& c = cnf_.clauses[cid];

        if (c.lits.empty()) {
            return false;
        }

        if (c.lits.size() == 1) {
            c.w1 = 0;
            c.w2 = 0;

            int l = c.lits[0];
            watch_list_[l].push_back(cid);
        } else {
            c.w1 = 0;
            c.w2 = 1;

            watch_list_[c.lits[c.w1]].push_back(cid);
            watch_list_[c.lits[c.w2]].push_back(cid);
        }
    }

    return true;
}

int SatSolver::propagate() {
    while (qhead_ < trail_.size()) {
        int true_lit = trail_[qhead_++];
        int false_lit = lit_neg(true_lit);

        auto& wlist = watch_list_[false_lit];

        size_t i = 0;
        while (i < wlist.size()) {
            int cid = wlist[i];
            Clause& c = cnf_.clauses[cid];

            int wi;
            int other_wi;

            if (c.lits[c.w1] == false_lit) {
                wi = c.w1;
                other_wi = c.w2;
            } else {
                wi = c.w2;
                other_wi = c.w1;
            }

            int other_lit = c.lits[other_wi];

            // If the other watched literal is already true, the clause is satisfied.
            if (value_lit(other_lit) == 1) {
                ++i;
                continue;
            }

            // Try to move this watch to another literal that is not false.
            bool moved = false;

            for (int k = 0; k < static_cast<int>(c.lits.size()); ++k) {
                if (k == c.w1 || k == c.w2) continue;

                if (value_lit(c.lits[k]) != 0) {
                    // Move watch from false_lit to c.lits[k].
                    if (wi == c.w1) {
                        c.w1 = k;
                    } else {
                        c.w2 = k;
                    }

                    // Remove cid from current watch list by swap-pop.
                    wlist[i] = wlist.back();
                    wlist.pop_back();

                    // Add cid to the new watch list.
                    watch_list_[c.lits[k]].push_back(cid);

                    moved = true;
                    break;
                }
            }

            if (moved) {
                continue;
            }

            // No replacement found.
            // If the other watched literal is false, this clause is conflicting.
            if (value_lit(other_lit) == 0) {
                return cid;
            }

            // Otherwise, this clause is unit. Force other_lit to true.
            if (!enqueue(other_lit, cid)) {
                return cid;
            }

            ++i;
        }
    }

    return -1;
}

void SatSolver::new_decision_level() {
    trail_lim_.push_back(static_cast<int>(trail_.size()));
}

void SatSolver::undo_to(int trail_size, int num_levels) {
    for (int i = static_cast<int>(trail_.size()) - 1; i >= trail_size; --i) {
        int lit = trail_[i];
        int v = lit_var(lit);

        assign_[v] = -1;
        reason_[v] = -1;
        level_[v] = 0;
    }

    trail_.resize(trail_size);
    trail_lim_.resize(num_levels);

    // Everything before trail_size has already been propagated.
    qhead_ = trail_.size();
}

int SatSolver::pick_branch_lit() {
    int best_var = 0;
    double best_activity = -1.0;

    for (int v = 1; v <= nvars_; ++v) {
        if (assign_[v] == -1 && activity_[v] > best_activity) {
            best_activity = activity_[v];
            best_var = v;
        }
    }

    if (best_var == 0) {
        return -1;
    }

    int sign = saved_phase_[best_var];
    return 2 * (best_var - 1) + sign;
}

void SatSolver::bump_activity_from_conflict(int cid) {
    if (cid < 0 || cid >= nclauses_) return;

    const Clause& c = cnf_.clauses[cid];

    for (int lit : c.lits) {
        int v = lit_var(lit);
        activity_[v] += var_inc_;

        if (activity_[v] > 1e100) {
            for (int i = 1; i <= nvars_; ++i) {
                activity_[i] *= 1e-100;
            }
            var_inc_ *= 1e-100;
        }
    }

    var_inc_ *= (1.0 / var_decay_);
}

bool SatSolver::search() {
    int confl = propagate();

    if (confl != -1) {
        bump_activity_from_conflict(confl);
        return false;
    }

    int decision_lit = pick_branch_lit();

    // No unassigned variables remain.
    if (decision_lit == -1) {
        return true;
    }

    int saved_trail_size = static_cast<int>(trail_.size());
    int saved_num_levels = current_level();

    // First branch: try preferred polarity.
    new_decision_level();

    if (enqueue(decision_lit, -1)) {
        if (search()) {
            return true;
        }
    }

    undo_to(saved_trail_size, saved_num_levels);

    // Second branch: try opposite polarity.
    new_decision_level();

    if (enqueue(lit_neg(decision_lit), -1)) {
        if (search()) {
            return true;
        }
    }

    undo_to(saved_trail_size, saved_num_levels);

    return false;
}

bool SatSolver::solve(std::vector<int8_t>& assignment_out) {
    if (!init_watches()) {
        return false;
    }

    qhead_ = 0;

    // Initial unit clauses at level 0.
    for (int cid = 0; cid < nclauses_; ++cid) {
        const Clause& c = cnf_.clauses[cid];

        if (c.lits.size() == 1) {
            if (!enqueue(c.lits[0], cid)) {
                return false;
            }
        }
    }

    if (propagate() != -1) {
        return false;
    }

    bool sat = search();

    if (!sat) {
        return false;
    }

    assignment_out.assign(nvars_ + 1, 0);

    for (int v = 1; v <= nvars_; ++v) {
        assignment_out[v] = (assign_[v] == -1) ? 0 : assign_[v];
    }

    return true;
}