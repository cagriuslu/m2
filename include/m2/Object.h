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

	    [[nodiscard]] MonitorID monitor_id() const;
	    [[nodiscard]] PhysiqueID physique_id() const;
	    [[nodiscard]] GraphicID graphic_id() const;
	    [[nodiscard]] GraphicID terrain_graphic_id() const;
	    [[nodiscard]] LightID light_id() const;
	    [[nodiscard]] DefenseID defense_id() const;
	    [[nodiscard]] OffenseID offense_id() const;

        [[nodiscard]] comp::Monitor& monitor() const;
        [[nodiscard]] comp::Physique& physique() const;
        [[nodiscard]] comp::Graphic& graphic() const;
        [[nodiscard]] comp::Graphic& terrain_graphic() const;
        [[nodiscard]] comp::Light& light() const;
        [[nodiscard]] m2g::comp::Defense& defense() const;
        [[nodiscard]] m2g::comp::Offense& offense() const;

        [[nodiscard]] comp::Monitor& add_monitor();
        [[nodiscard]] comp::Physique& add_physique();
        [[nodiscard]] comp::Graphic& add_graphic();
        [[nodiscard]] comp::Graphic& add_terrain_graphic();
        [[nodiscard]] comp::Light& add_light();
        [[nodiscard]] m2g::comp::Defense& add_defense();
        [[nodiscard]] m2g::comp::Offense& add_offense();

	private:
	    // Components
	    MonitorID _monitor_id{};
	    PhysiqueID _physique_id{};
	    GraphicID _graphic_id{};
	    GraphicID _terrain_graphic_id{};
	    LightID _light_id{};
	    DefenseID _defense_id{};
	    OffenseID _offense_id{};
    };

    std::pair<Object&, ObjectID> create_object(const m2::Vec2f& position);
	std::function<void(void)> create_object_deleter(ObjectID id);
}

#endif
