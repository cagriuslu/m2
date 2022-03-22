#ifndef OBJECT_H
#define OBJECT_H

#include "game/component.hh"
#include "m2/Component.hh"
#include "m2/Automaton.hh"
#include "m2/Cfg.hh"
#include "m2/Vec2F.h"
#include <memory>

namespace m2::object {
    struct Data {
        virtual ~Data() = default;
    };

    /// Basis of all objects in the game.
    /// How to decide if a component should reside in Pool or data?
    /// If the component is iterated by the Main Game Loop => Pool
    /// If the component is created and destroyed rapidly => Pool
    /// Else => Data
    struct Object {
        m2::vec2f position;
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
        std::unique_ptr<Data> data_new;

        Object() = default;
        explicit Object(const m2::vec2f& position);
        // Copy not allowed
        Object(const Object& other) = delete;
        Object& operator=(const Object& other) = delete;
        // Move constructors
        Object(Object&& other) noexcept;
        Object& operator=(Object&& other) noexcept;
        // Destructor
        ~Object();

        [[nodiscard]] Monitor& monitor() const;
        [[nodiscard]] Physique& physique() const;
        [[nodiscard]] Graphic& graphic() const;
        [[nodiscard]] Graphic& terrain_graphic() const;
        [[nodiscard]] Light& light() const;
        [[nodiscard]] game::Defense& defense() const;
        [[nodiscard]] game::Offense& offense() const;

        [[nodiscard]] Monitor& add_monitor();
        [[nodiscard]] Physique& add_physique();
        [[nodiscard]] Graphic& add_graphic();
        [[nodiscard]] Graphic& add_terrain_graphic();
        [[nodiscard]] Light& add_light();
        [[nodiscard]] game::Defense& add_defense();
        [[nodiscard]] game::Offense& add_offense();
    };

    std::pair<Object&, ID> create(const m2::vec2f& position);
}

#endif
