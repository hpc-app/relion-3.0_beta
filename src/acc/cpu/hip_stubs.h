#ifndef HIP_STUBS_H
#define HIP_STUBS_H

#undef HIP
#typedef float hipStream_t;
#typedef double HipCustomAllocator;
#typedef int dim3;
#define hipStreamPerThread 0
#define CUSTOM_ALLOCATOR_REGION_NAME( name ) //Do nothing
#define LAUNCH_PRIVATE_ERROR(func, status)
#define LAUNCH_HANDLE_ERROR( err )
#define DEBUG_HANDLE_ERROR( err )
#define HANDLE_ERROR( err ) 
#endif
