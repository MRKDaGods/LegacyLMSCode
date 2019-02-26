#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "Objects.h"
#include "ClassInfo.h"
#include "Tokens.h"
#include "Expressions.h"
#include "SubVector.h"
#include "Registry.h"

#define _CORE LMS::Core::

namespace LMS
{
	struct ParserSourceFileInfo;
	class Parser;

	extern Parser *CurrentParserInstance;

	struct ParserError
	{
		_STD string Message;
		size_t StartIndex;
		size_t Length;
		bool IsWarning;
		ParserSourceFileInfo *Owner;
	};

	struct ParserResult
	{
	public:
		ImageInfo Img;
		_STD vector<ParserError> Errors;
	};

	struct ParserSourceFileInfo
	{
		_STD string Text;
		_STD string Path;
		_STD string Name;
		_STD string Extension;
	};

	struct ParserSourceFile
	{
		ParserSourceFileInfo Info;
		_STD vector<_STD string> NamespacesUsed;
		_STD vector<ClassToken> Classes;
		_STD vector<FieldToken> Fields;
		_STD vector<MethodToken> Methods;
	};

	class Parser
	{
	private:
		_STD vector<ParserSourceFile> m_SrcFiles;
		const char *m_Libname;
		ParserResult *m_CurrentResult;
		ParserSourceFile *m_CurrentSourceFile;

		_STD vector<ParserToken> CollectTokens(ParserSourceFileInfo *src);
		bool IsIgnorableCharacter(char ch);
		bool IsHexCh(char ch);
		KeywordInfo *AsKeyword(_STD string model);
		ModifierInfo *AsModifier(_STD string model);
		ExpressionInfo *ParseExpression(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result);
		_CLASSINFO *SearchClass(_STD string name, ParserSourceFile *current, _CLASSINFO *clazzScope);
		void HandleDepthExpression(_METHODINFO *method, DepthExprResult *result, ParserSourceFile *current, _STD vector<ParserError> *errors);
		void HandleInvocationExpression(_METHODINFO *method, InvocationExprResult *result, ParserSourceFile *current, _STD vector<ParserError> *errors, _STD vector<ExpressionResult> *expressions, int idx);
	public:
		Parser(_STD vector<ParserSourceFileInfo> srcFiles, const char *libname = 0);
		ParserResult *Start();
		ParserSourceFile *CurrentSourceFile();
		ParserResult *ElapsedResult();
		static ParserError NewError(_STD string msg, size_t sIndex, size_t length, bool isWarning, ParserSourceFileInfo *owner);
	};
}