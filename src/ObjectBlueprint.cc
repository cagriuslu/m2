#include <m2/ObjectBlueprint.h>
#include <m2/protobuf/Detail.h>

std::vector<m2::ObjectBlueprint> m2::LoadObjectBlueprints(const std::vector<ObjectBlueprint>& objectBlueprintList) {
	const auto objectTypeCount = pb::enum_value_count<m2g::pb::ObjectType>();

	// Fill every object blueprint with the correct ObjectType
	std::vector<ObjectBlueprint> objectBlueprintVector(objectTypeCount);
	for (int i = pb::enum_index(m2g::pb::ObjectType_MIN); i <= pb::enum_index(m2g::pb::ObjectType_MAX); ++i) {
		objectBlueprintVector[i].objectType = pb::enum_value<m2g::pb::ObjectType>(i);
	}

	std::vector<bool> hasEncountered(objectTypeCount);

	for (const auto& objectBlueprint : objectBlueprintList) {
		const auto objectTypeIndex = pb::enum_index(objectBlueprint.objectType);

		// Check if already loaded
		if (hasEncountered[objectTypeIndex]) {
			throw M2_ERROR("ObjectBlueprint has duplicate definition: " + m2::ToString(objectBlueprint.objectType));
		}

		// Load
		hasEncountered[objectTypeIndex] = true;
		objectBlueprintVector[objectTypeIndex] = objectBlueprint;
	}

	return objectBlueprintVector;
}
