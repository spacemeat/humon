#include "common.h"


typedef struct
{
    PyObject_HEAD
    huToken const * tokenPtr;
} TokenObject;


static void Token_dealloc(TokenObject * self)
{
#ifdef CAVEPERSON
    if (self->tokenPtr == NULL)
        { printf("%sToken dealloc - <null>%s\n", ansi_darkBlue, ansi_off); }
    else
    {
        printf("%sToken dealloc - %.*s%s\n", ansi_darkBlue, 
            (int) self->tokenPtr->str.size, self->tokenPtr->str.ptr, ansi_off);
    }
#endif
    
    Py_TYPE(self)->tp_free((PyObject *) self);
}


static PyObject * Token_new(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
#ifdef CAVEPERSON
    printf("%sToken new%s\n", ansi_darkBlue, ansi_off);
#endif

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
#ifdef CAVEPERSON
    printf("%sToken init%s\n", ansi_darkMagenta, ansi_off);
#endif

    PyObject * capsule = NULL;
    if (! PyArg_ParseTuple(args, "O", & capsule))
        { return -1; }

    if (capsule == NULL)
        { return -1; }

    Py_INCREF(capsule);
    if (PyCapsule_CheckExact(capsule))
    {
        // token will never be NULL; a capsule can't own a NULL. I'm sure there's a reason.
        huToken const * token = PyCapsule_GetPointer(capsule, NULL);
        self->tokenPtr = token;
    }
    else if (capsule == Py_None)
    {
        // so pass None to get a nullish token.
        self->tokenPtr = NULL;
    }
    else
        { PyErr_SetString(PyExc_ValueError, "Arg must be an encapsulated pointer, or None."); }

    Py_DECREF(capsule);

    return PyErr_Occurred() ? -1 : 0;
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


static PyObject * Token_get_isNullish(TokenObject * self, void * closure)
{
    PyObject * res = Py_False;
    if (self->tokenPtr)
        { res = Py_True; }
    Py_INCREF(res);
    return res;
}


static PyObject * Token_get_kind(TokenObject * self, void * closure)
{
    huEnumType_t kind = HU_TOKENKIND_NULL;
    if (self->tokenPtr)
        { kind = self->tokenPtr->kind; }

    return getEnumValue("TokenKind", kind);
}


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
    PyObject * kind = Token_get_kind(self, NULL);
    PyObject * str = PyObject_Str((PyObject *) self);

    PyObject * retstr = PyUnicode_FromFormat(
        "Token: kind: %S; value: %S", kind, str);

    Py_DECREF(str);
    Py_DECREF(kind);

    return retstr;
}


static PyObject * Token_str(TokenObject * self)
{
    if (self->tokenPtr)
        { return PyUnicode_FromStringAndSize(self->tokenPtr->str.ptr, self->tokenPtr->str.size); }
    else
        { return PyUnicode_FromString("<null>"); }
}


static PyObject * Token_richCompare(PyObject * self, PyObject * other, int op)
{
    PyObject * result = NULL;

    if (other == NULL)
        { result = Py_NotImplemented; }
    else if (op == Py_EQ)
    {
        if (((TokenObject *) self)->tokenPtr == ((TokenObject *) other)->tokenPtr ||
            (((TokenObject *) self)->tokenPtr == NULL && other == Py_None))
            { result = Py_True; }
        else
            { result = Py_False; }
    }
    else if (op == Py_NE)
    {
        if (((TokenObject *) self)->tokenPtr != ((TokenObject *) other)->tokenPtr ||
            (((TokenObject *) self)->tokenPtr == NULL && other == Py_None))
            { result = Py_True; }
        else
            { result = Py_False; }
    }
    else
        { result = Py_NotImplemented; }
    
    Py_XINCREF(result);
    return result;
}


static int Token_bool(PyObject * self)
{
    return ((TokenObject *) self)->tokenPtr != NULL;
}


static PyMemberDef Token_members[] = 
{
    { NULL }
};


static PyGetSetDef Token_getsetters[] = 
{
    { "isNullish", (getter) Token_get_isNullish, (setter) NULL, PyDoc_STR("Whether the token is nullish."), NULL },
    { "kind", (getter) Token_get_kind, (setter) NULL, PyDoc_STR("The kind of token this is."), NULL},
    { "line", (getter) Token_get_line, (setter) NULL, PyDoc_STR("The line number in the file where the token begins."), NULL},
    { "col", (getter) Token_get_col, (setter) NULL, PyDoc_STR("The column number in the file where the token begins."), NULL},
    { "endLine", (getter) Token_get_endLine, (setter) NULL, PyDoc_STR("The line number in the file where the token ends."), NULL},
    { "endCol", (getter) Token_get_endCol, (setter) NULL, PyDoc_STR("The column number in the file where the token ends."), NULL},
    { NULL }
};


static PyMethodDef Token_methods[] = 
{
    { NULL }
};


static PyNumberMethods numberMethods = 
{
    .nb_bool = (inquiry) Token_bool
};


PyTypeObject TokenType =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "humon.humon.Token",
    .tp_doc = PyDoc_STR("Encodes a Humon data token."),
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
    .tp_str = (reprfunc) Token_str,
    .tp_richcompare = (richcmpfunc) Token_richCompare,
    .tp_as_number = & numberMethods
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
    Py_INCREF(& TokenType);

    PyObject * capsule = NULL;
    if (tokenPtr == NULL)
    {
        capsule = Py_None;
        Py_INCREF(capsule);
    }
    else
        { capsule = PyCapsule_New((void *) tokenPtr, NULL, NULL); }

    PyObject * tokenObj = NULL;
    if (capsule != NULL)
    {
        tokenObj = PyObject_CallFunction((PyObject *) & TokenType, "(O)", capsule);
        if (tokenObj == NULL && ! PyErr_Occurred())
            { PyErr_SetString(PyExc_RuntimeError, "Could not create Token"); }

        Py_DECREF(capsule);
    }

    Py_DECREF(& TokenType);
    
    return tokenObj;
}
