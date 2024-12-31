#pragma once

inline int ClampIncomePoints(const int ip) { return std::clamp(ip, -10, 89); }
inline int ClampIncomeLevel(const int il) { return std::clamp(il, -10, 30); }

/// Convert income points to income level.
int IncomeLevelFromIncomePoints(int ip);

/// Returns the highest income points for the given level, which is useful while taking a loan.
int HighestIncomePointsOfLevel(int level);
