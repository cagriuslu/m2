#include <m2g/Object.h>
#include <mine/object/Dwarf.h>

m2::VoidValue m2g::fg_object_loader(m2::Object& obj, pb::ObjectType object_type) {
	switch (object_type) {
		case pb::ObjectType::DWARF:
			return create_dwarf(obj);
		default:
			return m2::failure("Invalid sprite index");
	}
}
