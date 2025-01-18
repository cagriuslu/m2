#include <m2/video/Shape.h>
#include <m2/M2.h>

namespace {
	// (0,0) will be at the middle of the rect. The function deals with the dstRect offset itself.
	void CircleSetPixel(const SDL_Surface* dstSurface, const m2::RectI& dstRect, const int radius, const int x, const int y) {
		auto* dstPixels = static_cast<uint32_t*>(dstSurface->pixels);
		auto* dstPixel = dstPixels + (dstRect.x + radius + x + (dstRect.y + radius + y) * dstSurface->w);
		*dstPixel = SDL_MapRGBA(dstSurface->format, 255, 255, 255, 255);
	}

	// This function requires the location of the pixel only on the first 45 degree of the first quadrant. It'll draw
	// the other points automatically.
	void CircleSetPixelOnFirstHalf(const SDL_Surface* dstSurface, const m2::RectI& dstRect, const int radius, const int x, const int y) {
		// Our circle drawing algorith produces a circle of size 2 * radius + 1.
		// Thus, we need to do some hacks to get the circle fit into 2 * radius circle.
		// We do this by pulling the bottom right half quadrants toward top left by one pixel.
		CircleSetPixel(dstSurface, dstRect, radius, x - 1, y - 1);
		CircleSetPixel(dstSurface, dstRect, radius, -x, y);
		CircleSetPixel(dstSurface, dstRect, radius, x - 1, -y - 1);
		CircleSetPixel(dstSurface, dstRect, radius, -x, -y);
		CircleSetPixel(dstSurface, dstRect, radius, y - 1, x - 1);
		CircleSetPixel(dstSurface, dstRect, radius, -y - 1, x - 1);
		CircleSetPixel(dstSurface, dstRect, radius, y, -x);
		CircleSetPixel(dstSurface, dstRect, radius, -y, -x);
	}

	void DiskSetPixelOnFirstHalf(const SDL_Surface* dstSurface, const m2::RectI& dstRect, const int radius, const int x, const int y) {
		// x is assumed to be positive
		const auto drawHorizontalLine = [&](const int x_, const int y_) {
			for (int i = -x_; i <= x_; ++i) {
				CircleSetPixel(dstSurface, dstRect, radius, i, y_);
			}
		};

		drawHorizontalLine(x - 1, y - 1);
		drawHorizontalLine(x - 1, -y - 1);
		drawHorizontalLine(y - 1, x - 1);
		drawHorizontalLine(y, -x);
	}

	void ForEachCirclePoints(const int radiusPx, const std::function<void(int,int)>& action) {
		// Midpoint circle algorithm
		int x = radiusPx, y = 0;

		// First point is at (r,0)
		action(x, y);

		// The next pixel is either at (x-1,y+1) or (x,y+1)
		// The middle point of these pixels is (x-1/2,y+1)
		// We need to check if the middle point is inside or outside the perimeter of the circle.
		// P = x^2 + y^2 - r^2
		// If P == 0, the middle point is at the perimeter.
		// If P < 0, the middle point is INSIDE the perimeter, otherwise, OUTSIDE.
		// We need to check

		// Substituting P = (r - 1/2)^2 + 1^2 - r^2
		// P = 1.25 - r (before rounding)
		int nextP = 1 - radiusPx;
		// When P <= 0, nextP = P + 2(y+1) + 1
		// Otherwise , nextP = P + 2(y+1) – 2(x–1) + 1
		// https://www.geeksforgeeks.org/mid-point-circle-drawing-algorithm/

		while (y < x) {
			++y;
			// Mid-point is inside or on the perimeter
			if (nextP <= 0) {
				nextP = nextP + 2 * y + 1;
			} else {
				// Mid-point is outside the perimeter
				--x;
				nextP = nextP + 2 * y - 2 * x + 1;
			}

			// All the perimeter points have already been printed
			if (x < y) {
				break;
			}
			action(x, y);
		}
	}
}

m2::ShapeType m2::Circle::Type() const {
	return ShapeType::CIRCLE;
}
size_t m2::Circle::Hash() const {
	return std::hash<int>{}(I(Type())) ^ std::hash<int>{}(radiusPx);
}
bool m2::Circle::operator==(const Shape& other) const {
	if (other.Type() == Type()) {
		return dynamic_cast<const Circle&>(other).radiusPx == radiusPx;
	}
	return false;
}
m2::VecI m2::Circle::Dimensions() const {
	return {2 * radiusPx, 2 * radiusPx};
}
void m2::Circle::Draw(const SDL_Surface* dstSurface, const RectI& dstRect) const {
	ForEachCirclePoints(radiusPx, [&](const int x, const int y) {
		CircleSetPixelOnFirstHalf(dstSurface, dstRect, radiusPx, x, y);
	});
}

m2::ShapeType m2::Disk::Type() const {
	return ShapeType::DISK;
}
size_t m2::Disk::Hash() const {
	return std::hash<int>{}(I(Type())) ^ std::hash<int>{}(radiusPx);
}
bool m2::Disk::operator==(const Shape& other) const {
	if (other.Type() == Type()) {
		return dynamic_cast<const Disk&>(other).radiusPx == radiusPx;
	}
	return false;
}
m2::VecI m2::Disk::Dimensions() const {
	return {2 * radiusPx, 2 * radiusPx};
}
void m2::Disk::Draw(MAYBE const SDL_Surface* dstSurface, MAYBE const RectI& dstRect) const {
	ForEachCirclePoints(radiusPx, [&](const int x, const int y) {
		DiskSetPixelOnFirstHalf(dstSurface, dstRect, radiusPx, x, y);
	});
}

m2::RectI m2::ShapeCache::ShapeGenerator::operator()(const std::shared_ptr<Shape>& shape) {
	// Allocate space
	const auto dims = shape->Dimensions();
	const auto [dstSurface, dstRect] = _dynamicSheet.Alloc(dims.x, dims.y);

	// Check pixel stride
	if (dstSurface->format->BytesPerPixel != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	// Lock, then draw
	SDL_LockSurface(dstSurface);
	shape->Draw(dstSurface, dstRect);
	SDL_UnlockSurface(dstSurface);

	_dynamicSheet.RecreateTexture(false);
	return dstRect;
}

bool m2::ShapeCache::ShapeComparator::operator()(const std::shared_ptr<Shape>& lhs, const std::shared_ptr<Shape>& rhs) const {
	return *lhs == *rhs;
}
