#pragma once
#include <m2/RectF.h>
#include <m2/ui/PanelBlueprint.h>

inline m2::RectF custom_hud_window_ratio() { return {0.15f, 0.0f, 0.7f, 0.2f}; }

m2::ui::PanelBlueprint generate_custom_hud_blueprint(int player_count);
