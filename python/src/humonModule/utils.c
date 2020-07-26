#include "common.h"


PyObject * getEnumValue(char const * enumName, int value)
{
    PyObject * enums = PyImport_AddModule("humon.enums");
    if (enums == NULL) {
        PyErr_SetString(PyExc_ValueError, "Could not find module");
        return NULL;
    }

    PyObject * enumType = PyObject_GetAttrString(enums, enumName);
    if (enumType == NULL) {
        PyErr_SetString(PyExc_ValueError, "Could not find enum");
        return NULL;
    }

    PyObject * inst = PyObject_CallFunction(enumType, "(i)", value);
    if (inst == NULL) {
        PyErr_SetString(PyExc_ValueError, "Could not find enum value");
        Py_DECREF(enumType);
        return NULL;
    }

    Py_DECREF(enumType);
    return inst;
}
