#include "mex.h"

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:initialize:nrhs",
                "This function does not have input arguments.");
    }
    if(nlhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:initialize:nlhs",
                "This function does not have output arguments.");
    }

    /* Call the fncs::initialize subroutine. */
    fncs::initialize();
}

