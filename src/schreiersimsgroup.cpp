/**
 * schreiersimsgroup.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <iostream>
#include <assert.h>
#include <string.h>
#include <map>
#include <set>
#include <vector>
#include "common.h"
#include "permutationpool.h"

#ifdef MARGOTTIMERS
#include "timer.h"
#endif

#include "generatedgroup.h"
#include "nibacexception.h"
#include "schreiersimsgroup.h"


#ifdef MARGOTTIMERS
static Timer entertime;
static Timer downtime;
static Timer revdowntime;
static Timer orbcanontime;
static Timer slowcanontime;
static int entercalls = 0;
static int downcalls = 0;
static int revdowncalls = 0;
static int orbcanoncalls = 0;
static int slowcanoncalls = 0;
#endif

namespace vorpal::nibac {
// Static initializaters.
    int *SchreierSimsGroup::tmpperm1 = 0;
    int *SchreierSimsGroup::tmpperm2 = 0;
    std::vector<int *> SchreierSimsGroup::rstack;
    bool *SchreierSimsGroup::used = 0;
    int *SchreierSimsGroup::remain = 0;
    int *SchreierSimsGroup::pos = 0;
    int **SchreierSimsGroup::hperms = 0;
    int **SchreierSimsGroup::locperms = 0;
    std::vector<int> SchreierSimsGroup::Jk;
    std::map<int, int *>::iterator *SchreierSimsGroup::mapiters = 0;


    void SchreierSimsGroup::initialize(int basesetsize) {
        GeneratedGroup::initialize(basesetsize);
        tmpperm1 = pool->newPermutation();
        tmpperm2 = pool->newPermutation();
        rstack.reserve(1000);
        used = new bool[basesetsize];
        for (int i = 0; i < basesetsize; ++i)
            used[i] = false;
        remain = new int[basesetsize];
        pos = new int[basesetsize + 1];
        pos[0] = -1;
        hperms = new int *[basesetsize + 1];
        locperms = new int *[basesetsize + 1];
        for (int i = 0; i <= basesetsize; ++i) {
            hperms[i] = pool->newPermutation();
            locperms[i] = pool->newPermutation();
        }
        getIdentityPermutation(hperms[0]);
        getIdentityPermutation(locperms[0]);
        Jk.reserve(basesetsize);
        mapiters = new std::map<int, int *>::iterator[basesetsize];
    }


    void SchreierSimsGroup::destroy() {
        delete[] mapiters;

        // Delete the things we used in the canonicity backtracking.
        for (int i = 0; i <= x; ++i) {
            pool->freePermutation(locperms[i]);
            pool->freePermutation(hperms[i]);
        }
        delete[] locperms;
        delete[] hperms;
        delete[] pos;
        delete[] remain;
        delete[] used;

        // Return the work permutations to the pool.
        pool->freePermutation(tmpperm1);
        pool->freePermutation(tmpperm2);

#ifdef MARGOTTIMERS
        std::cerr << "Down: " << downtime << ", " << downcalls << std::endl;
        std::cerr << "RevDown: " << revdowntime << ", " << revdowncalls << std::endl;
        std::cerr << "Enter: " << entertime << ", " << entercalls << std::endl;
        std::cerr << "Combined: " << orbcanontime << ", " << orbcanoncalls << std::endl;
        std::cerr << "Slow: " << slowcanontime << ", " << slowcanoncalls << std::endl;
#endif

        GeneratedGroup::destroy();
    }


    SchreierSimsGroup::SchreierSimsGroup(int *pbase, int *pbaseinv)
            : base(new int[x]), baseinv(new int[x]) {
#ifdef USEUSETS
        // usets needs to be an sx by sx array of permutations.
        usets = new int**[x];
        for (int i=0; i < x; ++i)
          usets[i] = new int*[x];
#endif

        // lists needs to be an array of length x of maps of elems to permutations.
        lists = new std::map<int, int *>[x];

        // If we specified a default base, set it.
        if (pbase) {
            memcpy(base, pbase, memsize);
            if (pbaseinv)
                memcpy(baseinv, pbaseinv, memsize);
            else
                invert(base, baseinv);
        } else
            // Set the base and the base inverse to be the identity element
            for (int i = 0; i < x; ++i)
                base[i] = baseinv[i] = i;

        // Make sure that each uset contains the identity element
        initializeSets();
    }


    SchreierSimsGroup::~SchreierSimsGroup() {
        // Delete the sets
        deleteSets();

        // Delete the base and its inverse
        delete[] base;
        delete[] baseinv;
    }


    void SchreierSimsGroup::initializeSets() {
#ifdef USEUSETS
        for (int i=0; i < x; ++i) {
          memset(usets[i], 0, x * sizeof(int*));
          usets[i][i] = idperm;
        }
#endif
    }


    void SchreierSimsGroup::deleteSets() {
        // Using the lists, delete all the permutations.
        std::map<int, int *>::iterator beginIter;
        std::map<int, int *>::iterator endIter;
        for (int i = 0; i < x; ++i) {
            beginIter = lists[i].begin();
            endIter = lists[i].end();
            for (; beginIter != endIter; ++beginIter)
                pool->freePermutation((*beginIter).second);
        }

        // Delete the lists
        delete[] lists;
        lists = 0;

#ifdef USEUSETS
        // Delete the table structure.
        for (int i=0; i < x; ++i)
          delete[] usets[i];
        delete[] usets;
        usets = 0;
#endif
    }


    int *SchreierSimsGroup::getPermutation(int row, int col) {
#ifdef USEUSETS
        return usets[row][col];
#else
        // If we are requesting the identity, return idperm.
        // NOTE: idperm might be corrupted by some other method. We should probably
        // reset it, but in the interests of efficiency and since our code never
        // changes it, we do not do so.
        if (row == col)
            return idperm;

        // Find an iterator to the permutation in question to determine
        // if it exists.
        std::map<int, int *>::iterator iter = lists[row].find(col);
        return (iter == lists[row].end() ? 0 : (*iter).second);
#endif
    }


    int SchreierSimsGroup::test(int *p, int first) {
        for (int i = first; i < x; ++i) {
            if (base[i] == p[base[i]])
                // This is the identity, so ignore
                continue;

            int *h = getPermutation(base[i], p[base[i]]);
            if (!h)
                return i;

            invert(h, tmpperm1);
            multiply(tmpperm1, p, tmpperm2);
            memcpy(p, tmpperm2, memsize);
        }
        return x;
    }


    void SchreierSimsGroup::enter(int *p) {
        enter(p, 0, true);
    }


#ifdef RECURSIVEENTER
    void SchreierSimsGroup::enter(int *p, int first, bool copyflg)
    {
      int *newperm;

      // Determine if the permutation needs to be copied, and if so,
      // perform the copy.
      int *perm, *tmpperm;
      if (copyflg) {
        perm = pool->newPermutation();
        memcpy(perm, p, memsize);
      }
      else
        perm = p;

      // Perform the call to test.
      int modifiedrow = test(perm, first);
      if (modifiedrow == x) {
        // We will abandon the permutation at this point, so there
        // is no reason to keep it around, and if it belonged to
        // the calling code, then copyflg should have been true
        // in the first place so perm is a copy and the original
        // will have been left intact.
        pool->freePermutation(perm);
        return;
      }

      // Set the indices for faster computation.
      int rowindex = base[modifiedrow];
      int colindex = perm[rowindex];

      // Insert it, deleting anything that was previously there
      tmpperm = getPermutation(rowindex, colindex);
      if (tmpperm) {
#ifdef USEUSETS
        usets[rowindex][colindex] = 0;
#endif
        lists[rowindex].erase(colindex);
        pool->freePermutation(tmpperm);
      }
#ifdef USEUSETS
      usets[rowindex][colindex] = perm;
#endif
      lists[rowindex][colindex] = perm;

      // Now we determine the new permutations that we have to insert.
      std::map< int, int* >::iterator beginIter, endIter;
      for (int j=first; j <= modifiedrow; ++j) {
        beginIter = lists[base[j]].begin();
        endIter   = lists[base[j]].end();
        for (; beginIter != endIter; ++beginIter) {
          // We don't want to consider null permutations or the identity,
          // but as the lists don't contain either, we need not concern
          // ourselves with checking for this.
          tmpperm = (*beginIter).second;

          newperm = pool->newPermutation();
          multiply(perm, tmpperm, newperm);
          enter(newperm, first, false);
        }
      }

      for (int j=modifiedrow; j < x; ++j) {
        beginIter = lists[base[j]].begin();
        endIter   = lists[base[j]].end();
        for (; beginIter != endIter; ++beginIter) {
          // We don't want to consider null permutations or the identity,
          // but as the lists don't contain either, we need not concern
          // ourselves with checking for this.
          tmpperm = (*beginIter).second;

          newperm = pool->newPermutation();
          multiply(tmpperm, perm, newperm);
          enter(newperm, first, false);
        }
      }
    }


#else


    void SchreierSimsGroup::enter(int *p, int first, bool copyflg) {
#ifdef MARGOTTIMERS
        entertime.start();
#endif

        // We first determine if the permutation needs to be copied, and
        // if so, we do so.
        if (copyflg) {
            int *pc = pool->newPermutation();
            memcpy(pc, p, memsize);
            rstack.push_back(pc);
        } else
            rstack.push_back(p);

        // Now while there are more permutations left on the stack to process,
        // we repeat.
        int *newperm;
        int *perm, *tmpperm;
        int rowindex, colindex;
        int modifiedrow;
        std::map<int, int *>::iterator beginIter, endIter;
        while (!rstack.empty()) {
#ifdef MARGOTTIMERS
            ++entercalls;
#endif

            // Get the permutation at the top of the stack and pop it.
            perm = rstack.back();
            rstack.pop_back();

            // Check which row will be modified by the addition of perm to
            // our group. If no such row exists, then perm is already
            // in our group and we can simply continue.
            modifiedrow = test(perm, first);
            if (modifiedrow == x) {
                // We will abandon the permutation at this point, so there
                // is no reason to keep it around, and if it belonged to
                // the calling code, then copyflg should have been true
                // in the first place so perm is a copy and the original
                // will have been left intact.
                pool->freePermutation(perm);
                continue;
            }

            // Set the indices for faster computation.
            rowindex = base[modifiedrow];
            colindex = perm[rowindex];

            // Insert it, deleting anything that was previously there
            tmpperm = getPermutation(rowindex, colindex);
            if (tmpperm) {
#ifdef USEUSETS
                usets[rowindex][colindex] = 0;
#endif
                lists[rowindex].erase(colindex);
                pool->freePermutation(tmpperm);
            }
#ifdef USEUSETS
            usets[rowindex][colindex] = perm;
#endif
            lists[rowindex][colindex] = perm;

            // Now we determine the new permutations that we have to insert.
            for (int j = first; j <= modifiedrow; ++j) {
                beginIter = lists[base[j]].begin();
                endIter = lists[base[j]].end();
                for (; beginIter != endIter; ++beginIter) {
                    // We don't want to consider null permutations or the identity,
                    // but as the lists don't contain either, we need not concern
                    // ourselves with checking for this.
                    tmpperm = (*beginIter).second;

                    newperm = pool->newPermutation();
                    multiply(perm, tmpperm, newperm);
                    rstack.push_back(newperm);
                }
            }

            for (int j = modifiedrow; j < x; ++j) {
                beginIter = lists[base[j]].begin();
                endIter = lists[base[j]].end();
                for (; beginIter != endIter; ++beginIter) {
                    // We don't want to consider null permutations or the identity,
                    // but as the lists don't contain either, we need not concern
                    // ourselves with checking for this.
                    tmpperm = (*beginIter).second;

                    newperm = pool->newPermutation();
                    multiply(tmpperm, perm, newperm);
                    rstack.push_back(newperm);
                }
            }
        }

#ifdef MARGOTTIMERS
        entertime.stop();
#endif
    }

#endif


    void SchreierSimsGroup::down(int r, int s) {
        // If we are already in the proper position, we have nothing to do.
        if (s == r)
            return;

        // Two different cases to consider; s < r, and s > r.
        // Margot only deals with the case s > r, so we use his
        // algorithm for this case, and improvise our own technique for
        // the former.
        if (s < r) {
#ifdef MARGOTTIMERS
            revdowntime.start();
            ++revdowncalls;
#endif
            // Essentially, it is sufficient to clear the table of all
            // permutations in rows s to r-1, swap s and r, and re-enter.
            std::vector<int *> permstoenter;
            std::map<int, int *>::iterator beginIter, endIter;
            for (int i = s; i < r; ++i) {
                beginIter = lists[base[i]].begin();
                endIter = lists[base[i]].end();
                for (; beginIter != endIter; ++beginIter) {
                    permstoenter.push_back((*beginIter).second);
#ifdef USEUSETS
                    // Clear the entry from the table.
                    usets[base[i]][(*beginIter).first] = 0;
#endif
                }

                // Clear the row from the lists.
                lists[base[i]].clear();
            }

            // Swap the base.
            int tmp = base[r];
            base[r] = base[s];
            base[s] = tmp;

            // Recalculate the inverse
            invert(base, baseinv);

            // Re-enter the permutations. They clearly fix base[0],...,base[s-1].
            std::vector<int *>::iterator pbeginIter = permstoenter.begin();
            std::vector<int *>::iterator pendIter = permstoenter.end();
            for (; pbeginIter != pendIter; ++pbeginIter)
                enter(*pbeginIter, s, false);
#ifdef MARGOTTIMERS
            revdowntime.stop();
#endif
        } else {
#ifdef MARGOTTIMERS
            downtime.start();
            ++downcalls;
#endif
            // We move the point at position r to position s
            // Accumulate the non-empty entries in row usets[base[r]]
            // and set the row to be the identity row

            // We store the non-empty, non identity permutations in
            // row base[r] in p.
            int index = base[r];
            std::map<int, int *> p = lists[index];

            // We set the row to be the identity row in the lists
            lists[index].clear();

            std::map<int, int *>::iterator beginIter = p.begin();
            std::map<int, int *>::iterator endIter = p.end();
#ifdef USEUSETS
            // We set the row to be the identity row in the sets
            // As the identity permutation isn't in the list for this row,
            // and we're only removing permutations from the list, we don't
            // need to explicitly re-enter the identity.
            for (; beginIter != endIter; ++beginIter)
              usets[index][(*beginIter).first] = 0;
#endif

            // Reorder the base
            int t = base[r];
            for (int i = r + 1; i <= s; ++i)
                base[i - 1] = base[i];
            base[s] = t;

            // Recalculate the inverse
            invert(base, baseinv);

            // Reinsert the permutations we removed
            beginIter = p.begin();
            endIter = p.end();
            for (; beginIter != endIter; ++beginIter)
                enter((*beginIter).second, r, false);
#ifdef MARGOTTIMERS
            downtime.stop();
#endif
        }
    }


    int SchreierSimsGroup::getNumGenerators(void) {
        int size = 0;
        for (int i = 0; i < x; ++i)
            size += lists[i].size();
        return size;
    }

    unsigned long SchreierSimsGroup::getSize(void) {
        unsigned long size = 1;
        for (int i = 0; i < x; ++i)
            size *= (((unsigned long) lists[i].size()) + 1);
        return size;
    }


#ifdef DEBUG
    void SchreierSimsGroup::printTableStructure()
    {
      std::cerr << "+ TABLE STRUCTURE +" << std::endl;
      std::map< int, int* >::iterator beginIter, endIter;
      for (int i=0; i < x; ++i) {
        std::cerr << i << ":";
        beginIter = lists[i].begin();
        endIter   = lists[i].end();
        for (; beginIter != endIter; ++beginIter)
          std::cerr << " " << (*beginIter).first;
        std::cerr << std::endl;
      }
      std::cerr << std::endl;
    }

#endif


    bool SchreierSimsGroup::isCanonicalAndOrbInStab(int p, int k,
                                                    std::set<int> &orbit, int *part_zero,
                                                    bool canonflag, bool orbflag,
                                                    bool quicktest) {
        // GOAL: If canonflag is set, we test to see if {B[0], ... , B[k-1], p} is the
        // lexicographically first in its orbit. If orbflag is set, we want to calculate
        // the orbit of p in the stabilizer of {B[0], ..., B[k-1]}.

        int index;
        bool orbitflag;
        bool lex_min;
        int bound;
        bool flag;
        int elem;
        int *h;
        std::vector<int>::iterator vbeginIter, vendIter;

        // One of the flags needs to be set or this call makes no sense.
        assert(canonflag || orbflag);

        // If we need to, move p to B[k]. This must be done for both canon and orbit
        // calculations.
        if (baseinv[p] != k)
            down(baseinv[p], k);

        // If the quick canonicity test is turned off and we want orbits, this is erroneous.
        // TODO: Why? This is silly, but should still be possible.
        if (canonflag && orbflag && !quicktest)
            throw IllegalOperationException(
                    "SchreierSimsGroup trying to calculate orbits when quick canonicity test turned off");

        // If we are calculating canonicity and not using the quick test, call isCanonical.
        if (!quicktest && canonflag)
            return isCanonical(k);

#ifdef MARGOTTIMERS
        // Start timing.
        ++orbcanoncalls;
        orbcanontime.start();
#endif

        // Set up the remain array. If we are checking canonicity, this is
        // required to be B[0], ..., B[k]. If we are just checking orbits,
        // this should be B[0], ..., B[k-1].
        memcpy(remain, base, (canonflag ? (k + 1) : k) * sizeof(int));

        if (orbflag) {
            // Set up the basic orbit Jk. We add p manually since we don't actually
            // include the identity permutation in the permutation lists.
            Jk.clear();
            Jk.push_back(p);
            std::map<int, int *>::iterator beginIter = lists[p].begin();
            std::map<int, int *>::iterator endIter = lists[p].end();
            for (; beginIter != endIter; ++beginIter)
                Jk.push_back((*beginIter).first);
        }

        // Set up the backtracking. We build up on index under the assumption
        // that our permutation has not been covered for the orbit calculation
        // already (orbitflag), and that we are lexicographically smallest in
        // our orbit.
        index = 0;
        orbitflag = true;
        lex_min = true;

        // Used should already be false, and pos[0] should be -1. This is all
        // that is important, so we have no initialization to do with regards
        // to these two arrays.
        // Begin the backtracking.
        while (index >= 0) {
            // Have we reached a point where we have a permutation that will stabilize
            // 0,...,k-1 and that has not already been used to compute the orbit? Note
            // we check that k is not used because if it is, then the permutation
            // might not stabilize base elements 0, ..., k-1 and we have instead taken
            // a backtracking branch for canonicity.
            if (index == k && orbflag && orbitflag && !used[k]) {
                // Record that we have now covered this permutation in the orbit.
                orbitflag = false;

                // Add the permutation executed on the basic orbit to the orbit.
                vbeginIter = Jk.begin();
                vendIter = Jk.end();
                for (; vbeginIter != vendIter; ++vbeginIter)
                    orbit.insert(locperms[index][*vbeginIter]);

                // If the canonflag is not set, we cannot extend, so we backtrack.
                if (!canonflag) {
                    --index;
                    continue;
                }
            }

            // If index is k+1, we cannot extend, and so we backtrack.
            if (index == k + 1) {
                --index;
                continue;
            }

            // If we are in a position where we will select a new element and
            // we've already selected an element here, return the element to the
            // remaining set.
            if (pos[index] >= 0)
                used[pos[index]] = false;

            // Get the next valid remaining elem (i.e. the element to which we
            // will permute base[index]) while testing canonicity. There are only
            // k or k+1 elems in remain, so we never look past this. Note that if we
            // already know that we are not canonical, then we never need to
            // consider element k in our permutation, as we're simply interested
            // in determining the orbit.
            bound = (canonflag && lex_min ? k + 1 : k);
            flag = false;
            for (++pos[index]; pos[index] < bound; ++pos[index]) {
                // If it has already been used, skip it.
                if (used[pos[index]])
                    continue;

                // Determine the element.
                elem = hperms[index][remain[pos[index]]];

                // Test canonicity.
                if (canonflag && baseinv[elem] >= part_zero[index]) {
                    // We are not canonical. Indicate this.
                    lex_min = false;

                    // We still want to compute the orbit, because we only use the
                    // orbit if we are not canonical. However, if we have already
                    // used element k of our base, then this is a branch of
                    // backtracking that cannot be extended for orbit calculation
                    // purposes, so we can backtrack to the point before k was chosen.
                    if (used[k]) {
                        // Continuously backtrack until k was used, and then backtrack
                        // once more.
                        while (pos[index] != k) {
                            // Re-mark the element as being unused.
                            used[pos[index]] = false;

                            // Reset the selection counter.
                            pos[index] = -1;

                            // Move back.
                            --index;
                        }

                        // Now we are at the position where k was chosen, so return it
                        // and move back once more.
                        used[k] = false;
                        pos[index] = -1;
                        --index;

                        // Now we want to backtrack.
                        flag = true;
                        break;
                    }
                }

                // Now we want to determine if we can map B[index] to elem. If not,
                // we try another elem in remain.
                h = getPermutation(base[index], elem);
                if (!h)
                    continue;

                // If we reach this point, we've found a permutation that meets our
                // criterion, so we break.
                break;
            }

            // If the flag was set, it was because we deemed ourself non-canonical
            // and backtracked to a point where our backtracking was relevant to
            // orbit calculations. We are ready to simply loop at this point.
            if (flag)
                continue;

            // If we have exhausted the remain set and could not map B[index] to
            // anything, then we don't need to extend. We cannot stabilize B[0],...,B[k-1]
            // and we can't construct a permutation mapping B[0],...,B[k] to anything
            // lexicographically smaller.
            if (pos[index] >= bound) {
                pos[index] = -1;
                --index;
                continue;
            }

            // We have now reached a point where we can extend.
            // Record our use of the new element, indicating that, clearly, as
            // we've determined a new permutation, we haven't covered it in the orbit
            // calculation.
            orbitflag = true;
            used[pos[index]] = true;

            // Increment index, reset the counter, and calculate the new hperm and
            // locperm.
            ++index;
            invert(h, tmpperm1);
            multiply(tmpperm1, hperms[index - 1], hperms[index]);
            multiply(locperms[index - 1], h, locperms[index]);
            pos[index] = -1;

            // We can now safely extend by looping.
        }

#ifdef MARGOTTIMERS
        orbcanontime.stop();
#endif

        // We have calculated the orbit and stored it in orbit. We also know
        // whether we are canonical; return this information.
        return lex_min;
    }


    bool SchreierSimsGroup::isCanonical(int k) {
        // We will determine if {B[0], ..., B[k]} is canonical. This will be accomplished
        // by constructing permutations over the elements and seeing if we can't map to
        // something lexicographically smaller. We use an STL vector to store the permuted
        // elements in a stored fashion.
        std::vector<int> sorted;

        // Some static members that we need.
        static int index, indexp1;
        static std::vector<int>::iterator iter;

#ifdef MARGOTTIMERS
        ++slowcanoncalls;
        slowcanontime.start();
#endif

        // We use the remain array to determine if we checked for the identity permutation
        // in each row, since it is not explicitly included in lists.
        remain[0] = 0;

        // Initialize the first map iterator.
        static std::map<int, int *>::iterator mendIter;
        mapiters[0] = lists[base[0]].begin();

        // Begin the backtracking.
        for (index = 0; index >= 0;) {
            indexp1 = index + 1;

            // Pick the next permutation for B[index].
            mendIter = lists[base[index]].end();

            // If we have not yet picked the identity, do so.
            if (!remain[index]) {
                memcpy(hperms[indexp1], hperms[index], sizeof(int) * x);
                remain[index] = 1;
            }
                // We have picked the identity. Proceed with the normal permutations.
            else {
                // If there are no more permutations, then we cannot proceed and must backtrack.
                if (mapiters[index] == mendIter) {
                    if (index > 0)
                        // As we are moving back, we must remove the previous element from our
                        // sorted set.
                        removeSorted(hperms[index][base[index - 1]], sorted);
                    --index;
                    continue;
                }

                // We now have a permutation that moves index. Create the total
                // permutation and insert into the next position of hperms.
                multiply(hperms[index], (*(mapiters[index])).second, hperms[indexp1]);

                // Indicate that we have tried this position by extending mapiters.
                ++mapiters[index];
            }

            // Permute base[index] and add it to the sorted set.
            insertSorted(hperms[indexp1][base[index]], sorted);

            // To test canonicity, we compare the two sets. Our original idea was to do
            // so from epos on only, but this does not work as the earlier part may play
            // an important role (think 0 5 7 9 and 1 4 5 8, with epos = 3). We could
            // optimize this slightly and retain what parts are canonically equal, but
            // we do not bother with this now.
            iter = sorted.begin();
            for (int i = 0; i <= index; ++i, ++iter)
                if (*iter < base[i]) {
#ifdef MARGOTTIMERS
                    slowcanontime.stop();
#endif
                    return false;
                } else if (*iter > base[i])
                    break;

            // At this point, either sorted is lex. larger than B or they are lex. the same,
            // so we try to extend.

            // If we have covered all the elements, we cannot proceed, but we do
            // want to test the rest of the permutations in this row, so loop.
            if (index == k) {
                removeSorted(hperms[indexp1][base[index]], sorted);
                continue;
            }

            // Otherwise, it is possible to extend, so we do so.
            index = indexp1;
            if (index <= k)
                remain[index] = 0;
            mapiters[index] = lists[base[index]].begin();
        }

        // If we reached this point, we have tried every permutation and the set
        // is canonical.
#ifdef MARGOTTIMERS
        slowcanontime.stop();
#endif

        return true;
    }


    int SchreierSimsGroup::insertSorted(int elem, std::vector<int> &elems) {
        // Binary search the elems for the proper position to insert.
        static int firstpos, lastpos, epos;

        // If there are no items in elem, we simply insert this element.
        if (!elems.size()) {
            elems.push_back(elem);
            return 0;
        }

        firstpos = 0;
        lastpos = elems.size() - 1;
        while (firstpos <= lastpos) {
            epos = (firstpos + lastpos + 1) / 2;

            // If elem is at pos, we have problems as these are sets and duplicates
            // are not permitted.
            assert(elems[epos] != elem);

            if (elems[epos] < elem)
                firstpos = epos + 1;
            else
                lastpos = epos - 1;
        }

        // We now want to insert into elems.begin() + pos.
        elems.insert(elems.begin() + firstpos, elem);
        return epos;
    }


    void SchreierSimsGroup::removeSorted(int elem, std::vector<int> &elems) {
        // Binary search the elems for the proper position to insert.
        static int firstpos, lastpos, epos;

        // The set cannot be empty.
        assert(elems.size() > 0);

        firstpos = 0;
        lastpos = elems.size() - 1;
        while (firstpos <= lastpos) {
            epos = (firstpos + lastpos + 1) / 2;

            // If elem is at pos, we are at the place we want to remove.
            if (elems[epos] == elem) {
                elems.erase(elems.begin() + epos);
                return;
            }

            if (elems[epos] < elem)
                firstpos = epos + 1;
            else
                lastpos = epos - 1;
        }

        // If we reach this point, there was a serious error; we tried to remove something
        // that wasn't in the set, which should never be done!
        // TODO: This was assert(true). Why?
        assert(false);
    }
};
#ifdef NODEGROUPS
Group *SchreierSimsGroup::makeCopy()
{
  int *p;
  SchreierSimsGroup *newgroup = new SchreierSimsGroup(base, baseinv);

  // Make copies of all the permutations and insert them into the new group.
  std::map< int, int* >::iterator beginIter, endIter;
  for (int i=0; i < x; ++i) {
    beginIter = lists[i].begin();
    endIter   = lists[i].end();
    for (; beginIter != endIter; ++beginIter) {
      p = pool->newPermutation();
      memcpy(p, (*beginIter).second, memsize);
      (newgroup->lists[i])[(*beginIter).first] = p;
#ifdef USEUSETS
      // NOT ADVISABLE TO USE USETS WITH NODEGROUPS! This will consume huge amounts of RAM.
      usets[i][(*beginIter).first] = p;
#endif
    }
  }

  return newgroup;
}
#endif
