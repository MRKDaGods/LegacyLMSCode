#include "ClassInfo.h"

namespace LMS
{
	namespace Core
	{
		_CLASSINFO *ClassInfo::New(_STD string name, _STD string namespaze)
		{
			return new _CLASSINFO {
				strcpy(new char[strlen(namespaze.c_str()) + 1], namespaze.c_str()),
				strcpy(new char[strlen(name.c_str()) + 1], name.c_str())
			};
		}
	}
}