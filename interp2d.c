#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include "interp2d.h"

#define DISCARD_STATUS(s) if ((s) != GSL_SUCCESS) { GSL_ERROR_VAL("interpolation error", (s),  GSL_NAN); }

interp2d* interp2d_alloc(const interp2d_type* T, size_t xsize, size_t ysize) {
    interp2d* interp;
    if (xsize < T->min_size || ysize < T->min_size) {
        GSL_ERROR_NULL("insufficient number of points for interpolation type", GSL_EINVAL);
    }
    interp = (interp2d*)malloc(sizeof(interp2d));
    if (interp == NULL) {
        GSL_ERROR_NULL("failed to allocate space for interp2d struct", GSL_ENOMEM);
    }
    interp->type = T;
    interp->xsize = xsize;
    interp->ysize = ysize;
    if (interp->type->alloc == NULL) {
        interp->state = NULL;
        return interp;
    }
    interp->state = interp->type->alloc(xsize);
    if (interp->state == NULL) {
        free(interp);
        GSL_ERROR_NULL("failed to allocate space for interp2d state", GSL_ENOMEM);
    }
    return interp;
}

int interp2d_init(interp2d* interp, const double xarr[], const double yarr[], const double zarr[], size_t xsize, size_t ysize) {
    size_t i;
    if (xsize != interp->xsize || ysize != interp->ysize) {
        GSL_ERROR("data must match size of interpolation object", GSL_EINVAL);
    }
    for (i = 1; i < xsize; i++) {
        if (xarr[i-1] >= xarr[i]) {
            GSL_ERROR("x values must be strictly increasing", GSL_EINVAL);
        }
    }
    for (i = 1; i < ysize; i++) {
        if (yarr[i-1] >= yarr[i]) {
            GSL_ERROR("y values must be strictly increasing", GSL_EINVAL);
        }
    }
    interp->xmin = xarr[0];
    interp->xmax = xarr[xsize - 1];
    interp->ymin = yarr[0];
    interp->ymax = yarr[ysize - 1];
    {
        int status = interp->type->init(interp->state, xarr, yarr, zarr, xsize, ysize);
        return status;
    }
}

void interp2d_free(interp2d* interp) {
    if (!interp) {
        return;
    }
    if (interp->type->free) {
        interp->type->free(interp->state);
    }
    free(interp);
}

double interp2d_eval(const interp2d* interp, const double xarr[], const double yarr[], const double zarr[], const double x, const double y, gsl_interp_accel* xa, gsl_interp_accel* ya) {
    double z;
    int status;
    if (x < interp->xmin || x > interp->xmax) {
        GSL_ERROR_VAL("interpolation error", GSL_EDOM, GSL_NAN);
    }
    if (y < interp->ymin || y > interp->ymax) {
        GSL_ERROR_VAL("interpolation error", GSL_EDOM, GSL_NAN);
    }
    status = interp->type->eval(interp->state, xarr, yarr, zarr, interp->xsize, interp->ysize, x, y, xa, ya, &z);
    DISCARD_STATUS(status);
    return z;
}

size_t interp2d_type_min_size(const interp2d_type* T) {
    return T->min_size;
}

size_t interp2d_min_size(const interp2d* interp) {
    return interp->type->min_size;
}

const char* interp2d_name(const interp2d* interp) {
    return interp->type->name;
}
