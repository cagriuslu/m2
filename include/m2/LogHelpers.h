#pragma once

// This header is separated from Log.h so that the following functions can be included from a header.

namespace m2 {
	// Sets the name of the thread so that it could be used during logging
	void SetThreadNameForLogging(const char* thread_name);
}
