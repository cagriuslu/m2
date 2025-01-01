#pragma once
#include <m2/math/RectF.h>
#include <m2/ui/PanelBlueprint.h>

m2::RectF custom_hud_window_ratio();

m2::ui::PanelBlueprint generate_custom_hud_blueprint(int player_count);
