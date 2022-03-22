#ifndef OBJECT_H
#define OBJECT_H

#include <game/component.hh>
#include <m2/Component.hh>
#include <m2/Automaton.hh>
#include <m2/Cfg.hh>
#include <m2/Vec2F.h>
#include <memory>
#include <stdint.h>

namespace m2 {
	namespace object {
		struct ObjectData {
			virtual ~ObjectData() = default;
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
			std::unique_ptr<m2::object::ObjectData> data_new;

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

			[[nodiscard]] ComponentMonitor& monitor() const;
			[[nodiscard]] ComponentPhysique& physique() const;
			[[nodiscard]] ComponentGraphic& graphic() const;
			[[nodiscard]] ComponentGraphic& terrain_graphic() const;
			[[nodiscard]] ComponentLight& light() const;
			[[nodiscard]] game::component_defense& defense() const;
			[[nodiscard]] game::component_offense& offense() const;

			[[nodiscard]] ComponentMonitor* get_monitor() const;
			[[nodiscard]] ComponentPhysique* get_physique() const;
			[[nodiscard]] ComponentGraphic* get_graphic() const;
			[[nodiscard]] ComponentGraphic* get_terrain_graphic() const;
			[[nodiscard]] ComponentLight* get_light() const;
			[[nodiscard]] game::component_defense* get_defense() const;
			[[nodiscard]] game::component_offense* get_offense() const;

			[[nodiscard]] ComponentMonitor& add_monitor();
			[[nodiscard]] ComponentPhysique& add_physique();
			[[nodiscard]] ComponentGraphic& add_graphic();
			[[nodiscard]] ComponentGraphic& add_terrain_graphic();
			[[nodiscard]] ComponentLight& add_light();
			[[nodiscard]] game::component_defense& add_defense();
			[[nodiscard]] game::component_offense& add_offense();
		};
	}
}

// Different Object types
int ObjectTile_InitFromCfg(m2::object::Object* obj, CfgSpriteIndex spriteIndex, m2::vec2f position);
M2Err ObjectCamera_Init(m2::object::Object* obj);
int ObjectPointer_Init(m2::object::Object* obj);

#endif
