#include "mex.h"

#include <stdint.h>

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_values_size:nrhs",
                "This function takes one string.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_values_size:nlhs",
                "This function has one output argument.");
    }

    /* input must be a string */
    if (!mxIsChar(prhs[0])) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_values_size:inputNotString",
                "Input 1 must be a string.");
    }

    /* copy the string data from prhs into a C string */
    char *key = mxArrayToString(prhs[0]);
    if (key == NULL) {
        mexErrMsgIdAndTxt("MATLAB:fncs:get_values_size:conversionFailed",
                "Could not convert input to string.");
    }

    /* Call the fncs::get_values subroutine. */
    size_t result = fncs::get_values(key).size();

    /* Allocate return value. */
    plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);

    /* Assign return value. */
    uint64_t * data = (uint64_t *) mxGetData(plhs[0]);
    data[0] = result;

    /* clean up temporary strings */
    mxFree(key);
}

