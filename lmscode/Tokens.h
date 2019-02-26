#pragma once

#include <string>
#include <vector>

namespace LMS
{
	struct ParserToken;

	enum ModifierType : unsigned short
	{
		MNone = 0,
		Public = 1,
		Private = 2,
		Protected = 4,
		Internal = 8,
		Abstract = 16,
		Virtual = 32,
		Override = 64,
		Static = 128,
		Native = 256,
		Entrypoint = 512
	};

	struct ClassToken
	{
		_STD string Name;
		_STD string ParentString;
		_STD string Namespaze;
	};

	struct FieldToken
	{
		ParserToken *Token;
		size_t ClassIdx;
		_STD string Name;
		ModifierType Modifiers;
		_STD string TypeString;
		_STD string DefaultValueString;
	};

	struct ParameterToken
	{
		_STD string TypeName;
		_STD string Name;
		int Pos;
	};

	struct VariableToken
	{
		_STD string TypeName;
		_STD string Name;
	};

	struct ExpressionResult
	{
		size_t ExpressionType;
		void *Result;
	};

	struct MethodToken
	{
		_STD string TypeName;
		_STD string Name;
		_STD size_t ClassIdx;
		ModifierType Modifiers;
		_STD vector<ParameterToken> Parameters;
		_STD vector<VariableToken> Variables;
		_STD vector<ExpressionResult> Expressions;
	};

	enum ParserTokenType
	{
		None,
		Directive,
		Number,
		Symbol,
		Identifier
	};

	enum DirectiveType
	{
		Use,
		Def,
		Nms,
		Lms
	};

	struct DirectiveInfo
	{
		DirectiveType Type;
		_STD string Model;
	};

	struct ModifierInfo
	{
		ModifierType Type;
		_STD string Model;
	};

	enum KeywordType
	{
		Routine,
		Class,
		Object,
		If,
		Elif,
		Else,
		Field,
		Method,
		Var
	};

	struct KeywordInfo
	{
		KeywordType Type;
		_STD string Model;
	};

	enum ParserTokenContextualType
	{
		Unknown,
		Sbyte,
		Byte,
		Short,
		UShort,
		Int,
		UInt,
		Long,
		ULong,
		Double,
		Float,
		Bool,
		String,
		LeftParen,
		RightParen,
		OpenBrace,
		ClosedBrace,
		Comma,
		Asterisk,
		Dot,
		Keyword,
		Modifier,
		Equal,
		Minus,
		Plus,
		Slash,
		SSlash,
		Colon
	};

	struct ParserToken
	{
		ParserTokenType TokenType;
		ParserTokenContextualType TokenContextualType;
		char SbyteValue;
		unsigned char ByteValue;
		short ShortValue;
		unsigned short UShortValue;
		int IntValue;
		unsigned int UIntValue;
		long LongValue;
		unsigned long ULongValue;
		double DoubleValue;
		float FloatValue;
		bool BoolValue;
		_STD string StringValue;
		DirectiveInfo *DirectiveValue;
		KeywordInfo *KeywordValue;
		ModifierInfo *ModifierValue;
		bool IsHex;
		bool IsBinary;
		bool BadToken;
		_STD string Info;
		size_t StartIndex;
		size_t Length;
	};

	void operator |=(ModifierType &rhs, ModifierType lhs);
}