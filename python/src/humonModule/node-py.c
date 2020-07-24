#include "common.h"


typedef struct
{
    PyObject_HEAD
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
        self->nodePtr = NULL;
    }

    return (PyObject *) self;
}

static int Node_init(NodeObject * self, PyObject * args, PyObject * kwds)
{
    // TODO: Py_ADDREF / Py_DECREF on args, self?
    PyObject * capsule = NULL;
    if (! PyArg_ParseTuple(args, "O", & capsule))
        { return -1; }
    
    if (! PyCapsule_CheckExact(capsule))
    {
        PyErr_SetString(PyExc_ValueError, "Arg must be an encapsulated pointer.");
        return -1;
    }

    // node will never be NULL
    huNode const * node = PyCapsule_GetPointer(capsule, NULL);
    self->nodePtr = node;

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
    { "kind", (getter) Node_get_kind, (setter) NULL, "The kind of node this is." },
    { NULL }
};

static PyMethodDef Node_methods[] = 
{
    { "str", (PyCFunction) Node_str, METH_NOARGS, "The node string." },
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
