/*
 * fncs_agentRegister.cpp
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
    if(nrhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:agentRegister:nrhs",
                "This function does not have input arguments.");
    }
    if(nlhs!=0) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:agentRegister:nlhs",
                "This function does not have output arguments.");
    }

    /* Call the fncs::agentRegister subroutine. */
    fncs::agentRegister();
}
