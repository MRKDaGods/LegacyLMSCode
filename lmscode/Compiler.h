#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <map>

#include "Parser.h"

#define LMS_COMPILATION_KEY ""

namespace LMS
{
	enum CompilerBuildTarget
	{
		LMS,
		CSharp,
		CPP
	};

	struct CompilerResult
	{
		_STD vector<ParserError> Errors;
		const char *OutputPath;
	};

	struct CompilerOptions
	{
		CompilerBuildTarget BuildTarget;
		const char *OutputPath;
		const char *Libname;
	};

	enum InstructionType
	{
		locvar,
		depth,
		type_ref,
		push_locvar,
		push_class,
		push_field,
		push_method,
		depth_child,
		depth_end,
		invoke,
		invoke_args_start,
		invoke_args_end,
		push_int
	};

	enum InstructionValueType
	{
		VNone,
		VString,
		VInt32
	};

	struct Instruction
	{
		InstructionType Type;
		InstructionValueType ValueType;
		_STD string Value;

		static Instruction New(InstructionType type, InstructionValueType vtype, _STD string val);
	};

	class Compiler
	{
	private:
		static _STD map<size_t, _STD map<size_t, size_t>> ms_Barriers;
		_STD vector<LMS::ParserSourceFileInfo> m_SrcFiles;
		CompilerOptions m_Options;
		Parser **m_MobileParser;
		ParserResult *m_ParserResult;

		_STD vector<Instruction> GenerateInstructions(_METHODINFO *method);
		_STD vector<Instruction> GenerateMobileInstructions(size_t index, size_t exprCount, ExpressionResult *exprs, size_t exprOffset, size_t depth);
	public:
		Compiler(_STD vector<LMS::ParserSourceFileInfo> info, CompilerOptions options, Parser **pInstance = 0);
		CompilerResult Start();
		void CompileLMS();
		static void AddInstructionRegionBarrier(size_t index, size_t f, size_t c);
	};
}
