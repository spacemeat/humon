#include "common.h"


PyObject * getEnumValue(char const * enumName, int value)
{
    PyObject * inst = NULL;

    PyObject * enums = PyImport_AddModule("humon.enums");
    if (enums != NULL)
    {
        Py_INCREF(enums);
        PyObject * enumType = PyObject_GetAttrString(enums, enumName);
        if (enumType != NULL)
        {
            inst = PyObject_CallFunction(enumType, "(i)", value);
            if (inst == NULL)
                { PyErr_SetString(PyExc_ValueError, "Could not find enum value "); }

            Py_DECREF(enumType);
        }
        else
            { PyErr_SetString(PyExc_ValueError, "Could not find enum"); }

        Py_DECREF(enums);
    }
    else
        { PyErr_SetString(PyExc_ValueError, "Could not find module"); }

    return inst;
}
