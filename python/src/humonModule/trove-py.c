#include "common.h"


typedef struct
{
    PyObject_HEAD
    huTrove const * trovePtr;
} TroveObject;


static void Trove_dealloc(TroveObject * self)
{
    printf("Trove dealloc\n");
    huDestroyTrove(self->trovePtr);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * Trove_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    printf("Trove new\n");
    TroveObject * self = (TroveObject *) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->trovePtr = NULL;
    }

    return (PyObject *) self;
}


static int Trove_init(TroveObject * self, PyObject * args, PyObject * kwds)
{
    printf("Trove init\n");

    static char * keywords[] = {"tokenstream", "tabsize"};

    char const * string = NULL;
    Py_ssize_t stringLen = 0;
    int tabSize = 4;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#|i", keywords,
        & string, & stringLen, & tabSize))
        { return -1; }
    
    if (string == NULL)
    {
        PyErr_SetString(PyExc_ValueError, "argument must be a string");
        return -1;
    }

    huDeserializeOptions params;    // TODO: Use my decoding?
    huInitDeserializeOptions(& params, HU_ENCODING_UTF8, true, tabSize);
    int error = huDeserializeTroveN(& self->trovePtr, string, stringLen, & params, HU_ERRORRESPONSE_MUM);
    switch(error)
    {
    case HU_ERROR_NOERROR:
        return 0;
    case HU_ERROR_OUTOFMEMORY:
        PyErr_NoMemory();
        return -1;
    case HU_ERROR_TROVEHASERRORS:
        PyErr_SetString(PyExc_ValueError, "Trove has errors");
        return -1;
        // TODO: More special exceptional cases.
    default:
        PyErr_SetString(PyExc_TypeError, "Trove has errors");
        return -1;
    }
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


static PyObject * Trove_get_root(TroveObject * self, PyObject * Py_UNUSED(ignored))
{
    if (! checkYourSelf(self))
        { return NULL; }

    huNode const * node = huGetRootNode(self->trovePtr);
    if (node == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Could not get root node");
        return NULL;
    }

    PyObject * nodeObj = makeNode((PyObject *) self, node);
    if (nodeObj == NULL)
        { return NULL; }
    
    return nodeObj;
}


static PyObject * Trove_get_numErrors(TroveObject * self, PyObject * Py_UNUSED(ignored))
    { return checkYourSelf(self)
        ? PyLong_FromLong(huGetNumErrors(self->trovePtr))
        : NULL; }

static PyObject * Trove_get_numTroveAnnotations(TroveObject * self, PyObject * Py_UNUSED(ignored))
    { return checkYourSelf(self)
        ? PyLong_FromLong(huGetNumTroveAnnotations(self->trovePtr))
        : NULL; }

static PyObject * Trove_get_numTroveComments(TroveObject * self, PyObject * Py_UNUSED(ignored))
    { return checkYourSelf(self)
        ? PyLong_FromLong(huGetNumTroveComments(self->trovePtr))
        : NULL; }


static PyObject * Trove_get_tokenStream(TroveObject * self, PyObject * Py_UNUSED(ignored))
{
    if (! checkYourSelf(self))
        { return NULL; }

    huStringView sv = huGetTroveTokenStream(self->trovePtr);
    return Py_BuildValue("s#", sv.ptr, sv.size);
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

    PyObject * nodeObj = makeNode((PyObject *) self, node);
    if (nodeObj == NULL)
        { return NULL; }
    
    return nodeObj;
}


static PyObject * Trove_getNodeByAddress(TroveObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    char * address = NULL;
    int addressLen = 0;
    if (!PyArg_ParseTuple(args, "s#", & address, & addressLen))
        { return NULL; }

    return makeNode((PyObject *) self, huGetNodeByAddressN(self->trovePtr, address, addressLen));
}


static PyObject * Trove_getError(TroveObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int idx = -1;
    if (!PyArg_ParseTuple(args, "i", & idx))
        { return NULL; }

    huError const * err = huGetError(self->trovePtr, idx);
    return Py_BuildValue("(OO)", getEnumValue("ErrorCode", err->errorCode), makeToken(err->token));
}


static PyObject * Trove_getTroveAnnotations(TroveObject * self, PyObject * args, PyObject * kwargs)
{
    if (! checkYourSelf(self))
        { return NULL; }

    static char * keywords[] = { "key", "value", NULL };

    char * key = NULL;
    int keyLen = 0;
    char * value = NULL;
    int valueLen = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s#s#", keywords, & key, & keyLen, & value, & valueLen))
        { return NULL; }

    if (key && ! value)
    {
        // look up by key, return one token
        huToken const * tok = huGetTroveAnnotationWithKeyN(self->trovePtr, key, keyLen);
        return makeToken(tok);
    }
    else if (! key && value)
    {
        // look up by value, return n tokens
        // TODO: The returned list seems to be leaking
        PyObject * list = Py_BuildValue("[]");
        int cursor = 0;
        huToken const * tok = NULL;
        do
        {
            tok = huGetTroveAnnotationWithValueN(self->trovePtr, value, valueLen, & cursor);
            if (tok)
            {
                if (PyList_Append(list, makeToken(tok)) < 0)
                    { return NULL; }
            }
        } while (tok != NULL);

        return list;
    }
    else if (key && value)
    {
        // look up by key, return whether key->value (bool)
        huToken const * tok = huGetTroveAnnotationWithKeyN(self->trovePtr, key, keyLen);
        return PyBool_FromLong(strncmp(tok->str.ptr, value, min(tok->str.size, valueLen)) == 0);
    }

    return NULL;
}


static PyObject * Trove_getTroveComment(TroveObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int idx = -1;

    if (! PyArg_ParseTuple(args, "i", & idx))
        { return NULL; }

    if (idx < 0)
        { return NULL; }

    return makeToken(huGetTroveComment(self->trovePtr, idx));
}


static PyObject * Trove_findNodesWithAnnotations(TroveObject * self, PyObject * args, PyObject * kwargs)
{
    if (! checkYourSelf(self))
        { return NULL; }

    static char * keywords[] = { "key", "value", NULL };

    char * key = NULL;
    int keyLen = 0;
    char * value = NULL;
    int valueLen = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s#s#", keywords, & key, & keyLen, & value, & valueLen))
        { return NULL; }

    PyObject * list = Py_BuildValue("[]");
    int cursor = 0;
    huNode const * node = NULL;

    if (key && ! value)
    {
        // look up by key
        do
        {
            node = huFindNodesWithAnnotationKeyN(self->trovePtr, key, keyLen, & cursor);
            if (node)
            {
                if (PyList_Append(list, makeNode((PyObject *) self, node)) < 0)
                    { return NULL; }
            }
        } while (node);
    }
    else if (! key && value)
    {
        // look up by value
        do
        {
            node = huFindNodesWithAnnotationValueN(self->trovePtr, value, valueLen, & cursor);
            if (node)
            {
                if (PyList_Append(list, makeNode((PyObject *) self, node)) < 0)
                    { return NULL; }
            }
        } while (node != NULL);
    }
    else if (key && value)
    {
        // look up by key and value
        do
        {
            node = huFindNodesWithAnnotationKeyValueNN(self->trovePtr, key, keyLen, value, valueLen, & cursor);
            if (node)
            {
                if (PyList_Append(list, makeNode((PyObject *) self, node)) < 0)
                    { return NULL; }
            }
        } while (node);
    }

    return list;
}


static PyObject * Trove_findNodesByCommentContaining(TroveObject * self, PyObject * args, PyObject * kwargs)
{
    if (! checkYourSelf(self))
        { return NULL; }

    char * text = NULL;
    int textLen = 0;
    if (!PyArg_ParseTuple(args, "s#", & text, & textLen))
        { return NULL; }

    PyObject * list = Py_BuildValue("[]");
    int cursor = 0;
    huNode const * node = NULL;

    do
    {
        node = huFindNodesByCommentContainingN(self->trovePtr, text, textLen, & cursor);
        if (node)
        {
            PyList_Append(list, makeNode((PyObject *) self, node));
        }
    } while (node);

    return list;
}


static PyMemberDef Trove_members[] = 
{
    { NULL }
};

static PyGetSetDef Trove_getsetters[] = 
{
    { "numTokens",              (getter) Trove_get_numTokens, (setter) NULL, "The trove tracking this node." },
    { "numNodes",               (getter) Trove_get_numNodes, (setter) NULL, "The index of this node in its trove's tracking array." },
    { "root",                   (getter) Trove_get_root, (setter) NULL, "The kind of node this is." },
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
    .tp_name = "humon.humon.Trove",
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
        return -1;
    }

    return 0;
}
