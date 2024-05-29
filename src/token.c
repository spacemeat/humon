#include "humon.internal.h"

huTokenKind huGetTokenKind(huToken const * token)
{
	return token->kind;
}


huStringView const * huGetRawString(huToken const * token)
{
	return & token->rawStr;
}


huStringView const * huGetString(huToken const * token)
{
	return & token->str;
}


huLine_t huGetLine(huToken const * token)
{
	return token->line;
}


huCol_t huGetColumn(huToken const * token)
{
	return token->col;
}


huLine_t huGetEndLine(huToken const * token)
{
	return token->endLine;
}


huCol_t huGetEndColumn(huToken const * token)
{
	return token->endCol;
}
