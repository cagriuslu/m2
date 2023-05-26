#pragma once
#include <m2/Object.h>
#include <m2/Pool.hh>
#include <m2/VecI.h>

namespace m2::level_editor {
	constexpr std::string_view paint_button_label = "PAINT";
	constexpr std::string_view place_button_label = "PLACE";

	using BackgroundPlaceholderMap = std::unordered_map<VecI, std::pair<Id, m2g::pb::SpriteType>, Vec2iHash>;
	using ForegroundPlaceholderMap = std::unordered_map<VecI, std::pair<Id, m2::pb::LevelObject>, Vec2iHash>;

	/// Shifts placeholders between [x1, x2] and [y1, y2] by (x_shift_count, y_shift_count)
	template <typename PlaceholderMapType>
	void shift_placeholders(PlaceholderMapType& placeholders, Pool<Object>& objects, int x1, int x2, int y1, int y2, unsigned x_shift_count, unsigned y_shift_count) {
		static_assert(std::is_same<std::remove_cvref_t<PlaceholderMapType>, BackgroundPlaceholderMap>() ||
				std::is_same<std::remove_cvref_t<PlaceholderMapType>, ForegroundPlaceholderMap>(), "Given type is not a known placeholder map");
		// Gather the placeholders that need to be moved
		std::vector<VecI> placeholders_to_move;
		transform_copy_if(placeholders.begin(), placeholders.end(), std::back_inserter(placeholders_to_move),
				[=](const typename PlaceholderMapType::value_type& kv) {
					const auto& pos = kv.first;
					return x1 <= pos.x && pos.x <= x2 && y1 <= pos.y && pos.y <= y2;
				},
				[](const typename PlaceholderMapType::value_type& kv) { return kv.first; });

		// Sort bg placeholders from right to left, because unordered_map does not allow multiple values in the same position
		std::sort(placeholders_to_move.begin(), placeholders_to_move.end(), Vec2iCompareBottomRightToTopLeft{});

		// Move the placeholders
		for (const auto& ph_position : placeholders_to_move) {
			auto node = placeholders.extract(ph_position); // Extract node
			// Move node
			node.key().x += x_shift_count;
			node.key().y += y_shift_count;
			// Move object
			objects[node.mapped().first].position.x += x_shift_count;
			objects[node.mapped().first].position.y += y_shift_count;
			// Move LevelObject
			if constexpr (std::is_same<std::remove_cvref_t<PlaceholderMapType>, ForegroundPlaceholderMap>()) {
				node.mapped().second.mutable_position()->set_x(node.key().x);
				node.mapped().second.mutable_position()->set_y(node.key().y);
			}
			placeholders.insert(std::move(node)); // Insert back into the map
		}
	}
}
