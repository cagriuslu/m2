#include <m2/DrawList.h>
#include <m2/Game.h>

bool m2::DrawList::Vec2fComparator::operator()(const VecF& lhs, const VecF& rhs) const {
	return lhs.y < rhs.y;
}

m2::DrawList::ConstIterator& m2::DrawList::ConstIterator::operator++() {
	map_it++;
	return *this;
}

bool m2::DrawList::ConstIterator::operator==(const ConstIterator& other) const {
	return map_it == other.map_it;
}

m2::GraphicId m2::DrawList::ConstIterator::operator*() const {
	return map_it->second.gfx_id;
}

void m2::DrawList::insert(ObjectId id) {
	auto& obj = M2_LEVEL.objects[id];
	auto it = draw_map.insert({obj.position, {id, obj.graphic_id()}});
	id_lookup.insert({id, it});
}

void m2::DrawList::queue_update(ObjectId id, const VecF& pos) {
	update_queue.emplace_back(id, pos);
}

void m2::DrawList::update() {
	for (auto& [id, pos]: update_queue) {
		auto id_lookup_it = id_lookup.find(id);
		if (id_lookup_it != id_lookup.end()) {
			auto draw_item = id_lookup_it->second->second; // Save DrawItem for later
			draw_map.erase(id_lookup_it->second); // Erase item from map
			auto new_it = draw_map.emplace(pos, draw_item); // Add new item with new position
			id_lookup_it->second = new_it; // Store new iterator into id_lookup table
		}
	}
	update_queue.clear();
}

void m2::DrawList::remove(ObjectId id) {
	auto it = id_lookup.find(id);
	if (it != id_lookup.end()) {
		draw_map.erase(it->second);
		id_lookup.erase(it);
	}
}

m2::DrawList::ConstIterator m2::DrawList::begin() const {
	return {draw_map.begin()};
}

m2::DrawList::ConstIterator m2::DrawList::end() const {
	return {draw_map.end()};
}
