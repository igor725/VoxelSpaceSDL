#ifndef VSERROR_H
#define VSERROR_H
typedef enum eErrors {
	ERROR_OK,
	ERROR_MAPLOAD_FILE,
	ERROR_MAPLOAD_IMGSIZE,
	ERROR_MAPLOAD_WIDTHINVALID,
	ERROR_MAPLOAD_SCALE,
	ERROR_MAPLOAD_MAPSMISMATCH,
	ERROR_MALLOC_FAIL,
	ERROR_MAX
} Errors;

#ifdef VSERROR_STRINGS
static const char *Errors_Strings[ERROR_MAX] = {
	"ERROR_OK",
	"ERROR_MAPLOAD_FILE",
	"ERROR_MAPLOAD_IMGSIZE",
	"ERROR_MAPLOAD_WIDTHINVALID",
	"ERROR_MAPLOAD_SCALE",
	"ERROR_MAPLOAD_MAPSMISMATCH",
	"ERROR_MALLOC_FAIL"
};
#endif
#endif