/*
 * fncs_agentRegisterConfig.cpp
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
		mexErrMsgIdAndTxt( "MATLAB:fncs:agentRegisterConfig:nrhs",
				"This function takes one string.");
	}
	if(nlhs!=0) {
		mexErrMsgIdAndTxt( "MATLAB:fncs:agentRegisterConfig:nlhs",
				"This function does not have output arguments.");
	}

	/* input must be a string */
	if (!mxIsChar(prhs[0])) {
		mexErrMsgIdAndTxt( "MATLAB:fncs:agentRegisterConfig:inputNotString",
				"Input 1 must be a string.");
	}

	/* copy the string data from prhs into a C string */
	char *config = mxArrayToString(prhs[0]);
	if (config == NULL) {
		mexErrMsgIdAndTxt("MATLAB:fncs:agentRegisterConfig:conversionFailed",
				"Could not convert input to string.");
	}
    /* Call the fncs::agentRegister subroutine. */
    fncs::agentRegister(std::string(config));

    /* Clean up temporary strings. */
    mxfree(config);
}
