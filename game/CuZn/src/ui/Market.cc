#include <cuzn/ui/Market.h>
#include <m2/Game.h>
#include <m2/ui/widget/Image.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

m2::RectF market_window_ratio() {
	return m2::RectF{0.35f, 0.05f, 0.3f, 0.9f};
}

m2::ui::PanelBlueprint generate_market_window() {
	return {
		.w = 34,
		.h = 78,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 1,
				.y = 1,
				.w = 32,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "Market Status" }
			},
			WidgetBlueprint{
				.x = 1,
				.y = 8,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "Cost" }
			},
			WidgetBlueprint{
				.x = 12,
				.y = 8,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "Coal" }
			},
			WidgetBlueprint{
				.x = 23,
				.y = 8,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "Iron" }
			},
			WidgetBlueprint{
				.x = 1,
				.y = 15,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£8" }
			},
			WidgetBlueprint{
				.x = 12,
				.y = 15,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = ImageBlueprint{ .initial_sprite = m2g::pb::COAL_CUBE }
			},
			WidgetBlueprint{
				.x = 1,
				.y = 22,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£7" }
			},
			WidgetBlueprint{
				.x = 13,
				.y = 22,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (1 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 17,
				.y = 22,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (2 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 29,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£6" }
			},
			WidgetBlueprint{
				.x = 13,
				.y = 29,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (3 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 17,
				.y = 29,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (4 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 25,
				.y = 29,
				.w = 6,
				.h = 6,
				.border_width = 0,
				.variant = ImageBlueprint{ .initial_sprite = m2g::pb::IRON_CUBE }
			},
			WidgetBlueprint{
				.x = 1,
				.y = 36,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£5" }
			},
			WidgetBlueprint{
				.x = 13,
				.y = 36,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (5 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 17,
				.y = 36,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (6 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 24,
				.y = 36,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (1 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 28,
				.y = 36,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (2 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 43,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£4" }
			},
			WidgetBlueprint{
				.x = 13,
				.y = 43,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (7 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 17,
				.y = 43,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (8 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 24,
				.y = 43,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (3 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 28,
				.y = 43,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (4 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 50,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£3" }
			},
			WidgetBlueprint{
				.x = 13,
				.y = 50,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (9 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 17,
				.y = 50,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (10 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 24,
				.y = 50,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (5 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 28,
				.y = 50,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (6 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 57,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£2" }
			},
			WidgetBlueprint{
				.x = 13,
				.y = 57,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (11 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 17,
				.y = 57,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (12 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 24,
				.y = 57,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (7 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 28,
				.y = 57,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (8 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 64,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£1" }
			},
			WidgetBlueprint{
				.x = 13,
				.y = 64,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (13 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 17,
				.y = 64,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (14 <= M2G_PROXY.market_coal_count()) {
							img.set_sprite(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 24,
				.y = 64,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (9 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 28,
				.y = 64,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.on_create = [](Image& img) {
						if (10 <= M2G_PROXY.market_iron_count()) {
							img.set_sprite(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 71,
				.w = 32,
				.h = 6,
				.variant = TextBlueprint{
					.text = "Dismiss",
					.kb_shortcut = SDL_SCANCODE_RETURN,
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action();
					}
				}
			}
		}
	};
}
