#include "common.h"


static PyModuleDef humonModule =
{
    PyModuleDef_HEAD_INIT,
    .m_name = "humon",
    .m_doc = "Human Usable Machine Object Notation",
    .m_size = -1
};


PyMODINIT_FUNC PyInit_humon(void)
{
    PyObject * module = PyModule_Create(& humonModule);
    if (module == NULL)
        { return NULL; }
    
    // Import the enums module, where ".enums"
    // is the full name of the enums module
    PyObject * enums = PyImport_ImportModule("humon.enums");
    if (enums == NULL) {
        Py_DECREF(module);
        return NULL;
    }

//    Py_DECREF(enums);
    
    if (RegisterTokenType(module) < 0)
        { return NULL; }
    if (RegisterNodeType(module) < 0)
        { return NULL; }
    if (RegisterTroveType(module) < 0)
        { return NULL; }
    
    return module;
}
