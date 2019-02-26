#pragma once

#include <vector>

namespace LMS
{
	template<typename T>
	class SubVector : public _STD vector<T*>
	{
	public:
		SubVector(size_t start, size_t end, _STD vector<T> *vec);
	};

	template<typename T>
	inline SubVector<T>::SubVector(size_t start, size_t end, _STD vector<T> *vec)
	{
		if (start >= vec->size() || end > vec->size())
			throw _STD exception("start or end value could not be larger than the origin");
		for (size_t s = start; s < end; s++)
		{
			this->push_back(&(*vec)[s]);
		}
	}
}