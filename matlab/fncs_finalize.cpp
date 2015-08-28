#include "mex.h"

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs>0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs::finalize:maxrhs",
                "Too many input arguments.");
    }
    if(nlhs>0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs::finalize:maxlhs",
                "Too many output arguments.");
    }

    /* Call the fncs::finalize subroutine. */
    fncs::finalize();
}

