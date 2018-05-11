/**
 * generatedgroup.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <nauty.h>
#include "common.h"
#include "generatedgroup.h"
#include "bitstring.h"
#include "formulation.h"
#include "group.h"
#include "nibacexception.h"
#include "permutationpool.h"
#include "timer.h"

namespace vorpal::nibac {
    void GeneratedGroup::createSymmetryGroup(Formulation &ilp) {
        // By default, we use the nauty technique, since it is by far the most efficient.
        findSymmetryGroup3(ilp);
    }


    void GeneratedGroup::firstSnPerm(int n, int *perm) {
        for (int i = 0; i < n; ++i)
            perm[i] = i;
    }


    bool GeneratedGroup::nextSnPerm(int n, int *perm, int *wksp) {
        int i, j, k;

        // Clear out the workspace array.
        memset(wksp, 0, n * sizeof(int));

        // Moving right to left, find the first position that we can increment,
        // while keeping track of what we have found so far.
        int highest = 0;
        for (i = n - 1; i >= 0; --i) {
            if (perm[i] > highest)
                highest = perm[i];
            wksp[perm[i]] = 1;

            // If we have found something higher, we can stop.
            if (highest > perm[i])
                break;
        }

        // If we have reached -1, perm was the last permutation, so we can stop.
        if (i < 0)
            return false;

        // Insert the highest element after perm[i] into pos i
        for (j = perm[i] + 1; !wksp[j]; ++j);
        perm[i] = j;
        wksp[j] = 0;

        // Now insert, in order, the remaining elements.
        for (j = i + 1, k = 0; j < n; ++j) {
            for (; !wksp[k]; ++k);
            perm[j] = k;
            ++k;
        }

        return true;
    }


    void GeneratedGroup::findSymmetryGroup1(Formulation &ilp) {
        bool flag1;
        bool flag2;
        bool flag3;

        // Get a permutation from the pool.
        int *perm = pool->newPermutation();

        // Create a row permutation and a workspace.
        int r = ilp.getNumberRows();
        int *rowperm = new int[r];
        int *wksp = new int[r];

        // Iterate over all the perms in S_x.
        flag1 = true;

        // Since this method is so painfully slow and we can't even complete it in short periods of time for
        // a small symmetry group, we time each column permutation so that we can at least compute an estimate.
        Timer timer;
        for (firstSnPerm(x, perm); flag1; flag1 = nextSnPerm(x, perm, wksp)) {
            timer.reset();
            timer.start();

            // The first thing we need to verify is that this permutation fixes c.
            // Otherwise, it is not worth considering.
            flag2 = true;
            for (int i = 0; i < x; ++i)
                if (ilp.getObjectiveCoefficient(i) != perm[ilp.getObjectiveCoefficient(i)]) {
                    flag2 = false;
                    break;
                }

            // If flag2 is set, it does not, so loop and try the next permutation.
            if (!flag2)
                goto NEXTPERM;

            // Now we need to try to find a suitable row permutation that fixes both
            // the bounds of the inequalities and the matrix of the coefficients.
            flag2 = true;
            for (firstSnPerm(r, rowperm); flag2; flag2 = nextSnPerm(r, rowperm, wksp)) {
                // We first check to see if we fix the bounds.
                flag3 = true;
                for (int i = 0; i < r; ++i)
                    if (ilp.getBCoefficient(i) != rowperm[ilp.getBCoefficient(i)]) {
                        flag3 = false;
                        break;
                    }

                // If not, check the next row permutation.
                if (!flag3)
                    continue;

                // Now we investigate whether the matrix of coefficients is fixed by
                // both permutations.
                flag3 = true;
                for (int i = 0; i < r; ++i) {
                    for (int j = 0; j < x; ++j)
                        if (ilp.getMatrixCoefficient(i, j) != ilp.getMatrixCoefficient(rowperm[i], perm[j])) {
                            flag3 = false;
                            break;
                        }

                    // If the matrix wasn't fixed, we break out of this loop, too.
                    if (!flag3)
                        break;
                }

                // If flag3 is no longer set, then this permutation doesn't fix the matrix.
                // Try the next one.
                if (!flag3)
                    continue;

                // We have found a permutation, so enter it.
                enter(perm);

                // We no longer need to check row permutations, since we've found one that matched
                // all the sufficient conditions.
                break;
            }

            NEXTPERM:
            // Output information on how long it took to consider the column permutation.
            timer.stop();
            std::cerr << "Time to consider column permutation: " << timer << std::endl;
        }

        // Free the memory.
        delete[] wksp;
        delete[] rowperm;
        pool->freePermutation(perm);
    }


    void GeneratedGroup::findSymmetryGroup2(Formulation &ilp) {
        int i, j;
        bool flg, flg2;

        // We now allocate memory to manage the partitioning scheme
        std::vector <std::map<int, bitstring>> partscheme(ilp.getNumberColumns());
        std::vector <std::vector<bitstring>> refinement(x);
        for (i = 0; i < x; ++i)
            refinement[i].resize(ilp.getNumberRows());

        // Precalculate the partitioning scheme
        // We do this as follows:
        // For each column of our original matrix, we determine where each
        // coefficient can be mapped, and this information is stored in a
        // bitmask. For example, if we have column 12422412, read from top
        // to bottom, the partitioning scheme for this column is:
        // partscheme[col][1] = 10000010 (in reverse)
        // partscheme[col][2] = 01011001 (in reverse)
        // partscheme[col][4] = 00100100 (in reverse)

        // Now we iterate over the columns, calculating
        for (i = 0; i < ilp.getNumberColumns(); ++i)
            for (j = 0; j < ilp.getNumberRows(); ++j)
                partscheme[i][ilp.getMatrixCoefficient(j, i)].set(j);

        // Note: now the partitioning scheme is complete, and we never have to recalculate
        // it. This simplifies finding row permutations; we simply keep refining until
        // the b vector is no longer fixed or until some row does not map to another.

        // Create a column permutation and clear it
        assert(ilp.getNumberColumns() == x);
        int *colperm = pool->newPermutation();
        for (i = 0; i < x; ++i)
            colperm[i] = -1;

        // Indicate that all elements are unused
        bool *colunused = new bool[ilp.getNumberColumns()];
        for (i = 0; i < ilp.getNumberColumns(); ++i)
            colunused[i] = true;

        // Before backtracking, we create a convenience bitset of all 1s of the
        // appropriate size.
        bitstring allones;
        allones.flip();

        // We now create a string of 1s of length corresponding to the number of
        // rows in our matrix; this is used to check surjectivity.
        bitstring varones;
        for (i = 0; i < ilp.getNumberRows(); ++i)
            varones.set(i);

        // Begin backtracking on the column permutation
        int permpos = 0;
        while (permpos >= 0) {
            // Return this element to the unused pile
            int start = colperm[permpos] + 1;
            if (colperm[permpos] >= 0) {
                colunused[colperm[permpos]] = true;
                colperm[permpos] = -1;
            }

            // Add the next highest element to the permutation
            for (j = start; j < ilp.getNumberColumns(); ++j)
                // Only look for unused elements that fix the c vector at permpos
                if (colunused[j] && ilp.getObjectiveCoefficient(permpos) == ilp.getObjectiveCoefficient(j)) {
                    // Set this element
                    colperm[permpos] = j;
                    colunused[j] = false;
                    break;
                }

            // If we haven't added anything, then there was nothing possible for us to add.
            if (colperm[permpos] < 0) {
                --permpos;
                continue;
            }

            // We now refine the row partitioning
            for (i = 0; i < ilp.getNumberRows(); ++i) {
                refinement[permpos][i] = (permpos > 0 ? refinement[permpos - 1][i] : allones);
                refinement[permpos][i].band(partscheme[permpos][ilp.getMatrixCoefficient(i, colperm[permpos])]);
            }

            // Check if the refinement is empty, or does not fix the b vector, in
            // which case we begin backtracking
            // Another thing we check is surjectivity. If we cannot extend our
            // permutation into a surjective mapping, then we backtrack immediately.
            // This is not a necessary check; continuing the extension of our column
            // permutation would reveal this eventually. However, pruning permutations
            // early will result in more efficiency.
            flg = false;
            bitstring tmp;

            for (i = 0; i < ilp.getNumberRows(); ++i) {
                tmp.bor(refinement[permpos][i]);

                // Check if empty - i.e. no row permutation can fix the matrix
                if (refinement[permpos][i] == 0) {
                    flg = true;
                    break;
                }

                // Check if it cannot fix the b vector
                flg2 = true;
                for (j = 0; j < ilp.getNumberRows(); ++j)
                    if (refinement[permpos][i].get(j) && ilp.getBCoefficient(i) == ilp.getBCoefficient(j)) {
                        flg2 = false;
                        break;
                    }

                // If flg2 is still set, then we cannot fix the b vector
                if (flg2) {
                    flg = true;
                    break;
                }
            }

            // If flg is set at this point, then either there was an empty partition
            // (i.e. we cannot map row i to any other row in the current partial
            // column permutation so that the matrix will be fixed), or there is
            // no row permutation that will fix the b vector, so we backtrack.
            // If tmp doesn't have 1s corresponding to every row, then there
            // exists some row that we cannot map to (i.e. we cannot extend our
            // partial row permutation to a complete surjective mapping).
            if (flg)
                continue;
            if (tmp != varones)
                continue;

            // If the column permutation needs to be extended, we proceed
            if (permpos < (ilp.getNumberColumns() - 1)) {
                ++permpos;
                continue;
            }

            // Otherwise, we have a complete column permutation, so we enter
            // it into our collection of permutations. We backtrack, which does
            // not require us to change permpos; we want to proceed on this
            // position.
            enter(colperm);
        }

        // We are done, so we free all the intermediary memory
        delete[] colunused;
        pool->freePermutation(colperm);
    }


    void GeneratedGroup::findSymmetryGroup3(Formulation &ilp) {
        // Dynamically allocate required data structures.
        DYNALLSTAT(graph, g, g_sz);
        DYNALLSTAT(int, lab, lab_sz);
        DYNALLSTAT(int, ptn, ptn_sz);
        DYNALLSTAT(int, orbits, orbits_sz);
        static DEFAULTOPTIONS(options);
        statsblk(stats);
        setword workspace[100];
        set *gv, *gv2;

        // Generate a file to which we write.
        pid_t pid = getpid();
        std::ostringstream strm;
        strm << "/tmp/nauty." << pid;
        const char *filename = strm.str().c_str();

        FILE *f = fopen(filename, "w");
        if (!f)
            throw FileOutputException(filename);

        // Configure the options.
        options.writeautoms = TRUE;
        options.cartesian = TRUE;
        options.defaultptn = FALSE;
        options.outfile = f;

        // Create the graph.
        int c = ilp.getNumberColumns();
        int r = ilp.getNumberRows();
        int n = c + r;
        int m = (n + WORDSIZE - 1) / WORDSIZE;

        // Check the version of nauty.
        nauty_check(WORDSIZE, m, n, NAUTYVERSIONID);

        // Further allocation calls.
        char mstr[7];
        strcpy(mstr, "malloc");
        DYNALLOC2(graph, g, g_sz, m, n, mstr);
        DYNALLOC1(int, lab, lab_sz, n, mstr);
        DYNALLOC1(int, ptn, ptn_sz, n, mstr);
        DYNALLOC1(int, orbits, orbits_sz, n, mstr);

        // *** COLOURING ***
        // We begin by determining the colouring. This is done by first sorting the objective function
        // by coefficient and assigning a colour class to each coefficient. After this is done, we in
        // turn consider each constraint. We again sort by coefficient, and divide colour classes so
        // that only variables that have same coefficients are in the same colour class.
        std::vector <std::set<int>> colourclasses;

        // A map used for sorting purposes.
        std::map<int, std::set<int> > sorted;

        // Process the objective function.
        const std::vector<int> &objective = ilp.getObjectiveFunction();
        auto vobjbeginIter = objective.cbegin();
        auto vobjendIter = objective.cend();
        for (int i = 0; vobjbeginIter != vobjendIter; ++vobjbeginIter, ++i)
            (sorted[*vobjbeginIter]).insert(i);

        // We now have several colour classes, one for each coefficient.
        auto msbeginIter = sorted.begin();
        auto msendIter = sorted.end();
        for (; msbeginIter != msendIter; ++msbeginIter)
            colourclasses.push_back((*msbeginIter).second);

        // Now, for each constraint, partition the colours.
        const auto &constraints = ilp.getConstraints();
        auto cbeginIter = constraints.begin();
        auto cendIter = constraints.end();
        for (; cbeginIter != cendIter; ++cbeginIter) {
            Constraint *constraint = (*cbeginIter).second;

            // For each constraint, we sort by coefficient using a technique similar to above
            // with the objective function.
            sorted.clear();
            auto &positions = constraint->getPositions();
            auto vpbeginIter = positions.begin();
            auto vpendIter = positions.end();
            auto &coefficients = constraint->getCoefficients();
            auto vcbeginIter = coefficients.begin();
            auto vcendIter = coefficients.end();
            for (; vpbeginIter != vpendIter; ++vpbeginIter, ++vcbeginIter)
                (sorted[*vcbeginIter]).insert(*vpbeginIter);

            // We now have the condition that no two elements with different coefficients can
            // have the same colour. For each set of vertices sorted by coefficient, if they are
            // not contained in one colour class, we split into several colour classes, maintaining
            // one per nontrivial intersection with each existing colour class. For instance, if we
            // have {x, y, z} and {a, b, c, d} as two colour classes and coefficient set {x, a, b, c},
            // then we:
            // 1) Create new colour classes {x, a, b, c} int {x, y, z} = {x} and
            //    {x, a, b, c} int {a, b, c, d} = {a, b, c}.
            // 2) Remove {x, a, b, c} from previous colour classes, giving {y, z} and {d}.
            // We ultimately have the following colour classes:
            // {x}, {y, z}, {a, b, c}, {d}.

            // Iterate over the coefficient sets.
            msbeginIter = sorted.begin();
            msendIter = sorted.end();
            for (; msbeginIter != msendIter; ++msbeginIter) {
                // Fetch this coefficient set.
                std::set<int> &coeffset = (*msbeginIter).second;

                // Now we determine the intersection of this coefficient set with each preexisting
                // colour class.
                auto ccbeginIter = colourclasses.begin();
                auto ccendIter = colourclasses.end();

                // This will make a list of new colour classes, which we want to record (as opposed
                // to modifying the old list, which will mess up iterators).
                std::vector <std::set<int>> newcolourclasses;

                for (; ccbeginIter != ccendIter; ++ccbeginIter) {
                    // Fetch this colour class.
                    auto &colourclass = *ccbeginIter;

                    // Determine the intersection with the coefficient set.
                    std::set<int> intersection;
                    std::insert_iterator <std::set<int>> ii_intersection { intersection, intersection.begin() };
                    std::set_intersection(coeffset.begin(), coeffset.end(),
                                          colourclass.begin(), colourclass.end(),
                                          ii_intersection);

                    // If the intersection is trivial, the colour class completely contains the
                    // coefficient set, or the coefficient set completely contains the colour
                    // class, then we have nothing to do.
                    if (intersection.empty()
                        || intersection.size() == coeffset.size()
                        || intersection.size() == colourclass.size()) {
                        newcolourclasses.push_back(colourclass);
                        continue;
                    }

                    // Otherwise, we must perform the refinement as detailed above.
                    // This involves creating two nontrivial (guaranteed by above checks) colour
                    // classes, namely:
                    // A = colourclass - coeffset
                    // B = colourclass int coeffset
                    // We have already calculated B above (intersection), so it simply remains to calculate
                    // A (difference).
                    std::set<int> difference;
                    std::insert_iterator <std::set<int>> ii_difference { difference, difference.begin() };
                    std::set_difference(colourclass.begin(), colourclass.end(),
                                        coeffset.begin(), coeffset.end(),
                                        ii_difference);

                    // Insert the two new colour classes.
                    newcolourclasses.push_back(difference);
                    newcolourclasses.push_back(intersection);
                }

                // Copy the new classes over to the old ones.
                colourclasses = newcolourclasses;
            }
        }

        // It now remains to impose colouring classes on the constraint variables. This will be done
        // simply by placing constraints with the same lower and upper bound in the same colouring class.
        // We sort the constraints by lbound and ubound. We will assign colour
        // classes later by traversing our sorting.
        assert(constraints.size() + c == c + r);
        std::map < int, std::map < int, std::set < int > > > sortedconstraints;
        cbeginIter = constraints.begin();
        cendIter = constraints.end();
        for (auto vertexnumber = c; cbeginIter != cendIter; ++cbeginIter, ++vertexnumber) {
            auto *constraint = (*cbeginIter).second;
            ((sortedconstraints[constraint->getLowerBound()])[constraint->getUpperBound()]).insert(vertexnumber);
        }

        // Assign the colour classes by traversing the data structure.
        auto sortedcbeginIter = sortedconstraints.begin();
        auto sortedcendIter = sortedconstraints.end();
        for (; sortedcbeginIter != sortedcendIter; ++sortedcbeginIter) {
            auto &tmpmap = (*sortedcbeginIter).second;
            auto tmpbeginIter = tmpmap.begin();
            auto tmpendIter = tmpmap.end();

            for (; tmpbeginIter != tmpendIter; ++tmpbeginIter) {
                // Create a colour class out of the set of int simply by inserting it into the new position in the
                // collection of colour classes.
                auto &colourclass = (*tmpbeginIter).second;
                colourclasses.push_back(colourclass);
            }
        }

        // *** NAUTY FORMULATION ***
        // We begin by imposing the colouring in a format that nauty will understand.
        // We set the labeling and the ptn by iterating over the colour classes.
        auto ccbeginIter = colourclasses.begin();
        auto ccendIter = colourclasses.end();
        auto bound = c + r;
        auto count = 0;
        for (; ccbeginIter != ccendIter; ++ccbeginIter) {
            auto &colourclass = *ccbeginIter;
            auto tmpbeginIter = colourclass.begin();
            auto tmpendIter = colourclass.end();
            for (; tmpbeginIter != tmpendIter; ++tmpbeginIter, ++count) {
                assert(count < bound);
                lab[count] = *tmpbeginIter;
                ptn[count] = 1;
            }

            // Terminate the partition.
            ptn[count - 1] = 0;
        }

        // Make sure all variables were covered.
        assert(count == bound);

#ifdef DEBUG
        std::cerr << "Labeling: ";
        for (int i=0; i < bound; ++i)
          std::cerr << lab[i] << " ";
        std::cerr << "\n";
        std::cerr << "Partition: ";
        for (int i=0; i < bound; ++i)
          std::cerr << ptn[i] << " ";
        std::cerr << "\n";
#endif

        // Clear all the sets in the graph.
        for (auto i = 0; i < bound; ++i) {
            gv = GRAPHROW(g, i, m);
            EMPTYSET(gv, m);
        }

        // For each constraint, we want an edge between its vertex and all vertices corresponding
        // to variables contained in it.
        cbeginIter = constraints.begin();
        cendIter = constraints.end();
        auto constraintvertex = c;
        for (; cbeginIter != cendIter; ++cbeginIter, ++constraintvertex) {
            gv = GRAPHROW(g, constraintvertex, m);

            // Iterate over the positions.
            auto *cs = (*cbeginIter).second;
            auto vpbeginIter = cs->getPositions().begin();
            auto vpendIter = cs->getPositions().end();
            for (; vpbeginIter != vpendIter; ++vpbeginIter) {
                auto variablevertex = *vpbeginIter;
                ADDELEMENT(gv, variablevertex);

                gv2 = GRAPHROW(g, variablevertex, m);
                ADDELEMENT(gv2, constraintvertex);
            }
        }

        // We now have the completed graph, so call nauty.
        nauty(g, lab, ptn, NULL, orbits, &options, &stats, workspace,
              100, m, n, NULL);

        // Close the file.
        fclose(f);

        // We now have the file buf containing a set of generators for the autom
        // group of the graph using cartesian format. We need to read them in, parse
        // them, change them into symmetry permutations, and enter them.
        std::ifstream ifstr(filename);

        // Create a permutation over the columns.
        auto *perm = pool->newPermutation();
        int dummy;
        auto index = 0;
        while (!ifstr.eof()) {
            // Read in an element of a permutation.
            ifstr >> (index < x ? perm[index] : dummy);

#ifdef DEBUG
            // Ensure this is valid.
            if (index < x)
              assert(perm[index] < x);
#endif

            // Increment index.
            ++index;

            // If we have a complete permutation, process it.
            if (index % n == 0) {
#ifdef DEBUG
                std::cerr << "Entering permutation: \n";
                for (auto i=0; i < x; ++i)
                    std::cerr << perm[i] << " ";
                std::cerr << "\n";
#endif
                enter(perm);
                index = 0;
            }
        }

        // Free the permutation.
        pool->freePermutation(perm);

        // We are done. Close the stream and unlink the file.
        ifstr.close();
        remove(filename);
    }
};
