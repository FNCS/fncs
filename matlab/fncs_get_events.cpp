#include "mex.h"

#include <stdint.h>
#include <vector>

using namespace std;

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_events:nrhs",
                "This function does not take input arguments.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:get_events:nlhs",
                "This function has one output argument.");
    }

    /* Call the fncs::get_events subroutine. */
    vector<string> events = fncs::get_events();
    mwSize size = events.size();

    /* convert vector<string> to cell matrix */
    mxArray *array = mxCreateCellMatrix(size, 1);
    if (array == NULL) {
        mexErrMsgIdAndTxt("MATLAB:fncs:get_events:mxCreateCellMatrix",
                "Unable to create cell matrix.");
    }
    for (mwIndex i=0; i<size; ++i) {
        mxSetCell(array, i, mxCreateString(events[i].c_str()));
    }

    /* Allocate return value. */
    plhs[0] = array;
}

