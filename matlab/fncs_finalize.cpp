#include "mex.h"

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:finalize:nrhs",
                "This function does not have input arguments.");
    }
    if(nlhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:finalize:nlhs",
                "This function does not have output arguments.");
    }

    /* Call the fncs::finalize subroutine. */
    fncs::finalize();
}

