#pragma once

#include <stdio.h>

namespace sp{

void raiseError(const char *const msg) noexcept{
	fputs(msg, stderr);
	exit(1);
}

} // END OF NAMESPACE
