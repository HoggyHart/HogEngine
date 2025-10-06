#pragma once
#include <SDL.h>
#include <array>

struct SizeHint {
	// 0.0 <= size_hint <= 1.0 are actual size hints
	//if 0.0 > size_hint > 1.0: <0 means ignored, >1.0 means a mistake has happened
	float size_hint_x{ 1.0f };
	float size_hint_y{ 1.0f };
};
struct Size {
	int width;
	int height;
};
struct PixelPadding {

	//padding from edges of container in pixels
	int paddingLeft{ 0 };
	int paddingRight{ 0 };
	int paddingTop{ 0 };
	int paddingBottom{ 0 };

	//padding from edges of container as % of container height/width
	//a size hint will override a pixel padding if it is between 0 and 1.0
	float paddingSizeHintL{ -1.0f }; //padding from left as % of container width
	float paddingSizeHintR{ -1.0f }; //padding from right as % of container width
	float paddingSizeHintT{ -1.0f }; //padding from top as % of container height
	float paddingSizeHintB{ -1.0f }; //padding from bottom as % of container height

};


struct UI_Element_Info {

private:
	//padding hint indicates percentage of padding on each size if UI Element does not take up whole of container
	//a size hint will override a pixel padding if it is not nullptr
	float* paddingHintL{ nullptr }; //padding from left as % of container width
	float* paddingHintR{ nullptr }; //padding from right as % of container width
	float* paddingHintT{ nullptr }; //padding from top as % of container height
	float* paddingHintB{ nullptr }; //padding from bottom as % of container height

	//size_hint indicates % of container's width/height this Element takes up
	float* size_hint_x{ new float(1.0f) };
	float* size_hint_y{ new float(1.0f) };
	//minimum size hints for if total size hints for a parent add up to over 1.0 and size_hint must be shrunk.
	// at what point does the entity stop shrinking and cause the parent to get bigger/ children to overlap
	float* min_size_hint_x{ new float(0.0f) };
	float* min_size_hint_y{ new float(0.0f) };
	inline void setSizeHint(float*& sizeHintToChange, float*& newValue) {

		if (sizeHintToChange != nullptr) {
			delete sizeHintToChange;
		}

		sizeHintToChange = newValue;
		//if both are nullptr then nothing happens
	}

public:

	Size size;


	int paddingLeft{ 0 };
	int paddingRight{ 0 };
	int paddingTop{ 0 };
	int paddingBottom{ 0 };

	

	//set method primarily used to enable/disable hints via changing between nullptr and float ptr
	inline void setSizeHints(float* shx, float* shy) {
		setSizeHint(size_hint_x, shx);
		setSizeHint(size_hint_y, shy);
	}
	//get method can be used to change value of size hints if they have a value
	//returns x, y
	inline std::array<float*, 2> getSizeHints() const {
		return { size_hint_x, size_hint_y };
	}

	//if values are passed in they must be passed in as new float(VALUE)
	inline void setPaddingHints(float* left, float* right, float* top, float* bottom) {
		setSizeHint(paddingHintL, left);
		setSizeHint(paddingHintR, right);
		setSizeHint(paddingHintT, top);
		setSizeHint(paddingHintB, bottom);
	}
	// returns left, right, top, bottom
	inline std::array<float*, 4> getPaddingHints() const{
		return { paddingHintL, paddingHintR, paddingHintT, paddingHintB };
	}
};

enum FitType { FitType_None, FitType_Fit, FitType_Fill };

struct UI_Image_Scaling_Info {

	bool lockProportions{ true };

	FitType fitType{ FitType_None };

	//will shrink the image to fit in the area, if it is already small enough, leave it be
	void scaleToFit(float& width, float& height, float& scale, int boundsW, int boundsH) const {
		//if the image is too wide to fit, scale the width to fit perfectly
		if (width * scale > boundsW) {
			float sizeChange = (boundsW /scale) / width;
			width = boundsW / scale;
			//if proportions are to be locked, record the scale factor of the shrink and apply the same to the height of the image
			if (lockProportions) {
				height *= sizeChange;
			}
		}
		//repeat for height
		if (height * scale > boundsH) {
			float sizeChange = (boundsH / scale) / height;
			height = boundsH / scale;
			if (lockProportions) {
				width *= sizeChange;
			}
		}
	}

	//will scale the image to fit the area exactly, so either height or width are maxed
	void scaleToFill(float& width, float& height, float& scale, int boundsW, int boundsH) const{
		//scale to fill the width
		if (width * scale != boundsW) {
			float sizeChange = (boundsW / scale) / width;
			width = boundsW / scale;
			//if proportions are to be locked, record the scale factor of the shrink and apply the same to the height of the image
			if (lockProportions) {
				height *= sizeChange;
			}
		}

		//then if this results in it being too tall, shrink the height
		if (height * scale > boundsH) {
			float sizeChange = (boundsH / scale) / height;
			height = boundsH / scale;
			if (lockProportions) {
				width *= sizeChange;
			}
		}
	}
};