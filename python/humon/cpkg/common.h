#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include "humon/humon.h"
#include "humon/ansiColors.h"

//#define CAVEPERSON

enum VectorElementTypeEnum
{
    VECTORKIND_TOKENS,
    VECTORKIND_NODES,
    VECTORKIND_ANNOTATIONS,
    VECTORKIND_COMMENTS,
    VECTORKIND_ERRORS
};


int min(int a, int b);
int max(int a, int b);


extern PyTypeObject TokenType;
extern PyTypeObject NodeType;
extern PyTypeObject TroveType;

void AddEnumConstantsToModule(PyObject * module);

PyObject * getEnumValue(char const * enumName, int value);
PyObject * makeToken(huToken const * token);
PyObject * makeNode(PyObject * trove, huNode const * node);
PyObject * makeTrove(huTrove const * trove);


int RegisterTokenType(PyObject * module);
int RegisterNodeType(PyObject * module);
int RegisterTroveType(PyObject * module);

