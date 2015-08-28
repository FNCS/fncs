#include "mex.h"

#include <fncs.hpp>

static inline void to_fncs_time(fncs::time &value,const mxArray *mat_buf) {
    if (!mxIsUint64(mat_buf)) {
        mexPrintf("Integer scalar is not uint64, but that's OK.\n");
    }
    if (mxGetNumberOfElements(mat_buf) == 1) {
        value = mxGetScalar(mat_buf);
    } else {
        mexErrMsgTxt("Integer scalar is not of size == [1 1].\n");
    }
}

