#include "common.h"


typedef struct
{
    PyObject_HEAD
    huToken const * tokenPtr;
} TokenObject;


static void Token_dealloc(TokenObject * self)
{
    printf("Token dealloc\n");
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * Token_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    printf("Token new\n");
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
    printf("Token init\n");
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
        ? getEnumValue("TokenKind", self->tokenPtr->kind)
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


static PyObject * Token_repr(TokenObject * self)
{
    if (! checkYourSelf(self))
        { return NULL; }

//    PyObject * str = PyUnicode_FromStringAndSize(
//        self->tokenPtr->str.ptr, 
//        self->tokenPtr->str.size);
//    PyObject * str = Py_BuildValue("s#",
//        self->tokenPtr->str.ptr, 
//        self->tokenPtr->str.size);

    return Py_BuildValue("s",
        huNodeKindToString(self->tokenPtr->kind));


//    return PyUnicode_FromFormat("Token: kind: %s; value: %S",
//        huNodeKindToString(self->tokenPtr->kind), str);
}


static PyObject * Token_str(TokenObject * self)
{
    if (! checkYourSelf(self))
        { return NULL; }

    return Py_BuildValue("s#",
        self->tokenPtr->str.ptr, 
        self->tokenPtr->str.size);

//    return checkYourSelf(self)
//        ? PyUnicode_FromStringAndSize(self->tokenPtr->str.ptr, self->tokenPtr->str.size)
//        : NULL;
}


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
    { NULL }
};

PyTypeObject TokenType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "humon.humon.Token",
    .tp_doc = "Encodes a Humon data token.",
    .tp_basicsize = sizeof(TokenObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Token_new,
    .tp_init = (initproc) Token_init,
    .tp_dealloc = (destructor) Token_dealloc,
    .tp_members = Token_members,
    .tp_getset = Token_getsetters,
    .tp_methods = Token_methods,
    .tp_repr = (reprfunc) Token_repr,
    .tp_str = (reprfunc) Token_str
};


int RegisterTokenType(PyObject * module)
{
    if (PyType_Ready(& TokenType) < 0)
        { return -1; }
        
    Py_INCREF(& TokenType);
    if (PyModule_AddObject(module, "Token", (PyObject *) & TokenType) < 0)
    {
        Py_DECREF(& TokenType);
        return -1;
    }

    return 0;
}


PyObject * makeToken(huToken const * tokenPtr)
{
    PyObject * tokenObj = NULL;

    if (tokenPtr != NULL)
    {
        PyObject * capsule = PyCapsule_New((void *) tokenPtr, NULL, NULL);
        if (capsule != NULL)
        {
            tokenObj = PyObject_CallFunction((PyObject *) & TokenType, "(O)", capsule);
            if (tokenObj == NULL && ! PyErr_Occurred())
                { PyErr_SetString(PyExc_RuntimeError, "Could not create Token"); }

            Py_DECREF(capsule);
        }
        else
            { PyErr_SetString(PyExc_RuntimeError, "Could not make new capsule"); }
    }
    else
        { PyErr_SetString(PyExc_ValueError, "Attempt to make a null Token"); }
    
    return tokenObj;
}
