//#include <signal.h>
//#include <hip_runtime.h>
//#include "src/acc/settings.h"
//#include "src/acc/acc_backprojector.h"
//#include "src/acc/hip/hip_kernels/hip_device_utils.hpp"
//#include "src/acc/acc_projector.h"

size_t AccBackprojector::setMdlDim(
			int xdim, int ydim, int zdim,
			int inity, int initz,
			int max_r, int paddingFactor)
{
	if (xdim != mdlX ||
		ydim != mdlY ||
		zdim != mdlZ ||
		inity != mdlInitY ||
		initz != mdlInitZ ||
		max_r != maxR ||
		paddingFactor != padding_factor)
	{
		clear();

		mdlX = xdim;
		mdlY = ydim;
		mdlZ = zdim;
		if (mdlZ < 1) mdlZ = 1;
		mdlXYZ = (size_t)xdim*(size_t)ydim*(size_t)zdim;
		mdlInitY = inity;
		mdlInitZ = initz;
		maxR = max_r;
		maxR2 = max_r*max_r;
		padding_factor = paddingFactor;

		//Allocate space for model
#ifdef HIP
		HANDLE_ERROR(hipMalloc( (void**) &d_mdlReal,   mdlXYZ * sizeof(XFLOAT)));
		HANDLE_ERROR(hipMalloc( (void**) &d_mdlImag,   mdlXYZ * sizeof(XFLOAT)));
		HANDLE_ERROR(hipMalloc( (void**) &d_mdlWeight, mdlXYZ * sizeof(XFLOAT)));
#else
		if (posix_memalign((void **)&d_mdlReal,   MEM_ALIGN, mdlXYZ * sizeof(XFLOAT))) CRITICAL(RAMERR);
		if (posix_memalign((void **)&d_mdlImag,   MEM_ALIGN, mdlXYZ * sizeof(XFLOAT))) CRITICAL(RAMERR);
		if (posix_memalign((void **)&d_mdlWeight, MEM_ALIGN, mdlXYZ * sizeof(XFLOAT))) CRITICAL(RAMERR);

		mutexes = new tbb::spin_mutex[mdlZ*mdlY];

#endif

		allocaton_size = mdlXYZ * sizeof(XFLOAT) * 3;
	}

	return allocaton_size;
}

void AccBackprojector::initMdl()
{
#ifdef DEBUG_HIP
	if (mdlXYZ == 0)
	{
        printf("Model dimensions must be set with setMdlDim before call to initMdl.");
        CRITICAL(ERR_MDLDIM);
	}
	if (voxelCount != 0)
	{
        printf("DEBUG_ERROR: Duplicated call to model setup");
        CRITICAL(ERR_MDLSET);
	}
#endif

	//Initiate model with zeros
#ifdef HIP
	DEBUG_HANDLE_ERROR(hipMemset( d_mdlReal,   0, mdlXYZ * sizeof(XFLOAT)));
	DEBUG_HANDLE_ERROR(hipMemset( d_mdlImag,   0, mdlXYZ * sizeof(XFLOAT)));
	DEBUG_HANDLE_ERROR(hipMemset( d_mdlWeight, 0, mdlXYZ * sizeof(XFLOAT)));
#else
	memset(d_mdlReal,     0, mdlXYZ * sizeof(XFLOAT));
	memset(d_mdlImag,     0, mdlXYZ * sizeof(XFLOAT));
	memset(d_mdlWeight,   0, mdlXYZ * sizeof(XFLOAT));
#endif

    voxelCount = mdlXYZ;
}


void AccBackprojector::getMdlData(XFLOAT *r, XFLOAT *i, XFLOAT * w)
{
#ifdef HIP
	DEBUG_HANDLE_ERROR(hipStreamSynchronize(stream)); //Make sure to wait for remaining kernel executions

	DEBUG_HANDLE_ERROR(hipMemcpyAsync( r, d_mdlReal,   mdlXYZ * sizeof(XFLOAT), hipMemcpyDeviceToHost, stream));
	DEBUG_HANDLE_ERROR(hipMemcpyAsync( i, d_mdlImag,   mdlXYZ * sizeof(XFLOAT), hipMemcpyDeviceToHost, stream));
	DEBUG_HANDLE_ERROR(hipMemcpyAsync( w, d_mdlWeight, mdlXYZ * sizeof(XFLOAT), hipMemcpyDeviceToHost, stream));

	DEBUG_HANDLE_ERROR(hipStreamSynchronize(stream)); //Wait for copy
#else
	memcpy(r, d_mdlReal,   mdlXYZ * sizeof(XFLOAT));
	memcpy(i, d_mdlImag,   mdlXYZ * sizeof(XFLOAT));
	memcpy(w, d_mdlWeight, mdlXYZ * sizeof(XFLOAT));
#endif
}

void AccBackprojector::getMdlDataPtrs(XFLOAT *& r, XFLOAT *& i, XFLOAT *& w)
{
#ifndef HIP
	r = d_mdlReal;
	i = d_mdlImag;
	w = d_mdlWeight;
#endif
}

void AccBackprojector::clear()
{
	mdlX = 0;
	mdlY = 0;
	mdlZ = 0;
	mdlXYZ = 0;
	mdlInitY = 0;
	mdlInitZ = 0;
	maxR = 0;
	maxR2 = 0;
	padding_factor = 0;
	allocaton_size = 0;

	if (d_mdlReal != NULL)
	{
#ifdef HIP
		DEBUG_HANDLE_ERROR(hipFree(d_mdlReal));
		DEBUG_HANDLE_ERROR(hipFree(d_mdlImag));
		DEBUG_HANDLE_ERROR(hipFree(d_mdlWeight));
#else
		free(d_mdlReal);
		free(d_mdlImag);
		free(d_mdlWeight);
		delete [] mutexes;
#endif 

		d_mdlReal = d_mdlImag = d_mdlWeight = NULL;
	}
}

AccBackprojector::~AccBackprojector()
{
	clear();
}
