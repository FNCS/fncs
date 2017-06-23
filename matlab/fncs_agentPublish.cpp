/*
 * fncs_agentPublish.cpp
 *
 *  Created on: Jun 23, 2017
 *      Author: fish334
 */
#include "mex.h"

#include <fncs.hpp>

void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[] )
{
    /* Check for proper number of arguments. */
    if(nrhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:agentPublish:nrhs",
                "This function takes one string.");
    }
    if(nlhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:agentPublish:nlhs",
                "This function does not have output arguments.");
    }

    /* inputs must be strings */
    if (!mxIsChar(prhs[0])) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:agentPublish:inputNotString",
                "Input must be a string.");
    }

    /* copy the string data from prhs into a C string */
    char *value = mxArrayToString(prhs[0]);
    if (key == NULL) {
        mexErrMsgIdAndTxt("MATLAB:fncs:agentPublish:conversionFailed",
                "Could not convert input to string.");
    }

    /* Call the fncs::agentPublish subroutine. */
    fncs::agentPublish(std::string(value));

    /* clean up temporary strings */
    mxFree(value);
}
