#ifndef M2_UI_H
#define M2_UI_H

#include "Sprite.h"
#include "Events.h"
#include <SDL2/SDL_ttf.h>
#include <SDL.h>
#include <functional>
#include <vector>
#include <variant>
#include <optional>
#include <string>
#include <list>

namespace m2::ui {
	enum class Action {
		CONTINUE = 0,
		RETURN,
		QUIT
	};

	enum TextAlignment {
		CENTER [[maybe_unused]],
		LEFT,
		RIGHT
	};

	struct Blueprint {
		struct Widget {
			struct NestedUi {
				const Blueprint* ui; // TODO convert to reference
			};
			struct Image {
				m2g::pb::SpriteType initial_sprite;
				std::function<std::pair<Action,std::optional<m2g::pb::SpriteType>>(void)> update_callback;
				std::function<Action(void)> action_callback;
				SDL_Scancode kb_shortcut;
			};
			struct Text {
				std::string_view initial_text;
				TextAlignment alignment;
				std::function<std::pair<Action,std::optional<std::string>>(void)> update_callback;
				std::function<Action(void)> action_callback;
				SDL_Scancode kb_shortcut;
			};
			struct ProgressBar {
				float initial_progress;
				SDL_Color bar_color;
				std::function<float(void)> update_callback;
			};
			struct TextInput {
				std::string_view initial_text;
				std::function<Action(std::stringstream&)> action_callback;
			};
			struct ImageSelection {
				std::vector<m2g::pb::SpriteType> list;
				unsigned initial_selection;
				std::function<Action(m2g::pb::SpriteType selection)> action_callback;
			};
			struct TextSelection {
				std::vector<std::string> list;
				unsigned initial_selection;
				std::function<Action(const std::string& selection)> action_callback;
			};
			struct IntegerSelection {
				int min_value, max_value; /// Values are inclusive
				int initial_value;
				std::function<Action(int value)> action_callback;
			};

			unsigned x{}, y{}, w{}, h{}; // unitless
			unsigned border_width_px{};
			unsigned padding_width_px{};
			SDL_Color background_color{};

			using Variant = std::variant<
				NestedUi,
				Image,
				ProgressBar,
				Text,
				TextInput,
				ImageSelection,
				TextSelection,
				IntegerSelection>;
			Variant variant;
		};

		unsigned w{}, h{}; // unitless
		unsigned border_width_px{};
		SDL_Color background_color{};
		std::vector<Widget> widgets;
	};

	struct State {
		struct Widget {
			const Blueprint::Widget* blueprint;
			SDL_Rect rect_px;

			explicit Widget(const Blueprint::Widget* blueprint);
			virtual ~Widget() = default;
			virtual void update_position(const SDL_Rect& rect_px);
			virtual Action handle_events(Events& events);
			virtual Action update_content();
			virtual void draw();

		protected:
			static SDL_Texture* generate_font_texture(const char* text);
			static SDL_Texture* generate_font_texture(const std::string& text);
			static void draw_text(const SDL_Rect& rect, SDL_Texture& texture, TextAlignment align);
		};
		struct AbstractButton : public Widget {
			SDL_Scancode kb_shortcut;
			bool depressed;

			explicit AbstractButton(const Blueprint::Widget* blueprint);
			Action handle_events(Events& events) final;
		};
		struct Image : public AbstractButton {
			const Sprite* sprite;

			explicit Image(const Blueprint::Widget* blueprint);
			Action update_content() override;
			void draw() override;
		};
		struct Text : public AbstractButton {
			SDL_Texture* font_texture;

			explicit Text(const Blueprint::Widget* blueprint);
			~Text() override;
			Action update_content() override;
			void draw() override;
		};
		struct ProgressBar : public Widget {
			float progress;

			explicit ProgressBar(const Blueprint::Widget* blueprint);
			Action update_content() override;
			void draw() override;
		};
		struct TextInput : public Widget {
			std::stringstream text_input;
			SDL_Texture* font_texture;
			std::string font_texture_str;

			explicit TextInput(const Blueprint::Widget* blueprint);
			~TextInput() override;
			Action handle_events(Events& events) override;
			Action update_content() override;
			void draw() override;
		};
		struct ImageSelection : public Widget {
			unsigned selection;
			bool inc_depressed{};
			bool dec_depressed{};

			explicit ImageSelection(const Blueprint::Widget* blueprint);
			Action handle_events(Events& events) override;
			void draw() override;
		};
		struct TextSelection : public Widget {
			unsigned selection;
			SDL_Texture* font_texture;
			bool inc_depressed{};
			bool dec_depressed{};

			explicit TextSelection(const Blueprint::Widget* blueprint);
			~TextSelection() override;
			Action handle_events(Events& events) override;
			void draw() override;
		};
		struct IntegerSelection : public Widget {
			int value;
			SDL_Texture* font_texture;
			bool inc_depressed{};
			bool dec_depressed{};

			explicit IntegerSelection(const Blueprint::Widget* blueprint);
			~IntegerSelection() override;
			Action handle_events(Events& events) override;
			void draw() override;
		};
		struct NestedUi : public Widget {
			std::unique_ptr<State> ui;

			explicit NestedUi(const Blueprint::Widget* blueprint);
			void update_position(const SDL_Rect& rect_px) final;
			Action handle_events(Events& events) final;
			Action update_content() final;
			void draw() final;
		};

		const Blueprint* blueprint;
		SDL_Rect rect_px;
		std::list<std::unique_ptr<Widget>> widgets;

		State();
        explicit State(const Blueprint* blueprint);
        void update_positions(const SDL_Rect& rect);
        Action handle_events(Events& events);
        Action update_contents();
        void draw();

	private:
		static std::unique_ptr<State::Widget> create_widget_state(const Blueprint::Widget& blueprint);

    public:
        static void draw_background_color(const SDL_Rect& rect, const SDL_Color& color);
        static void draw_border(const SDL_Rect& rect, unsigned border_width_px);
	};

    Action execute_blocking(const Blueprint* blueprint);
	Action execute_blocking(const Blueprint* blueprint, SDL_Rect rect);

    extern const Blueprint console_ui;
}

#endif //M2_UI_H
