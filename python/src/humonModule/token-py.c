#include "common.h"


typedef struct
{
    PyObject_HEAD
    huToken const * tokenPtr;
} TokenObject;


static void Token_dealloc(TokenObject * self)
{
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * Token_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    TokenObject * self;
    self = (TokenObject *) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->tokenPtr = NULL;
    }

    return (PyObject *) self;
}

static int Token_init(TokenObject * self, PyObject * args, PyObject * kwds)
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

    // token will never be NULL; a capsule can't own a NULL. I'm sure there's a reason.
    huToken const * token = PyCapsule_GetPointer(capsule, NULL);
    self->tokenPtr = token;

    return 0;
}

static bool checkYourSelf(TokenObject * self)
{
    if (self->tokenPtr == NULL)
    {
        PyErr_SetString(PyExc_ValueError, "accessing nullish token");
        return false;
    }

    return true;
}

static PyObject * Token_get_kind(TokenObject * self, void * closure)
    { return checkYourSelf(self)
        ? getEnumValue("humon.enums", "TokenKind", self->tokenPtr->kind)
        : NULL; }

static PyObject * Token_get_line(TokenObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(self->tokenPtr->line)
        : NULL; }

static PyObject * Token_get_col(TokenObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(self->tokenPtr->col)
        : NULL; }

static PyObject * Token_get_endLine(TokenObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(self->tokenPtr->endLine)
        : NULL; }

static PyObject * Token_get_endCol(TokenObject * self, void * closure)
    { return checkYourSelf(self)
        ? PyLong_FromLong(self->tokenPtr->endCol)
        : NULL; }

static PyObject * Token_str(TokenObject * self, PyObject * Py_UNUSED(ignored))
    { return checkYourSelf(self)
        ? PyUnicode_FromStringAndSize(self->tokenPtr->str.ptr, self->tokenPtr->str.size)
        : NULL; }

static PyMemberDef Token_members[] = 
{
    { NULL }
};

static PyGetSetDef Token_getsetters[] = 
{
    { "kind", (getter) Token_get_kind, (setter) NULL, "The kind of token this is." },
    { "line", (getter) Token_get_line, (setter) NULL, "The line number in the file where the token begins.", NULL},
    { "col", (getter) Token_get_col, (setter) NULL, "The column number in the file where the token begins.", NULL},
    { "endLine", (getter) Token_get_endLine, (setter) NULL, "The line number in the file where the token ends.", NULL},
    { "endCol", (getter) Token_get_endCol, (setter) NULL, "The column number in the file where the token ends.", NULL},
    { NULL }
};

static PyMethodDef Token_methods[] = 
{
    { "str", (PyCFunction) Token_str, METH_NOARGS, "The token string." },
    { NULL }
};

PyTypeObject TokenType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "humon.Token",
    .tp_doc = "Encodes a Humon data token.",
    .tp_basicsize = sizeof(TokenObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Token_new,
    .tp_init = (initproc) Token_init,
    .tp_dealloc = (destructor) Token_dealloc,
    .tp_members = Token_members,
    .tp_getset = Token_getsetters,
    .tp_methods = Token_methods
};


int RegisterTokenType(PyObject * module)
{
    if (PyType_Ready(& TokenType) < 0)
        { return -1; }
        
    Py_INCREF(& TokenType);
    if (PyModule_AddObject(module, "Token", (PyObject *) & TokenType) < 0)
    {
        Py_DECREF(& TokenType);
        Py_DECREF(module);
        return -1;
    }

    return 0;
}


static PyObject * makeToken(huToken const * tokenPtr)
{
    PyObject * tokenObj = NULL;

    if (tokenPtr != NULL)
    {
        PyObject * capsule = PyCapsule_New((void *) tokenPtr, NULL, NULL);
        if (capsule != NULL)
        {
            PyObject * newArgs = Py_BuildValue("(O)", capsule);
            if (newArgs != NULL)
            {
                PyObject * tokenObj = PyObject_CallObject((PyObject *) & TokenType, newArgs);
                Py_DECREF(newArgs);
            }
            Py_DECREF(capsule);
        }
    }

    return tokenObj;
}
