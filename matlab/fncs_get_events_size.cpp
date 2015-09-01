#include "mex.h"

#include <stdint.h>

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_events_size:nrhs",
                "This function does not take input arguments.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_events_size:nlhs",
                "This function has one output argument.");
    }

    /* Call the fncs::get_events_size subroutine. */
    size_t result = fncs::get_events().size();

    /* Allocate return value. */
    plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);

    /* Assign return value. */
    uint64_t * data = (uint64_t *) mxGetData(plhs[0]);
    data[0] = result;
}

