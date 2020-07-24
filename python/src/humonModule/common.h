#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include "humon.h"


enum VectorElementTypeEnum
{
    VECTORKIND_TOKENS,
    VECTORKIND_NODES,
    VECTORKIND_ANNOTATIONS,
    VECTORKIND_COMMENTS,
    VECTORKIND_ERRORS
};


extern PyTypeObject TokenType;
extern PyTypeObject NodeType;
extern PyTypeObject TroveType;

PyObject * getEnumValue(char const * moduleName, char const * enumName, int value);

int RegisterTokenType(PyObject * module);
int RegisterNodeType(PyObject * module);
int RegisterTroveType(PyObject * module);

