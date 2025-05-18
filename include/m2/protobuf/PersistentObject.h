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
		bool _isDirty{};

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

		[[nodiscard]] const std::filesystem::path& GetPath() const { return _path; }
		[[nodiscard]] const PbObjectType& GetCache() const { return _cache; }
		[[nodiscard]] bool IsDirty() const { return _isDirty; }

		// Modifiers

		void Mutate(const std::function<void(PbObjectType&)>& mutator) {
			mutator(_cache);
			_isDirty = true;
		}
		void_expected MutateAndSave(const std::function<void(PbObjectType&)>& mutator) {
			Mutate(mutator);
			return Save();
		}
		void_expected Save() {
			const auto result = message_to_json_file(_cache, _path);
			if (result) {
				_isDirty = false;
			}
			return result;
		}
	};

}
