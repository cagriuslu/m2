#include <cuzn/ui/Market.h>
#include <m2/Game.h>
#include <m2/ui/widget/Image.h>

using namespace m2;
using namespace m2::widget;

m2::RectF market_window_ratio() {
	return m2::RectF{0.35f, 0.05f, 0.3f, 0.9f};
}

m2::UiPanelBlueprint generate_market_window() {
	return {
		.name = "MarketPanel",
		.w = 34,
		.h = 78,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1,
				.y = 1,
				.w = 32,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "Market Status" }
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 8,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "Cost" }
			},
			UiWidgetBlueprint{
				.x = 12,
				.y = 8,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "Coal" }
			},
			UiWidgetBlueprint{
				.x = 23,
				.y = 8,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "Iron" }
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 15,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£8" }
			},
			UiWidgetBlueprint{
				.x = 12,
				.y = 15,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = ImageBlueprint{ .initial_sprite = m2g::pb::COAL_CUBE }
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 22,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£7" }
			},
			UiWidgetBlueprint{
				.x = 13,
				.y = 22,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (1 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 17,
				.y = 22,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (2 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 29,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£6" }
			},
			UiWidgetBlueprint{
				.x = 13,
				.y = 29,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (3 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 17,
				.y = 29,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (4 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 25,
				.y = 29,
				.w = 6,
				.h = 6,
				.border_width = 0,
				.variant = ImageBlueprint{ .initial_sprite = m2g::pb::IRON_CUBE }
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 36,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£5" }
			},
			UiWidgetBlueprint{
				.x = 13,
				.y = 36,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (5 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 17,
				.y = 36,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (6 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 24,
				.y = 36,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (1 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 28,
				.y = 36,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (2 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 43,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£4" }
			},
			UiWidgetBlueprint{
				.x = 13,
				.y = 43,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (7 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 17,
				.y = 43,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (8 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 24,
				.y = 43,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (3 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 28,
				.y = 43,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (4 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 50,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£3" }
			},
			UiWidgetBlueprint{
				.x = 13,
				.y = 50,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (9 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 17,
				.y = 50,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (10 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 24,
				.y = 50,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (5 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 28,
				.y = 50,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (6 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 57,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£2" }
			},
			UiWidgetBlueprint{
				.x = 13,
				.y = 57,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (11 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 17,
				.y = 57,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (12 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 24,
				.y = 57,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (7 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 28,
				.y = 57,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (8 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 64,
				.w = 10,
				.h = 6,
				.border_width = 0,
				.variant = TextBlueprint{ .text = "£1" }
			},
			UiWidgetBlueprint{
				.x = 13,
				.y = 64,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (13 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 17,
				.y = 64,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (14 <= M2G_PROXY.market_coal_count()) {
							img.SetSpriteType(m2g::pb::COAL_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 24,
				.y = 64,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (9 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 28,
				.y = 64,
				.w = 4,
				.h = 6,
				.variant = ImageBlueprint{
					.onCreate = [](Image& img) {
						if (10 <= M2G_PROXY.market_iron_count()) {
							img.SetSpriteType(m2g::pb::IRON_CUBE);
						}
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 71,
				.w = 32,
				.h = 6,
				.variant = TextBlueprint{
					.text = "Dismiss",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction();
					}
				}
			}
		}
	};
}
