#include <m2/game/SpatialObjectLoader.h>
#include <unordered_set>

void m2::SpatialObjectLoader::move(const m2::RectF &viewport) {
	// TODO this is a very naive, low performance implementation

	// Expand the viewport by 1
	auto expanded_viewport = viewport.expand(1.0f);
	auto expanded_viewport_cells = RectI::from_intersecting_cells(expanded_viewport);

	std::unordered_set<VecI, VecIHash> loaded_positions;
	// Extract keys of loaded object into set
	transform(_loaded_objs.begin(), _loaded_objs.end(), // Iterate over loaded objects
			std::inserter(loaded_positions, loaded_positions.end()), // Inserter into loaded positions
			[](const auto& kv_pair) { return kv_pair.first; }); // Extract the keys

	// Load everything in viewport
	for (auto y = expanded_viewport_cells.y; y < expanded_viewport_cells.y + expanded_viewport_cells.h; ++y) {
		for (auto x = expanded_viewport_cells.x; x < expanded_viewport_cells.x + expanded_viewport_cells.w; ++x) {
			auto position = VecI{x, y};
			// Check if load has been called for the position already
			if (_loaded_objs.contains(position)) {
				// Remove from loaded positions
				loaded_positions.erase(position);
			} else {
				// Call load
				_loaded_objs[position] = load(position);
			}
		}
	}

	// Unload leftover positions in loaded_positions
	for (const auto& position : loaded_positions) {
		auto id = _loaded_objs[position];
		if (id) {
			unload(id);
		}
		_loaded_objs.erase(position);
	}

	// Verticals
	if (_loaded_verticals.empty()) {
		// Iterate over verticals
		for (auto x = expanded_viewport_cells.x; x < expanded_viewport_cells.x + expanded_viewport_cells.w; ++x) {
			// Load vertical
			_loaded_verticals[x] = load_vertical(x);
		}
	} else {
		// Load new verticals left of the loaded_verticals
		for (auto low_x = expanded_viewport_cells.x; low_x < _loaded_verticals.begin()->first; ++low_x) {
			_loaded_verticals[low_x] = load_vertical(low_x);
		}
		// Unload verticals left of expanded_viewport
		for (auto low_x = _loaded_verticals.begin()->first; low_x < expanded_viewport_cells.x; ++low_x) {
			auto id = _loaded_verticals[low_x];
			if (id) {
				unload(id);
			}
			_loaded_verticals.erase(low_x);
		}
		// Load new verticals right of the loaded_verticals
		for (auto high_x = _loaded_verticals.rbegin()->first + 1; high_x < expanded_viewport_cells.x + expanded_viewport_cells.w; ++high_x) {
			_loaded_verticals[high_x] = load_vertical(high_x);
		}
		// Unload verticals right of expanded_viewport
		for (auto high_x = expanded_viewport_cells.x + expanded_viewport_cells.w; high_x < _loaded_verticals.rbegin()->first + 1; ++high_x) {
			auto id = _loaded_verticals[high_x];
			if (id) {
				unload(id);
			}
			_loaded_verticals.erase(high_x);
		}
	}

	// Horizontals
	if (_loaded_horizontals.empty()) {
		// Iterate over horizontals
		for (auto y = expanded_viewport_cells.y; y < expanded_viewport_cells.y + expanded_viewport_cells.h; ++y) {
			// Load vertical
			_loaded_horizontals[y] = load_horizontal(y);
		}
	} else {
		// Load new horizontals above loaded_verticals
		for (auto low_y = expanded_viewport_cells.y; low_y < _loaded_horizontals.begin()->first; ++low_y) {
			_loaded_horizontals[low_y] = load_horizontal(low_y);
		}
		// Unload horizontals above expanded_viewport
		for (auto low_y = _loaded_horizontals.begin()->first; low_y < expanded_viewport_cells.y; ++low_y) {
			auto id = _loaded_horizontals[low_y];
			if (id) {
				unload(id);
			}
			_loaded_horizontals.erase(low_y);
		}
		// Load new horizontals below the loaded_horizontals
		for (auto high_y = _loaded_horizontals.rbegin()->first + 1; high_y < expanded_viewport_cells.y + expanded_viewport_cells.h; ++high_y) {
			_loaded_horizontals[high_y] = load_horizontal(high_y);
		}
		// Unload horizontals below of expanded_viewport
		for (auto high_y = expanded_viewport_cells.y + expanded_viewport_cells.h; high_y < _loaded_horizontals.rbegin()->first + 1; ++high_y) {
			auto id = _loaded_horizontals[high_y];
			if (id) {
				unload(id);
			}
			_loaded_horizontals.erase(high_y);
		}
	}
}

m2::ObjectId m2::SpatialObjectLoader::load(MAYBE const VecI& position) {
	return 0;
}

m2::ObjectId m2::SpatialObjectLoader::load_vertical(MAYBE int x) {
	return 0;
}

m2::ObjectId m2::SpatialObjectLoader::load_horizontal(MAYBE int y) {
	return 0;
}
