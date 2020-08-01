#include "common.h"


typedef struct
{
    PyObject_HEAD
    PyObject * trove;
    huNode const * nodePtr;
} NodeObject;


static void Node_dealloc(NodeObject * self)
{
    printf("Node dealloc\n");
    Py_DECREF(self->trove);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * Node_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    printf("Node new\n");
    NodeObject * self = (NodeObject *) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->trove = NULL;
        self->nodePtr = NULL;
    }

    return (PyObject *) self;
}

static int Node_init(NodeObject * self, PyObject * args, PyObject * kwds)
{
    printf("Node init\n");
    PyObject * trove = NULL;
    PyObject * capsule = NULL;
    if (! PyArg_ParseTuple(args, "OO", & trove, & capsule))
        { return -1; }

    if (trove == NULL || capsule == NULL)
        { return -1; }

    PyTypeObject * type = Py_TYPE(trove);
    if (type != & TroveType)
    {
        PyErr_SetString(PyExc_ValueError, "Argument 1 must be a humon.Trove.");
        return -1;
    }
    
    if (! PyCapsule_CheckExact(capsule))
    {
        PyErr_SetString(PyExc_ValueError, "Argument 2 must be an encapsulated pointer.");
        return -1;
    }

    PyObject * oldTrove = self->trove;
    Py_INCREF(trove);
    self->trove = trove;
    if (oldTrove != NULL)
    {
        Py_DECREF(oldTrove);
    }

    // node will never be NULL
    huNode const * node = PyCapsule_GetPointer(capsule, NULL);
    self->nodePtr = node;

    Py_DECREF(capsule);

    return 0;
}


static bool checkYourSelf(NodeObject * self)
{
    if (self->nodePtr == NULL)
    {
        PyErr_SetString(PyExc_ValueError, "accessing nullish node");
        return false;
    }

    return true;
}


static PyObject * IncRefOf(PyObject * obj)
{
    Py_INCREF(obj);
    return obj;
}


static PyObject * Node_get_trove(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? IncRefOf(self->trove)
        : NULL; }

static PyObject * Node_get_nodeIdx(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(self->nodePtr->nodeIdx)
        : NULL; }

static PyObject * Node_get_kind(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? getEnumValue("NodeKind", self->nodePtr->kind)
        : NULL; }

static PyObject * Node_get_firstToken(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? makeToken(self->nodePtr->firstToken)
        : NULL; }

static PyObject * Node_get_keyToken(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? makeToken(self->nodePtr->keyToken)
        : NULL; }

static PyObject * Node_get_valueToken(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? makeToken(self->nodePtr->valueToken)
        : NULL; }

static PyObject * Node_get_lastValueToken(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? makeToken(self->nodePtr->lastValueToken)
        : NULL; }

static PyObject * Node_get_lastToken(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? makeToken(self->nodePtr->lastToken)
        : NULL; }

static PyObject * Node_get_parentNodeIdx(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(self->nodePtr->parentNodeIdx)
        : NULL; }

static PyObject * Node_get_parent(NodeObject * self, PyObject * Py_UNUSED(ignored))
    { return checkYourSelf(self)
        ? makeNode((PyObject *) self->trove, huGetParent(self->nodePtr))
        : NULL; }

static PyObject * Node_get_childOrdinal(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(self->nodePtr->childOrdinal)
        : NULL; }

static PyObject * Node_get_numChildren(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(huGetNumChildren(self->nodePtr))
        : NULL; }

static PyObject * Node_get_firstChild(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? makeNode((PyObject *) self->trove, huGetFirstChild(self->nodePtr))
        : NULL; }

static PyObject * Node_get_nextSibling(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? makeNode((PyObject *) self->trove, huGetNextSibling(self->nodePtr))
        : NULL; }


static PyObject * Node_get_address(NodeObject * self, void * closure)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int strLen = 0;
    char * str = NULL;
    huGetAddress(self->nodePtr, str, & strLen);
    str = malloc(strLen + 1);
    if (str == NULL)
        { return NULL; }
    huGetAddress(self->nodePtr, str, & strLen);
    str[strLen] = '\0';
//    PyObject * pystr = PyUnicode_FromStringAndSize(str, strLen);
    PyObject * pystr = Py_BuildValue("s#", str, strLen);
    free(str);

    return pystr;
}


static PyObject * Node_get_hasKey(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyBool_FromLong(huHasKey(self->nodePtr))
        : NULL; }


static PyObject * Node_get_tokenStream(NodeObject * self, void * closure)
{
    if (! checkYourSelf(self))
        { return NULL; }

    huStringView str = huGetTokenStream(self->nodePtr);
    return Py_BuildValue("s#", str.ptr, str.size);
}


static PyObject * Node_get_numAnnotations(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(huGetNumAnnotations(self->nodePtr))
        : NULL; }

static PyObject * Node_get_numComments(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(huGetNumComments(self->nodePtr))
        : NULL; }


static PyObject * Node_getChild(NodeObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    PyObject * arg = NULL;
    if (!PyArg_ParseTuple(args, "O", & arg))
        { return NULL; }
    
    huNode const * node = NULL;

    // Theoretically, pass int or string to getChild().
    if (PyLong_Check(arg))
    {
        int idx = PyLong_AsLong(arg);
        if (idx >= 0)
            { node = huGetChildByIndex(self->nodePtr, idx); }
    }
    else if (PyUnicode_Check(arg))
    {
        char const * buf = NULL;
        Py_ssize_t bufLen = 0;
        buf = PyUnicode_AsUTF8AndSize(arg, & bufLen);
        if (buf)
        {
            if (bufLen > 0)
                { node = huGetChildByKeyN(self->nodePtr, buf, bufLen); }
        }
    }

    PyObject * nodeObj = makeNode((PyObject *) self->trove, node);

    Py_DECREF(arg);
    
    return nodeObj;
}


static PyObject * Node_getRelative(NodeObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    char const * arg = NULL;
    int argLen = 0;
    if (!PyArg_ParseTuple(args, "s#", & arg, & argLen))
        { return NULL; }
    
    huNode const * node = huGetRelativeN(self->nodePtr, arg, argLen);
    PyObject * nodeObj = makeNode((PyObject *) self->trove, node);
    if (nodeObj == NULL)
        { return NULL; }
    
    return nodeObj;
}


static PyObject * Node_getAnnotations(NodeObject * self, PyObject * args, PyObject * kwargs)
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
        huToken const * tok = huGetAnnotationWithKeyN(self->nodePtr, key, keyLen);
        return makeToken(tok);
    }
    else if (! key && value)
    {
        // look up by value, return n tokens
        PyObject * list = Py_BuildValue("[]");
        int cursor = 0;
        huToken const * tok = NULL;
        do
        {
            tok = huGetAnnotationWithValueN(self->nodePtr, value, valueLen, & cursor);
            if (tok)
            {
                PyObject * pytok = makeToken(tok);
                PyList_Append(list, pytok);
                Py_DECREF(pytok);
            }
        } while (tok != NULL);

        return list;        
    }
    else if (key && value)
    {
        // look up by key, return whether key->value (bool)
        huToken const * tok = huGetAnnotationWithKeyN(self->nodePtr, key, keyLen);
        return PyBool_FromLong(strncmp(tok->str.ptr, value, min(tok->str.size, valueLen)) == 0);
    }

    return NULL;
}


static PyObject * Node_getComment(NodeObject * self, PyObject * args)
{
    if (! checkYourSelf(self))
        { return NULL; }

    int idx = -1;

    if (! PyArg_ParseTuple(args, "i", & idx))
        { return NULL; }

    if (idx < 0)
    {
        PyErr_SetString(PyExc_ValueError, "index argument must be >= 0");
        return NULL;
    }

    return makeToken(huGetComment(self->nodePtr, idx));
}


static PyObject * Node_getCommentsContaining(NodeObject * self, PyObject * args, PyObject * kwargs)
{
    if (! checkYourSelf(self))
        { return NULL; }

    char * str = NULL;
    int strLen = 0;
    if (! PyArg_ParseTuple(args, "s#", & str, & strLen))
        { return NULL; }

    PyObject * list = Py_BuildValue("[]");
    int cursor = 0;
    huToken const * tok = NULL;
    do
    {
        tok = huGetCommentsContainingN(self->nodePtr, str, strLen, & cursor);
        if (tok)
        {
            PyObject * pytok = makeToken(tok);
            PyList_Append(list, pytok);
            Py_DECREF(pytok);
        }
    } while (tok != NULL);

    return list;
}


static PyObject * Node_repr(NodeObject * self)
{
    if (! checkYourSelf(self))
        { return NULL; }

    return Py_BuildValue("s",
        huNodeKindToString(self->nodePtr->kind));

//    return PyUnicode_FromFormat("Node: kind: %s",
//        huNodeKindToString(self->nodePtr->kind));
}


static PyObject * Node_str(NodeObject * self)
{
    if (! checkYourSelf(self))
        { return NULL; }

    if (self->nodePtr->kind == HU_NODEKIND_VALUE)
    {
        return Py_BuildValue("s#",
            self->nodePtr->valueToken->str.ptr, 
            self->nodePtr->valueToken->str.size);
//        return PyUnicode_FromStringAndSize(
//            self->nodePtr->valueToken->str.ptr, 
//            self->nodePtr->valueToken->str.size);
    }
    else
    {
        PyObject * str = Py_BuildValue("s", "no value here");
        return str;
//        return PyUnicode_New(0, 127);
//        PyUnicode_FromStringAndSize("", 0);
    }
}


static PyMemberDef Node_members[] = 
{
    { NULL }
};

static PyGetSetDef Node_getsetters[] = 
{
    { "trove",          (getter) Node_get_trove, (setter) NULL, "The trove tracking this node." },
    { "nodeIdx",        (getter) Node_get_nodeIdx, (setter) NULL, "The index of this node in its trove's tracking array." },
    { "kind",           (getter) Node_get_kind, (setter) NULL, "The kind of node this is." },
    { "firstToken",     (getter) Node_get_firstToken, (setter) NULL, "The first token which contributes to this node, including any annotation and comment tokens." },
    { "keyToken",       (getter) Node_get_keyToken, (setter) NULL, "The key token if the node is inside a dict." },
    { "valueToken",     (getter) Node_get_valueToken, (setter) NULL, "The first token of this node's actual value; for a container, it points to the opening brac(e|ket)." },
    { "lastValueToken", (getter) Node_get_lastValueToken, (setter) NULL, "The last token of this node's actual value; for a container, it points to the closing brac(e|ket)." },
    { "lastToken",      (getter) Node_get_lastToken, (setter) NULL, "The last token of this node, including any annotation and comment tokens." },
    { "parentNodeIdx",  (getter) Node_get_parentNodeIdx, (setter) NULL, "The parent node's index, or -1 if this node is the root." },
    { "parent",         (getter) Node_get_parent, (setter) NULL, "The node's parent." },
    { "childOrdinal",   (getter) Node_get_childOrdinal, (setter) NULL, "The index of this node vis a vis its sibling nodes (starting at 0)." },
    { "numChildren",    (getter) Node_get_numChildren, (setter) NULL, "The number of children a node has." },
    { "firstChild",     (getter) Node_get_firstChild, (setter) NULL, "The first child of node (index 0)." },
    { "nextSibling",    (getter) Node_get_nextSibling, (setter) NULL, "The next sibling in the child index order of a node." },
    { "address",        (getter) Node_get_address, (setter) NULL, "The full address of a node." },
    { "hasKey",         (getter) Node_get_hasKey, (setter) NULL, "Returns whether a node has a key token tracked. (If it's a member of a dict.)" },
    { "nestedValue",    (getter) Node_get_tokenStream, (setter) NULL, "The entire nested text of a node, including child nodes and associated comments and annotations." },
    { "numAnnotations", (getter) Node_get_numAnnotations, (setter) NULL, "Return the number of annotations associated to a node." },
    { "numComments",    (getter) Node_get_numComments, (setter) NULL, "Return the number of comments associated to a node." },
    { NULL }
};

static PyMethodDef Node_methods[] = 
{
    { "getChild",               (PyCFunction) Node_getChild,                METH_VARARGS | METH_KEYWORDS,   "Get a node's parent." },
    { "getRelative",            (PyCFunction) Node_getRelative,             METH_VARARGS,                   "Get a node by relative address." },
    { "getAnnotations",         (PyCFunction) Node_getAnnotations,          METH_VARARGS | METH_KEYWORDS,   "Get a node's annotations." },
    { "getComment",             (PyCFunction) Node_getComment,              METH_VARARGS,                   "Get a node's comment by index." },
    { "getCommentsContaining",  (PyCFunction) Node_getCommentsContaining,   METH_VARARGS,                   "Get a node's comments which contain the specified substring." },
    { NULL }
};

// TODO: Should this be static and wrapped up in one file?
PyTypeObject NodeType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "humon.humon.Node",
    .tp_doc = "Encodes a Humon data node.",
    .tp_basicsize = sizeof(NodeObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Node_new,
    .tp_init = (initproc) Node_init,
    .tp_dealloc = (destructor) Node_dealloc,
    .tp_members = Node_members,
    .tp_getset = Node_getsetters,
    .tp_methods = Node_methods,
    .tp_repr = (reprfunc) Node_repr,
    .tp_str = (reprfunc) Node_str
};


int RegisterNodeType(PyObject * module)
{
    if (PyType_Ready(& NodeType) < 0)
        { return -1; }
        
    Py_INCREF(& NodeType);
    if (PyModule_AddObject(module, "Node", (PyObject *) & NodeType) < 0)
    {
        Py_DECREF(& NodeType);
        return -1;
    }

    return 0;
}


PyObject * makeNode(PyObject * trove, huNode const * nodePtr)
{
    PyObject * nodeObj = NULL;

    if (nodePtr != NULL)
    {        
        PyObject * capsule = PyCapsule_New((void *) nodePtr, NULL, NULL);
        if (capsule != NULL)
        {
            nodeObj = PyObject_CallFunction((PyObject *) & NodeType, "(OO)", trove, capsule);
            if (nodeObj == NULL && ! PyErr_Occurred())
                { PyErr_SetString(PyExc_RuntimeError, "Could not create Node"); }

            Py_DECREF(capsule);
        }
        else
            { PyErr_SetString(PyExc_RuntimeError, "Could not make new capsule"); }
    }
    else
        { PyErr_SetString(PyExc_ValueError, "Attempt to make a null Token"); }

    return nodeObj;
}
