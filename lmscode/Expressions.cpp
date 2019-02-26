#include "Expressions.h"
#include "Parser.h"

#define SkipToken idx + 1 < collection->size()

namespace LMS
{
	bool ExecutionExpression::Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result)
	{
		//x(y)
		int idx = *cIdx;
		ParserToken *current = start;
		switch (current->TokenType)
		{
		case ParserTokenType::Identifier:
			break;
		}
		return false;
	}

	bool DepthExpression::Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result)
	{
		if (start->TokenType == ParserTokenType::Identifier)
		{
			int idx = *cIdx;
			if (idx + 1 < collection->size())
			{
				idx++;
				ParserToken *next = &(*collection)[idx];
				if (next->TokenContextualType == ParserTokenContextualType::Dot)
				{
					if (idx + 1 < collection->size())
					{
						idx++;
						next = &(*collection)[idx];
						if (next->TokenType == ParserTokenType::Identifier)
						{
							*cIdx = idx;
							*result = new DepthExprResult {
								start->StringValue,
								next->StringValue,
								next
							};
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	bool InvocationExpression::Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result)
	{
		if (start->TokenType == ParserTokenType::Identifier)
		{
			_STD string potentialMethodName = start->StringValue;
			int idx = *cIdx;
			idx++;
			if (idx + 1 >= collection->size())
				return false;
			ParserToken *next = &(*collection)[idx];
			if (next->TokenContextualType == ParserTokenContextualType::LeftParen)
			{
				_STD string params;
				int openBrace = 0;
				int closedBrace = 0;
				while (true)
				{
					if (idx + 1 >= collection->size())
						return false;
					next = &(*collection)[idx];
					switch (next->TokenContextualType)
					{
					case ParserTokenContextualType::RightParen:
						closedBrace++;
						break;
					case ParserTokenContextualType::LeftParen:
						openBrace++;
						break;
					case ParserTokenContextualType::String:
						params += next->StringValue + ' ';
						break;
					case ParserTokenContextualType::Int:
						params += _STD to_string(next->IntValue) + ' ';
						break;
					case ParserTokenContextualType::Float:
						params += _STD to_string(next->FloatValue) + ' ';
						break;
					case ParserTokenContextualType::Double:
						params += _STD to_string(next->DoubleValue) + ' ';
						break;
					case ParserTokenContextualType::Dot:
						params += ".";
						break;
					}
					if (closedBrace == openBrace)
						break;
					idx++;
				}
				*cIdx = *cIdx + 1;
				*result = new InvocationExprResult{
					*cIdx + 1,
					idx,
					params,
					potentialMethodName
				};
				return true;
			}
		}
		return false;
	}

	bool IdentifierExpression::Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result)
	{
		return start->TokenType == ParserTokenType::Identifier;
	}

	bool ArithmeticExpression::Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result)
	{
		int idx = *cIdx;
		ParserToken dummyToken = ParserToken {
			ParserTokenType::Number
		};
		while (true)
		{
			ParserToken *next = &(*collection)[idx];
			if (start->TokenType == ParserTokenType::Number)
			{
				if (dummyToken.TokenContextualType == ParserTokenContextualType::Unknown)
					dummyToken.TokenContextualType = start->TokenContextualType;
				dummyToken.IntValue = start->IntValue;
				break;
			}
			if (SkipToken)
				idx++;
			else break;
		}
		if (dummyToken.TokenContextualType == ParserTokenContextualType::Unknown)
			return false;
		*cIdx = idx + 1;
		*result = new ArithmeticResult{
			dummyToken.IntValue
		};
		return true;
	}

	bool IfExpression::Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result)
	{
		int idx = *cIdx;
		if (SkipToken)
		{
			idx++;
			ParserToken *next = &(*collection)[idx];
			while (true)
			{
				if (idx + 1 >= collection->size())
					return false;
				next = &(*collection)[idx];
				if (next->TokenContextualType == ParserTokenContextualType::OpenBrace)
					break;
				idx++;
			}
			*cIdx = *cIdx + 1;
			*result = new InvocationExprResult {
				*cIdx + 1,
				idx
			};
			return true;
		}
		return false;
	}

#define OP_REF_1(prim, allowed) case ParserTokenContextualType::prim: \
	{ \
		primary = prim; \
		if (SkipToken) \
		{ \
			idx++; \
			ParserToken *next = &(*collection)[idx]; \
			if (next->TokenType == ParserTokenType::Symbol) \
			{ \
				switch (next->TokenContextualType) \
				{ \
					case ParserTokenContextualType::allowed: \
						secondary = next->TokenContextualType; \
					break; \
					default: \
						CurrentParserInstance->ElapsedResult()->Errors.push_back(Parser::NewError("Invalid secondary operator", next->StartIndex, next->Length, false, &CurrentParserInstance->CurrentSourceFile()->Info)); \
						return false; \
					break; \
				} \
			} \
		} \
	} \
	break; \

#define OP_REF_2(prim, allowed, allowedT) case ParserTokenContextualType::prim: \
	{ \
		primary = prim; \
		if (SkipToken) \
		{ \
			idx++; \
			ParserToken *next = &(*collection)[idx]; \
			if (next->TokenType == ParserTokenType::Symbol) \
			{ \
				switch (next->TokenContextualType) \
				{ \
					case ParserTokenContextualType::allowed: \
					case ParserTokenContextualType::allowedT: \
						secondary = next->TokenContextualType; \
					break; \
					default: \
						CurrentParserInstance->ElapsedResult()->Errors.push_back(Parser::NewError("Invalid secondary operator", next->StartIndex, next->Length, false, &CurrentParserInstance->CurrentSourceFile()->Info)); \
					break; \
				} \
			} \
		} \
	} \
	break; \

	bool OperatorExpression::Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result)
	{
		if (start->TokenType != ParserTokenType::Symbol)
			return false;
		int idx = *cIdx;
		ParserTokenContextualType primary = ParserTokenContextualType::Unknown;
		ParserTokenContextualType secondary = ParserTokenContextualType::Unknown;
		switch (start->TokenContextualType)
		{
			OP_REF_2(Asterisk, Asterisk, Equal) //** *=
			OP_REF_1(Equal, Equal) //==
			OP_REF_2(Plus, Plus, Equal)
			OP_REF_2(Minus, Minus, Equal)
			OP_REF_2(Slash, Slash, Equal)
		default:
			return false;
			break;
		}
		*cIdx += secondary == ParserTokenContextualType::Unknown ? 1 : 2;
		*result = new OperatorExpressionResult{
			primary,
			secondary
		};
		return true;
	}
}