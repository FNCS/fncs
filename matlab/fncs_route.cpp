#include "mex.h"

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    const int NRHS = 4;

    /* Check for proper number of arguments. */
    if(nrhs!=NRHS) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:route:nrhs",
                "This function takes %d strings.", NRHS);
    }
    if(nlhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:route:nlhs",
                "This function does not have output arguments.");
    }

    /* inputs must be strings */
    for (int i=0; i<NRHS; ++i) {
        if (!mxIsChar(prhs[i])) {
            mexErrMsgIdAndTxt( "MATLAB:fncs:route:inputNotString",
                    "Input %d must be a string.", i);
        }
    }

    /* copy the string data from prhs into a C string */
    char * strings[NRHS];
    for (int i=0; i<NRHS; ++i) {
        char *key = mxArrayToString(prhs[0]);
        if (key == NULL) {
            mexErrMsgIdAndTxt("MATLAB:fncs:route:conversionFailed",
                    "Could not convert input %d to string.", i);
        }
        strings[i] = key;
    }

    /* Call the fncs::route subroutine. */
    fncs::route(strings[0], strings[1], strings[2], strings[3]);

    /* clean up temporary strings */
    for (int i=0; i<NRHS; ++i) {
        mxFree(strings[0]);
    }
}

