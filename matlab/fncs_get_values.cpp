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
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_values:nrhs",
                "This function takes one string.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_values:nlhs",
                "This function has one output argument.");
    }

    /* input must be a string */
    if (!mxIsChar(prhs[0])) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_values:inputNotString",
                "Input 1 must be a string.");
    }

    /* copy the string data from prhs into a C string */
    char *key = mxArrayToString(prhs[0]);
    if (key == NULL) {
        mexErrMsgIdAndTxt("MATLAB:fncs:get_values:conversionFailed",
                "Could not convert input to string.");
    }

    /* Call the fncs::get_values subroutine. */
    vector<string> values = fncs::get_values(key);
    mwSize size = values.size();

    /* convert vector<string> to cell matrix */
    mxArray *array = mxCreateCellMatrix(size, 1);
    if (array == NULL) {
        mexErrMsgIdAndTxt("MATLAB:fncs:get_values:mxCreateCellMatrix",
                "Unable to create cell matrix.");
    }
    for (mwIndex i=0; i<size; ++i) {
        mxSetCell(array, i, mxCreateString(values[i].c_str()));
    }

    /* Allocate return value. */
    plhs[0] = array;

    /* clean up temporary strings */
    mxFree(key);
}

