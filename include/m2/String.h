#pragma once
#include "Meta.h"
#include <vector>
#include <string>

namespace m2 {
	std::vector<std::string> SplitString(std::string str, char delimiter);
	std::string TrimRight(std::string str);
	std::string TrimLeft(std::string str);
	std::string Trim(std::string str);

	template<int N>
	struct CompileTimeString {
		char bytes[N]{};

		// from a "string literal":
		explicit constexpr CompileTimeString(char const(&arr)[N]) {
			for (int i = 0; i < N; ++i) {
				bytes[i] = arr[i];
			}
		}

		constexpr char& operator[](int i) { return bytes[i]; }
		constexpr char const& operator[](int i) const { return bytes[i]; }

		/// String may have an early termination
		[[nodiscard]] constexpr int GetSize() const {
			int r = 0;
			while(r + 1 < N && bytes[r]) { ++r; }
			return r;
		}

		/// Returns -1 if not found
		[[nodiscard]] constexpr int Find(const char c) const {
			for (int i = 0; i < GetSize(); ++i) {
				if (bytes[i] == c) { return i; }
			}
			return -1;
		}
	};
}
