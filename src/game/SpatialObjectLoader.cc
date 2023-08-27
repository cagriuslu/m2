#include <m2/game/SpatialObjectLoader.h>
#include <unordered_set>

void m2::SpatialObjectLoader::move(const m2::RectF &viewport) {
	// TODO this is a very naive, low performance implementation

	std::unordered_set<VecI, VecIHash> loaded_positions;
	// Extract keys of loaded object into set
	transform(_loaded_objs.begin(), _loaded_objs.end(), // Iterate over loaded objects
			std::inserter(loaded_positions, loaded_positions.end()), // Inserter into loaded positions
			[](const auto& kv_pair) { return kv_pair.first; }); // Extract the keys

	// Load everything in viewport
	for (const auto& position : viewport.intersecting_cells()) {
		// Check if load has been called for the position already
		if (_loaded_objs.contains(position)) {
			// Remove from loaded positions
			loaded_positions.erase(position);
		} else {
			// Call load
			_loaded_objs[position] = load(position);
		}
	}

	// Unload leftover positions in loaded_positions
	for (const auto& position : loaded_positions) {
		unload(_loaded_objs[position]);
		_loaded_objs.erase(position);
	}
}
