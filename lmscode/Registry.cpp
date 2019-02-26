#include "Registry.h"

_STD vector<_CLASSINFO*> LMS::Core::Registry::ms_Classes;
size_t global_method_index;

namespace LMS
{
	namespace Core
	{
		void Registry::Clean()
		{
			ms_Classes.clear();
			global_method_index = 0;
		}

		_IMAGEINFO Registry::SubmitImage(_STD string name)
		{
			return _IMAGEINFO {
				strcpy(new char[name.size() + 1], name.c_str()),
				ms_Classes.size(),
				ms_Classes.data()
			};
		}

		_CLASSINFO *Registry::FindClass(_STD string name, _STD string namespaze)
		{
			for (_STD vector<_CLASSINFO*>::iterator iter = ms_Classes.begin(); iter != ms_Classes.end(); iter++)
			{
				_CLASSINFO *info = *iter;
				if (!strcmp(info->name, name.c_str()) && !strcmp(info->namespaze, namespaze.c_str()))
					return info;
			}
			return 0;
		}

		_CLASSINFO *Registry::RegisterClass(_STD string name, _STD string namespaze)
		{
			_CLASSINFO *info = ClassInfo::New(name, namespaze);
			info->index = ms_Classes.size();
			ms_Classes.push_back(info);
			return info;
		}

		_FIELDINFO *Registry::FindField(_CLASSINFO *declaring, _STD string name)
		{
			for (int i = 0; i < declaring->field_count; i++)
			{
				_FIELDINFO *info = &declaring->fields[i];
				if (!strcmp(info->name, name.c_str()))
					return info;
			}
			return 0;
		}

		_FIELDINFO *Registry::RegisterField(_CLASSINFO *declaring, _STD string name, _CLASSINFO *type)
		{
			Utils::AddArrayMember<_FIELDINFO>(declaring->fields, declaring->field_count, FieldInfo::New(name, declaring, type));
			declaring->field_count++;
			return &declaring->fields[declaring->field_count - 1];
		}

		_METHODINFO *Registry::FindMethod(_CLASSINFO *declaring, _STD string name, uint64_t paramMagic)
		{
			for (int i = 0; i < declaring->method_count; i++)
			{
				_METHODINFO *info = declaring->methods[i];
				if (!strcmp(info->name, name.c_str()) && (paramMagic == -1 || info->parameter_magic == paramMagic))
					return info;
			}
			return 0;
		}

		_METHODINFO *Registry::RegisterMethod(_CLASSINFO *declaring, _CLASSINFO *returntype, _STD string name, uint64_t pmagic)
		{
			_METHODINFO *info = new _METHODINFO {
				strcpy(new char[name.size() + 1], name.c_str()),
				returntype,
				0,
				declaring,
				0,
				pmagic
			};
			info->global_index = global_method_index;
			global_method_index++;
			Utils::AddArrayMember<_METHODINFO*>(declaring->methods, declaring->method_count, info);
			declaring->method_count++;
			return info;
		}

		_FIELDINFO *Registry::RegisterLocalVariable(_METHODINFO *method, _CLASSINFO *type, _STD string name)
		{
			Utils::AddArrayMember<_FIELDINFO>(method->localvariables, method->localvariable_count, FieldInfo::New(name, 0, type));
			method->localvariable_count++;
			return &method->localvariables[method->localvariable_count - 1];
		}

		_FIELDINFO *Registry::FindLocalVariable(_METHODINFO *method, _STD string name)
		{
			for (size_t v = 0; v < method->localvariable_count; v++)
			{
				_FIELDINFO *info = &method->localvariables[v];
				if (!strcmp(info->name, name.c_str()))
					return info;
			}
			return 0;
		}
	}
}