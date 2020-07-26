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

static PyObject * Trove_get_numTokens(TroveObject * self, PyObject * Py_UNUSED(ignored))
    { return checkYourSelf(self)
        ? PyLong_FromLong(huGetNumTokens(self->trovePtr))
        : NULL; }

static PyObject * Trove_get_numNodes(TroveObject * self, PyObject * Py_UNUSED(ignored))
    { return checkYourSelf(self)
        ? PyLong_FromLong(huGetNumNodes(self->trovePtr))
        : NULL; }


static PyObject * Trove_get_rootNode(TroveObject * self, PyObject * Py_UNUSED(ignored))
{
    if (! checkYourSelf(self))
        { return NULL; }

    huNode const * node = huGetRootNode(self->trovePtr);
    if (node == NULL)
        { return NULL; }

    PyObject * nodeObj = makeNode(self, node);
    if (nodeObj == NULL)
        { return NULL; }
    
    return nodeObj;
}


static PyObject * Trove_getToken(TroveObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int idx = -1;
    if (!PyArg_ParseTuple(args, "i", & idx))
        { return NULL; }

    huToken const * token = huGetToken(self->trovePtr, idx);
    if (token == NULL)
        { return NULL; }

    PyObject * tokenObj = makeToken(token);
    if (tokenObj == NULL)
        { return NULL; }
    
    Py_DECREF(args);

    return tokenObj;
}


static PyObject * Trove_getNodeByIndex(TroveObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int idx = -1;
    if (!PyArg_ParseTuple(args, "i", & idx))
        { return NULL; }

    huNode const * node = huGetNodeByIndex(self->trovePtr, idx);
    if (node == NULL)
        { return NULL; }

    PyObject * nodeObj = makeNode(self, node);
    if (nodeObj == NULL)
        { return NULL; }
    
    return nodeObj;
}

static PyMemberDef Trove_members[] = 
{
    { NULL }
};

static PyGetSetDef Trove_getsetters[] = 
{
    { "numTokens",              (getter) Trove_get_numTokens, (setter) NULL, "The trove tracking this node." },
    { "numNodes",               (getter) Trove_get_numNodes, (setter) NULL, "The index of this node in its trove's tracking array." },
    { "rootNode",               (getter) Trove_get_rootNode, (setter) NULL, "The kind of node this is." },
    { "numErrors",              (getter) Trove_get_numErrors, (setter) NULL, "The number of errors encountered when loading a trove." },
    { "numTroveAnnotations",    (getter) Trove_get_numTroveAnnotations, (setter) NULL, "The number of annotations associated to a trove." },
    { "numTroveComments",       (getter) Trove_get_numTroveComments, (setter) NULL, "The number of comments associated to a trove." },
    { "tokenStream",            (getter) Trove_get_tokenStream, (setter) NULL, "The entire text of a trove, including all nodes and all comments and annotations." },
    { NULL }
};

static PyMethodDef Trove_methods[] = 
{
    { "getToken", (PyCFunction) Trove_getToken, METH_VARARGS, "Return a token from a trove by index." },
    { "getNodeByIndex", (PyCFunction) Trove_getNodeByIndex, METH_VARARGS, "Return a token from a trove by index." },
    { "getNodeByAddress", (PyCFunction) Trove_getNodeByAddress, METH_VARARGS, "Return a token from a trove by index." },
    { "getError", (PyCFunction) Trove_getError, METH_VARARGS, "Return an error from a trove by index." },
    { "getTroveAnnotations", (PyCFunction) Trove_getTroveAnnotations, METH_VARARGS | METH_KEYWORDS, "Return aannotations from a trove." },
    { "getTroveComment", (PyCFunction) Trove_getTroveComment, METH_VARARGS | METH_KEYWORDS, "Return a comment from a trove by index." },
    { "findNodesWithAnnotations", (PyCFunction) Trove_findNodesWithAnnotations, METH_VARARGS | METH_KEYWORDS, "Returns a list of all nodes in a trove with a specific annotation key, value, or both." },
    { "findNodesByCommentContaining", (PyCFunction) Trove_findNodesByCommentContaining, METH_VARARGS, "Returns a list of all nodes in a trove with an associated comment containing the given text." },
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
    .tp_getset = Trove_getsetters,
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
