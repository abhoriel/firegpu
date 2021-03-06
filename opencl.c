#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif
#include "flame.h"
#include "opencl.h"
#include "clerror.h"
#include "log.h"

static char *getPlatformInfo(cl_platform_id platformId, cl_platform_info name);
static void printPlatformInfo(cl_platform_id platformId);
static char *getDeviceInfo(cl_device_id deviceID, cl_device_info name);
static void printDeviceInfo(cl_device_id deviceID);
static cl_device_id getDevice(unsigned int desiredPlatform, unsigned int desiredDevice);

static cl_context context = NULL;
static cl_command_queue commandQueue = NULL;
static cl_program program = NULL;
static cl_kernel fractalKernel = NULL;
static cl_device_id deviceID = NULL;

int openclInit(int desiredPlatform, int desiredDevice) {
	cl_int ret;
	
	deviceID = getDevice(desiredPlatform, desiredDevice);
	if (deviceID == NULL) {
		return -1;
	}
	
	// Create an OpenCL context
	context = clCreateContext( NULL, 1, &deviceID, NULL, NULL, &ret);
	if (context == NULL) {
		plog(LOG_ERROR, "error creating opencl context: %s\n", openclGetError(ret));
		return -1;
	}

	// Create a command queue
	// This invocation is now deprecated as of opencl 2.0, but seems to be all that works on OSX currently
//#ifdef __APPLE__
	commandQueue = clCreateCommandQueue(context, deviceID, 0, &ret);
//#else
	// this one is ok though
//	commandQueue = clCreateCommandQueueWithProperties(context, deviceID, NULL, &ret);
//#endif
	if (commandQueue == NULL) {
		plog(LOG_ERROR, "error creating opencl command queue: %s\n", openclGetError(ret));
		return -1;
	}
	return 0;
}


int openclBuildProgram(const char *source) {
	cl_int ret;
	
	// Create a program from the kernel source
	size_t sourceSize = strlen(source);
	program = clCreateProgramWithSource(context, 1, 
			(const char **)&source, (const size_t *)&sourceSize, &ret);
			
	if (program == NULL) {
		plog(LOG_INFO, "error creating opencl program: %s\n", openclGetError(ret));
		return -1;
	}
	
	// these build options are documented here: 
	// https://www.khronos.org/registry/cl/sdk/2.0/docs/man/xhtml/clBuildProgram.html
	ret = clBuildProgram(program, 1, &deviceID, "-D FLOAT=float -cl-denorms-are-zero -cl-fast-relaxed-math -Werror", NULL, NULL);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error building opencl program: %s\n", openclGetError(ret));
		// Determine the size of the log
		size_t logSize;
		clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
		// Allocate memory for the log
		char *log = malloc(logSize);
		// Get the log
		clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
		// Print the log
		plog(LOG_ERROR, "%s\n", log);
		free(log);
		return -1;
	}

	// Create the OpenCL kernel
	fractalKernel = clCreateKernel(program, "generate", &ret);
	if (fractalKernel == NULL) {
		plog(LOG_ERROR, "error creating opencl kernel: %s\n", openclGetError(ret));
		return -1;
	}

	return 0;
}


int openclExecMandlebrot(int w, int h, double scale, double topLeftX, double topLeftY, int limit, double *samples) {
	cl_int ret;
	
	// allocate a buffer for the samples
	cl_mem samplesBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
			w * h * sizeof(double), NULL, &ret);

	// Set the arguments of the kernel
	ret = clSetKernelArg(fractalKernel, 0, sizeof(int), &w);
	ret = clSetKernelArg(fractalKernel, 1, sizeof(double), &scale);
	ret = clSetKernelArg(fractalKernel, 2, sizeof(double), &topLeftX);
	ret = clSetKernelArg(fractalKernel, 3, sizeof(double), &topLeftY);
	ret = clSetKernelArg(fractalKernel, 4, sizeof(int), &limit);
	ret = clSetKernelArg(fractalKernel, 5, sizeof(cl_mem), (void *)&samplesBuffer);
	
	// Execute the OpenCL kernel on the list
	size_t globalItemSize[] = {(size_t)w, (size_t)h};	// Process the entire lists
	ret = clEnqueueNDRangeKernel(commandQueue, fractalKernel, 2, NULL, 
			globalItemSize, NULL, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error enqueuing command: %s\n", openclGetError(ret));
		return -1;
	}
	
	// read the samples back
	ret = clEnqueueReadBuffer(commandQueue, samplesBuffer, CL_TRUE, 0, w * h * sizeof(double), samples, 0, NULL, NULL);
	clReleaseMemObject(samplesBuffer);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error enqueuing read buffer: %s\n", openclGetError(ret));
		return -1;
	}
	
	return 0;
}

int openclExecFlame(Flame *flame, int *xfd, int xfdSize, Pixel *pixels, int nPixels) {
	cl_int ret;

	FlameOpenCL flameOpenCL;
	flameOpenCL.w = flame->w;
	flameOpenCL.h = flame->h;
	flameOpenCL.supersample = flame->supersample;
	flameOpenCL.iterations = flame->iterations;
	flameOpenCL.seed = time(NULL);

	XformOpenCL *xforms = malloc(sizeof(XformOpenCL) * flame->nXforms);
	for (int i = 0; i < flame->nXforms; i++) {
		xforms[i].a = flame->xforms[i].coMain.a;
		xforms[i].b = flame->xforms[i].coMain.b;
		xforms[i].c = flame->xforms[i].coMain.c;
		xforms[i].d = flame->xforms[i].coMain.d;
		xforms[i].e = flame->xforms[i].coMain.e;
		xforms[i].f = flame->xforms[i].coMain.f;
		xforms[i].opacity = flame->xforms[i].opacity;
		xforms[i].colour.r = flame->xforms[i].colour.r;
		xforms[i].colour.g = flame->xforms[i].colour.g;
		xforms[i].colour.b = flame->xforms[i].colour.b;
	}
	
	// allocate a buffer for the flame struct
	cl_mem flameStructBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(FlameOpenCL), NULL, &ret);
	if (flameStructBuffer == NULL) {
		plog(LOG_ERROR, "error creating flame struct buffer: %s\n", openclGetError(ret));
		return -1;
	}
	ret = clEnqueueWriteBuffer(commandQueue, flameStructBuffer, CL_TRUE, 0, sizeof(FlameOpenCL), &flameOpenCL, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error enqueuing flame struct buffer: %s\n", openclGetError(ret));
		return -1;
	}

	// allocate a buffer for the xform structs
	cl_mem xformStructBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(XformOpenCL) * flame->nXforms, NULL, &ret);
	if (xformStructBuffer == NULL) {
		plog(LOG_ERROR, "error creating xform struct buffer: %s\n", openclGetError(ret));
		return -1;
	}
	ret = clEnqueueWriteBuffer(commandQueue, xformStructBuffer, CL_TRUE, 0, sizeof(XformOpenCL) * flame->nXforms, xforms, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error enqueuing xform struct buffer: %s\n", openclGetError(ret));
		return -1;
	}

	// allocate a buffer for the pixels
	cl_mem pixelsBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, nPixels * sizeof(Pixel), NULL, &ret);
	if (pixelsBuffer == NULL) {
		plog(LOG_ERROR, "error creating pixel buffer: %s\n", openclGetError(ret));
		return -1;
	}
	ret = clEnqueueWriteBuffer(commandQueue, pixelsBuffer, CL_TRUE, 0, nPixels * sizeof(Pixel), pixels, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error enqueuing write pixel buffer: %s\n", openclGetError(ret));
		return -1;
	}

	// allocate a buffer for the xfd
	cl_mem xfdBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, xfdSize * sizeof(int), NULL, &ret);
	if (xfdBuffer == NULL) {
		plog(LOG_ERROR, "error creating xfd buffer: %s\n", openclGetError(ret));
		return -1;
	}
	ret = clEnqueueWriteBuffer(commandQueue, xfdBuffer, CL_TRUE, 0, xfdSize * sizeof(int), xfd, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error enqueuing write xfd buffer: %s\n", openclGetError(ret));
		return -1;
	}

	// Set the arguments of the kernel
	ret = clSetKernelArg(fractalKernel, 0, sizeof(cl_mem), &flameStructBuffer);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error setting kernel argument 1: %s\n", openclGetError(ret));
		return -1;
	}

	ret = clSetKernelArg(fractalKernel, 1, sizeof(cl_mem), &xformStructBuffer);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error setting kernel argument 2: %s\n", openclGetError(ret));
		return -1;
	}

	ret = clSetKernelArg(fractalKernel, 2, sizeof(cl_mem), &xfdBuffer);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error setting kernel argument 3: %s\n", openclGetError(ret));
		return -1;
	}
	ret = clSetKernelArg(fractalKernel, 3, sizeof(cl_mem), &pixelsBuffer);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error setting kernel argument 4: %s\n", openclGetError(ret));
		return -1;
	}
	
	// Execute the OpenCL kernel on the list
	size_t globalItemSize[] = {(size_t)flame->quality * flame->w * flame->h};
	ret = clEnqueueNDRangeKernel(commandQueue, fractalKernel, 1, NULL, 
			globalItemSize, NULL, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error enqueuing command: %s\n", openclGetError(ret));
		return -1;
	}
	
	// read the pixels back
	ret = clEnqueueReadBuffer(commandQueue, pixelsBuffer, CL_TRUE, 0, nPixels * sizeof(Pixel), pixels, 0, NULL, NULL);
	if (ret != CL_SUCCESS) {
		plog(LOG_ERROR, "error enqueuing read buffer: %s\n", openclGetError(ret));
		return -1;
	}
	clReleaseMemObject(flameStructBuffer);
	clReleaseMemObject(xformStructBuffer);
	clReleaseMemObject(pixelsBuffer);
	clReleaseMemObject(xfdBuffer);
	free(xforms);
	
	return 0;
}

void openclFiniProgram() {
	clReleaseKernel(fractalKernel);
	clReleaseProgram(program);
}

void openclFini() {
	clFlush(commandQueue);
	clFinish(commandQueue);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
}


static cl_device_id getDevice(unsigned int desiredPlatform, unsigned int desiredDevice) {
	const int max = 8;
	
	int platformNumber, deviceNumber;
	// Get platform and device information
	cl_platform_id platformId[max];
	cl_device_id deviceID[max];
	cl_uint devicesCount;
	cl_uint platformCount;
	clGetPlatformIDs(1, platformId, &platformCount);
	plog(LOG_VERBOSE, "total opencl platforms: %d\n", platformCount);
	
	if (platformCount < 1) {
		plog(LOG_ERROR, "no opencl platforms available\n");
		return NULL;
	}
	
	// if a desiredPlatform is provided, we will attempt to get that platform
	if ((desiredPlatform > 0) && (desiredPlatform <= platformCount)) {
		platformNumber = desiredPlatform - 1;
	} else {
		platformNumber = 0;
	}
	
	//printf("\n");
	plog(LOG_VERBOSE, "\n");
	printPlatformInfo(platformId[platformNumber]);
	plog(LOG_VERBOSE, "\n");
	
	clGetDeviceIDs(platformId[platformNumber], CL_DEVICE_TYPE_ALL, max, 
			deviceID, &devicesCount);
	plog(LOG_VERBOSE, "total opencl devices %d\n", devicesCount);
	
	if (devicesCount < 1) {
		plog(LOG_ERROR, "no opencl devices available on platform\n");
		return NULL;
	}
	if ((desiredDevice > 0) && (desiredDevice <= devicesCount)) {
		deviceNumber = desiredDevice - 1;
	} else {
		deviceNumber = 0;
	}
	
	plog(LOG_VERBOSE, "\n");
	printDeviceInfo(deviceID[deviceNumber]);
	plog(LOG_VERBOSE, "\n");
	
	return deviceID[deviceNumber];
}

// get platform info. returns string which must be free()'d
static char *getPlatformInfo(cl_platform_id platformId, cl_platform_info name) {
	char *value = NULL;
	size_t size = 0;
	// get size of profile char array
	clGetPlatformInfo(platformId, name, 0, value, &size);
	value = malloc(size);
	// get profile char array
	clGetPlatformInfo(platformId, name, size, value, NULL);
	return value;
}

static void printPlatformInfo(cl_platform_id platformId) {
	char *str;
	plog(LOG_VERBOSE,"Platform info\n");
	str = getPlatformInfo(platformId, CL_PLATFORM_NAME);
	plog(LOG_VERBOSE,"Name: %s\n", str);
	free(str);
	str = getPlatformInfo(platformId, CL_PLATFORM_VENDOR);
	plog(LOG_VERBOSE,"Vendor: %s\n", str);
	free(str);
	str = getPlatformInfo(platformId, CL_PLATFORM_VERSION);
	plog(LOG_VERBOSE,"Version: %s\n", str);
	free(str);
	str = getPlatformInfo(platformId, CL_PLATFORM_EXTENSIONS);
	plog(LOG_VERBOSE,"Extensions: %s\n", str);
	free(str);
}

// get device info. returns string which must be free()'d
static char *getDeviceInfo(cl_device_id deviceId, cl_device_info name) {
	char *value = NULL;
	size_t size = 0;
	// get size of profile char array
	clGetDeviceInfo(deviceId, name, 0, value, &size);
	value = malloc(size);
	// get profile char array
	clGetDeviceInfo(deviceId, name, size, value, NULL);
	return value;
}

static void printDeviceInfo(cl_device_id deviceId) {
	char *str;
	plog(LOG_VERBOSE,"Device info\n");
	str = getDeviceInfo(deviceId, CL_DEVICE_NAME);
	plog(LOG_VERBOSE,"Name: %s\n", str);
	free(str);

	str = getDeviceInfo(deviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY);
	plog(LOG_VERBOSE,"Frequency: %d MHz\n", *((unsigned int *)str));
	free(str);

	str = getDeviceInfo(deviceId, CL_DEVICE_MAX_COMPUTE_UNITS);
	plog(LOG_VERBOSE,"Compute units: %d\n", *((unsigned int *)str));
	free(str);

	str = getDeviceInfo(deviceId, CL_DEVICE_GLOBAL_MEM_SIZE);
	plog(LOG_VERBOSE,"Global memory size (B): %lu\n", *((unsigned long *)str));
	free(str);

	str = getDeviceInfo(deviceId, CL_DEVICE_LOCAL_MEM_SIZE);
	plog(LOG_VERBOSE,"Local memory size (B): %lu\n", *((unsigned long *)str));
	free(str);

	str = getDeviceInfo(deviceId, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);
	plog(LOG_VERBOSE,"Preferred float width: %d\n", *((unsigned int *)str));
	free(str);

	str = getDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE);
	plog(LOG_VERBOSE,"Max work group size: %zu\n", *((size_t *)str));
	free(str);

	str = getDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
	unsigned int maxWorkItemDimensions = *((unsigned int *)str);
	free(str);

	str = getDeviceInfo(deviceId, CL_DEVICE_MAX_WORK_ITEM_SIZES);
	for (unsigned i = 0; i < maxWorkItemDimensions; i++) {
		plog(LOG_VERBOSE, "Dimension %u max work item size: %zu\n", i, ((size_t *)str)[i]);
	}
	free(str);

	str = getDeviceInfo(deviceId, CL_DEVICE_EXTENSIONS);
	plog(LOG_VERBOSE,"Extensions: %s\n", str);
	free(str);
}
