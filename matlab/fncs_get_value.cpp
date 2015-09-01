#include "mex.h"

#include <stdint.h>
#include <vector>

using namespace std;

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_value:nrhs",
                "This function takes one string.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_value:nlhs",
                "This function has one output string.");
    }

    /* input must be a string */
    if (!mxIsChar(prhs[0])) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_value:inputNotString",
                "Input 1 must be a string.");
    }

    /* copy the string data from prhs into a C string */
    char *key = mxArrayToString(prhs[0]);
    if (key == NULL) {
        mexErrMsgIdAndTxt("MATLAB:fncs:get_value:conversionFailed",
                "Could not convert input to string.");
    }

    /* Call the fncs::get_value subroutine. */
    string value = fncs::get_value(key);

    plhs[0] = mxCreateString(value.c_str());

    /* clean up temporary strings */
    mxFree(key);
}

