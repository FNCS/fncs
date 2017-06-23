/*
 * fncs_agentGetEvents.cpp
 *
 *  Created on: Jun 23, 2017
 *      Author: fish334
 */
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
        mexErrMsgIdAndTxt( "MATLAB:fncs:agentGetEvents:nrhs",
                "This function does not take input arguments.");
    }
    if(nlhs!=1) {
        mexErrMsgIdAndTxt( "MATLAB:fncs:agentGetEvents:nlhs",
                "This function has one output string.");
    }


    /* Call the fncs::agentGetEvents subroutine. */
    string value = fncs::agentGetEvents();

    plhs[0] = mxCreateString(value.c_str());
}
