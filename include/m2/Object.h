#ifndef OBJECT_H
#define OBJECT_H

#include "component/Monitor.h"
#include "ObjectImpl.h"
#include <m2g/component/Defense.h>
#include <m2g/component/Offense.h>
#include "m2/Component.h"
#include "m2/FSM.h"
#include "m2/Vec2f.h"
#include "component/Monitor.h"
#include "component/Physique.h"
#include "component/Graphic.h"
#include "component/Light.h"
#include <memory>

namespace m2 {
	using ObjectID = ID;
	using MonitorID = ID;
	using PhysiqueID = ID;
	using GraphicID = ID;
	using LightID = ID;
	using DefenseID = ID;
	using OffenseID = ID;

    /// Basis of all objects in the game.
    /// How to decide if a component should reside in Pool or data?
    /// If the component is iterated by the Main Game Loop => Pool
    /// If the component is created and destroyed rapidly => Pool
    /// Else => Data
    struct Object {
        m2::Vec2f position;
        // Components
		MonitorID monitor_id{};
		PhysiqueID physique_id{};
		GraphicID graphic_id{};
		GraphicID terrain_graphic_id{};
		LightID light_id{};
		DefenseID defense_id{};
		OffenseID offense_id{};
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

        [[nodiscard]] comp::Monitor& monitor() const;
        [[nodiscard]] comp::Physique& physique() const;
        [[nodiscard]] comp::Graphic& graphic() const;
        [[nodiscard]] comp::Graphic& terrain_graphic() const;
        [[nodiscard]] comp::Light& light() const;
        [[nodiscard]] m2g::component::Defense& defense() const;
        [[nodiscard]] m2g::component::Offense& offense() const;

        [[nodiscard]] comp::Monitor& add_monitor();
        [[nodiscard]] comp::Physique& add_physique();
        [[nodiscard]] comp::Graphic& add_graphic();
        [[nodiscard]] comp::Graphic& add_terrain_graphic();
        [[nodiscard]] comp::Light& add_light();
        [[nodiscard]] m2g::component::Defense& add_defense();
        [[nodiscard]] m2g::component::Offense& add_offense();
    };

    std::pair<Object&, ObjectID> create_object(const m2::Vec2f& position);
}

#endif
