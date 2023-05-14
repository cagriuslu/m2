#ifndef M2_META_H
#define M2_META_H

namespace m2 {
	template <typename InputIt, typename OutputIt, typename UnaryPredicate, typename UnaryOperation>
	OutputIt transform_copy_if(InputIt first, InputIt last, OutputIt destination, UnaryPredicate predicate, UnaryOperation operation) {
		for (; first != last; ++first) {
			if (predicate(*first)) {
				*destination = operation(*first);
				++destination;
			}
		}
		return destination;
	}
}

#endif //M2_META_H
