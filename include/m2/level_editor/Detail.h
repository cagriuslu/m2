#ifndef M2_LEVELEDITOR_DETAIL_H
#define M2_LEVELEDITOR_DETAIL_H

#include <m2/Object.h>
#include <m2/Pool.hh>
#include <m2/Vec2i.h>

namespace m2::level_editor {
	using BackgroundPlaceholderMap = std::unordered_map<Vec2i, std::pair<Id, m2g::pb::SpriteType>, Vec2iHash>;
	using ForegroundPlaceholderMap = std::unordered_map<Vec2i, std::pair<Id, m2::pb::LevelObject>, Vec2iHash>;

	template <typename PlaceholderMapType>
	void shift_placeholders_right(PlaceholderMapType& placeholders, Pool<Object>& objects, int x) {
		// Gather the placeholders that need to be moved
		std::vector<Vec2i> placeholders_to_move;
		for (const auto& [ph_position, id] : placeholders) {
			if (x <= ph_position.x) {
				placeholders_to_move.emplace_back(ph_position); // Add to list
			}
		}
		// Sort bg placeholders from right to left
		std::sort(placeholders_to_move.begin(), placeholders_to_move.end(), Vec2iCompareRightToLeft{});
		// Move the bg placeholders
		for (const auto& ph_position : placeholders_to_move) {
			auto node = placeholders.extract(ph_position); // Extract node
			node.key().x++; // Move node
			objects[node.mapped().first].position.x = static_cast<float>(node.key().x); // Move object
			placeholders.insert(std::move(node)); // Insert back into the map
		}
	}

	template <typename PlaceholderMapType>
	void shift_placeholders_down(PlaceholderMapType& placeholders, Pool<Object>& objects, int y) {
		// Gather the placeholders that need to be moved
		std::vector<Vec2i> placeholders_to_move;
		for (const auto& [ph_position, id] : placeholders) {
			if (y <= ph_position.y) {
				placeholders_to_move.emplace_back(ph_position); // Add to list
			}
		}
		// Sort bg placeholders from right to left
		std::sort(placeholders_to_move.begin(), placeholders_to_move.end(), Vec2iCompareBottomToTop{});
		// Move the bg placeholders
		for (const auto& ph_position : placeholders_to_move) {
			auto node = placeholders.extract(ph_position); // Extract node
			node.key().y++; // Move node
			objects[node.mapped().first].position.y = static_cast<float>(node.key().y); // Move object
			placeholders.insert(std::move(node)); // Insert back into the map
		}
	}

	template <typename PlaceholderMapType>
	void shift_placeholders_right_down(PlaceholderMapType& placeholders, Pool<Object>& objects, int x, int y) {
		// Gather the placeholders that need to be moved
		std::vector<Vec2i> placeholders_to_move;
		for (const auto& [ph_position, id] : placeholders) {
			if (x <= ph_position.x && y <= ph_position.y) {
				placeholders_to_move.emplace_back(ph_position); // Add to list
			}
		}
		// Sort bg placeholders from right bottom to left top
		std::sort(placeholders_to_move.begin(), placeholders_to_move.end(), Vec2iCompareRightBottomToLeftTop{});
		// Move the bg placeholders
		for (const auto& ph_position : placeholders_to_move) {
			auto node = placeholders.extract(ph_position); // Extract node
			node.key().x++; // Move node
			node.key().y++; // Move node
			objects[node.mapped().first].position.x = static_cast<float>(node.key().x); // Move object
			objects[node.mapped().first].position.y = static_cast<float>(node.key().y); // Move object
			placeholders.insert(std::move(node)); // Insert back into the map
		}
	}
}

#endif //M2_LEVELEDITOR_DETAIL_H
