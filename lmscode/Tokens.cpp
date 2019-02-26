#include "Tokens.h"

namespace LMS
{
	void LMS::operator|=(ModifierType &rhs, ModifierType lhs)
	{
		rhs = (ModifierType)((int)rhs | (int)lhs);
	}
}