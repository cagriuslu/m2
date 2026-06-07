#pragma once
#include <m2/math/VecF.h>
#include <cstdint>

const m2::VecF gLevelDimensions = {16.0f, 27.0f};
const m2::VecF gLevelCenter = gLevelDimensions / 2.0f;

// Pinball fakes the ball moving between a ground level and an elevated platform level using collision bits, while
// everything actually lives in a single physics world. Objects carry one of these layer bits in their `belongsTo`, and
// the ball toggles its `collidesWith` mask to select which level it interacts with.
constexpr uint16_t gPinballSharedLayer = 0x0800;   // Present on both levels (e.g. the outer boundary)
constexpr uint16_t gPinballPlatformLayer = 0x1000; // Present only on the elevated platform level
// Ground ball collides with everything except the platform-only level.
constexpr uint16_t gBallGroundMask = static_cast<uint16_t>(0xFFFF & ~gPinballPlatformLayer);
// Platform ball collides with only the platform-only level and the shared objects.
constexpr uint16_t gBallPlatformMask = gPinballPlatformLayer | gPinballSharedLayer;
