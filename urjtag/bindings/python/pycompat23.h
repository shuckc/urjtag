/*
 * some compatibility macros for python 2  / python 3
 * by Steve Tell
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 */

#if PY_MAJOR_VERSION >= 3

#define MODINIT_ERROR_VAL NULL
#define MODINIT_SUCCESS_VAL(val) val
#define MODINIT_DECL(name) PyMODINIT_FUNC PyInit_##name(void)

#else  /* assume python 2 */

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

#define MODINIT_ERROR_VAL
#define MODINIT_SUCCESS_VAL(val)
#define MODINIT_DECL(name) PyMODINIT_FUNC init##name(void)

struct PyModuleDef {
    int dc1;
    const char *name;
    const char *doc;
    int dc2;
    PyMethodDef *methods;
};
#define PyModuleDef_HEAD_INIT 0
#define PyModule_Create(dp) Py_InitModule3((dp)->name, (dp)->methods, (dp)->doc)

#endif
