from enum import IntEnum

from . import _enumConsts

class Encoding(IntEnum):
    UTF8 = _enumConsts.HU_ENCODING_UTF8      # Specifies UTF-8 encoding.
    UTF16_BE = _enumConsts.HU_ENCODING_UTF16_BE		# Specifies UTF-16 big-endian encoding.
    UTF16_LE = _enumConsts.HU_ENCODING_UTF16_LE		# Specifies UTF-16 little-endian encoding.
    UTF32_BE = _enumConsts.HU_ENCODING_UTF32_BE		# Specifies UTF-32 big-endian encoding.
    UTF32_LE = _enumConsts.HU_ENCODING_UTF32_LE		# Specifies UTF-32 little-endian encoding.
    UNKNOWN = _enumConsts.HU_ENCODING_UNKNOWN		# Specifies that the encoding is unknown.

class TokenKind(IntEnum):
    NULL = _enumConsts.HU_TOKENKIND_NULL		# Invalid token. Malformed, or otherwise nonexistent.
    EOF = _enumConsts.HU_TOKENKIND_EOF		# The end of the token stream or string.
    STARTLIST = _enumConsts.HU_TOKENKIND_STARTLIST		# The opening '[' of a list.
    ENDLIST = _enumConsts.HU_TOKENKIND_ENDLIST		# The closing ']' of a list.
    STARTDICT = _enumConsts.HU_TOKENKIND_STARTDICT		# The opening '{' of a dict.
    ENDDICT = _enumConsts.HU_TOKENKIND_ENDDICT		# The closing '}' of a dict.
    KEYVALUESEP = _enumConsts.HU_TOKENKIND_KEYVALUESEP		# The separating ':' of a key-value pair.
    ANNOTATE = _enumConsts.HU_TOKENKIND_ANNOTATE		# The annotation mark '@'.
    WORD = _enumConsts.HU_TOKENKIND_WORD		# Any key or value string, quoted or unquoted.
    COMMENT = _enumConsts.HU_TOKENKIND_COMMENT		# Any comment token. An entire comment is considered one token.

class NodeKind(IntEnum):
    NULL = _enumConsts.HU_NODEKIND_NULL		# Invalid node. An invalid address returns a null node.
    LIST = _enumConsts.HU_NODEKIND_LIST		# List node. The node contains a sequence of unassociated objects in maintained order.
    DICT = _enumConsts.HU_NODEKIND_DICT		# Dict node. The node contains a sequence of string-associated objects in maintained order.
    VALUE = _enumConsts.HU_NODEKIND_VALUE		# Value node. The node contains a string value, and no children.

class WhitespaceFormat(IntEnum):
    XERO = _enumConsts.HU_WHITESPACEFORMAT_XERO		# Byte-for-byte copy of the original.
    MINIMAL = _enumConsts.HU_WHITESPACEFORMAT_MINIMAL		# Reduces as much whitespace as possible.
    PRETTY = _enumConsts.HU_WHITESPACEFORMAT_PRETTY		# Formats the text in a standard, human-friendly way.

class Error(IntEnum):
    NOERROR = _enumConsts.HU_ERROR_NOERROR		# No error.
    BADENCODING = _enumConsts.HU_ERROR_BADENCODING		# The Unicode encoding is malformed.
    UNFINISHEDQUOTE = _enumConsts.HU_ERROR_UNFINISHEDQUOTE		# The quoted text was not endquoted.
    UNFINISHEDCSTYLECOMMENT = _enumConsts.HU_ERROR_UNFINISHEDCSTYLECOMMENT		# The C-style comment was not closed.
    UNEXPECTEDEOF = _enumConsts.HU_ERROR_UNEXPECTEDEOF		# The text ended early.
    TOOMANYROOTS = _enumConsts.HU_ERROR_TOOMANYROOTS		# There is more than one root node detected.
    NONUNIQUEKEY = _enumConsts.HU_ERROR_NONUNIQUEKEY		# A non-unique key was encountered in a dict or annotation.
    SYNTAXERROR = _enumConsts.HU_ERROR_SYNTAXERROR		# General syntax error.
    NOTFOUND = _enumConsts.HU_ERROR_NOTFOUND		# No node could be found at the address.
    ILLEGAL = _enumConsts.HU_ERROR_ILLEGAL		# The address or node was illegal.
    BADPARAMETER = _enumConsts.HU_ERROR_BADPARAMETER		# An API parameter is malformed or illegal.
    BADFILE = _enumConsts.HU_ERROR_BADFILE		# An attempt to open or operate on a file failed.
    OUTOFMEMORY = _enumConsts.HU_ERROR_OUTOFMEMORY		# An internal memory allocation failed.
    TROVEHASERRORS = _enumConsts.HU_ERROR_TROVEHASERRORS		# The loading function succeeded, but the loaded trove has errors.

class ColorCode(IntEnum):
    TOKENSTREAMBEGIN = _enumConsts.HU_COLORCODE_TOKENSTREAMBEGIN		# Beginning-of-token stream color code.
    TOKENSTREAMEND = _enumConsts.HU_COLORCODE_TOKENSTREAMEND		# End-of-token stream color code.
    TOKENEND = _enumConsts.HU_COLORCODE_TOKENEND		# End-of-token color code.
    PUNCLIST = _enumConsts.HU_COLORCODE_PUNCLIST		# List punctuation style. ([,]) 
    PUNCDICT = _enumConsts.HU_COLORCODE_PUNCDICT		# Dict punctuation style. ({,})
    PUNCKEYVALUESEP = _enumConsts.HU_COLORCODE_PUNCKEYVALUESEP		# Key-value separator style. (:)
    PUNCANNOTATE = _enumConsts.HU_COLORCODE_PUNCANNOTATE		# Annotation mark style. (@)
    PUNCANNOTATEDICT = _enumConsts.HU_COLORCODE_PUNCANNOTATEDICT		# Annotation dict punctuation style. ({,})
    PUNCANNOTATEKEYVALUESEP = _enumConsts.HU_COLORCODE_PUNCANNOTATEKEYVALUESEP		# Annotation key-value separator style. (:)
    KEY = _enumConsts.HU_COLORCODE_KEY		# Key style.
    VALUE = _enumConsts.HU_COLORCODE_VALUE		# Value style.
    COMMENT = _enumConsts.HU_COLORCODE_COMMENT		# Comment style.
    ANNOKEY = _enumConsts.HU_COLORCODE_ANNOKEY		# Annotation key style.
    ANNOVALUE = _enumConsts.HU_COLORCODE_ANNOVALUE		# Annotation value style.
    WHITESPACE = _enumConsts.HU_COLORCODE_WHITESPACE		# Whitespace style (including commas).
    NUMCOLORKINDS = _enumConsts.HU_COLORCODE_NUMCOLORKINDS		# One past the last style code.

class VectorKind(IntEnum):
    COUNTING = _enumConsts.HU_VECTORKIND_COUNTING		# The vector is set up to count characters only.
    PREALLOCATED = _enumConsts.HU_VECTORKIND_PREALLOCATED		# The vector is set with a preallocated, maximum buffer.
    GROWABLE = _enumConsts.HU_VECTORKIND_GROWABLE		# The vector is set up with an unbounded growable buffer.
