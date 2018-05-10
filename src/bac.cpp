/**
 * bac.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <float.h>
#include <map>
#include <set>
#include <vector>
#include "common.h"
#include "bac.h"
#include "bacoptions.h"
#include "cutproducer.h"
#include "formulation.h"
#include "lpsolver.h"
#include "nibacexception.h"
#include "node.h"
#include "nodestack.h"
#include "solutionmanager.h"
#include "statistics.h"

namespace vorpal::nibac {
    BAC::BAC(Formulation &pformulation, BACOptions &poptions)
            : formulation(pformulation),
              options(poptions) {
        // Finish setting up.
        bestSolutionValue = (formulation.getProblemType() == Formulation::MAXIMIZATION ? INT_MIN : INT_MAX);
    }


    void BAC::initialize(void) {
        // Tell the statistics how many cutters we are using.
        options.getStatistics().setNumberCutProducers(options.getCutProducers().size());

        // Create the default node. Note that this node is handled by the nodeStack and
        // we do not need to deallocate it.
        Node *node = new Node(*this,
                              formulation,
                              0,
                              formulation.getNumberVariables(),
                              formulation.getNumberBranchingVariables(),
                              &(options.getInitial0Fixings()),
                              &(options.getInitial1Fixings()));

        // We must have a branching scheme defined in order to create a NodeStack.
        if (options.getBranchingScheme() == 0)
            throw NoBranchingSchemeException();

        // Create the nodeStack and add the new node to it.
        nodeStack = new NodeStack(*(options.getBranchingScheme()), node, options.getStatistics());
    }


    BAC::~BAC() {
        delete nodeStack;
    }


    int BAC::preprocess(Node &node) {
        return TRUE;
    }


    int BAC::checkSolutionForGeneration(Node &node) {
        return TRUE;
    }


    int BAC::process(Node &node) {
        return TRUE;
    }


    int BAC::postprocess(Node &node) {
        return TRUE;
    }


    void BAC::solve() {
#ifdef DEBUG
        std::cerr << "* Beginning branch-and-cut *" << std::endl;
#endif

        // Preliminary declarations.
        Node *node;
        bool validSubtreeFlag;
        bool finishedEarlyFlag;

        int baseSolutionValue;
        int numberNonInteger;
        double minimumViolation;
        double violationTolerance;
        bool terminateCuttingPlaneFlag;
        bool solveLPFlag;
        bool isInteger;
        int globalNumberCuts;
        int localNumberCuts;
        double globalMaximumViolation;
        double localMaximumViolation;

        // References into statistics to improve processing time.
        Statistics &statistics = options.getStatistics();
        const int &lowerBound = options.getLowerBound();
        const int &upperBound = options.getUpperBound();
        const std::vector<CutProducer *> &cutProducers = options.getCutProducers();
        if (options.getSolutionManager() == 0)
            throw NoSolutionManagerException();
        SolutionManager &solutionManager = *(options.getSolutionManager());

        // Start the timer.
        statistics.getTotalTimer().start();

        // Begin with necessary initialization.
#ifdef DEBUG
        std::cerr << "+ Beginning initialization (variable fixings, etc...)" << std::endl;
#endif
        initialize();
#ifdef DEBUG
        std::cerr << "- Initialization complete." << std::endl;
#endif

        // Determine if we are solving LPs or just testing feasibility.
        // We always want to solve an LP if we are performing a full search or a generation.
        // If we are performing a maximal generation or generation of every solution and a bound
        // has been specified, we'd also like to do this so that we can prune early.
        solveLPFlag = formulation.getSolutionType() == Formulation::SEARCH
                      || formulation.getSolutionType() == Formulation::GENERATION
                      || (formulation.getProblemType() == Formulation::MAXIMIZATION && lowerBound > INT_MIN)
                      || (formulation.getProblemType() == Formulation::MINIMIZATION && upperBound < INT_MAX);

        // The finishedEarlyFlag flag is used to terminate the processing early (i.e. before
        // the nodeStack is empty). This is done, for example, when generating and
        // reaching an integer solution that demonstrates itself to be optimal
        // (vis-a-vis bounds).
        finishedEarlyFlag = false;

        while ((node = nodeStack->getNextNode())) {
#ifdef DEBUG
            std::cerr << "& Got a new node from the stack "
             << "(branched on variable " << node->getBranchVariableIndex()
             << " with value " << node->getBranchVariableValue()
             << "), depth=" << node->getDepth() << std::endl;
#endif

            // *** INDICATE ANOTHER NODE HAS BEEN VISITED ***
            // This also records depth information.
            statistics.reportNode(*node);

            // We initially believe that the subtree rooted at this node is valid until we have
            // reason not to.
            validSubtreeFlag = true;

            // *** PREPROCESS ***
#ifdef DEBUG
            std::cerr << "+ Beginning preprocessing..." << std::endl;
#endif
            if (!(validSubtreeFlag = preprocess(*node))) {
#ifdef DEBUG
                std::cerr << "* Preprocessing failed." << std::endl;
#endif
                goto CLEANUP;
            }
#ifdef DEBUG
            std::cerr << "- Preprocessing done." << std::endl;
#endif

            // *** CONTINUOUSLY SOLVE THE LP RELAXATION OR EQUIVALENT ***
            terminateCuttingPlaneFlag = false;
            for (;;) {
                // *** SOLVE LP / TEST FEASIBILITY ***
                // If the problem requires, solve an LP.
                if (solveLPFlag) {
                    // Solve the node using the technique of the implemented LP solver
#ifdef DEBUG
                    std::cerr << "+ Executing solver..." << std::endl;
#endif

                    // We use the specific solver instance built into the LP solver class.
                    // If the node depth is greater than the depth to which NIBAC is supposed
                    // to solve, we ask the LPSolver to solve the node to completion.
                    statistics.getLPSolverTimer().start();
                    int numberNodesSolved = LPSolver::getInstance()->solveNode(*this, *node,
                                                                               node->getDepth() >= options.getDepth());
                    statistics.getLPSolverTimer().stop();

                    // Report to the node and statistics that we solved an LP.
                    node->reportLPSolved();
                    statistics.reportLPSolved();

                    if (numberNodesSolved < 0) {
                        validSubtreeFlag = false;
#ifdef DEBUG
                        std::cerr << "- Solver failed. Cleaning up." << std::endl;
#endif
                        goto CLEANUP;
                    }

                    // Report any other subnodes that were processed by the solver.
                    statistics.reportNodesWithoutDepth(numberNodesSolved);

#ifdef DEBUG
                    std::cerr << "- Solver completed." << std::endl;
                    std::cerr << "\t= Solution value: " << node->getSolutionValue() << std::endl;
                    std::cerr << "\t= Solution vector:";
                    int numberVariables = formulation.getNumberVariables();
                    double *fractionalSolution = node->getSolutionVariableArray();
                    for (int i=0; i < numberVariables; ++i)
                      std::cerr << " " << i << ":" << fractionalSolution[i];
                    std::cerr << std::endl;
                    std::cerr << "\t= Ones:";
                    for (int i=0; i < numberVariables; ++i)
                      if (isone(fractionalSolution[i]))
                        std::cerr << " " << i;
                    std::cerr << std::endl;
#endif
                }

                    // Otherwise, we test feasibility.
                else {
#ifdef DEBUG
                    std::cerr << "+ Testing feasibility..." << std::endl;
#endif

                    if (!formulation.checkPartialFeasibility(node->getPartialSolutionArray())) {
#ifdef DEBUG
                        std::cerr << "- Partial solution cannot be extended to a feasible complete solution. Cleaning up." << std::endl;
#endif

                        goto CLEANUP;
                    }

#ifdef DEBUG
                    std::cerr << "- Solution is feasible." << std::endl;
#endif
                }

                // *** CHECK BOUNDS ***
                // We can only do this if an LP was solved.
                if (solveLPFlag) {
#ifdef DEBUG
                    std::cerr << "+ Checking bound requirements..." << std::endl;
#endif

                    // Find the base solution: if we are dealing with a maximization problem, the solution in the
                    // subtree can be no greater than the floor, and the ceil if this is a minimization problem.
                    baseSolutionValue = (formulation.getProblemType() == Formulation::MAXIMIZATION
                                         ? floor(node->getSolutionValue())
                                         : ceil(node->getSolutionValue()));

                    // *** DOES THE SOLUTION SATISFY THE BOUND REQUIREMENTS? ***
                    if (formulation.getProblemType() == Formulation::MAXIMIZATION) {
                        if (baseSolutionValue < lowerBound) {
                            validSubtreeFlag = false;
#ifdef DEBUG
                            std::cerr << "* Maximization problem did not satisfy lower bound: "
                                  << baseSolutionValue << " < bound=" << lowerBound << std::endl;
#endif
                            goto CLEANUP;
                        }
                    } else if (baseSolutionValue > upperBound) {
                        validSubtreeFlag = false;
#ifdef DEBUG
                        std::cerr << "* Minimization problem did not satisfy upper bound: "
                              << baseSolutionValue << " > bound=" << upperBound << std::endl;
#endif
                        goto CLEANUP;
                    }

                    // *** CHECK ACCEPTABILITY OF SOLUTION ***
                    // We only do this if we are not working with maximal solutions or all solutions.
                    if (formulation.getSolutionType() != Formulation::MAXIMALGENERATION &&
                        formulation.getSolutionType() != Formulation::ALLGENERATION) {
                        if (formulation.getProblemType() == Formulation::MAXIMIZATION &&
                            baseSolutionValue < bestSolutionValue) {
                            validSubtreeFlag = false;
#ifdef DEBUG
                            std::cerr << "* Maximization problem solution does not meet currently optimal solution: "
                                  << baseSolutionValue << " < optimal=" << bestSolutionValue << std::endl;
#endif
                            goto CLEANUP;
                        }
                        if (formulation.getProblemType() == Formulation::MINIMIZATION &&
                            baseSolutionValue > bestSolutionValue) {
                            validSubtreeFlag = false;
#ifdef DEBUG
                            std::cerr << "* Minimization problem solution does not meet currently optimal solution: "
                                  << baseSolutionValue << " > optimal=" << bestSolutionValue << std::endl;
#endif
                            goto CLEANUP;
                        }
                        if (formulation.getSolutionType() == Formulation::SEARCH &&
                            baseSolutionValue == bestSolutionValue) {
                            validSubtreeFlag = false;
#ifdef DEBUG
                            std::cerr << "* Search problem solution is no better than currently optimal solution: "
                                  << baseSolutionValue << " == optimal=" << bestSolutionValue << std::endl;
#endif
                            goto CLEANUP;
                        }
                    }
#ifdef DEBUG
                    std::cerr << "- Bounds requirements satisfied." << std::endl;
#endif
                }


                // *** PROCESS SOLUTIONS ***
                isInteger = false;

                // If we solved an LP, we need to determine if this is the best solution we've found so far.
                if (solveLPFlag) {
#ifdef DEBUG
                    std::cerr << "+ Checking whether solution is integer..." << std::endl;
#endif
                    if (isInteger = isIntegerSolution(*node, numberNonInteger)) {
#ifdef DEBUG
                        std::cerr << "- Solution is integer.\n";
#endif
                        // We have already pruned solutions that are not as good as this one,
                        // so this one is clearly the best (or tied for the best) so far. We
                        // indicate this by setting the bestSolutionValue variable.
#ifdef DEBUG
                        std::cerr << "\t= Setting the new optimal solution: old=" << bestSolutionValue
                              << ", new=" << node->getSolutionValue() << std::endl;
#endif
                        bestSolutionValue = round(node->getSolutionValue());
                    }
                }

                // If we are performing a search, we must do this differently, since we don't need to be at a leaf node
                // in order to terminate.
                if (formulation.getSolutionType() == Formulation::SEARCH) {
                    // If this solution is integer (as tested above), then we report to the solution manager.
                    if (isInteger) {
#ifdef DEBUG
                        std::cerr << "\t= Reporting the solution to the solution manager." << std::endl;
#endif
                        // Send the solution to the solution manager.
                        solutionManager.newSolution(*node);

                        // We definitely don't want to process any further into the subtree; we
                        // are generating, and this is integer, so we cannot find a better
                        // solution in the subtree rooted here.
                        validSubtreeFlag = false;

                        // In the best case, if we have specified a bound for the optimal
                        // solution, if we have met this bound, then there is no reason to
                        // continue looking at the tree at all. We can terminate the
                        // branch-and-cut algorithm immediately.
                        if (formulation.getProblemType() == Formulation::MAXIMIZATION) {
                            if (equals(node->getSolutionValue(), upperBound)) {
#ifdef DEBUG
                                std::cerr << "* Maximization search problem has reached upper bound: "
                                  << node->getSolutionValue() << " == bound=" << upperBound << std::endl;
#endif
                                finishedEarlyFlag = true;
                                goto CLEANUP;
                            }
                        } else if (equals(node->getSolutionValue(), lowerBound)) {
#ifdef DEBUG
                            std::cerr << "* Minimization search problem has reached lower bound: "
                              << node->getSolutionValue() << " == bound=" << lowerBound << std::endl;
#endif
                            finishedEarlyFlag = true;
                            goto CLEANUP;
                        }

                        // We have not met the bound, or no bound was specified, so we simply
                        // prune this node.
#ifdef DEBUG
                        std::cerr << "* Search problem reached integer solution, so stop processing this branch." << std::endl;
#endif
                        goto CLEANUP;
                    }
                }

                    // In this case, we are not searching, so we require leaf nodes. All three techniques share a considerable
                    // amount in common, so we lump them together here and specialize.
                else {
                    // If no LP was solved, set the current solution to the partial solution.
                    if (!solveLPFlag)
                        node->setSolutionToPartial();

#ifdef DEBUG
                    std::cerr << "+ Determining if the node has been solved (i.e. no free variables)." << std::endl;
#endif
                    if (isSolved(*node)) {
#ifdef DEBUG
                        std::cerr << "- The node has been solved." << std::endl;
#endif

                        // We can't go further into the tree, as this is a leaformulation.
                        validSubtreeFlag = false;

                        // Perform any additional necessary checks here. Currently, the only required one is maximality.
                        if (formulation.getSolutionType() == Formulation::MAXIMALGENERATION) {
#ifdef DEBUG
                            std::cerr << "+ Checking to see if the solution is maximal." << std::endl;
#endif
                            if (!isMaximal(*node)) {
#ifdef DEBUG
                                std::cerr << "- Solution is not maximal. Pruning." << std::endl;
#endif
                                goto CLEANUP;
                            }
#ifdef DEBUG
                            std::cerr << "- Solution is maximal." << std::endl;
#endif
                        }

#ifdef DEBUG
                        std::cerr << "+ Checking to see if the solution meets the required criteria." << std::endl;
#endif

                        if (checkSolutionForGeneration(*node)) {
#ifdef DEBUG
                            std::cerr << "- Solution meets our criteria." << std::endl;
                            std::cerr << "\t= Reporting the solution to the solution manager." << std::endl;
#endif
                            solutionManager.newSolution(*node);
                        } else {
                            // If we reached this point, the solution is not suitable.
#ifdef DEBUG
                            std::cerr << "- Solution does not meet our criteria." << std::endl;
#endif
                        }
#ifdef DEBUG
                        std::cerr << "* Nothing left to process on this branch." << std::endl;
#endif
                        goto CLEANUP;
                    }
#ifdef DEBUG
                    else
                      std::cerr << "- The node has not been solved." << std::endl;
#endif
                }

                // If no LP was solved, we obviously can't perform the cutting plane algorithm.
                if (!solveLPFlag) {
#ifdef DEBUG
                    std::cerr << "= LP not solved. Cannot perform cutting plane." << std::endl;
#endif
                    break;
                }

                // If, at this point, the node is still active but was deemed integer, there is no improvement
                // that can be done via the cutting plane.
                if (isInteger) {
#ifdef DEBUG
                    std::cerr << "\t= Not reported to solution manager. Cannot improve through cutting plane." << std::endl;
#endif
                    break;
                }

                // *** DETERMINE IF WE SHOULD CONTINUE ***
                // This is used to stop the cutting plane algorithm. If this is true,
                // the cutting plane algorithm didn't give us sufficient cuts to motivate
                // running it for another iteration.
                if (terminateCuttingPlaneFlag) {
#ifdef DEBUG
                    std::cerr << "\t= Terminating the cutting plane algorithm." << std::endl;
#endif
                    break;
                }

                // *** PROCESS ***
#ifdef DEBUG
                std::cerr << "+ Beginning processing..." << std::endl;
#endif
                validSubtreeFlag = (process(*node) == TRUE);
                if (!validSubtreeFlag) {
#ifdef DEBUG
                    std::cerr << "* Processing failed." << std::endl;
#endif
                    goto CLEANUP;
                }
#ifdef DEBUG
                std::cerr << "- Processing done." << std::endl;
#endif

                // *** GENERATE CUTS ***
                // If there are no cutProducers, we are done at this point.
                if (cutProducers.size() == 0) {
#ifdef DEBUG
                    std::cerr << "= Not performing cutting plane as there are no cut producers." << std::endl;
#endif
                    break;
                }

#ifdef DEBUG
                std::cerr << "+ Generating cuts..." << std::endl;
#endif
                statistics.getSeparationTimer().start();

                // Begin by determining the violation tolerance and the minimum worthwhile
                // violation using the supplied bounds and the number of fractional variables.
                determineViolationBounds(numberNonInteger, minimumViolation, violationTolerance);
#ifdef DEBUG
                std::cerr << "\t= Minimum violation required for this iteration: " << minimumViolation << std::endl;
                std::cerr << "\t= Violation tolerance for this iteration: " << violationTolerance << std::endl;
#endif

                globalNumberCuts = 0;
                globalMaximumViolation = 0;

                std::vector<unsigned long>::iterator numCutsIter = statistics.getNumberCuts().begin();
                for (std::vector<CutProducer *>::const_iterator cutsIter = cutProducers.begin();
                     cutsIter != cutProducers.end();
                     ++cutsIter, ++numCutsIter) {
#ifdef DEBUG
                    std::cerr << "\t+ Beginning cut producer." << std::endl;
#endif
                    (*cutsIter)->generateCuts(*this, *node, violationTolerance, localNumberCuts, localMaximumViolation);
#ifdef DEBUG
                    std::cerr << "\t- Cut producer generated " << localNumberCuts << " cuts with maximum violation " << localMaximumViolation << std::endl;
#endif
                    (*numCutsIter) += localNumberCuts;
                    globalNumberCuts += localNumberCuts;
                    if (greaterthan(localMaximumViolation, globalMaximumViolation))
                        globalMaximumViolation = localMaximumViolation;
                }
                if (lessthan(globalMaximumViolation, minimumViolation))
                    terminateCuttingPlaneFlag = true;
                if (globalNumberCuts < options.getMinimumNumberOfCuts())
                    terminateCuttingPlaneFlag = true;

                statistics.getSeparationTimer().stop();
#ifdef DEBUG
                std::cerr << "- Finished generating cuts (" << globalNumberCuts << " generated, maximum violation was "
                 << globalMaximumViolation << ")." << std::endl;
#endif

                // If no cuts were generated, there is no point in solving the LP again.
                if (globalNumberCuts == 0)
                    break;
            }

            // *** POSTPROCESS ***
#ifdef DEBUG
            std::cerr << "+ Beginning postprocessing..." << std::endl;
#endif
            validSubtreeFlag = (postprocess(*node) == TRUE);
            if (!validSubtreeFlag) {
#ifdef DEBUG
                std::cerr << "* Postprocessing failed." << std::endl;
#endif
                goto CLEANUP;
            }
#ifdef DEBUG
            std::cerr << "- Postprocessing done." << std::endl;
#endif

            CLEANUP:
            // Perform any cleanup here; currently, this is taken care of automatically by the
            // nodeStack and is not required.
            // However, if validSubtreeFlag is false, we do not want to continue to explore this
            // subtree, so we will prune it from the nodeStack.
            if (!validSubtreeFlag) {
                // We must report the branch depth, as a fathomed node might be deeper than
                // a solution node.
                statistics.reportBranchDepth(node->getDepth());
                nodeStack->pruneTop();

                // If finishedEarlyFlag is set, then we are done, so we simply return.
                if (finishedEarlyFlag)
                    return;
            } else
                // *** REMOVE UNVIOLATED CUTS ***
                // This is done only if we are not pruning this node from the search space.
                // There is no point to keeping unviolated cuts around; they simply waste
                // space. We check to see if the cuts aren't violated and if not, we simply
                // remove them from the node. They may be violated in other subtrees, and
                // by calling Node::removeConstraint, they will be added when we backtrack
                // to the parent node.
            if (cutProducers.size() > 0) {
#ifdef DEBUG
                std::cerr << "+ Removing inactive cuts." << std::endl;
#endif
                removeInactiveCuts(*node);
#ifdef DEBUG
                std::cerr << "- Done removing inactive cuts." << std::endl;
#endif
            }

#ifdef DEBUG
            std::cerr << "% Finished processing node." << std::endl;
            std::cerr << "-------------------------------------------------------------" << std::endl;
#endif
        }

        statistics.getTotalTimer().stop();
    }


    void BAC::removeInactiveCuts(Node &node) {
        // Iterate over the cuts. If they are inactive
        // cuts, we remove them from the node. Removing them
        // immediately from the node isn't possible; this would
        // result in changes to the list, which would invalidate
        // our iterator. Instead, we find the inactive constraints,
        // store them in a vector, and remove them after we have
        // checked the entire list.
        std::vector<Constraint *> removedCuts;

        // Intermediate values or storage variables.
        const std::map<unsigned long, Constraint *> &cuts = formulation.getCuts();
        Constraint *cut;
        double *solutionVariableArray = node.getSolutionVariableArray();

        for (std::map<unsigned long, Constraint *>::const_iterator iter = cuts.begin();
             iter != cuts.end();
             ++iter) {
            cut = (Constraint *) (*iter).second;

            // Check if this cut is unviolated.
            if (cut->isInactive(solutionVariableArray, options.getActivityTolerance()))
                removedCuts.push_back(cut);
        }

#ifdef DEBUG
        std::cerr << "\t= Removing " << removedCuts.size() << " cuts." << std::endl;
#endif

        // Now remove all the constraints that we need to.
        while (!removedCuts.empty()) {
            cut = removedCuts.back();
            removedCuts.pop_back();
            node.removeCut(cut);
        }
    }


    void BAC::determineViolationBounds(int numberNonInteger, double &minimumViolation, double &violationTolerance) {
        double frac = ((double) numberNonInteger) / ((double) formulation.getNumberVariables());
        minimumViolation = options.getMinimumViolationL() +
                           (options.getMinimumViolationU() - options.getMinimumViolationL()) * frac;
        violationTolerance = options.getViolationToleranceL() +
                             (options.getViolationToleranceU() - options.getViolationToleranceL()) * frac;
    }


    bool BAC::isIntegerSolution(Node &node, int &numberNonInteger) {
        double *solutionVariableArray = node.getSolutionVariableArray();
        numberNonInteger = 0;

        // We only need to check the free variables.
        std::set<int> &freeVariables = node.getFreeVariables();
        for (std::set<int>::iterator iter = freeVariables.begin();
             iter != freeVariables.end();
             ++iter)
            if (!isint(solutionVariableArray[*iter]))
                ++numberNonInteger;
        return (numberNonInteger == 0);
    }


    bool BAC::isSolved(Node &node) {
        return node.getFreeVariables().empty();
    }


    bool BAC::isMaximal(Node &node) {
        // We can do some preliminary maximality rejection based
        // on the flag possiblyMaximal, stored in the node.
        if (!node.possiblyMaximal())
            return false;

        // Flag used to determine maximality.
        bool isMaximal;

        // We don't know if the problem is maximal; the only way
        // to tell this is to test the variables fixed to 0 and see
        // if we can't fix one of them to 1.
        int numberVariables = node.getNumberBranchingVariables();
        double *solutionVariableArray = node.getSolutionVariableArray();

        // What we will first do, to simplify processing later on, is evaluate each of the
        // constraints with the current solution and store the values. This will allow us
        // to determine easily if changing the value of a variable will invalidate the constraint.
        const std::map<unsigned long, Constraint *> constraints = formulation.getConstraints();
        int numberRows = formulation.getNumberRows();
        double *constraintEvaluations = new double[numberRows];

        int j = 0;
        for (std::map<unsigned long, Constraint *>::const_iterator iter = constraints.begin();
             iter != constraints.end();
             ++j, ++iter)
            constraintEvaluations[j] = ((*iter).second)->evaluateConstraint(solutionVariableArray);

        // We need to perform a binary search. Declare the necessary variables.
        int position, checkpos, value, first, last;
        double fractionalValue;
        double newConstraintEvaluation;

        // Iterate over the array, checking for 0s
        for (int i = 0; i < numberVariables; ++i) {
            // If we are not 0, we do not process.
            if (!iszero(solutionVariableArray[i]))
                continue;

            // First, we check that fixing this variable to 1 will indeed increase
            // or decrease the variable of the objective function.
            // Should these be < > or <= >=?
            // Get the coefficient of the variable in the objective and add it to the
            // previous evaluated objective value. This is equivalent to fixing the variable
            // to 1 if it was set to 0 and re-evaluating the entire objective function.
            fractionalValue = node.getSolutionValue() + formulation.getObjectiveCoefficient(i);

            if ((formulation.getProblemType() == Formulation::MAXIMIZATION &&
                 lessthan(fractionalValue, node.getSolutionValue()))
                || (formulation.getProblemType() == Formulation::MINIMIZATION &&
                    greaterthan(fractionalValue, node.getSolutionValue())))
                continue;

            // We begin with the assumption that we are not maximal and try to prove that we are wrong.
            isMaximal = false;

            // Changing this variable WILL beneficially affect the objective. Now we
            // test the feasibility of setting this variable. To do this, we must iterate
            // over the constraints and check each one.
            // We do this using a binary search.
            j = 0;
            for (std::map<unsigned long, Constraint *>::const_iterator iter = constraints.begin();
                 iter != constraints.end();
                 ++j, ++iter) {
                Constraint *constraint = (*iter).second;

                // We have a row; see if this position exists in the row via binary search.
                // We *could* use STL's binary search, but this just gives us incidence; we
                // also want the exact position offset so that we can use this to get the
                // coefficient.
                std::vector<int>::iterator positionIter = constraint->getPositions().begin();
                position = -1;
                first = 0;
                last = constraint->getPositions().size();

                while (first <= last) {
                    checkpos = (first + last) / 2;
                    value = *(positionIter + checkpos);
                    if (value == i) {
                        position = checkpos;
                        break;
                    }
                    if (value < i)
                        first = checkpos + 1;
                    else
                        last = checkpos - 1;
                }

                // Now, if position is still set to -1, the variable is not found in this
                // constraint, so we can just continue.
                if (position == -1)
                    continue;

                // We check feasibility by investigating sense and evaluating the constraint
                // with the new coefficient, which simply equates to adding the coefficient
                // at position position in this row to the old constraint value.
                newConstraintEvaluation = constraintEvaluations[j] + (constraint->getCoefficients())[position];

                // Check if the coefficient still holds; if not, we set maximal to false to
                // indicate that flipping this variable is inadmissible.
                if (lessthan(newConstraintEvaluation, constraint->getLowerBound())
                    || greaterthan(newConstraintEvaluation, constraint->getUpperBound())) {
                    isMaximal = true;
                    break;
                }
            }

            // If isMaximal is still false at this point, then we tried every constraint and found
            // that they were all satisfied by flipping variable i to 1. Hence, we have found a
            // "larger" solution that contains the original one, and thus, the solution is
            // not maximal.
            if (!isMaximal) {
                delete[] constraintEvaluations;
                return false;
            }
        }

        // No feasible solution could be found that would positively benefit the objective
        // function by flipping a variable from 0 to 1. Hence, we conclude that the solution
        // is maximal.
        delete[] constraintEvaluations;
        return true;
    }


    bool BAC::fixVariableTo1(Node &node, int variable, bool _) {
        node.fixVariable(variable, 1);
        return true;
    }


    bool BAC::fixVariableTo0(Node &node, int variable, bool _) {
        node.fixVariable(variable, 0);
        return true;
    }
};