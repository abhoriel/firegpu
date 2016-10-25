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
	xform->nVars = 0;

	xform->colour = 0.f;

	memset(&xform->coMain, 0, sizeof(Coefficients));
	memset(&xform->coFinal, 0, sizeof(Coefficients));

	xform->hasFinal = 0;

	xform->weight = 0.5f;
	xform->symmetry = 0.f;

	return xform;
}

void xformDestroy(Xform *xform) {
	if (xform->vars != NULL) {
		free(xform->vars);
	}
	free(xform);
}

void xformAddVariation(Xform *xform, int variation, FLOAT weight) {
	xform->vars = realloc(xform->vars, sizeof(XformVariation) * (xform->nVars + 1));
	if (xform->vars == NULL) {
		plog(LOG_ERROR, "xformAddVariation: memory allocation failed\n");
		return;
	}
	xform->vars[xform->nVars].var = variation;
	xform->vars[xform->nVars].weight = weight;
	xform->nVars++;
}

