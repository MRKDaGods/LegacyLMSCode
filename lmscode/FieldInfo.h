#pragma once

#include <string>

#include "Objects.h"

namespace LMS
{
	namespace Core
	{
		class FieldInfo
		{
		public:
			static _FIELDINFO New(_STD string name, _CLASSINFO *declaring, _CLASSINFO *type);
		};
	}
}
