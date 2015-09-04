#pragma OPENCL EXTENSION cl_khr_fp64: enable

// NB: "FLOAT" is just a macro, usually defined to "double", but could be "float" if desired.
__kernel void fractal(int w, FLOAT scale, FLOAT topLeftX, FLOAT topLeftY, int limit, __global FLOAT *buffer) {
	int i;
	FLOAT zx, zy, zxSq, zySq, cx, cy;
	const FLOAT escapeRadius = 2;
	int x = get_global_id(0);
	int y = get_global_id(1);
	zx = 0;
	zy = 0;
	zxSq = 0;
	zySq = 0;
	cx = (x * scale) + topLeftX;
	cy = (y * scale) + topLeftY;
	for (i = 1; i < limit && zxSq + zySq < 4; i++) {
		zy = (2 * zx * zy) + cy;
		zx = zxSq - zySq + cx;
		zxSq = zx * zx;
		zySq = zy * zy;
	}
	if (i == limit)
		buffer[y * w + x] = 1;
	else {
		// some weird smoothing code
		FLOAT foo = log((FLOAT)sqrt((FLOAT)(zxSq + zySq))) / log((FLOAT)escapeRadius);
		buffer[y * w + x] = (i - log((FLOAT)foo) / log((FLOAT)2)) / limit;
	}
}

// NB: "FLOAT" is just a macro, usually defined to "double", but could be "float" if desired.
__kernel void julia(int w, FLOAT scale, FLOAT topLeftX, FLOAT topLeftY, FLOAT cx, FLOAT cy, int limit, __global FLOAT *buffer) {
	int i;
	FLOAT zx, zy, zxSq, zySq;
	int x = get_global_id(0);
	int y = get_global_id(1);
	zx = (x * scale) + topLeftX;
	zy = (y * scale) + topLeftY;
	zxSq = zx * zx;
	zySq = zy * zy;
	for (i = 1; i < limit && zxSq + zySq < 4; i++) {
		zy = (2 * zx * zy) + cy;
		zx = zxSq - zySq + cx;
		zxSq = zx * zx;
		zySq = zy * zy;
	}
	buffer[y * w + x] = ((FLOAT)i) / limit;
}
