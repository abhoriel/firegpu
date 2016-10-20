#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "xform.h"

Xform *xformCreate() {
	Xform *xform = malloc(sizeof(Xform));
	if (xform == NULL) {
		return NULL;
	}

	xform->vars = NULL;
	xform->varCoefficients = NULL;
	xform->nVars = 0;
	xform->colourIndex = 0;

	memset(&xform->coMain, 0, sizeof(Coefficients));
	memset(&xform->coFinal, 0, sizeof(Coefficients));

	xform->weight = 0.5;
	xform->symmetry = 0;

	return xform;
}

void xformDestroy(Xform *xform) {
	if (xform->vars != NULL) {
		free(xform->vars);
	}
	if (xform->varCoefficients != NULL) {
		free(xform->varCoefficients);
	}
	free(xform);
}

void xformAddVariation(Xform *xform, int variation, FLOAT coefficient) {
	xform->vars = realloc(xform->vars, sizeof(int) * (xform->nVars + 1));
	xform->varCoefficients = realloc(xform->varCoefficients, sizeof(int) * (xform->nVars + 1));
	if ((xform->vars == NULL) || (xform->varCoefficients == NULL)) {
		plog(LOG_ERROR, "xformAddVariation: memory allocation failed\n");
		return;
	}
	xform->vars[xform->nVars] = variation;
	xform->varCoefficients[xform->nVars] = coefficient;
	xform->nVars++;
}

