#pragma once

// Convert income points to income level.
int income_level_from_income_points(int ip);

// Returns the highest income points for the given level, which is useful while taking a loan.
int highest_income_points_of_level(int level);
