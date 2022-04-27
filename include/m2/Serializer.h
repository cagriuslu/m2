#ifndef M2_SERIALIZER_H
#define M2_SERIALIZER_H

#include "VSON.hh"
#include <functional>
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <deque>
#include <type_traits>

namespace m2::ser {

	struct MemberDef {
		std::string_view name;
		std::function<VSON()> serializer;
	};

	inline long register_long(std::vector<MemberDef>& members, const std::string_view& name, const long* ref) {
		members.emplace_back(name, [ref]() -> m2::VSON {
			return VSON::string(std::to_string(*ref));
		});
		return 0;
	}

	inline double register_double(std::vector<MemberDef>& members, const std::string_view& name, const double* ref) {
		members.emplace_back(name, [ref]() -> m2::VSON {
			return VSON::string(std::to_string(*ref));
		});
		return 0.0;
	}

	inline std::string register_string(std::vector<MemberDef>& members, const std::string_view& name, const std::string* ref) {
		members.emplace_back(name, [ref]() -> m2::VSON {
			return VSON::string(*ref);
		});
		return {};
	}

	inline std::vector<long> register_long_array(std::vector<MemberDef>& members, const std::string_view& name, const std::vector<long>* ref) {
		members.emplace_back(name, [ref]() -> m2::VSON {
			auto vson = VSON::array();
			size_t i = 0;
			for (const auto& l : *ref) {
				vson[i++] = std::to_string(l);
			}
			return vson;
		});
		return {};
	}

	inline std::vector<double> register_double_array(std::vector<MemberDef>& members, const std::string_view& name, const std::vector<double>* ref) {
		members.emplace_back(name, [ref]() -> m2::VSON {
			auto vson = VSON::array();
			size_t i = 0;
			for (const auto& f : *ref) {
				vson[i++] = std::to_string(f);
			}
			return vson;
		});
		return {};
	}

	inline std::vector<std::string> register_string_array(std::vector<MemberDef>& members, const std::string_view& name, const std::vector<std::string>* ref) {
		members.emplace_back(name, [ref]() -> m2::VSON {
			auto vson = VSON::array();
			size_t i = 0;
			for (const auto& s : *ref) {
				vson[i++] = s;
			}
			return vson;
		});
		return {};
	}

	template <typename Struct>
	inline std::deque<Struct> register_struct_array(std::vector<MemberDef>& members, const std::string_view& name, const std::deque<Struct>* ref) {
		members.emplace_back(name, [ref]() -> m2::VSON {
			auto vson = VSON::array();
			size_t i = 0;
			for (const auto& s : *ref) {
				vson[i++] = s.serialize();
			}
			return vson;
		});
		return {};
	}

	template <typename Struct>
	inline Struct register_struct(std::vector<MemberDef>& members, const std::string_view& name, const Struct* ref) {
		members.emplace_back(name, [ref]() -> m2::VSON {
			return ref->serialize();
		});
		return {};
	}
}

#define begin_serializable(ClassName)                                                                                  \
	struct ClassName {                                                                                                 \
    private:                                                                                                           \
		std::vector<m2::ser::MemberDef> _members;                                                                      \
    public:                                                                                                            \
		ClassName() = default;                                                                                         \
		ClassName(const ClassName&) = delete;                                                                          \
		ClassName(ClassName&&) = delete;                                                                               \
		ClassName& operator=(const ClassName&) = delete;                                                               \
		ClassName& operator=(ClassName&&) = delete

#define serializable_long(member) \
	long member{m2::ser::register_long(this->_members, #member, &this->member)}

#define serializable_double(member) \
	double member{m2::ser::register_double(this->_members, #member, &this->member)}

#define serializable_string(member) \
	std::string member{m2::ser::register_string(this->_members, #member, &this->member)}

#define serializable_long_array(member) \
	std::vector<long> member{m2::ser::register_long_array(this->_members, #member, &this->member)}

#define serializable_double_array(member) \
	std::vector<double> member{m2::ser::register_double_array(this->_members, #member, &this->member)}

#define serializable_string_array(member) \
	std::vector<std::string> member{m2::ser::register_string_array(this->_members, #member, &this->member)}

#define serializable_struct_array(type, member) \
	std::deque<type> member{m2::ser::register_struct_array<type>(this->_members, #member, &this->member)}

#define serializable_struct(type, member) \
	type member{m2::ser::register_struct<type>(this->_members, #member, &this->member)}

#define end_serializable()                                                                                             \
		inline m2::VSON serialize() const {                                                                            \
            auto vson = m2::VSON::object();                                                                            \
			for (const auto& member : _members) {                                                                      \
                vson[member.name] = member.serializer();                                                               \
			}                                                                                                          \
			return vson;                                                                                               \
		}                                                                                                              \
	}

#endif //M2_SERIALIZER_H
