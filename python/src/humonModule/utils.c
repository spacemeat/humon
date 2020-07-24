#include "common.h"


PyObject * getEnumValue(char const * moduleName, char const * enumName, int value)
{
    PyObject * enums = PyImport_AddModule(moduleName);
    if (enums == NULL) {
        return NULL;
    }

    PyObject * enumType = PyObject_GetAttrString(enums, enumName);
    if (enumType == NULL) {
        return NULL;
    }

    PyObject * inst = PyObject_CallFunction(enumType, "(i)", value);
    if (inst == NULL) {
        Py_DECREF(enumType);
        return NULL;
    }

    Py_DECREF(enumType);
    return inst;
}


