#pragma once
#include <m2/video/DynamicSheet.h>
#include <m2/containers/Cache.h>
#include <variant>

namespace m2 {
	enum class ShapeType : int {
		CIRCLE,
		DISK,
	};

	struct Shape {
		virtual ~Shape() = default;
		virtual ShapeType Type() const = 0;
		virtual size_t Hash() const = 0;
		virtual bool operator==(const Shape& other) const = 0;
		virtual VecI Dimensions() const = 0;
		virtual void Draw(const SDL_Surface* dstSurface, const RectI& dstRect) const = 0;
	};
	struct Circle final : Shape {
		int radiusPx;
		explicit Circle(const int r) : radiusPx(r) {}
		ShapeType Type() const override;
		size_t Hash() const override;
		bool operator==(const Shape& other) const override;
		VecI Dimensions() const override;
		void Draw(const SDL_Surface* dstSurface, const RectI& dstRect) const override;
	};
	struct Disk final : Shape {
		int radiusPx;
		explicit Disk(const int r) : radiusPx(r) {}
		ShapeType Type() const override;
		size_t Hash() const override;
		bool operator==(const Shape& other) const override;
		VecI Dimensions() const override;
		void Draw(const SDL_Surface* dstSurface, const RectI& dstRect) const override;
	};

	class ShapeCache {

		class ShapeGenerator {
			DynamicSheet _dynamicSheet;
		public:
			explicit ShapeGenerator(SDL_Renderer* renderer) : _dynamicSheet(renderer) {}
			[[nodiscard]] SDL_Texture* Texture() const { return _dynamicSheet.Texture(); }
			RectI operator()(const std::shared_ptr<Shape>&);
		};
		struct ShapeHash {
			size_t operator()(const std::shared_ptr<Shape>& shape) const { return shape->Hash(); }
		};
		struct ShapeComparator {
			bool operator()(const std::shared_ptr<Shape>& lhs, const std::shared_ptr<Shape>& rhs) const;
		};

		Cache<
				std::shared_ptr<Shape>, // Key
				RectI, // Value
				ShapeGenerator, // Value generator
				ShapeHash, // Key hash function
				ShapeComparator // Key comparator
				> _cache;

	public:
		explicit ShapeCache(SDL_Renderer* renderer) : _cache(ShapeGenerator{renderer}) {}

		// Accessors

		[[nodiscard]] SDL_Texture* Texture() const { return _cache.Generator().Texture(); }

		// Modifiers

		RectI Create(const std::shared_ptr<Shape>& shape) { return _cache(shape); }
	};
}
