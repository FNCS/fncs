#include "mex.h"

#include <stdint.h>

#include <fncs.hpp>
#include "to_fncs_time.hpp"

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    fncs::time time_requested;
    fncs::time time_response;

    /* Check for proper number of arguments. */
    if(nrhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs::time_request:maxrhs",
                "Too many input arguments.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs::time_request:maxlhs",
                "Too many output arguments.");
    }

    /* Convert input argument. */
    to_fncs_time(time_requested, prhs[0]);

    /* Call the fncs::time_request subroutine. */
    time_response = fncs::time_request(time_requested);

    /* Allocate return value. */
    plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);

    /* Assign return value. */
    uint64_t * data = (uint64_t *) mxGetData(plhs[0]);
    data[0] = time_response;
}

