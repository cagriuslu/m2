#pragma once
#include <m2/math/VecF.h>
#include <m2/math/VecE.h>
#include <m2/ProxyTypes.h>

namespace m2::physics {
    class StaticBody {
        VecFE position{};
        FE orientation{};   // In radians
        bool _enabled{};

    public:
        StaticBody() = default;
        StaticBody(const StaticBody&) = delete;
        StaticBody& operator=(const StaticBody&) = delete;
        StaticBody(StaticBody&&) = default;
        StaticBody& operator=(StaticBody&&) = default;
        ~StaticBody() = default;

        // Accessors
        [[nodiscard]] bool IsEnabled() const { return _enabled; }
        [[nodiscard]] VecF GetPosition() const;
        [[nodiscard]] FE GetAngle() const;            // In radians
        [[nodiscard]] VecF GetLinearVelocity() const;
        [[nodiscard]] FE GetAngularVelocity() const;

        // Modifiers
        void SetEnabled(bool);
        void SetPosition(const VecFE&);
        void SetAngle(FE angle);                      // In radians
        void SetLinearVelocity(const VecFE&);
        void SetAngularVelocity(FE w);
        void ApplyForceToCenter(const VecFE&);
        void TeleportToAnother(const StaticBody& other);

        void OnStep() {}                              // No-op
    };
}
