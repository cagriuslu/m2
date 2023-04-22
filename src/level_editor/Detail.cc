#include <m2/level_editor/Detail.h>

void m2::level_editor::detail::shift_placeholders_right(PlaceholderMap& placeholders, Pool<Object>& objects, int x) {
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
		objects[node.mapped()].position.x = static_cast<float>(node.key().x); // Move object
		placeholders.insert(std::move(node)); // Insert back into the map
	}
}

void m2::level_editor::detail::shift_placeholders_down(PlaceholderMap& bg_placeholders, Pool<Object>& objects, int y) {

}

void m2::level_editor::detail::shift_placeholders_right_down(PlaceholderMap& bg_placeholders, Pool<Object>& objects, int x, int y) {

}
