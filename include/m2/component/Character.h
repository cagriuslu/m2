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

		virtual void automatic_update() = 0;

		[[nodiscard]] bool has_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] virtual bool has_item(const pb::Item& item) const = 0;
		[[nodiscard]] size_t count_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] virtual size_t count_item(const pb::Item& item) const = 0;
		/// Add item. If the usage type is IMMEDIATE, item will be used
		void add_item(m2g::pb::ItemType item_type);
		virtual void add_item(const pb::Item& item) = 0;
		/// Return success
		bool use_item(m2g::pb::ItemType item_type, float resource_multiplier = 1.0f);
		bool use_item(const pb::Item& item, float resource_multiplier = 1.0f);
		/// Return success
		void remove_item(m2g::pb::ItemType item_type);
		virtual void remove_item(const pb::Item& item) = 0;
		/// Return success
		void clear_item(m2g::pb::ItemType item_type);
		virtual void clear_item(const pb::Item& item) = 0;

		[[nodiscard]] virtual bool has_resource(m2g::pb::ResourceType resource_type) const = 0;
		[[nodiscard]] virtual float get_resource(m2g::pb::ResourceType resource_type) const = 0;
		/// Amount can be negative, but total amount never goes negative
		virtual float add_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		/// Return remaining amount
		virtual float remove_resource(m2g::pb::ResourceType resource_type, float amount) = 0;
		virtual void clear_resource(m2g::pb::ResourceType resource_type) = 0;

	protected:
		static float get_resource_amount(const pb::Resource& resource);
	};

	class TinyCharacter : public CharacterBase {
		std::optional<pb::Item> _item;
		std::optional<std::pair<m2g::pb::ResourceType,float>> _resource;

	public:
		TinyCharacter() = default;
		explicit TinyCharacter(uint64_t object_id);
		void automatic_update() override;
		[[nodiscard]] bool has_item(const pb::Item& item) const override;
		[[nodiscard]] size_t count_item(const pb::Item& item) const override;
		void add_item(const pb::Item& item) override;
		void remove_item(const pb::Item& item) override;
		void clear_item(const pb::Item& item) override;
		[[nodiscard]] bool has_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const override;
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		void clear_resource(m2g::pb::ResourceType resource_type) override;
	};

	class FullCharacter : public CharacterBase {
		std::vector<pb::Item> _items;
		std::array<float, m2g::pb::ResourceType_ARRAYSIZE> _resources{};

	public:
		FullCharacter() = default;
		explicit FullCharacter(uint64_t object_id);
		void automatic_update() override;
		[[nodiscard]] bool has_item(const pb::Item& item) const override;
		[[nodiscard]] size_t count_item(const pb::Item& item) const override;
		void add_item(const pb::Item& item) override;
		void remove_item(const pb::Item& item) override;
		void clear_item(const pb::Item& item) override;
		[[nodiscard]] bool has_resource(m2g::pb::ResourceType resource_type) const override;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const override;
		float add_resource(m2g::pb::ResourceType resource_type, float amount) override;
		float remove_resource(m2g::pb::ResourceType resource_type, float amount) override;
		void clear_resource(m2g::pb::ResourceType resource_type) override;
	};

	using CharacterVariant = std::variant<TinyCharacter,FullCharacter>;
	CharacterBase& get_character_base(CharacterVariant& v);
}

#endif //M2_CHARACTER_H
