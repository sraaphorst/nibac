/**
 * cplexsolver.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <ilcplex/ilocplex.h>
#include <map>
#include <vector>
#include "common.h"
#include "formulation.h"
#include "bac.h"
#include "bacoptions.h"
#include "node.h"
#include "lpsolver.h"
#include "cplexsolver.h"

namespace vorpal::nibac {
    CPLEXSolver::CPLEXSolver() {
#ifdef DEBUG
#ifdef CPLEX8
        env.setOut(cerr);
        env.setError(cerr);
        env.setWarning(cerr);
#endif
#else
        // Clear the CPLEX output channels
#ifdef CPLEX8
        env.setOut(env.getNullStream());
        env.setError(env.getNullStream());
        env.setWarning(env.getNullStream());
#else
        env.out().clear();
        env.error().clear();
        env.warning().clear();
#endif
#endif
    }


    CPLEXSolver::~CPLEXSolver() {
        // Terminate the environment
        env.end();
    }


    void CPLEXSolver::setupFormulation(Formulation &f) {
        // Create a new struct.
        CPLEXInfo *info = new CPLEXInfo;
        info->varsL = new IloNumVarArray(env, f.getNumberVariables(), 0.0, 1.0, IloNumVar::Float);
        info->varsI = new IloNumVarArray(env, f.getNumberVariables(), 0.0, 1.0, IloNumVar::Int);
        info->modelL = new IloModel(env);
        info->modelI = new IloModel(env);

        info->cplexL = new IloCplex(env);
#ifdef CPLEX8
        info->cplexL->setParam(IloCplex::RootAlg, IloCplex::Dual);
        info->cplexL->setParam(IloCplex::NodeAlg, IloCplex::Dual);
#else
        info->cplexL->setRootAlgorithm(IloCplex::Dual);
        info->cplexL->setNodeAlgorithm(IloCplex::Dual);
#endif
        info->cplexL->setParam(IloCplex::DPriInd, IloCplex::DPriIndSteep);
        info->cplexL->setParam(IloCplex::EpInt, VARINTEPSILON);
        info->cplexL->extract(*(info->modelL));

        info->cplexI = new IloCplex(env);
#ifdef CPLEX8
        info->cplexI->setParam(IloCplex::RootAlg, IloCplex::Dual);
        info->cplexI->setParam(IloCplex::NodeAlg, IloCplex::Dual);
#else
        info->cplexI->setRootAlgorithm(IloCplex::Dual);
        info->cplexI->setNodeAlgorithm(IloCplex::Dual);
#endif
        info->cplexI->setParam(IloCplex::DPriInd, IloCplex::DPriIndSteep);
        info->cplexI->setParam(IloCplex::EpInt, VARINTEPSILON);
        info->cplexI->extract(*(info->modelI));

        f.setData((void *) info);

#ifdef DEBUG
#ifdef CPLEX8
        info->cplexL->setOut(std::cerr);
        info->cplexL->setError(std::cerr);
        info->cplexL->setWarning(std::cerr);
        info->cplexI->setOut(std::cerr);
        info->cplexI->setError(std::cerr);
        info->cplexI->setWarning(std::cerr);
#endif
#else
#ifdef CPLEX8
        info->cplexL->setOut(env.getNullStream());
        info->cplexL->setError(env.getNullStream());
        info->cplexL->setWarning(env.getNullStream());
        info->cplexI->setOut(env.getNullStream());
        info->cplexI->setError(env.getNullStream());
        info->cplexI->setWarning(env.getNullStream());
#else
        info->cplexL->out().clear();
        info->cplexL->error().clear();
        info->cplexL->warning().clear();
        info->cplexI->out().clear();
        info->cplexI->error().clear();
        info->cplexI->warning().clear();
#endif
#endif
    }


    void CPLEXSolver::cleanupFormulation(Formulation &f) {
        // Get the struct.
        CPLEXInfo *info = (CPLEXInfo *) f.getData();

        // Delete the algorithms.
        info->cplexI->end();
        delete info->cplexI;
        info->cplexL->end();
        delete info->cplexL;

        // Delete any remaining fixings.
        IloRange *range;
        std::map<int, IloRange *>::iterator beginIter, endIter;

        beginIter = info->fixingsI.begin();
        endIter = info->fixingsI.end();
        for (; beginIter != endIter; ++beginIter) {
            range = (*beginIter).second;
            range->end();
            delete range;
        }
        info->fixingsI.clear();

        beginIter = info->fixingsL.begin();
        endIter = info->fixingsL.end();
        for (; beginIter != endIter; ++beginIter) {
            range = (*beginIter).second;
            range->end();
            delete range;
        }
        info->fixingsL.clear();

        // Delete the model.
        info->modelI->end();
        delete info->modelI;
        info->modelL->end();
        delete info->modelL;

        // Delete the variables.
        delete info->varsI;
        delete info->varsL;

        // And finally, free the memory for the data.
        delete info;
    }


    void CPLEXSolver::addConstraint(Formulation &f, Constraint *c) {
        // Get the IloModel for this formulation and add the constraint to it.
        CPLEXInfo *info = ((CPLEXInfo *) f.getData());
        info->modelL->add(*(((ConstraintInfo *) c->getImplementation())->rangeL));
        info->modelI->add(*(((ConstraintInfo *) c->getImplementation())->rangeI));
    }


    void CPLEXSolver::removeConstraint(Formulation &f, Constraint *c) {
        // Get the IloModel for this formulation and remove the constraint from it.
        CPLEXInfo *info = ((CPLEXInfo *) f.getData());
        info->modelL->remove(*(((ConstraintInfo *) c->getImplementation())->rangeL));
        info->modelI->remove(*(((ConstraintInfo *) c->getImplementation())->rangeI));
    }


    void CPLEXSolver::addCut(Formulation &f, Constraint *c) {
        // Ultimately, we would like to add cuts directly from the Formulation's
        // std::map< unsigned long, Constraint* > using IloCplex::addCut. However, this
        // doesn't appear to be working and cuts are not respected, so for now,
        // we will simply add cuts directly to the model. Also, it is not clear how we
        // would remove cuts later on.
        //  IloCplex *cplex = ((CPLEXInfo*) f.getData())->cplex;
        //  cplex->addCut(*((IloRange*)c->getImplementation()));
        CPLEXInfo *info = ((CPLEXInfo *) f.getData());
        info->modelL->add(*(((ConstraintInfo *) c->getImplementation())->rangeL));
        info->modelI->add(*(((ConstraintInfo *) c->getImplementation())->rangeI));
    }


    void CPLEXSolver::removeCut(Formulation &f, Constraint *c) {
        // See above note.
        CPLEXInfo *info = ((CPLEXInfo *) f.getData());
        info->modelL->remove(*(((ConstraintInfo *) c->getImplementation())->rangeL));
        info->modelI->remove(*(((ConstraintInfo *) c->getImplementation())->rangeI));
    }


    void CPLEXSolver::fixVariable(Formulation &f, int var, int val) {
        CPLEXInfo *info = (CPLEXInfo *) f.getData();

        // Create a range and fix it.
        IloExpr exprL(env);
        IloExpr exprI(env);
        exprL += (*(info->varsL))[var];
        exprI += (*(info->varsI))[var];

        IloRange *rangeL = new IloRange(env, val, exprL, val);
        IloRange *rangeI = new IloRange(env, val, exprI, val);

        info->fixingsL[var] = rangeL;
        info->fixingsI[var] = rangeI;

        info->modelL->add(*rangeL);
        info->modelI->add(*rangeI);
    }


    void CPLEXSolver::unfixVariable(Formulation &f, int var) {
        CPLEXInfo *info = (CPLEXInfo *) f.getData();

        // Find the range and destroy it.
        std::map<int, IloRange *>::iterator iterL = info->fixingsL.find(var);
        assert(iterL != info->fixingsL.end());
        std::map<int, IloRange *>::iterator iterI = info->fixingsI.find(var);
        assert(iterI != info->fixingsI.end());

        IloRange *rangeL = (*iterL).second;
        info->modelL->remove(*rangeL);
        rangeL->end();
        delete rangeL;
        info->fixingsL.erase(iterL);

        IloRange *rangeI = (*iterI).second;
        info->modelI->remove(*rangeI);
        rangeI->end();
        delete rangeI;
        info->fixingsI.erase(iterI);
    }


    void
    CPLEXSolver::setObjectiveFunction(Formulation &f, std::vector<int> &objectivecoefficients, int lbound, int ubound) {
        // Get the info.
        CPLEXInfo *info = (CPLEXInfo *) f.getData();
        IloNumVarArray &xL = *(info->varsL);
        IloNumVarArray &xI = *(info->varsI);

        // Create the objective function.
        IloExpr objexprL(env);
        IloExpr objexprI(env);

        std::vector<int>::iterator beginIter = objectivecoefficients.begin();
        std::vector<int>::iterator endIter = objectivecoefficients.end();
        for (int i = 0; beginIter != endIter; ++beginIter, ++i)
            if (*beginIter) {
                objexprL += (*beginIter) * xL[i];
                objexprI += (*beginIter) * xI[i];
            }

        IloObjective objL(env, objexprL,
                          f.getProblemType() == Formulation::MINIMIZATION ?
                          IloObjective::Minimize : IloObjective::Maximize);
        info->modelL->add(objL);
        IloObjective objI(env, objexprI,
                          f.getProblemType() == Formulation::MINIMIZATION ?
                          IloObjective::Minimize : IloObjective::Maximize);
        info->modelI->add(objI);

        // Set the appropriate bounds on the algorithm.
        // This doesn't quite work, for some reason. Firstly, we should use CPLEX's epsilon, and not our
        // own (hence the issue with the 2-(14,3,1) packing generation being ridiculously slow with CPLEX).
        // We do not know for sure if this will work, but if we do not use it, the second issue arises:
        // certain problems reject from the LP solver even though we know they are correct. This is because
        // of int -> double conversions.
        info->cplexL->setParam(IloCplex::EpOpt, EPSILON);
        info->cplexL->setParam(IloCplex::ObjLLim, (double) lbound - EPSILON);
        info->cplexL->setParam(IloCplex::ObjULim, (double) ubound + EPSILON);
        info->cplexI->setParam(IloCplex::EpOpt, EPSILON);
        info->cplexI->setParam(IloCplex::ObjLLim, (double) lbound - EPSILON);
        info->cplexI->setParam(IloCplex::ObjULim, (double) ubound + EPSILON);

        IloRange objrangeL(env, lbound, objexprL, ubound);
        info->modelL->add(objrangeL);
        IloRange objrangeI(env, lbound, objexprI, ubound);
        info->modelI->add(objrangeI);
    }


    int CPLEXSolver::solveNode(BAC &bac, Node &n, bool fullsolve) {
        Formulation &f = bac.getFormulation();
        CPLEXInfo *info = (CPLEXInfo *) f.getData();

#ifdef LPDEBUG
        std::cerr << *(info->modelI) << std::endl;
#endif

        IloCplex *cplex = (fullsolve ? info->cplexI : info->cplexL);
        IloBool status = cplex->solve();

        // If we could not solve, end the algorithm and return.
        if (status == IloFalse)
            return -1;

        // Now we extract the solution to our node
        n.setSolutionValue(cplex->getObjValue());

        IloNumVarArray &numvararray = (fullsolve ? *(info->varsI) : *(info->varsL));
        double *solvars = n.getSolutionVariableArray();
        for (int i = 0; i < n.getNumberBranchingVariables(); ++i)
            solvars[i] = cplex->getValue(numvararray[i]);

        // Everything proceeded as expected.
        int numnodes = cplex->getNnodes();
        return numnodes;
    }


    void *CPLEXSolver::createConstraint(Formulation &f, std::vector<int> &positions,
                                        std::vector<int> &coefficients, int lbound, int ubound) {
        CPLEXInfo *info = (CPLEXInfo *) f.getData();
        IloNumVarArray &xL = *(info->varsL);
        IloNumVarArray &xI = *(info->varsI);

        // Create the constraint correspoinding to the cut
        IloExpr exprL(env);
        IloExpr exprI(env);
        std::vector<int>::iterator pbeginIter = positions.begin();
        std::vector<int>::iterator pendIter = positions.end();
        std::vector<int>::iterator cbeginIter = coefficients.begin();
        std::vector<int>::iterator cendIter = coefficients.end();
        for (; pbeginIter != pendIter; ++pbeginIter, ++cbeginIter) {
            exprL += *cbeginIter * xL[*pbeginIter];
            exprI += *cbeginIter * xI[*pbeginIter];
        }

        ConstraintInfo *cinfo = new ConstraintInfo;
        cinfo->rangeL = new IloRange(env, lbound, exprL, ubound);
        cinfo->rangeI = new IloRange(env, lbound, exprI, ubound);
        return cinfo;
    }


    void CPLEXSolver::deleteConstraint(void *data) {
        ConstraintInfo *cinfo = (ConstraintInfo *) data;
        ((IloRange *) cinfo->rangeL)->end();
        ((IloRange *) cinfo->rangeI)->end();
        delete ((IloRange *) cinfo->rangeL);
        delete ((IloRange *) cinfo->rangeI);
        delete cinfo;
    }


    bool CPLEXSolver::exportModel(Formulation &f, const char *filename) {
        CPLEXInfo *info = (CPLEXInfo *) f.getData();
        try {
            info->cplexI->exportModel(filename);
        }
        catch (IloException &e) {
            return false;
        }
        return true;
    }


#ifdef DEBUG
    void CPLEXSolver::printVariables(Formulation &f)
    {
      // Get the variables
      CPLEXInfo *info = (CPLEXInfo*) f.getData();
      IloNumVarArray &variables = *(info->varsL);
      cerr << variables << endl;
    }
#endif


    // Create a static instance of the CPLEXSolver; this will be assigned as the static instance
    // in LPSolver and made accessible to everyone without the need to explicitly do so. Then,
    // via this technique, by simply linking in a different object file derived from LPSolver,
    // we can pick our LP solver without changing a line of code.
    CPLEXSolver CPLEXSolver::_cplexsolver;
};