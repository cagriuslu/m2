#ifndef M2_CHARACTER_H
#define M2_CHARACTER_H

#include <m2g/Interaction.h>
#include "../Component.h"
#include <Item.pb.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace m2 {
	class CharacterBase : public Component {
	public:
		std::function<void(CharacterBase&)> update;
		std::function<void(CharacterBase&, CharacterBase&, m2g::InteractionType)> interact;

		CharacterBase() = default;
		explicit CharacterBase(uint64_t object_id);

		[[nodiscard]] virtual bool has_item(m2g::pb::ItemType item_type) const = 0;
		[[nodiscard]] virtual size_t count_item(m2g::pb::ItemType item_type) const = 0;
		/// Add item
		virtual void add_item(m2g::pb::ItemType item_type) = 0;
		/// Return success
		bool use_item(m2g::pb::ItemType item_type);
		/// Use item if resources allow
		bool use_item_if(m2g::pb::ItemType item_type);
		/// Return success
		virtual bool remove_item(m2g::pb::ItemType item_type) = 0;
		/// Return success
		virtual bool clear_item(m2g::pb::ItemType item_type) = 0;

		[[nodiscard]] virtual bool has_resource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float get_resource(m2g::pb::ResourceType resource_type) const = 0;
		/// Amount can be negative, but total amount never goes negative
		virtual float add_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		/// Return remaining amount
		virtual float remove_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		/// Remove resource if amount allows, return success
		virtual bool remove_resource_if(m2g::pb::ResourceType resource_type, float amount) = 0;
		/// Return success
		virtual bool clear_resource(m2g::pb::ResourceType resource_type) = 0;
		/// Clear resource if amount allows, return success
		virtual bool clear_resource_if(m2g::pb::ResourceType resource_type, float amount) = 0;
	};

	class TinyCharacter : public CharacterBase {
		std::optional<m2g::pb::ItemType> item;
		std::optional<std::pair<m2g::pb::ResourceType,float>> resource;

	public:
		TinyCharacter() = default;
		explicit TinyCharacter(uint64_t object_id);
		[[nodiscard]] bool has_item(m2g::pb::ItemType item_type) const override;
		[[nodiscard]] size_t count_item(m2g::pb::ItemType item_type) const override;
		void add_item(m2g::pb::ItemType item_type) override;
		bool remove_item(m2g::pb::ItemType item_type) override;
		bool clear_item(m2g::pb::ItemType item_type) override;
		[[nodiscard]] bool has_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const override;
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		bool remove_resource_if(m2g::pb::ResourceType resource_type, float amount) override;
		bool clear_resource(m2g::pb::ResourceType resource_type) override;
		bool clear_resource_if(m2g::pb::ResourceType resource_type, float amount) override;
	};

	class FullCharacter : public CharacterBase {
		std::unordered_multiset<m2g::pb::ItemType> items;
		std::unordered_map<m2g::pb::ResourceType, float> resources;

	public:
		FullCharacter() = default;
		explicit FullCharacter(uint64_t object_id);
		[[nodiscard]] bool has_item(m2g::pb::ItemType item_type) const override;
		[[nodiscard]] size_t count_item(m2g::pb::ItemType item_type) const override;
		void add_item(m2g::pb::ItemType item_type) override;
		bool remove_item(m2g::pb::ItemType item_type) override;
		bool clear_item(m2g::pb::ItemType item_type) override;
		[[nodiscard]] bool has_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const override;
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		bool remove_resource_if(m2g::pb::ResourceType resource_type, float amount) override;
		bool clear_resource(m2g::pb::ResourceType resource_type) override;
		bool clear_resource_if(m2g::pb::ResourceType resource_type, float amount) override;
	};

	using CharacterVariant = std::variant<TinyCharacter,FullCharacter>;
	CharacterBase& get_character_base(CharacterVariant& v);
}

#endif //M2_CHARACTER_H
