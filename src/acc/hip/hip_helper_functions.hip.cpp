#undef ALTCPU
#include <hip/hip_runtime.h>
#include "src/ml_optimiser.h"
#include "src/acc/acc_ptr.h"
#include "src/acc/acc_projector.h"
#include "src/acc/acc_projector_plan.h"
#include "src/acc/acc_backprojector.h"
#include "src/acc/hip/hip_settings.h"
#include "src/acc/hip/hip_fft.h"
#include "src/acc/hip/hip_kernels/hip_device_utils.hpp"

#ifdef HIP_FORCESTL
#include "src/acc/hip/hip_utils_stl.hpp"
#else
#include "src/acc/hip/hip_utils_cub.hpp"
#endif

#include "src/acc/utilities.h"
#include "src/acc/acc_helper_functions.h"
#include "src/acc/hip/hip_kernels/BP.hpp"
#include "src/macros.h"
#include "src/error.h"

#include "src/acc/acc_ml_optimiser.h"
#include "src/acc/hip/hip_ml_optimiser.h"
#include "src/acc/acc_helper_functions.h"


#include "src/acc/acc_helper_functions_impl.h"
