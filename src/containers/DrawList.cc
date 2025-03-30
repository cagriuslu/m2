#include <m2/containers/DrawList.h>
#include <m2/Game.h>

bool m2::DrawList::Vec2fComparator::operator()(const VecF& lhs, const VecF& rhs) const {
	return lhs.y < rhs.y;
}

m2::DrawList::ConstIterator& m2::DrawList::ConstIterator::operator++() {
	mapIt++;
	return *this;
}

bool m2::DrawList::ConstIterator::operator==(const ConstIterator& other) const {
	return mapIt == other.mapIt;
}

m2::GraphicId m2::DrawList::ConstIterator::operator*() const {
	return mapIt->second.gfxId;
}

void m2::DrawList::Insert(ObjectId id) {
	auto& obj = M2_LEVEL.objects[id];
	auto it = drawMap.insert({obj.position, {id, obj.GetGraphicId()}});
	idLookup.insert({id, it});
}

void m2::DrawList::QueueUpdate(ObjectId id, const VecF& pos) {
	updateQueue.emplace_back(id, pos);
}

void m2::DrawList::Update() {
	for (auto& [id, pos]: updateQueue) {
		auto id_lookup_it = idLookup.find(id);
		if (id_lookup_it != idLookup.end()) {
			auto draw_item = id_lookup_it->second->second; // Save DrawItem for later
			drawMap.erase(id_lookup_it->second); // Erase item from map
			auto new_it = drawMap.emplace(pos, draw_item); // Add new item with new position
			id_lookup_it->second = new_it; // Store new iterator into id_lookup table
		}
	}
	updateQueue.clear();
}

void m2::DrawList::Remove(ObjectId id) {
	auto it = idLookup.find(id);
	if (it != idLookup.end()) {
		drawMap.erase(it->second);
		idLookup.erase(it);
	}
}

m2::DrawList::ConstIterator m2::DrawList::begin() const {
	return {drawMap.begin()};
}

m2::DrawList::ConstIterator m2::DrawList::end() const {
	return {drawMap.end()};
}
