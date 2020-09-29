#include "common.h"


static PyModuleDef humonModule =
{
    PyModuleDef_HEAD_INIT,
    .m_name = "humon.humon",
    .m_doc = "Human Usable Machine Object Notation",
    .m_size = -1
};


PyMODINIT_FUNC PyInit_humon(void)
{
    PyObject * module = PyModule_Create(& humonModule);
    if (module == NULL)
        { return NULL; }

    AddEnumConstantsToModule(module);

    PyObject * enums = PyImport_ImportModule("humon.enums");
    if (enums == NULL)
    {
        Py_DECREF(module);
        return NULL;
    }    
    Py_DECREF(enums);

    PyObject * iterators = PyImport_ImportModule("humon.iterators");
    if (iterators == NULL)
    {
        Py_DECREF(module);
        return NULL;
    }    
    Py_DECREF(iterators);

    if (RegisterTokenType(module) < 0)
    {
        Py_DECREF(module);
        return NULL;
    }
    if (RegisterNodeType(module) < 0)
    {
        Py_DECREF(module);
        return NULL;
    }
    if (RegisterTroveType(module) < 0)
    {
        Py_DECREF(module);
        return NULL;
    }

    return module;
}
