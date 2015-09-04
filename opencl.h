#ifndef OPENCL_H
#define OPENCL_H

int openclInit(int desiredPlatform, int desiredDevice);
void openclFini();
int openclBuildProgram(const char *source);
void openclFiniProgram();
int openclExecMandlebrot(int w, int h, double scale, double topLeftX, double topLeftY, int limit, double *samples);

#endif // OPENCL_H

