#pragma once
#include "m2/common/Meta.h"

namespace m2::thirdparty::video {
	class Cursor {
		void* _cursor{};

		explicit Cursor(void* c) : _cursor(c) {}

	public:
		static expected<Cursor> Create();

		// Copy not allowed
		Cursor(const Cursor& other) = delete;
		Cursor& operator=(const Cursor& other) = delete;
		// Move allowed
		Cursor(Cursor&&) noexcept;
		Cursor& operator=(Cursor&&) noexcept;
		// Destructor
		virtual ~Cursor();

		void Load() const;
	};
}
