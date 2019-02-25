// [[Rcpp::depends(RcppEigen)]]
#include "icd_types.h"
#include "local.h"
#include "mapplus.h"
#include "refactor.h"
#include "relevant.h"

using namespace Rcpp;

#ifdef ICD_DEBUG
#define ICD_ASSIGN(row, col) mat.insert(row, col) = true; // bounds check
#else // !ICD_DEBUG
#define ICD_ASSIGN(row, col) mat.coeffRef(row, col) = true;
#endif
#define PRINTCORNERMAP(x) ((void)0);
#define PRINTCORNERSP(x) ((void)0);

void buildVisitCodesSparseWide(
  const DataFrame &data,
  const std::string id_name,
  const CV code_names,
  const bool validate,
  Relevant &rh,
  // output
  PtsSparse &visMat,
  // output: can get this from sparse matrix at end? Needed?
  VecStr &visitIds);
LogicalMatrix comorbidMatMulWide(const DataFrame &data,
                                 const List &map,
                                 const std::string id_name,
                                 const CV code_name,
                                 const bool validate = false);
