#ifndef OBJECT_H
#define OBJECT_H

#include "Group.h"
#include "Sprite.h"
#include "ObjectImpl.h"
#include <m2g/component/Defense.h>
#include <m2g/component/Offense.h>
#include "m2/Component.h"
#include "m2/Fsm.h"
#include "m2/Vec2f.h"
#include "component/Physique.h"
#include "component/Graphic.h"
#include "component/Light.h"
#include "component/Character.h"
#include "Pool.hh"
#include <memory>
#include <functional>

namespace m2 {
	using ObjectId = Id;
	using PhysiqueId = Id;
	using GraphicId = Id;
	using LightId = Id;
	using DefenseId = Id;
	using OffenseId = Id;
	using CharacterId = Id;

    /// Basis of all objects in the game.
    /// How to decide if a component should reside in Pool or data?
    /// If the component is iterated by the Main Game Loop => Pool
    /// If the component is created and destroyed rapidly => Pool
    /// Else => Data
    struct Object final {
        m2::Vec2f position;
        // Data
        std::unique_ptr<ObjectImpl> impl;

        Object() = default;
        explicit Object(const m2::Vec2f& position);
        // Copy not allowed
        Object(const Object& other) = delete;
        Object& operator=(const Object& other) = delete;
        // Move constructors
        Object(Object&& other) noexcept;
        Object& operator=(Object&& other) noexcept;
        // Destructor
        ~Object();

		[[nodiscard]] ObjectId id() const;
		[[nodiscard]] GroupId group_id() const;
	    [[nodiscard]] PhysiqueId physique_id() const;
	    [[nodiscard]] GraphicId graphic_id() const;
	    [[nodiscard]] GraphicId terrain_graphic_id() const;
	    [[nodiscard]] LightId light_id() const;
	    [[nodiscard]] DefenseId defense_id() const;
	    [[nodiscard]] OffenseId offense_id() const;
		[[nodiscard]] CharacterId character_id() const;

		[[nodiscard]] Group* group() const;
        [[nodiscard]] Physique& physique() const;
        [[nodiscard]] Graphic& graphic() const;
        [[nodiscard]] Graphic& terrain_graphic() const;
        [[nodiscard]] Light& light() const;
        [[nodiscard]] m2g::Defense& defense() const;
        [[nodiscard]] m2g::Offense& offense() const;
		[[nodiscard]] Character& character() const;

		void set_group(const GroupId& group_id, IndexInGroup group_index);

        Physique& add_physique();
        Graphic& add_graphic();
		Graphic& add_graphic(const Sprite& sprite);
		Graphic& add_terrain_graphic(const Sprite& sprite);
        Light& add_light();
        m2g::Defense& add_defense();
        m2g::Offense& add_offense();
		Character& add_character();

	private:
		GroupId _group_id{};
	    IndexInGroup _index_in_group{};
	    // Components
	    PhysiqueId _physique_id{};
	    GraphicId _graphic_id{};
	    GraphicId _terrain_graphic_id{};
	    LightId _light_id{};
	    DefenseId _defense_id{};
	    OffenseId _offense_id{};
		CharacterId _character_id{};
    };

    std::pair<Object&, ObjectId> create_object(const m2::Vec2f& position);
	std::function<void(void)> create_object_deleter(ObjectId id);
}

#endif
