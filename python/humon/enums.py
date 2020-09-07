from enum import IntEnum

from . import humon

class Encoding(IntEnum):
    UTF8 = humon.HU_ENCODING_UTF8      # Specifies UTF-8 encoding.
    UTF16_BE = humon.HU_ENCODING_UTF16_BE		# Specifies UTF-16 big-endian encoding.
    UTF16_LE = humon.HU_ENCODING_UTF16_LE		# Specifies UTF-16 little-endian encoding.
    UTF32_BE = humon.HU_ENCODING_UTF32_BE		# Specifies UTF-32 big-endian encoding.
    UTF32_LE = humon.HU_ENCODING_UTF32_LE		# Specifies UTF-32 little-endian encoding.
    UNKNOWN = humon.HU_ENCODING_UNKNOWN		# Specifies that the encoding is unknown.

class TokenKind(IntEnum):
    NULL = humon.HU_TOKENKIND_NULL		# Invalid token. Malformed, or otherwise nonexistent.
    EOF = humon.HU_TOKENKIND_EOF		# The end of the token stream or string.
    STARTLIST = humon.HU_TOKENKIND_STARTLIST		# The opening '[' of a list.
    ENDLIST = humon.HU_TOKENKIND_ENDLIST		# The closing ']' of a list.
    STARTDICT = humon.HU_TOKENKIND_STARTDICT		# The opening '{' of a dict.
    ENDDICT = humon.HU_TOKENKIND_ENDDICT		# The closing '}' of a dict.
    KEYVALUESEP = humon.HU_TOKENKIND_KEYVALUESEP		# The separating ':' of a key-value pair.
    ANNOTATE = humon.HU_TOKENKIND_ANNOTATE		# The annotation mark '@'.
    WORD = humon.HU_TOKENKIND_WORD		# Any key or value string, quoted or unquoted.
    COMMENT = humon.HU_TOKENKIND_COMMENT		# Any comment token. An entire comment is considered one token.

class NodeKind(IntEnum):
    NULL = humon.HU_NODEKIND_NULL		# Invalid node. An invalid address returns a null node.
    LIST = humon.HU_NODEKIND_LIST		# List node. The node contains a sequence of unassociated objects in maintained order.
    DICT = humon.HU_NODEKIND_DICT		# Dict node. The node contains a sequence of string-associated objects in maintained order.
    VALUE = humon.HU_NODEKIND_VALUE		# Value node. The node contains a string value, and no children.

class WhitespaceFormat(IntEnum):
    CLONED = humon.HU_WHITESPACEFORMAT_CLONED		# Byte-for-byte copy of the original.
    MINIMAL = humon.HU_WHITESPACEFORMAT_MINIMAL		# Reduces as much whitespace as possible.
    PRETTY = humon.HU_WHITESPACEFORMAT_PRETTY		# Formats the text in a standard, human-friendly way.

class Error(IntEnum):
    NOERROR = humon.HU_ERROR_NOERROR		# No error.
    BADENCODING = humon.HU_ERROR_BADENCODING		# The Unicode encoding is malformed.
    UNFINISHEDQUOTE = humon.HU_ERROR_UNFINISHEDQUOTE		# The quoted text was not endquoted.
    UNFINISHEDCSTYLECOMMENT = humon.HU_ERROR_UNFINISHEDCSTYLECOMMENT		# The C-style comment was not closed.
    UNEXPECTEDEOF = humon.HU_ERROR_UNEXPECTEDEOF		# The text ended early.
    TOOMANYROOTS = humon.HU_ERROR_TOOMANYROOTS		# There is more than one root node detected.
    NONUNIQUEKEY = humon.HU_ERROR_NONUNIQUEKEY		# A non-unique key was encountered in a dict or annotation.
    SYNTAXERROR = humon.HU_ERROR_SYNTAXERROR		# General syntax error.
    NOTFOUND = humon.HU_ERROR_NOTFOUND		# No node could be found at the address.
    ILLEGAL = humon.HU_ERROR_ILLEGAL		# The address or node was illegal.
    BADPARAMETER = humon.HU_ERROR_BADPARAMETER		# An API parameter is malformed or illegal.
    BADFILE = humon.HU_ERROR_BADFILE		# An attempt to open or operate on a file failed.
    OUTOFMEMORY = humon.HU_ERROR_OUTOFMEMORY		# An internal memory allocation failed.
    TROVEHASERRORS = humon.HU_ERROR_TROVEHASERRORS		# The loading function succeeded, but the loaded trove has errors.

class ColorCode(IntEnum):
    TOKENSTREAMBEGIN = humon.HU_COLORCODE_TOKENSTREAMBEGIN		# Beginning-of-token stream color code.
    TOKENSTREAMEND = humon.HU_COLORCODE_TOKENSTREAMEND		# End-of-token stream color code.
    TOKENEND = humon.HU_COLORCODE_TOKENEND		# End-of-token color code.
    PUNCLIST = humon.HU_COLORCODE_PUNCLIST		# List punctuation style. ([,]) 
    PUNCDICT = humon.HU_COLORCODE_PUNCDICT		# Dict punctuation style. ({,})
    PUNCKEYVALUESEP = humon.HU_COLORCODE_PUNCKEYVALUESEP		# Key-value separator style. (:)
    PUNCANNOTATE = humon.HU_COLORCODE_PUNCANNOTATE		# Annotation mark style. (@)
    PUNCANNOTATEDICT = humon.HU_COLORCODE_PUNCANNOTATEDICT		# Annotation dict punctuation style. ({,})
    PUNCANNOTATEKEYVALUESEP = humon.HU_COLORCODE_PUNCANNOTATEKEYVALUESEP		# Annotation key-value separator style. (:)
    KEY = humon.HU_COLORCODE_KEY		# Key style.
    VALUE = humon.HU_COLORCODE_VALUE		# Value style.
    COMMENT = humon.HU_COLORCODE_COMMENT		# Comment style.
    ANNOKEY = humon.HU_COLORCODE_ANNOKEY		# Annotation key style.
    ANNOVALUE = humon.HU_COLORCODE_ANNOVALUE		# Annotation value style.
    WHITESPACE = humon.HU_COLORCODE_WHITESPACE		# Whitespace style (including commas).
    NUMCOLORS = humon.HU_COLORCODE_NUMCOLORS		# One past the last style code.

class VectorKind(IntEnum):
    COUNTING = humon.HU_VECTORKIND_COUNTING		# The vector is set up to count characters only.
    PREALLOCATED = humon.HU_VECTORKIND_PREALLOCATED		# The vector is set with a preallocated, maximum buffer.
    GROWABLE = humon.HU_VECTORKIND_GROWABLE		# The vector is set up with an unbounded growable buffer.
