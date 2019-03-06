#include "src/acc/acc_projector_plan.h"
#include "src/time.h"
#include <hip/hip_runtime.h>

#ifdef HIP
//#include <hip_runtime.h>
#ifdef HIP_FORCESTL
#include "src/acc/hip/hip_utils_stl.hpp"
#else
#include "src/acc/hip/hip_utils_cub.hpp"
#endif
#endif

#include "src/acc/utilities.h"

#include "src/acc/acc_projector_plan_impl.h"
