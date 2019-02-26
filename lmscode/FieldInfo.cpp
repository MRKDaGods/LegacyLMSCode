#include "FieldInfo.h"

namespace LMS
{
	namespace Core
	{
		_FIELDINFO FieldInfo::New(_STD string name, _CLASSINFO *declaring, _CLASSINFO *type)
		{
			return _FIELDINFO {
				strcpy(new char[strlen(name.c_str()) + 1], name.c_str()),
				type,
				declaring,
				0,
				declaring ? declaring->field_count : -1
			};
		}
	}
}
