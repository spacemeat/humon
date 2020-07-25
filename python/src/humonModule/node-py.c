#include "common.h"


typedef struct
{
    PyObject_HEAD
    PyObject * trove;
    huNode const * nodePtr;
} NodeObject;


static void Node_dealloc(NodeObject * self)
{
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * Node_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    NodeObject * self;
    self = (NodeObject *) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->trove = NULL;
        self->nodePtr = NULL;
    }

    return (PyObject *) self;
}

static int Node_init(NodeObject * self, PyObject * args, PyObject * kwds)
{
    // TODO: Py_ADDREF / Py_DECREF on args, self?
    PyObject * trove = NULL;
    PyObject * capsule = NULL;
    if (! PyArg_ParseTuple(args, "OO", & trove, & capsule))
        { return -1; }
    
    // TODO: check type of arg0 to be a Trove
    PyTypeObject * type = Py_TYPE(trove);
    if (strncmp(type->tp_name, "humon.Trove", strlen("humon.Trove") != 0)
    {
        PyErr_SetString(PyExc_ValueError, "Argument 1 must be a humon.Trove.");
        return -1;
    }
    
    if (! PyCapsule_CheckExact(capsule))
    {
        PyErr_SetString(PyExc_ValueError, "Argument 2 must be an encapsulated pointer.");
        return -1;
    }

    Py_INCREF(trove);
    self->trove = trove;

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


static PyObject * Node_get_nodeIdx(NodeObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(self->nodePtr->nodeIdx)
        : NULL; }


static PyObject * Node_get_kind(NodeObject * self, void * closure)
{
    if (! checkYourSelf(self))
        { return NULL; }

    PyObject * val = getEnumValue("humon.enums", "NodeKind", self->nodePtr->kind);
    if (val == NULL)
        { return NULL; }

    return val;
}


static PyObject * Node_str(NodeObject * self, PyObject * Py_UNUSED(ignored))
{
    if (! checkYourSelf(self))
        { return NULL; }

    if (self->nodePtr->kind == HU_NODEKIND_VALUE)
    {
        return PyUnicode_FromStringAndSize(
            self->nodePtr->valueToken->str.ptr, 
            self->nodePtr->valueToken->str.size);
    }
    else
    {
        return PyUnicode_FromStringAndSize("", 0);
    }
}


static PyMemberDef Node_members[] = 
{
    { NULL }
};

static PyGetSetDef Node_getsetters[] = 
{
    { "trove", (getter) Node_get_trove, (setter) NULL, "The trove tracking this node." },
    { "nodeIdx", (getter) Node_get_nodeIdx, (setter) NULL, "The index of this node in its trove's tracking array." },
    { "kind", (getter) Node_get_kind, (setter) NULL, "The kind of node this is." },
    { "firstToken", (getter) Node_get_firstToken, (setter) NULL, "The first token which contributes to this node, including any annotation and comment tokens." },
    { "keyToken", (getter) Node_get_keyToken, (setter) NULL, "The key token if the node is inside a dict." },
    { "valueToken", (getter) Node_get_valueToken, (setter) NULL, "The first token of this node's actual value; for a container, it points to the opening brac(e|ket)." },
    { "lastValueToken", (getter) Node_get_lastValueToken, (setter) NULL, "The last token of this node's actual value; for a container, it points to the closing brac(e|ket)." },
    { "lastToken", (getter) Node_get_lastToken, (setter) NULL, "The last token of this node, including any annotation and comment tokens." },
    { "parentNodeIdx", (getter) Node_get_parentNodeIdx, (setter) NULL, "The parent node's index, or -1 if this node is the root." },
    { "childOrdinal", (getter) Node_get_childOrdinal, (setter) NULL, "The index of this node vis a vis its sibling nodes (starting at 0)." },
    { "numChildren", (getter) Node_get_numChildren, (setter) NULL, "The number of children a node has." },
    { "firstChild", (getter) Node_get_firstChild, (setter) NULL, "The first child of node (index 0)." },
    { "nextSibling", (getter) Node_get_nextSibling, (setter) NULL, "The next sibling in the child index order of a node." },
    { "hasKey", (getter) Node_get_hasKey, (setter) NULL, "Returns whether a node has a key token tracked. (If it's a member of a dict.)" },
    { "nestedValue", (getter) Node_get_nestedValue, (setter) NULL, "The entire nested text of a node, including child nodes and associated comments and annotations." },
    { "numAnnotations", (getter) Node_get_numAnnotations, (setter) NULL, "Return the number of annotations associated to a node." },
    { "numComments", (getter) Node_get_numComments, (setter) NULL, "Return the number of comments associated to a node." },
    { "address", (getter) Node_get_address, (setter) NULL, "The full address of a node." },
    { NULL }
};

static PyMethodDef Node_methods[] = 
{
    { "str", (PyCFunction) Node_str, METH_NOARGS, "The node string." },
    { "getParent", (PyCFunction) Node_getParent, METH_NOARGS, "Get a node's parent." },
    { "getChild", (PyCFunction) Node_getChild, METH_VARARGS | METH_KEYWORDS, "Get a node's parent." },
    { "getAnnotations", (PyCFunction) Node_str, METH_VARARGS | METH_KEYWORDS, "Get a node's annotations." },
    { "getComments", (PyCFunction) Node_str, METH_VARARGS | METH_KEYWORDS, "Get a node's comments." },
    { "getRelativeNode", (PyCFunction) Node_str, METH_VARARGS, "Get a node by relative address." },
    { NULL }
};

PyTypeObject NodeType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "humon.Node",
    .tp_doc = "Encodes a Humon data node.",
    .tp_basicsize = sizeof(NodeObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Node_new,
    .tp_init = (initproc) Node_init,
    .tp_dealloc = (destructor) Node_dealloc,
    .tp_members = Node_members,
    .tp_getset = Node_getsetters,
    .tp_methods = Node_methods
};


int RegisterNodeType(PyObject * module)
{
    if (PyType_Ready(& NodeType) < 0)
        { return -1; }
        
    Py_INCREF(& NodeType);
    if (PyModule_AddObject(module, "Node", (PyObject *) & NodeType) < 0)
    {
        Py_DECREF(& NodeType);
        Py_DECREF(module);
        return -1;
    }

    return 0;
}
