#pragma once
#include <m2/FileSystem.h>
#include <m2/protobuf/Detail.h>
#include <google/protobuf/message.h>
#include <filesystem>
#include <type_traits>

namespace m2::pb {
	/// PersistentObject allows changes made to a protobuf object to be persisted on the file system after each change.
	/// A cached version is kept in memory for quick access.
	template <typename PbObjectType>
	class PersistentObject {
		static_assert(std::is_base_of_v<::google::protobuf::Message, PbObjectType>, "PbObjectType must be a Protobuf Message");
		std::filesystem::path _path;
		PbObjectType _cache;

		PersistentObject(std::filesystem::path path, PbObjectType&& obj) : _path(std::move(path)), _cache(std::move(obj)) {}

	public:
		static expected<PersistentObject> LoadFile(std::filesystem::path path) {
			if (auto expectMessage = json_file_to_message<PbObjectType>(path)) {
				return PersistentObject{std::move(path), std::move(*expectMessage)};
			}
			// Ignore error, try to create new.
			return CreateFile(std::move(path));
		}
		static expected<PersistentObject> CreateFile(std::filesystem::path path) {
			if (auto expectNew = WriteToFile("", path)) {
				return PersistentObject{std::move(path), {}};
			} else {
				return make_unexpected(expectNew.error());
			}
		}

		// Accessors

		[[nodiscard]] const std::filesystem::path& Path() const { return _path; }
		[[nodiscard]] const PbObjectType& Cache() const { return _cache; }

		// Modifiers

		void_expected Mutate(const std::function<void(PbObjectType&)>& mutator) {
			mutator(_cache);
			return message_to_json_file(_cache, _path);
		}
	};

}
