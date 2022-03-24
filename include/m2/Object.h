#ifndef OBJECT_H
#define OBJECT_H

#include "component/Monitor.h"
#include "ObjectImpl.h"
#include "impl/public/Component.hh"
#include "m2/Component.h"
#include "m2/FSM.h"
#include "m2/Cfg.hh"
#include "m2/Vec2f.h"
#include "component/Monitor.h"
#include "component/Physique.h"
#include "component/Graphic.h"
#include "component/Light.h"
#include <memory>

using namespace m2;

namespace m2 {
    /// Basis of all objects in the game.
    /// How to decide if a component should reside in Pool or data?
    /// If the component is iterated by the Main Game Loop => Pool
    /// If the component is created and destroyed rapidly => Pool
    /// Else => Data
    struct Object {
        m2::Vec2f position;
        // Components
        ID monitor_id;
        ID physique_id;
        ID graphic_id;
        ID terrain_graphic_id;
        ID light_id;
        ID defense_id;
        ID offense_id;
        // Extra
        void *data;
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

        [[nodiscard]] component::Monitor& monitor() const;
        [[nodiscard]] component::Physique& physique() const;
        [[nodiscard]] component::Graphic& graphic() const;
        [[nodiscard]] component::Graphic& terrain_graphic() const;
        [[nodiscard]] component::Light& light() const;
        [[nodiscard]] impl::Defense& defense() const;
        [[nodiscard]] impl::Offense& offense() const;

        [[nodiscard]] component::Monitor& add_monitor();
        [[nodiscard]] component::Physique& add_physique();
        [[nodiscard]] component::Graphic& add_graphic();
        [[nodiscard]] component::Graphic& add_terrain_graphic();
        [[nodiscard]] component::Light& add_light();
        [[nodiscard]] impl::Defense& add_defense();
        [[nodiscard]] impl::Offense& add_offense();
    };

    std::pair<Object&, ID> create_object(const m2::Vec2f& position);
}

#endif
