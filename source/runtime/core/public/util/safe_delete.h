#ifndef SAFE_DELETE_H
#define SAFE_DELETE_H

#include <stdlib.h>

#define SAFE_FREE(p)         { free((p)); (p) = NULL; }
#define SAFE_RELEASE(p)      { if ((p)) { (p)->Release(); (p) = NULL; } }

#endif // SAFE_DELETE_H