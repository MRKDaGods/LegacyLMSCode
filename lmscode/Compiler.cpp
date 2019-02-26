#include "Compiler.h"

#define LMS_ENCRYPT
#define LMS_CLEAR_VIEW

namespace LMS
{
	_STD map<size_t, _STD map<size_t, size_t>> Compiler::ms_Barriers;

	Compiler::Compiler(_STD vector<LMS::ParserSourceFileInfo> info, CompilerOptions options, Parser **pInstance)
	{
		m_SrcFiles = info;
		m_Options = options;
		m_MobileParser = pInstance;
	}

	CompilerResult Compiler::Start()
	{
		CompilerResult cres;
		Parser parser = Parser(m_SrcFiles, m_Options.Libname);
		if (m_MobileParser)
			*m_MobileParser = &parser;
		m_ParserResult = parser.Start();
		if (m_ParserResult->Errors.size() > 0)
		{
			cres.Errors = m_ParserResult->Errors;
			cres.Errors.push_back(Parser::NewError("Please fix the previous errors", 0, 0, false, 0));
			return cres;
		}
		m_ParserResult->Img.name = (char*)m_Options.Libname;
		switch (m_Options.BuildTarget)
		{
		case CompilerBuildTarget::LMS:
			CompileLMS();
			break;
		}
		return cres;
	}

	void Append(_STD string &src, _STD string txt)
	{
		src += txt + '\n';
	}

	void Compiler::CompileLMS()
	{
		_STD ofstream ostream(m_Options.OutputPath);
		_STD stringstream stream;
		//lms header
		stream << "#LMS " <<
			LMSCODE_VERSION
			<< "'"
			<< m_ParserResult->Img.name
			<< "'"
			<< m_ParserResult->Img.classCount
			<< " ";
		for (size_t s = 0; s < m_ParserResult->Img.classCount; s++)
		{
			_CLASSINFO *clazz = m_ParserResult->Img.classes[s];
			stream << "c '"
				<< clazz->name
				<< "' "
				<< clazz->namespaze
				<< " xc ";
		}
		for (size_t s = 0; s < m_ParserResult->Img.classCount; s++)
		{
			_CLASSINFO *clazz = m_ParserResult->Img.classes[s];
			stream << "cdef "
				<< s
				<< " fc "
				<< clazz->field_count
				<< " ";
			for (size_t f = 0; f < clazz->field_count; f++)
			{
				_FIELDINFO *field = &clazz->fields[f];
				stream << "f '"
					<< field->name
					<< "' "
					<< field->fieldType->index
					<< " "
					<< field->modifiers
					<< " xf ";
			}
			stream << " xfc "
				<< clazz->method_count
				<< " ";
			for (size_t m = 0; m < clazz->method_count; m++)
			{
				_METHODINFO *method = clazz->methods[m];
				stream << "m '"
					<< method->name
					<< "' "
					<< method->returntype->index
					<< " "
					<< method->parameter_magic
					<< "uL "
					<< method->global_index
					<< " "
					<< method->modifiers
					<< " "
					<< method->parameter_count
					<< " ";
				for (size_t p = 0; p < method->parameter_count; p++)
				{
					_PARAMETERINFO *param = &method->parameters[p];
					stream << "p '"
						<< param->name
						<< "' "
						<< param->type->index
						<< " xp ";
				}
				_STD vector<Instruction> instructions = GenerateInstructions(method);
				stream << instructions.size()
					<< " ";
				for (size_t e = 0; e < instructions.size(); e++)
				{
					Instruction *inst = &instructions[e];
					stream << "i "
						<< inst->Type
						<< " "
						<< inst->ValueType
						<< " "
						<< "'I!#~%!$~@#~!INSTRUCTION_VALY*&^&^Y&T@%^WR!RF~T&R!~W%I' '"
						<< inst->Value
						<< "' 'I!#~%!$~@#~!INSTRUCTION_VALY*&^&^Y&T@%^WR!RF~T&R!~W%I'"
						<< " xi ";
				}
				stream << " xm ";
			}
			stream << "xcdef ";
		}
		_STD string complStr = stream.str();
//#undef LMS_ENCRYPT
#ifndef LMS_CLEAR_VIEW
		complStr = Utils::StringReplace(stream.str(), "\n", " ");
#endif
#ifdef LMS_ENCRYPT
		complStr = Utils::StringReplace(stream.str(), "\n", "$%#$%#*D&^WD^Z");
		complStr = Utils::StringReplace(complStr, " ", "(**(_(#JFIHF!~!~!@##@[}{)_8");
		complStr = Utils::StringReplace(complStr, "\r", "~$^&*()_E(&@^#@");
		complStr = Utils::StringReplace(complStr, "\t", "$%^&*()__()@!");
		for (int rep = 0; rep < 4; rep++)
		{
			for (int i = 0; i < complStr.size(); i++)
			{
				char c = complStr[i];
				complStr[i] = CHAR_TABLE.at(c);
			}
		}
#endif
		/*for (int rep = 0; rep < 4; rep++)
		{
			for (int i = 0; i < complStr.size(); i++)
			{
				char c = complStr[i];
				complStr[i] = CHAR_TABLE_REVERSE.at(c);
			}
		}*/
		ostream << complStr;
		ostream.close();
	}

	void Compiler::AddInstructionRegionBarrier(size_t index, size_t f, size_t c)
	{
		if (c == 0)
			return;
		auto srch = ms_Barriers.find(index);
		if (srch == ms_Barriers.end())
		{
			ms_Barriers.insert(_STD make_pair(index, _STD map<size_t, size_t>()));
			AddInstructionRegionBarrier(index, f, c);
			return;
		}
		srch->second.insert(_STD make_pair(f, c));
	}

	_STD vector<Instruction> Compiler::GenerateMobileInstructions(size_t index, size_t exprCount, ExpressionResult *exprs, size_t exprOffset, size_t depth)
	{
		_STD vector<Instruction> vec;
		if (exprCount == 0 || !exprs)
			return vec;
		auto barrier = ms_Barriers.find(index);
		bool barrierExists = depth > 0 ? false : barrier != ms_Barriers.end();
		for (size_t e = exprOffset; e < exprCount; e++)
		{
			if (barrierExists)
			{
				auto srch = barrier->second.find(e);
				if (srch != barrier->second.end())
				{
					e += srch->second - 1;
					continue;
				}
			}
			ExpressionResult *expr = &exprs[e];
			_FIELDINFO *finfo = 0;
			_CLASSINFO *cinfo = 0;
			_METHODINFO *minfo = 0;
			switch (expr->ExpressionType)
			{
			case ExpressionType::Depth:
			{
				CompilableDepthExpression *complExpr = (CompilableDepthExpression*)expr->Result;
				switch (complExpr->Mx)
				{
				case 0:
					finfo = (_FIELDINFO*)complExpr->Parent;
					vec.push_back(Instruction::New(InstructionType::push_locvar, InstructionValueType::VInt32, TO_STRING_UINT(finfo->index)));
					break;
				case 1:
					finfo = (_FIELDINFO*)complExpr->Parent;
					vec.push_back(Instruction::New(InstructionType::type_ref, InstructionValueType::VInt32, TO_STRING_UINT(finfo->declaringType->index)));
					vec.push_back(Instruction::New(InstructionType::push_field, InstructionValueType::VInt32, TO_STRING_UINT(finfo->index)));
					break;
				case 2:
					cinfo = (_CLASSINFO*)complExpr->Parent;
					vec.push_back(Instruction::New(InstructionType::type_ref, InstructionValueType::VInt32, TO_STRING_UINT(cinfo->index)));
					break;
				}
				vec.push_back(Instruction::New(InstructionType::depth_child, InstructionValueType::VNone, ""));
				switch (complExpr->Cx)
				{
				case 0:
					finfo = (_FIELDINFO*)complExpr->Child;
					vec.push_back(Instruction::New(InstructionType::push_field, InstructionValueType::VInt32, TO_STRING_UINT(finfo->index)));
					break;
				case 1:
					minfo = (_METHODINFO*)complExpr->Child;
					vec.push_back(Instruction::New(InstructionType::push_method, InstructionValueType::VInt32, TO_STRING_UINT(minfo->global_index)));
				}
				vec.push_back(Instruction::New(InstructionType::depth_end, InstructionValueType::VNone, ""));
			}
			case ExpressionType::Arithmetic:
				vec.push_back(Instruction::New(InstructionType::push_int, InstructionValueType::VInt32, TO_STRING_INT(((ArithmeticResult*)expr->Result)->Value)));
				break;
			case ExpressionType::Invocation:
				CompilableInvocationExpression *invokExpr = (CompilableInvocationExpression*)expr->Result;
				vec.push_back(Instruction::New(InstructionType::invoke_args_start, InstructionValueType::VNone, TO_STRING_UINT(invokExpr->MethodGlobalIndex)));
				for (Instruction inst : GenerateMobileInstructions(invokExpr->MethodGlobalIndex, invokExpr->LastIndex - invokExpr->FirstIndex, exprs, invokExpr->FirstIndex, 1))
				{
					vec.push_back(inst);
				}
				vec.push_back(Instruction::New(InstructionType::invoke_args_end, InstructionValueType::VNone, TO_STRING_UINT(invokExpr->MethodGlobalIndex)));
				vec.push_back(Instruction::New(InstructionType::invoke, InstructionValueType::VInt32, TO_STRING_UINT(invokExpr->MethodGlobalIndex)));
				break;
			}
		}
		return vec;
	}

	_STD vector<Instruction> Compiler::GenerateInstructions(_METHODINFO *method)
	{
		_STD vector<Instruction> vec;
		int varUnivIdx = 0;
		for (size_t v = 0; v < method->localvariable_count; v++)
		{
			_FIELDINFO *var = &method->localvariables[v];
			vec.push_back(Instruction::New(InstructionType::type_ref, InstructionValueType::VInt32, TO_STRING_INT(var->fieldType->index)));
			vec.push_back(Instruction::New(InstructionType::locvar, InstructionValueType::VNone, "_local_var_" + TO_STRING_INT(varUnivIdx)));
			varUnivIdx++;
		}
		for (Instruction inst : GenerateMobileInstructions(method->global_index, method->expression_count, method->expressions, 0, 0))
			vec.push_back(inst);
		return vec;
	}

	Instruction Instruction::New(InstructionType type, InstructionValueType vtype, _STD string val)
	{
		return Instruction {
			type,
			vtype,
			val
		};
	}
}
