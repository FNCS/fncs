#include "mex.h"

#include <stdint.h>

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:is_initialized:nrhs",
                "This function does not take input arguments.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:is_initialized:nlhs",
                "This function has one output argument.");
    }

    /* Call the fncs::is_initialized subroutine. */
    bool result = fncs::is_initialized();

    /* Allocate return value. */
    plhs[0] = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);

    /* Assign return value. */
    mxLogical * data = (mxLogical *) mxGetData(plhs[0]);
    data[0] = result;
}

