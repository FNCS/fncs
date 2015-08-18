#include "mex.h"

#include <fncs.h>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs>0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs_finalize:maxrhs",
                "Too many input arguments.");
    }
    if(nlhs>0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs_finalize:maxlhs",
                "Too many output arguments.");
    }

    /* Call the fncs_finalize subroutine. */
    fncs_finalize();
}

