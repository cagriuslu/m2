#pragma once
#include <m2/math/RectF.h>
#include <m2/ui/PanelBlueprint.h>

m2::RectF status_bar_window_ratio();

m2::ui::PanelBlueprint generate_status_bar_blueprint(int player_count);
