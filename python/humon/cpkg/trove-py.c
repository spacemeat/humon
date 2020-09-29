#include "common.h"


typedef struct
{
    PyObject_HEAD
    huTrove const * trovePtr;
} TroveObject;


static void Trove_dealloc(TroveObject * self)
{
#ifdef CAVEPERSON
    printf("%sTrove dealloc%s\n", ansi_darkBlue, ansi_off);
#endif
    huDestroyTrove(self->trovePtr);
    Py_TYPE(self)->tp_free((PyObject *) self);
}


static PyObject * Trove_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
#ifdef CAVEPERSON
    printf("%sTrove new%s\n", ansi_darkBlue, ansi_off);
#endif

    TroveObject * self = (TroveObject *) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->trovePtr = NULL;
    }

    return (PyObject *) self;
}


static int Trove_init(TroveObject * self, PyObject * args, PyObject * kwds)
{
#ifdef CAVEPERSON
    printf("%sTrove init%s\n", ansi_darkMagenta, ansi_off);
#endif

    PyObject * capsule = NULL;
    if (! PyArg_ParseTuple(args, "O", & capsule))
        { return -1; }

    if (capsule == NULL)
        { return -1; }
    
    Py_INCREF(capsule);
    
    if (PyCapsule_CheckExact(capsule))
    {
        // trove will never be NULL
        huTrove const * trove = PyCapsule_GetPointer(capsule, NULL);
        self->trovePtr = trove;
    }
    else if (capsule == Py_None)
    {
        // so pass None to get a nullish token.
        self->trovePtr = NULL;
    }
    else
        { PyErr_SetString(PyExc_TypeError, "Argument 1 must be an encapsulated pointer."); }

    Py_DECREF(capsule);

    return PyErr_Occurred() ? -1 : 0;
}


static bool checkYourSelf(TroveObject * self)
{
    if (self->trovePtr == NULL)
    {
        PyErr_SetString(PyExc_ValueError, "accessing nullish trove");
        return false;
    }
    else if (huGetNumErrors(self->trovePtr) > 0)
    {
        PyErr_SetString(PyExc_ValueError, "accessing erroneous trove");
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


static PyObject * Trove_fromString(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    static char * keywords[] = { "tokenstream", "tabsize", NULL };

    char const * string = NULL;
    Py_ssize_t stringLen = 0;
    huCol_t tabSize = 4;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#|i", keywords,
        & string, & stringLen, & tabSize))
        { return NULL; }
    
    if (string == NULL)
    {
        PyErr_SetString(PyExc_ValueError, "argument must be a string");
        return NULL;
    }

    huTrove const * trove = NULL;
    huDeserializeOptions params;
    huInitDeserializeOptions(& params, HU_ENCODING_UTF8, false, tabSize, NULL);
    huEnumType_t error = huDeserializeTroveN(& trove, string, stringLen, & params, HU_ERRORRESPONSE_STDERRANSICOLOR);
    switch(error)
    {
    case HU_ERROR_NOERROR:
    case HU_ERROR_TROVEHASERRORS:   // Return a valid trove with errors
        // TODO: init with trove
        return makeTrove(trove);
    case HU_ERROR_OUTOFMEMORY:
        PyErr_NoMemory();
        return NULL;
        // TODO: More special exceptional cases.
    default:
        PyErr_SetString(PyExc_TypeError, "Error loading trove");
        return NULL;
    }
}


static PyObject * Trove_fromFile(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    static char * keywords[] = { "file", "tabsize", NULL };

    char const * string = NULL;
    Py_ssize_t stringLen = 0;
    huCol_t tabSize = 4;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s#|i", keywords,
        & string, & stringLen, & tabSize))
        { return NULL; }
    
    if (string == NULL)
    {
        PyErr_SetString(PyExc_ValueError, "argument must be a string");
        return NULL;
    }

    huTrove const * trove = NULL;
    huDeserializeOptions params;
    huInitDeserializeOptions(& params, HU_ENCODING_UTF8, false, tabSize, NULL);
    huEnumType_t error = huDeserializeTroveFromFile(& trove, string, & params, HU_ERRORRESPONSE_STDERRANSICOLOR);
    switch(error)
    {
    case HU_ERROR_NOERROR:
    case HU_ERROR_TROVEHASERRORS:   // Return a valid trove with errors
        // TODO: init with trove
        return makeTrove(trove);
    case HU_ERROR_OUTOFMEMORY:
        PyErr_NoMemory();
        return NULL;
        // TODO: More special exceptional cases.
    default:
        PyErr_SetString(PyExc_TypeError, "Error loading trove");
        return NULL;
    }
}


static PyObject * Trove_getToken(TroveObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int idx = -1;
    if (!PyArg_ParseTuple(args, "i", & idx))
        { return NULL; }

    huToken const * token = huGetToken(self->trovePtr, (huSize_t) idx);
    PyObject * tokenObj = makeToken(token);
    if (tokenObj == NULL)
        { return NULL; }
    
    return tokenObj;
}


static PyObject * Trove_getNodeByIndex(TroveObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int idx = -1;
    if (!PyArg_ParseTuple(args, "i", & idx))
        { return NULL; }

    huNode const * node = huGetNodeByIndex(self->trovePtr, (huSize_t) idx);
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
    huSize_t addressLen = 0;
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

    huError const * err = huGetError(self->trovePtr, (huSize_t) idx);
    return Py_BuildValue("(OO)", getEnumValue("ErrorCode", err->errorCode), makeToken(err->token));
}


static PyObject * Trove_getTroveAnnotations(TroveObject * self, PyObject * args, PyObject * kwargs)
{
    if (! checkYourSelf(self))
        { return NULL; }

    static char * keywords[] = { "key", "value", NULL };

    char * key = NULL;
    char * value = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ss", keywords, & key, & value))
        { return NULL; }

    if (key && ! value)
    {
        // look up by key, return one token
        huToken const * tok = huGetTroveAnnotationWithKeyZ(self->trovePtr, key);
        return makeToken(tok);
    }
    else if (! key && value)
    {
        // look up by value, return n tokens
        // TODO: The returned list seems to be leaking
        PyObject * list = Py_BuildValue("[]");
        if (list == NULL)
            { return NULL; }

        huSize_t cursor = 0;
        huToken const * tok = NULL;
        do
        {
            tok = huGetTroveAnnotationWithValueZ(self->trovePtr, value, & cursor);
            if (tok)
            {
                if (PyList_Append(list, makeToken(tok)) < 0)
                {
                    Py_DECREF(tok);
                    break;
                }
            }
        } while (tok != NULL);

        if (PyErr_Occurred())
        {
            Py_DECREF(list);
            return NULL;
        }

        return list;
    }
    else if (key && value)
    {
        // look up by key, return whether key->value (bool)
        huToken const * tok = huGetTroveAnnotationWithKeyZ(self->trovePtr, key);
        return PyBool_FromLong(strncmp(tok->str.ptr, value, min(tok->str.size, strlen(value))) == 0);
    }

    return NULL;
}

/* This version would be faster than the above, but Python3.7.5 seems to have 
   trouble with s#. Or I'm doing somehting wrong.

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
        huToken const * tok = huGetTroveAnnotationWithKeyN(self->trovePtr, key, (huSize_t) keyLen);
        return makeToken(tok);
    }
    else if (! key && value)
    {
        // look up by value, return n tokens
        // TODO: The returned list seems to be leaking
        PyObject * list = Py_BuildValue("[]");
        if (list == NULL)
            { return NULL; }

        huSize_t cursor = 0;
        huToken const * tok = NULL;
        do
        {
            tok = huGetTroveAnnotationWithValueN(self->trovePtr, value, (huSize_t) valueLen, & cursor);
            if (tok)
            {
                if (PyList_Append(list, makeToken(tok)) < 0)
                {
                    Py_DECREF(tok);
                    break;
                }
            }
        } while (tok != NULL);

        if (PyErr_Occurred())
        {
            Py_DECREF(list);
            return NULL;
        }

        return list;
    }
    else if (key && value)
    {
        // look up by key, return whether key->value (bool)
        huToken const * tok = huGetTroveAnnotationWithKeyN(self->trovePtr, key, (huSize_t) keyLen);
        return PyBool_FromLong(strncmp(tok->str.ptr, value, min(tok->str.size, (huSize_t) valueLen)) == 0);
    }

    return NULL;
}
*/

static PyObject * Trove_getTroveComment(TroveObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int idx = -1;

    if (! PyArg_ParseTuple(args, "i", & idx))
        { return NULL; }

    if (idx < 0)
        { return NULL; }

    return makeToken(huGetTroveComment(self->trovePtr, (huSize_t) idx));
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
    if (list == NULL)
        { return NULL; }

    huSize_t cursor = 0;
    huNode const * node = NULL;

    if (key && ! value)
    {
        // look up by key
        do
        {
            node = huFindNodesWithAnnotationKeyN(self->trovePtr, key, (huSize_t) keyLen, & cursor);
            if (node)
            {
                if (PyList_Append(list, makeNode((PyObject *) self, node)) < 0)
                {
                    Py_DECREF(node);
                    break;
                }
            }
        } while (node);
    }
    else if (! key && value)
    {
        // look up by value
        do
        {
            node = huFindNodesWithAnnotationValueN(self->trovePtr, value, (huSize_t) valueLen, & cursor);
            if (node)
            {
                if (PyList_Append(list, makeNode((PyObject *) self, node)) < 0)
                {
                    Py_DECREF(node);
                    break;
                }
            }
        } while (node != NULL);
    }
    else if (key && value)
    {
        // look up by key and value
        do
        {
            node = huFindNodesWithAnnotationKeyValueNN(self->trovePtr, key, (huSize_t) keyLen, value, (huSize_t) valueLen, & cursor);
            if (node)
            {
                if (PyList_Append(list, makeNode((PyObject *) self, node)) < 0)
                {
                    Py_DECREF(node);
                    break;
                }
            }
        } while (node);
    }

    if (PyErr_Occurred())
    {
        Py_DECREF(list);
        return NULL;
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
    if (list == NULL)
        { return NULL; }

    huSize_t cursor = 0;
    huNode const * node = NULL;

    do
    {
        node = huFindNodesByCommentContainingN(self->trovePtr, text, (huSize_t) textLen, & cursor);
        if (node)
        {
            if (PyList_Append(list, makeNode((PyObject *) self, node)) < 0)
            {
                Py_DECREF(node);
                break;
            }
        }
    } while (node);

    if (PyErr_Occurred())
    {
        Py_DECREF(list);
        return NULL;
    }

    return list;
}


static PyObject * Trove_richCompare(PyObject * self, PyObject * other, int op)
{
    PyObject * result = NULL;

    if (other == NULL)
        { result = Py_NotImplemented; }
    else if (op == Py_EQ)
    {
        if (((TroveObject *) self)->trovePtr == ((TroveObject *) other)->trovePtr)
            { result = Py_True; }
        else
            { result = Py_False; }
    }
    else if (op == Py_NE)
    {
        if (((TroveObject *) self)->trovePtr != ((TroveObject *) other)->trovePtr)
            { result = Py_True; }
        else
            { result = Py_False; }
    }
    else
        { result = Py_NotImplemented; }
    
    Py_INCREF(result);
    return result;
}


static int Trove_bool(TroveObject * self)
{
    return self->trovePtr != NULL &&
           huGetNumErrors(self->trovePtr) == 0;
}


static PyMemberDef Trove_members[] = 
{
    { NULL }
};


static PyGetSetDef Trove_getsetters[] = 
{
    { "numTokens",              (getter) Trove_get_numTokens, (setter) NULL, PyDoc_STR("The trove tracking this node."), NULL },
    { "numNodes",               (getter) Trove_get_numNodes, (setter) NULL, PyDoc_STR("The index of this node in its trove's tracking array."), NULL },
    { "root",                   (getter) Trove_get_root, (setter) NULL, PyDoc_STR("The kind of node this is."), NULL },
    { "numErrors",              (getter) Trove_get_numErrors, (setter) NULL, PyDoc_STR("The number of errors encountered when loading a trove."), NULL },
    { "numTroveAnnotations",    (getter) Trove_get_numTroveAnnotations, (setter) NULL, PyDoc_STR("The number of annotations associated to a trove."), NULL },
    { "numTroveComments",       (getter) Trove_get_numTroveComments, (setter) NULL, PyDoc_STR("The number of comments associated to a trove."), NULL },
    { "tokenStream",            (getter) Trove_get_tokenStream, (setter) NULL, PyDoc_STR("The entire text of a trove, including all nodes and all comments and annotations."), NULL },
    { NULL }
};


static PyMethodDef Trove_methods[] = 
{
    { "fromString", (PyCFunction) Trove_fromString, METH_VARARGS | METH_KEYWORDS | METH_CLASS, PyDoc_STR("Create a Trove from a string.") },
    { "fromFile", (PyCFunction) Trove_fromFile, METH_VARARGS | METH_KEYWORDS | METH_CLASS, PyDoc_STR("Create a Trove from a file.") },
    { "getToken", (PyCFunction) Trove_getToken, METH_VARARGS, PyDoc_STR("Return a token from a trove by index.") },
    { "getNodeByIndex", (PyCFunction) Trove_getNodeByIndex, METH_VARARGS, PyDoc_STR("Return a token from a trove by index.") },
    { "getNodeByAddress", (PyCFunction) Trove_getNodeByAddress, METH_VARARGS, PyDoc_STR("Return a token from a trove by index.") },
    { "getError", (PyCFunction) Trove_getError, METH_VARARGS, PyDoc_STR("Return an error from a trove by index.") },
    { "getTroveAnnotations", (PyCFunction) Trove_getTroveAnnotations, METH_VARARGS | METH_KEYWORDS, PyDoc_STR("Return aannotations from a trove.") },
    { "getTroveComment", (PyCFunction) Trove_getTroveComment, METH_VARARGS | METH_KEYWORDS, PyDoc_STR("Return a comment from a trove by index.") },
    { "findNodesWithAnnotations", (PyCFunction) Trove_findNodesWithAnnotations, METH_VARARGS | METH_KEYWORDS, PyDoc_STR("Returns a list of all nodes in a trove with a specific annotation key, value, or both.") },
    { "findNodesByCommentContaining", (PyCFunction) Trove_findNodesByCommentContaining, METH_VARARGS, PyDoc_STR("Returns a list of all nodes in a trove with an associated comment containing the given text.") },
    { NULL }
};


static PyNumberMethods numberMethods = 
{
    .nb_bool = (inquiry) Trove_bool
};


PyTypeObject TroveType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "humon.humon.Trove",
    .tp_doc = PyDoc_STR("Encodes a Humon data trove."),
    .tp_basicsize = sizeof(TroveObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Trove_new,
    .tp_init = (initproc) Trove_init,
    .tp_dealloc = (destructor) Trove_dealloc,
    .tp_members = Trove_members,
    .tp_getset = Trove_getsetters,
    .tp_methods = Trove_methods,
    .tp_richcompare = (richcmpfunc) Trove_richCompare,
    .tp_as_number = & numberMethods
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


PyObject * makeTrove(huTrove const * trovePtr)
{
    Py_INCREF(& NodeType);
    PyObject * capsule = NULL;
    if (trovePtr == NULL)
    {
        capsule = Py_None;
        Py_INCREF(capsule);
    }
    else
        { capsule = PyCapsule_New((void *) trovePtr, NULL, NULL); }

    PyObject * troveObj = NULL;
    if (capsule != NULL)
    {
        troveObj = PyObject_CallFunction((PyObject *) & TroveType, "(O)", capsule);
        if (troveObj == NULL && ! PyErr_Occurred())
            { PyErr_SetString(PyExc_RuntimeError, "Could not create Trove"); }

        Py_DECREF(capsule);
    }

    Py_INCREF(& TroveType);

    return troveObj;
}
