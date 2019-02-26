#pragma once

#include "Expressions.h"

#define _CLASSINFO LMS::ClassInfo
#define _FIELDINFO LMS::FieldInfo
#define _METHODINFO LMS::MethodInfo
#define _PARAMETERINFO LMS::ParameterInfo
#define _IMAGEINFO LMS::ImageInfo

namespace LMS
{
	struct MethodInfo;
	struct ClassInfo;
	struct FieldInfo;
	struct ImageInfo;
	struct ParameterInfo;

	struct MethodInfo
	{
		char *name;
		ClassInfo *returntype;
		ParameterInfo *parameters;
		ClassInfo *declaringType;
		size_t parameter_count;
		uint64_t parameter_magic;
		FieldInfo *localvariables;
		size_t localvariable_count;
		ExpressionResult *expressions;
		size_t expression_count;
		size_t global_index;
		int modifiers;
	};
	
	struct ClassInfo
	{
		char *namespaze;
		char *name;
		MethodInfo **methods;
		FieldInfo *fields;
		ImageInfo *image;
		ClassInfo *parent;
		ClassInfo *classes;
		size_t method_count;
		size_t field_count;
		size_t index;
	};

	struct FieldInfo
	{
		char *name;
		ClassInfo *fieldType;
		ClassInfo *declaringType;
		void *value;
		size_t index;
		int modifiers;
	};

	struct ImageInfo
	{
		char *name;
		size_t classCount;
		ClassInfo **classes;
	};

	struct ParameterInfo
	{
		char *name;
		int index;
		ClassInfo *type;
	};
}