#pragma once

#include <vector>
#include <string>

#include "Objects.h"

namespace LMS
{
	namespace Core
	{
		class ClassInfo
		{
		public:
			static _CLASSINFO *New(_STD string name, _STD string namespaze);
		};
	}
}