#pragma once

#include <vector>
#include <functional>
#include <sstream>
#include <string>

#define LMSCODE_VERSION 1
#define TO_STRING_INT(x) Utils::ToString<int>(x)
#define TO_STRING_UINT(x) Utils::ToString<size_t>(x)

namespace LMS
{
	namespace Utils
	{
		template<typename T>
		inline void AddArrayMember(T *&ar, size_t sz, T mem)
		{
			T *tmp = new T[sz + 1];
			for (int i = 0; i < sz; i++)
				tmp[i] = ar[i];
			tmp[sz] = mem;
			ar = tmp;
			//delete[] tmp;
		}

		template<typename T>
		inline T RandomNext(T min, T max)
		{
			return min + (rand() % (max - min + 1));
		}

		template<typename T>
		inline void Iterate(_STD vector<T> *vec, _STD function<void(T*)> &callback)
		{
			for (size_t sz = 0; sz < vec->size(); sz++)
			{
				if (callback)
					callback(&(*vec)[sz]);
			}
		}

		inline _STD string StringReplace(_STD string source, _STD string from, _STD string to)
		{
			size_t pos = 0;
			while ((pos = source.find(from)) != -1)
			{
				source.replace(pos, from.size(), to);
				pos += to.size();
			}
			return source;
		}

		template<typename T>
		inline _STD string ToString(T val)
		{
			_STD stringstream stream;
			stream << val;
			return stream.str();
		}

		inline _STD string ReadFile(_STD string path, bool *error)
		{
			FILE *handle = fopen(path.c_str(), "rb");
			if (!handle)
			{
				*error = true;
				return "";
			}
			fseek(handle, 0, SEEK_END);
			long len = ftell(handle);
			fseek(handle, 0, SEEK_SET);
			char *contents = (char*)malloc(len + 1);
			fread(contents, len, 1, handle);
			fclose(handle);
			contents[len] = 0;
			return _STD string(contents);
		}
	}
}