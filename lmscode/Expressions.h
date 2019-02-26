#pragma once

#include "Tokens.h"

namespace LMS
{
	class Expression;

	enum ExpressionType
	{
		Depth,
		Invocation,
		EIdentifier,
		EIf,
		Arithmetic,
		Operator
	};

	struct ExpressionInfo
	{
		ExpressionType Type;
		Expression *Expr;
	};

	struct DepthExprResult
	{
		_STD string Parent;
		_STD string Child;
		ParserToken *Start;
	};

	struct InvocationExprResult
	{
		int Start;
		int End;
		_STD string ParamsString;
		_STD string PotentialMethodName;
	};

	struct ArithmeticResult
	{
		int Value;
	};

	struct OperatorExpressionResult
	{
		int Primary;
		int Secondary;
	};

	struct CompilableDepthExpression
	{
		unsigned short Mx;
		unsigned short Cx;
		void *Parent;
		void *Child;
	};

	struct CompilableInvocationExpression
	{
		size_t MethodGlobalIndex;
		size_t FirstIndex;
		size_t LastIndex;
	};

	class Expression
	{
	protected:
		ParserToken *m_Start;
	public:
		virtual bool Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result) = 0;
	};

	class DepthExpression : public Expression
	{
	public:
		bool Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result) override;
	};

	class InvocationExpression : public Expression
	{
	public:
		bool Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result) override;
	};

	class IdentifierExpression : public Expression
	{
	public:
		bool Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result) override;
	};

	class ArithmeticExpression : public Expression
	{
		bool Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result) override;
	};

	class OperatorExpression : public Expression
	{
		bool Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result) override;
	};

	class IfExpression : public Expression
	{
	public:
		bool Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result) override;
	};

	class ExecutionExpression : public Expression
	{
	public:
		bool Match(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result) override;
	};

	static const int ExpressionsCount = 6;
	static ExpressionInfo **Expressions = new ExpressionInfo*[ExpressionsCount]
	{
		new ExpressionInfo { ExpressionType::Depth, new DepthExpression },
		new ExpressionInfo { ExpressionType::Invocation, new InvocationExpression },
		new ExpressionInfo { ExpressionType::EIf, new IfExpression },
		new ExpressionInfo { ExpressionType::Arithmetic, new ArithmeticExpression },
		new ExpressionInfo { ExpressionType::Operator, new OperatorExpression },
		new ExpressionInfo { ExpressionType::EIdentifier, new IdentifierExpression } //lowest priority
	};
}