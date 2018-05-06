// glpksolver.cpp
//
// By Sebastian Raaphorst, 2006.

#include <assert.h>
extern "C" {
#include <glpk.h>
}
#include <float.h>
#include "bac.h"
#include "formulation.h"
#include "nibacexception.h"
#include "node.h"
#include "common.h"
#include "glpksolver.h"


GLPKSolver::GLPKSolver()
{
}


GLPKSolver::~GLPKSolver()
{
}


void GLPKSolver::setupFormulation(Formulation &f)
{
  // Create a new LP.
  LPXInfo *info = new LPXInfo;
  LPX *lp = lpx_create_prob();
  if (!lp)
    throw new OutOfMemoryException();

  // Configure it.
  lpx_set_int_parm(lp, LPX_K_MSGLEV, 1);
  lpx_set_int_parm(lp, LPX_K_DUAL, 1);
  lpx_set_real_parm(lp, LPX_K_TOLINT, EPSILON);
  //  lpx_set_int_param(lp, LPX_K_PRESOL, 1);

  // Make it a MIP.
  lpx_set_class(lp, LPX_MIP);

  // Add the columns and their bounds.
  int offset = lpx_add_cols(lp, f.getNumberVariables());
  for (int i=0; i < f.getNumberVariables(); ++i) {
    lpx_set_col_kind(lp, i+offset, LPX_IV);
    std::ostringstream stream;
    stream << "vx_" << i+offset;
    lpx_set_col_name(lp, i+offset, const_cast< char* >(stream.str().c_str()));
    lpx_set_col_bnds(lp, i+offset, LPX_DB, 0.0, 1.0);
  }
  info->lp = lp;
  info->offset = offset;
  f.setData((void*)info);
}


void GLPKSolver::cleanupFormulation(Formulation &f)
{
  // Get the structure.
  LPXInfo *info = (LPXInfo*) f.getData();
  lpx_delete_prob(info->lp);
  delete info;
  f.setData((void*)0);
}


void GLPKSolver::addConstraint(Formulation &f, Constraint *c)
{
  LPXInfo *info = (LPXInfo*) f.getData();

  // Now create a row for the constraint and name it.
  int rowoffset = lpx_add_rows(info->lp, 1);
  std::ostringstream stream;
  stream << rowoffset;
  lpx_set_row_name(info->lp, rowoffset, const_cast< char* >(stream.str().c_str()));
  c->setImplementation((void*)rowoffset);

  // Add the necessary coefficients and bounds.
  lpx_set_row_bnds(info->lp, rowoffset,
		   (c->getLowerBound() == c->getUpperBound() ? LPX_FX : LPX_DB),
		   c->getLowerBound(), c->getUpperBound());
  std::vector< int > &positions = c->getPositions();
  std::vector< int > &coefficients = c->getCoefficients();
  assert(positions.size() == coefficients.size());
  int *ind = new int[positions.size()+1];
  double *val = new double[coefficients.size()+1];

  int i=1;
  for (std::vector< int >::iterator iter = positions.begin();
       iter != positions.end();
       ++iter)
    ind[i++] = *iter + info->offset;
  i=1;
  for (std::vector< int >::iterator iter = coefficients.begin();
       iter != coefficients.end();
       ++iter)
    val[i++] = (double) *iter;
  lpx_set_mat_row(info->lp, rowoffset, positions.size(), ind, val);
}


void GLPKSolver::removeConstraint(Formulation &f, Constraint *c)
{
  LPXInfo *info = (LPXInfo*) f.getData();

  // Now create a row for the constraint and name it.
  std::ostringstream stream;
  stream << ((long)(c->getImplementation()));
  
  int num[2];
  int bound = lpx_get_num_rows(info->lp);
  for (int i=1; i <= bound; ++i) {
    const char *name = lpx_get_row_name(info->lp, i);
    if (name && strcmp(stream.str().c_str(), name) == 0) {
      num[1] = i;
      lpx_del_rows(info->lp, 1, num);
      break;
    }
  }

  c->setImplementation((void*)0);
}


void GLPKSolver::addCut(Formulation &f, Constraint *c)
{
  addConstraint(f, c);
}


void GLPKSolver::removeCut(Formulation &f, Constraint *c)
{
  removeConstraint(f, c);
}


void GLPKSolver::fixVariable(Formulation &f, int var, int val)
{
  LPXInfo *info = (LPXInfo*) f.getData();
  lpx_set_col_bnds(info->lp, var+info->offset, LPX_FX,
		   val, val);
}


void GLPKSolver::unfixVariable(Formulation &f, int var)
{
  LPXInfo *info = (LPXInfo*) f.getData();
  lpx_set_col_bnds(info->lp, var+info->offset, LPX_DB,
		   0.0, 1.0);
}


void GLPKSolver::setObjectiveFunction(Formulation &f,
				      std::vector< int > &objectivecoefficients,
				      int lbound, int ubound)
{
  LPXInfo *info = (LPXInfo*) f.getData();
  lpx_set_obj_dir(info->lp,
		  f.getProblemType() == Formulation::MINIMIZATION ?
		  LPX_MIN : LPX_MAX);

  // Keep track of the number of nonzero entries so that we can add a constraint
  // representing the bounds on the objective function.
  int count=0;
  int i=0;
  for (std::vector< int >::iterator iter = objectivecoefficients.begin();
       iter != objectivecoefficients.end();
       ++iter) {
    if (*iter)
      ++count;
    lpx_set_obj_coef(info->lp, i+info->offset, (double)(*iter));
    ++i;
  }

  // Constrain the objective function.
  lpx_set_real_parm(info->lp, LPX_K_OBJLL, lbound-EPSILON);
  lpx_set_real_parm(info->lp, LPX_K_OBJUL, ubound+EPSILON);
  return;
  // Now we create a constraint for the objective function to bound it.
  int *ind = new int[count+1];
  double *val = new double[count+1];
  i=1;
  int j=0;
  for (std::vector< int >::iterator iter = objectivecoefficients.begin();
       iter != objectivecoefficients.end();
       ++iter) {
    if (*iter) {
      ind[i] = j+info->offset;
      val[i] = *iter;
      ++i;
    }
    ++j;
  }

  int rowoffset = lpx_add_rows(info->lp, 1);
  lpx_set_row_bnds(info->lp, rowoffset, (lbound == ubound ? LPX_FX : LPX_DB),
		   (double)lbound, (double)ubound);
  lpx_set_mat_row(info->lp, rowoffset, count, ind, val);;
  lpx_set_row_name(info->lp, rowoffset, "obj");
}


int GLPKSolver::solveNode(BAC &bac, Node &n, bool fullsolve)
{
  Formulation &f = bac.getFormulation();
  LPXInfo *info = (LPXInfo*) f.getData();
  int status;

  // As, even for integer methods we require an LP solution to
  // have been calculated, we simply start by solving the LP using
  // the simplex method.
  lpx_adv_basis(info->lp);
  status = lpx_simplex(info->lp);
  if (status != LPX_E_OK)
    return -1;
  
  // Make sure that a feasible solution was found.
  status = lpx_get_status(info->lp);
  if (status == LPX_UNDEF || status == LPX_INFEAS
      || status == LPX_NOFEAS || status == LPX_UNBND)
    return -1;

  if (fullsolve) {
    // Solve the problem using MIP.
    status = lpx_integer(info->lp);
    if (status != LPX_E_OK)
      return -1;

    // Make sure that a feasible solution was found.
    status = lpx_mip_status(info->lp);
    if (status == LPX_I_UNDEF || status == LPX_I_NOFEAS)
      return -1;

    // Copy it to the node.
    n.setSolutionValue(lpx_mip_obj_val(info->lp));
    double *solvars = n.getSolutionVariableArray();
    for (int i=0; i < n.getNumberBranchingVariables(); ++i)
      solvars[i] = lpx_mip_col_val(info->lp, i+info->offset);
  }
  else {
    // Copy the LP solution to the node.
    n.setSolutionValue(lpx_get_obj_val(info->lp));
    double *solvars = n.getSolutionVariableArray();
    for (int i=0; i < n.getNumberBranchingVariables(); ++i)
      solvars[i] = lpx_get_col_prim(info->lp, i+info->offset);
  }    

  // I don't think there's a way to get the number of nodes visited by
  // GLPK, so we just return 1.
  return 1;
}


void *GLPKSolver::createConstraint(Formulation&, std::vector< int >&,
				   std::vector< int >&, int, int)
{
  // This method need not do anything. Everything will be handled
  // when the constraint is added.
  return (void*)0;
}


void GLPKSolver::deleteConstraint(void*)
{
}


bool GLPKSolver::exportModel(Formulation &f, const char *filename)
{
  LPXInfo *info = (LPXInfo*) f.getData();
  int status = lpx_write_cpxlp(info->lp, const_cast<char*>(filename));
  return (status == 0);
}


#ifdef DEBUG
void GLPKSolver::printVariables(Formulation&)
{
}
#endif


// Create a static instance of GLPKSolver; this will be assigned as the static
// instance in LPSolve and made accessible. Simply link this solver in with
// end-user code to obtain a program that uses GLPK.
GLPKSolver GLPKSolver::_glpksolver;
