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
		std::string name;
		std::function<VSON()> serializer;
		std::function<void(const VSON&)> deserializer;
	};

	inline long register_long(std::vector<MemberDef>& members, const std::string_view& name, long* ref) {
		members.emplace_back(
			std::string(name),
			[ref]() -> m2::VSON {
				return VSON::string(std::to_string(*ref));
			},
			[ref](const VSON& vson) {
				*ref = vson.long_value();
			}
		);
		return 0;
	}

	inline double register_double(std::vector<MemberDef>& members, const std::string_view& name, double* ref) {
		members.emplace_back(
			std::string(name),
			[ref]() -> m2::VSON {
				return VSON::string(std::to_string(*ref));
			},
			[ref](const VSON& vson) {
				*ref = vson.double_value();
			}
		);
		return 0.0;
	}

	inline std::string register_string(std::vector<MemberDef>& members, const std::string_view& name, std::string* ref) {
		members.emplace_back(
			std::string(name),
			[ref]() -> m2::VSON {
				return VSON::string(*ref);
			},
			[ref](const VSON& vson) {
				*ref = vson.string_value();
			}
		);
		return {};
	}

	inline std::vector<long> register_long_array(std::vector<MemberDef>& members, const std::string_view& name, std::vector<long>* ref) {
		members.emplace_back(
			std::string(name),
			[ref]() -> m2::VSON {
				auto vson = VSON::array();
				size_t i = 0;
				for (const auto& l : *ref) {
					vson[i++] = std::to_string(l);
				}
				return vson;
			},
			[ref](const VSON& vson) {
				ref->resize(vson.array_length());
				for (size_t i = 0; i < vson.array_length(); i++) {
					(*ref)[i] = vson[i].long_value();
				}
			}
		);
		return {};
	}

	inline std::vector<double> register_double_array(std::vector<MemberDef>& members, const std::string_view& name, std::vector<double>* ref) {
		members.emplace_back(
			std::string(name),
			[ref]() -> m2::VSON {
				auto vson = VSON::array();
				size_t i = 0;
				for (const auto& f : *ref) {
					vson[i++] = std::to_string(f);
				}
				return vson;
			},
			[ref](const VSON& vson) {
				ref->resize(vson.array_length());
				for (size_t i = 0; i < vson.array_length(); i++) {
					(*ref)[i] = vson[i].double_value();
				}
			}
		);
		return {};
	}

	inline std::vector<std::string> register_string_array(std::vector<MemberDef>& members, const std::string_view& name, std::vector<std::string>* ref) {
		members.emplace_back(
			std::string(name),
			[ref]() -> m2::VSON {
				auto vson = VSON::array();
				size_t i = 0;
				for (const auto &s: *ref) {
					vson[i++] = s;
				}
				return vson;
			},
			[ref](const VSON& vson) {
				ref->resize(vson.array_length());
				for (size_t i = 0; i < vson.array_length(); i++) {
					(*ref)[i] = vson[i].string_value();
				}
			}
		);
		return {};
	}

	template <typename Struct>
	inline std::deque<Struct> register_struct_array(std::vector<MemberDef>& members, const std::string_view& name, std::deque<Struct>* ref) {
		members.emplace_back(
			std::string(name),
			[ref]() -> m2::VSON {
				auto vson = VSON::array();
				size_t i = 0;
				for (const auto& s : *ref) {
					vson[i++] = s.to_vson();
				}
				return vson;
			},
			[ref](const VSON& vson) {
				ref->clear();
				ref->resize(vson.array_length());
				for (size_t i = 0; i < vson.array_length(); i++) {
					(*ref)[i].from_vson(vson[i]);
				}
			}
		);
		return {};
	}

	template <typename Struct>
	inline Struct register_struct(std::vector<MemberDef>& members, const std::string_view& name, Struct* ref) {
		members.emplace_back(
			std::string(name),
			[ref]() -> m2::VSON {
				return ref->to_vson();
			},
			[ref](const VSON& vson) {
				ref->~Struct();
				new (ref) Struct();
				ref->from_vson(vson);
			}
		);
		return {};
	}
}

#define begin_serializable(ClassName)                                                                                  \
	struct ClassName {                                                                                                 \
    private:                                                                                                           \
		std::vector<m2::ser::MemberDef> __members;                                                                     \
    public:                                                                                                            \
		ClassName() = default;                                                                                         \
		ClassName(const ClassName&) = delete;                                                                          \
		ClassName(ClassName&&) = delete;                                                                               \
		ClassName& operator=(const ClassName&) = delete;                                                               \
		ClassName& operator=(ClassName&&) = delete

#define serializable_long(member) \
	long member{m2::ser::register_long(this->__members, #member, &this->member)}

#define serializable_double(member) \
	double member{m2::ser::register_double(this->__members, #member, &this->member)}

#define serializable_string(member) \
	std::string member{m2::ser::register_string(this->__members, #member, &this->member)}

#define serializable_long_array(member) \
	std::vector<long> member{m2::ser::register_long_array(this->__members, #member, &this->member)}

#define serializable_double_array(member) \
	std::vector<double> member{m2::ser::register_double_array(this->__members, #member, &this->member)}

#define serializable_string_array(member) \
	std::vector<std::string> member{m2::ser::register_string_array(this->__members, #member, &this->member)}

#define serializable_struct_array(type, member) \
	std::deque<type> member{m2::ser::register_struct_array<type>(this->__members, #member, &this->member)}

#define serializable_struct(type, member) \
	type member{m2::ser::register_struct<type>(this->__members, #member, &this->member)}

#define end_serializable()                                                                                             \
	public:                                                                                                            \
		inline m2::VSON to_vson() const {                                                                              \
            auto vson = m2::VSON::object();                                                                            \
			for (const auto& member : __members) {                                                                     \
                vson[member.name] = member.serializer();                                                               \
			}                                                                                                          \
			return vson;                                                                                               \
		}                                                                                                              \
                                                                                                                       \
		inline void from_vson(const m2::VSON& vson) {                                                                  \
			for (const auto& member : __members) {                                                                     \
				member.deserializer(vson[member.name]);                                                                \
            }                                                                                                          \
        }                                                                                                              \
	}

#endif //M2_SERIALIZER_H
