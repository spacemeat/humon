#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include "humon/humon.h"


void AddEnumConstantsToModule(PyObject * module)
{
    PyModule_AddIntConstant(module, "HU_ENCODING_UTF8", HU_ENCODING_UTF8);
    PyModule_AddIntConstant(module, "HU_ENCODING_UTF16_BE", HU_ENCODING_UTF16_BE);
    PyModule_AddIntConstant(module, "HU_ENCODING_UTF16_LE", HU_ENCODING_UTF16_LE);
    PyModule_AddIntConstant(module, "HU_ENCODING_UTF32_BE", HU_ENCODING_UTF32_BE);
    PyModule_AddIntConstant(module, "HU_ENCODING_UTF32_LE", HU_ENCODING_UTF32_LE);
    PyModule_AddIntConstant(module, "HU_ENCODING_UNKNOWN", HU_ENCODING_UNKNOWN);

    PyModule_AddIntConstant(module, "HU_TOKENKIND_NULL", HU_TOKENKIND_NULL);
    PyModule_AddIntConstant(module, "HU_TOKENKIND_EOF", HU_TOKENKIND_EOF);
    PyModule_AddIntConstant(module, "HU_TOKENKIND_STARTLIST", HU_TOKENKIND_STARTLIST);
    PyModule_AddIntConstant(module, "HU_TOKENKIND_ENDLIST", HU_TOKENKIND_ENDLIST);
    PyModule_AddIntConstant(module, "HU_TOKENKIND_STARTDICT", HU_TOKENKIND_STARTDICT);
    PyModule_AddIntConstant(module, "HU_TOKENKIND_ENDDICT", HU_TOKENKIND_ENDDICT);
    PyModule_AddIntConstant(module, "HU_TOKENKIND_KEYVALUESEP", HU_TOKENKIND_KEYVALUESEP);
    PyModule_AddIntConstant(module, "HU_TOKENKIND_ANNOTATE", HU_TOKENKIND_ANNOTATE);
    PyModule_AddIntConstant(module, "HU_TOKENKIND_WORD", HU_TOKENKIND_WORD);
    PyModule_AddIntConstant(module, "HU_TOKENKIND_COMMENT", HU_TOKENKIND_COMMENT);

    PyModule_AddIntConstant(module, "HU_NODEKIND_NULL", HU_NODEKIND_NULL);
    PyModule_AddIntConstant(module, "HU_NODEKIND_LIST", HU_NODEKIND_LIST);
    PyModule_AddIntConstant(module, "HU_NODEKIND_DICT", HU_NODEKIND_DICT);
    PyModule_AddIntConstant(module, "HU_NODEKIND_VALUE", HU_NODEKIND_VALUE);

    PyModule_AddIntConstant(module, "HU_WHITESPACEFORMAT_CLONED", HU_WHITESPACEFORMAT_CLONED);
    PyModule_AddIntConstant(module, "HU_WHITESPACEFORMAT_MINIMAL", HU_WHITESPACEFORMAT_MINIMAL);
    PyModule_AddIntConstant(module, "HU_WHITESPACEFORMAT_PRETTY", HU_WHITESPACEFORMAT_PRETTY);

    PyModule_AddIntConstant(module, "HU_ERROR_NOERROR", HU_ERROR_NOERROR);
    PyModule_AddIntConstant(module, "HU_ERROR_BADENCODING", HU_ERROR_BADENCODING);
    PyModule_AddIntConstant(module, "HU_ERROR_UNFINISHEDQUOTE", HU_ERROR_UNFINISHEDQUOTE);
    PyModule_AddIntConstant(module, "HU_ERROR_UNFINISHEDCSTYLECOMMENT", HU_ERROR_UNFINISHEDCSTYLECOMMENT);
    PyModule_AddIntConstant(module, "HU_ERROR_UNEXPECTEDEOF", HU_ERROR_UNEXPECTEDEOF);
    PyModule_AddIntConstant(module, "HU_ERROR_TOOMANYROOTS", HU_ERROR_TOOMANYROOTS);
    PyModule_AddIntConstant(module, "HU_ERROR_NONUNIQUEKEY", HU_ERROR_NONUNIQUEKEY);
    PyModule_AddIntConstant(module, "HU_ERROR_SYNTAXERROR", HU_ERROR_SYNTAXERROR);
    PyModule_AddIntConstant(module, "HU_ERROR_NOTFOUND", HU_ERROR_NOTFOUND);
    PyModule_AddIntConstant(module, "HU_ERROR_ILLEGAL", HU_ERROR_ILLEGAL);
    PyModule_AddIntConstant(module, "HU_ERROR_BADPARAMETER", HU_ERROR_BADPARAMETER);
    PyModule_AddIntConstant(module, "HU_ERROR_BADFILE", HU_ERROR_BADFILE);
    PyModule_AddIntConstant(module, "HU_ERROR_OUTOFMEMORY", HU_ERROR_OUTOFMEMORY);
    PyModule_AddIntConstant(module, "HU_ERROR_TROVEHASERRORS", HU_ERROR_TROVEHASERRORS);

    PyModule_AddIntConstant(module, "HU_COLORCODE_TOKENSTREAMBEGIN", HU_COLORCODE_TOKENSTREAMBEGIN);
    PyModule_AddIntConstant(module, "HU_COLORCODE_TOKENSTREAMEND", HU_COLORCODE_TOKENSTREAMEND);
    PyModule_AddIntConstant(module, "HU_COLORCODE_TOKENEND", HU_COLORCODE_TOKENEND);
    PyModule_AddIntConstant(module, "HU_COLORCODE_PUNCLIST", HU_COLORCODE_PUNCLIST);
    PyModule_AddIntConstant(module, "HU_COLORCODE_PUNCDICT", HU_COLORCODE_PUNCDICT);
    PyModule_AddIntConstant(module, "HU_COLORCODE_PUNCKEYVALUESEP", HU_COLORCODE_PUNCKEYVALUESEP);
    PyModule_AddIntConstant(module, "HU_COLORCODE_PUNCANNOTATE", HU_COLORCODE_PUNCANNOTATE);
    PyModule_AddIntConstant(module, "HU_COLORCODE_PUNCANNOTATEDICT", HU_COLORCODE_PUNCANNOTATEDICT);
    PyModule_AddIntConstant(module, "HU_COLORCODE_PUNCANNOTATEKEYVALUESEP", HU_COLORCODE_PUNCANNOTATEKEYVALUESEP);
    PyModule_AddIntConstant(module, "HU_COLORCODE_KEY", HU_COLORCODE_KEY);
    PyModule_AddIntConstant(module, "HU_COLORCODE_VALUE", HU_COLORCODE_VALUE);
    PyModule_AddIntConstant(module, "HU_COLORCODE_COMMENT", HU_COLORCODE_COMMENT);
    PyModule_AddIntConstant(module, "HU_COLORCODE_ANNOKEY", HU_COLORCODE_ANNOKEY);
    PyModule_AddIntConstant(module, "HU_COLORCODE_ANNOVALUE", HU_COLORCODE_ANNOVALUE);
    PyModule_AddIntConstant(module, "HU_COLORCODE_WHITESPACE", HU_COLORCODE_WHITESPACE);
    PyModule_AddIntConstant(module, "HU_COLORCODE_NUMCOLORS", HU_COLORCODE_NUMCOLORS);

    PyModule_AddIntConstant(module, "HU_VECTORKIND_COUNTING", HU_VECTORKIND_COUNTING);
    PyModule_AddIntConstant(module, "HU_VECTORKIND_PREALLOCATED", HU_VECTORKIND_PREALLOCATED);
    PyModule_AddIntConstant(module, "HU_VECTORKIND_GROWABLE", HU_VECTORKIND_GROWABLE);
}
