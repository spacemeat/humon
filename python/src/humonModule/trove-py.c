#include "common.h"


typedef struct
{
    PyObject_HEAD
    huTrove const * trovePtr;
} TroveObject;


static void Trove_dealloc(TroveObject * self)
{
    huDestroyTrove(self->trovePtr);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * Trove_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    TroveObject * self;
    self = (TroveObject *) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->trovePtr = NULL;
    }

    return (PyObject *) self;
}

static int Trove_init(TroveObject * self, PyObject * args, PyObject * kwds)
{
    static char * kwlist[] = {"tokenstream", "tabsize"};

    char const * string = NULL;
    Py_ssize_t stringLen = 0;
    int tabSize = 4;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#|i", kwlist,
        & string, & stringLen, & tabSize))
        { return -1; }
    
    if (string == NULL)
        { return -1; }

    huLoadParams params;
    huInitLoadParams(& params, HU_ENCODING_UTF8, true, tabSize);
    int error = huMakeTroveFromStringN(& self->trovePtr, string, stringLen, & params);
    if (error != HU_ERROR_NOERROR)
        { return -1; }

    return 0;
}

static bool checkYourSelf(TroveObject * self)
{
    if (self->trovePtr == NULL)
    {
        PyErr_SetString(PyExc_ValueError, "accessing nullish trove");
        return false;
    }

    return true;
}

static PyObject * Trove_getNumTokens(TroveObject * self, PyObject * Py_UNUSED(ignored))
    { return checkYourSelf(self)
        ? PyLong_FromLong(huGetNumTokens(self->trovePtr))
        : NULL; }

static PyObject * Trove_getToken(TroveObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int idx = -1;
    if (!PyArg_ParseTuple(args, "(i)", & idx))
        { return NULL; }

    huToken const * token = huGetToken(self->trovePtr, idx);
    if (token == NULL)
        { return NULL; }

    PyObject * capsule = PyCapsule_New((void *) token, NULL, NULL);
    if (capsule == NULL)
        { return NULL; }

    PyObject * tokenObj = PyObject_CallObject((PyObject *) & TokenType, NULL);
    if (tokenObj == NULL)
        { return NULL; }
    
    return tokenObj;
}

static PyMemberDef Trove_members[] = 
{
    { NULL }
};

static PyMethodDef Trove_methods[] = 
{
    { "getNumTokens", (PyCFunction) Trove_getNumTokens, METH_NOARGS, "Return the number of tokens in a trove." },
    { "getToken", (PyCFunction) Trove_getToken, METH_VARARGS, "Return a token from a trove by index." },
    { NULL }
};

PyTypeObject TroveType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "humon.Trove",
    .tp_doc = "Encodes a Humon data trove.",
    .tp_basicsize = sizeof(TroveObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Trove_new,
    .tp_init = (initproc) Trove_init,
    .tp_dealloc = (destructor) Trove_dealloc,
    .tp_members = Trove_members,
    .tp_methods = Trove_methods
};


int RegisterTroveType(PyObject * module)
{
    if (PyType_Ready(& TroveType) < 0)
        { return -1; }
        
    Py_INCREF(& TroveType);
    if (PyModule_AddObject(module, "Trove", (PyObject *) & TroveType) < 0)
    {
        Py_DECREF(& TroveType);
        Py_DECREF(module);
        return -1;
    }

    return 0;
}
