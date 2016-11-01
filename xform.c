#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "xform.h"
#include "variation.h"

void xformInit(Xform *xform) {
	xform->vars = NULL;
	xform->nVars = 0;
	xform->precalcFlags = 0;

	xform->colourIndex = 0.f;
	xform->colour.r = 0.f;
	xform->colour.g = 0.f;
	xform->colour.b = 0.f;

	memset(&xform->coMain, 0, sizeof(Coefficients));
	memset(&xform->coPost, 0, sizeof(Coefficients));

	xform->hasPost = 0;

	xform->weight = 0.5f;
	xform->symmetry = 0.f;
}

void xformFini(Xform *xform) {
	if (xform->vars != NULL) {
		free(xform->vars);
	}
}

void xformAddVariation(Xform *xform, int variation, FLOAT weight) {
	xform->vars = realloc(xform->vars, sizeof(XformVariation) * (xform->nVars + 1));
	if (xform->vars == NULL) {
		plog(LOG_ERROR, "xformAddVariation: memory allocation failed\n");
		return;
	}
	variationInit(xform, &xform->vars[xform->nVars], variation, weight, 0.f, 0.f, 0.f, 0.f);
	xform->nVars++;
}

