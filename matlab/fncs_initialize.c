#include "mex.h"

#include <fncs.h>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs>0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs_initialize:maxrhs",
                "Too many input arguments.");
    }
    if(nlhs>0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs_initialize:maxlhs",
                "Too many output arguments.");
    }

    /* Call the fncs_initialize subroutine. */
    fncs_initialize();
}

