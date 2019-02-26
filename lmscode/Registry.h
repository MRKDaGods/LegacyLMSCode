#pragma once

#include <string>
#include <vector>
#include <map>

#include "Objects.h"
#include "ClassInfo.h"
#include "FieldInfo.h"
#include "Utils.h"
#include "Tokens.h"

namespace LMS
{
	namespace Core
	{
		static _STD map<char, int> *CHAR_MAPPINGS = new _STD map<char, int>();

		class Registry
		{
		private:
			static _STD vector<_CLASSINFO*> ms_Classes;
		public:
			static void Clean();
			static _IMAGEINFO SubmitImage(_STD string name);
			static _CLASSINFO *FindClass(_STD string name, _STD string namespaze);
			static _CLASSINFO *RegisterClass(_STD string name, _STD string namespaze);
			static _FIELDINFO *FindField(_CLASSINFO *declaring, _STD string name);
			static _FIELDINFO *RegisterField(_CLASSINFO *declaring, _STD string name, _CLASSINFO *type);
			static _METHODINFO *FindMethod(_CLASSINFO *declaring, _STD string name, uint64_t paramMagic);
			static _METHODINFO *RegisterMethod(_CLASSINFO *declaring, _CLASSINFO *returntype, _STD string name, uint64_t pmagic);
			static _FIELDINFO *RegisterLocalVariable(_METHODINFO *method, _CLASSINFO *type, _STD string name);
			static _FIELDINFO *FindLocalVariable(_METHODINFO *method, _STD string name);
		};

		inline uint64_t CalculateParamMagic(_STD vector<ParameterToken> *params)
		{
			uint64_t t = 0;
			for (int i = 0; i < params->size(); i++)
			{
				ParameterToken *tk = &(*params)[i];
				t |= tk->Pos;
				for (char c : tk->TypeName)
				{
					int mapped = -1;
					auto fx = CHAR_MAPPINGS->find(c);
					if (fx == CHAR_MAPPINGS->end())
					{
						mapped = Utils::RandomNext<int>(0, 999999);
						CHAR_MAPPINGS->insert(_STD make_pair(c, mapped));
					}
					else mapped = fx->second;
					t |= (c * mapped) ^ 2 * 7 / 3 << 2;
				}
			}
			return t;
		}

		inline uint64_t CalculateParamMagic(_STD vector<_PARAMETERINFO> *params)
		{
			uint64_t t = 0;
			for (int i = 0; i < params->size(); i++)
			{
				_PARAMETERINFO *tk = &(*params)[i];
				t |= tk->index;
				for (char c : _STD string(tk->type->name))
				{
					int mapped = -1;
					auto fx = CHAR_MAPPINGS->find(c);
					if (fx == CHAR_MAPPINGS->end())
					{
						mapped = Utils::RandomNext<int>(0, 999999);
						CHAR_MAPPINGS->insert(_STD make_pair(c, mapped));
					}
					else mapped = fx->second;
					t |= (c * mapped) ^ 2 * 7 / 3 << 2;
				}
			}
			return t;
		}

		inline uint64_t CalculateParamMagic(_METHODINFO *method)
		{
			uint64_t t = 0;
			for (int i = 0; i < method->parameter_count; i++)
			{
				_PARAMETERINFO *tk = &method->parameters[i];
				t |= tk->index;
				for (char c : _STD string(tk->type->name))
				{
					int mapped = -1;
					auto fx = CHAR_MAPPINGS->find(c);
					if (fx == CHAR_MAPPINGS->end())
					{
						mapped = Utils::RandomNext<int>(0, 999999);
						CHAR_MAPPINGS->insert(_STD make_pair(c, mapped));
					}
					else mapped = fx->second;
					t |= (c * mapped) ^ 2 * 7 / 3 << 2;
				}
			}
			return t;
		}
	}
}