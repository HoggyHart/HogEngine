#pragma once
#include "SDL.h"
#include "SDL_ttf.h"

#include <vector>

namespace Pixels {
	struct Pixel {
		Uint8 r{ 0 };
		Uint8 g{ 0 };
		Uint8 b{ 0 };
		Uint8 a{ 255 };
		Pixel() {};
		Pixel(Uint8 r, Uint8 g, Uint8 b, Uint8 a) :r(r), g(g), b(b), a(a) {}
	};
	struct PixelR {
		Uint8& r;
		Uint8& g;
		Uint8& b;
		Uint8& a;
	};
	struct RawPixel {
		void* r{ nullptr };
		void* g{ nullptr };
		void* b{ nullptr };
		void* a{ nullptr };
	};

	inline Pixel getPixel(SDL_Surface* surface, Uint32 x, Uint32 y) {
		unsigned char* pixels = (unsigned char*)surface->pixels;
		std::size_t pixDataIndex = y * surface->pitch + x * 4;

		Pixel px{};
		px.r = pixels[pixDataIndex + 0];
		px.g = pixels[pixDataIndex + 1];
		px.b = pixels[pixDataIndex + 2];
		px.a = pixels[pixDataIndex + 3];
		return px;
	}
	inline void setPixel(SDL_Surface* surface, Uint32 x, Uint32 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
		unsigned char* pixels = (unsigned char*)surface->pixels;
		std::size_t pixDataIndex = y * surface->pitch + x * 4;
		pixels[pixDataIndex + 0] = r;
		pixels[pixDataIndex + 1] = g;
		pixels[pixDataIndex + 2] = b;
		pixels[pixDataIndex + 3] = a;
	}

	inline std::ostream& operator<<(std::ostream& os, const Pixel& p) {
		return os << "(" << static_cast<int>(p.r) << ", " << static_cast<int>(p.g) << ", " << static_cast<int>(p.b) << ", " << static_cast<int>(p.a) << ")";
	}
}

inline std::ostream& operator<<(std::ostream& os, const SDL_Rect& r) {
	return os << "SDL_Rect{" << r.x << "," << r.y << "," << r.w << "," << r.h << "}";
}