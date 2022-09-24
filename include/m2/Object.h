#ifndef OBJECT_H
#define OBJECT_H

#include "Group.h"
#include "component/Monitor.h"
#include "Sprite.h"
#include "ObjectImpl.h"
#include <m2g/component/Defense.h>
#include <m2g/component/Offense.h>
#include "m2/Component.h"
#include "m2/Fsm.h"
#include "m2/Vec2f.h"
#include "component/Monitor.h"
#include "component/Physique.h"
#include "component/Graphic.h"
#include "component/Light.h"
#include "Pool.hh"
#include <memory>
#include <functional>

namespace m2 {
	using ObjectId = Id;
	using MonitorId = Id;
	using PhysiqueId = Id;
	using GraphicId = Id;
	using LightId = Id;
	using DefenseId = Id;
	using OffenseId = Id;

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

		[[nodiscard]] GroupId group_id() const;
	    [[nodiscard]] MonitorId monitor_id() const;
	    [[nodiscard]] PhysiqueId physique_id() const;
	    [[nodiscard]] GraphicId graphic_id() const;
	    [[nodiscard]] GraphicId terrain_graphic_id() const;
	    [[nodiscard]] LightId light_id() const;
	    [[nodiscard]] DefenseId defense_id() const;
	    [[nodiscard]] OffenseId offense_id() const;

		[[nodiscard]] Group* group() const;
        [[nodiscard]] comp::Monitor& monitor() const;
        [[nodiscard]] comp::Physique& physique() const;
        [[nodiscard]] comp::Graphic& graphic() const;
        [[nodiscard]] comp::Graphic& terrain_graphic() const;
        [[nodiscard]] comp::Light& light() const;
        [[nodiscard]] m2g::comp::Defense& defense() const;
        [[nodiscard]] m2g::comp::Offense& offense() const;

		void set_group(const GroupId& group_id, IndexInGroup group_index);
		comp::Monitor& add_monitor();
		comp::Monitor& add_monitor(const comp::Monitor::Callback& pre_phy);
		comp::Monitor& add_monitor(const comp::Monitor::Callback& pre_phy, const comp::Monitor::Callback& pre_gfx);
		comp::Monitor& add_monitor(const comp::Monitor::Callback& pre_phy, const comp::Monitor::Callback& post_phy, const comp::Monitor::Callback& pre_gfx);
		comp::Monitor& add_monitor(const comp::Monitor::Callback& pre_phy, const comp::Monitor::Callback& post_phy, const comp::Monitor::Callback& pre_gfx, const comp::Monitor::Callback& post_gfx);
        comp::Physique& add_physique();
        comp::Graphic& add_graphic();
		comp::Graphic& add_graphic(const Sprite& sprite);
		comp::Graphic& add_terrain_graphic(const Sprite& sprite);
        comp::Light& add_light();
        m2g::comp::Defense& add_defense();
        m2g::comp::Offense& add_offense();

	private:
		GroupId _group_id{};
	    IndexInGroup _group_index{};
		// TODO use parent-child relationship for objects that needs to be destroyed once their parent is destroyed
		ObjectId _parent_id{};
		std::unique_ptr<Pool<ObjectId,16>> _children;
	    // Components
	    MonitorId _monitor_id{};
	    PhysiqueId _physique_id{};
	    GraphicId _graphic_id{};
	    GraphicId _terrain_graphic_id{};
	    LightId _light_id{};
	    DefenseId _defense_id{};
	    OffenseId _offense_id{};
    };

    std::pair<Object&, ObjectId> create_object(const m2::Vec2f& position);
	std::function<void(void)> create_object_deleter(ObjectId id);
}

#endif
