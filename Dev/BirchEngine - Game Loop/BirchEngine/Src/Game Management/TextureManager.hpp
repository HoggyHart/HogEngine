#pragma once
#include "SDL_image.h"
#include <vector>
#include <iostream>
class Entity;
class TextureManager {
public:
	static SDL_Texture* LoadTexture(const char* filename);
	static void resetTextureRenderMods(SDL_Texture* tex) {
		SDL_SetTextureAlphaMod(tex, 255);
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_INVALID);
		SDL_SetTextureColorMod(tex, 255, 255, 255);
	}
	static void Draw(SDL_Texture* texture, SDL_Rect* src, SDL_Rect& dst, SDL_RendererFlip flip, double rotation);
	static void DrawMap(std::vector<Entity*>& group);

	static void DrawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, SDL_Color clr) {
		SDL_SetRenderDrawColor(renderer, clr.r, clr.g, clr.b, clr.a);
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}
	//uses midpoint algorithm
	static void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius)
	{
		//functions based off of x^2 + y^2 = r^2;
		//rearrange for x^2 + y^2 - r^2 = 0
		//check x^2 + y^2 - r^2, if < 0 : increase y, else if >=0 : decrease x;
		//draws from far right of circle up until x == y (45 degrees) and draws reflections in the other 7 octants to complete the circle

		//error vals used to calculate x^2 + y^2 -r^2 in advance
		//i.e. when y goes up because ^^^^^^^^^^^^^^^^ is < 0, it goes: 0,1,4,9,16,25 which goes up by -> 1,3,5,7,9 which goes up by -> 2,2,2,2
		//and x starts from 1 - radius*2 and increase the same way as y

		int32_t x = radius - 1; //distance in x from circle centerX
		int32_t y = 0; //distance in y from circle centerY;

		int32_t yError = 1;
		int32_t xError = 1 - radius * 2;
		int32_t error = xError;
		while (x >= y) {
			SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
			SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
			SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
			SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
			SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
			SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
			SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
			SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);
			if (error >= 0) {
				error += xError;
				xError += 2;
				--x;
			}
			else {
				error += yError;
				yError += 2;
				++y;
			}
		}
	}
	//th1 and th2 taken in radians
	static void DrawArc(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius, double th1, double th2)
	{
		//normalise so values become in range 0 - 2*M_PI
		while (th1 < 0) {
			th1 += 2 * M_PI;
		}
		while (th1 > 2 * M_PI) {
			th1 -= 2 * M_PI;
		}
		while (th2 < 0) {
			th2 += 2 * M_PI;
		}
		while (th2 > 2 * M_PI) {
			th2 -= 2 * M_PI;
		}

		//functions based off of x^2 + y^2 = r^2;
		//rearrange for x^2 + y^2 - r^2 = 0
		//check x^2 + y^2 - r^2, if < 0 : increase y, else if >=0 : decrease x;
		//draws from far right of circle up until x == y (45 degrees) and draws reflections in the other 7 octants to complete the circle

		//error vals used to calculate x^2 + y^2 -r^2 in advance
		//i.e. when y goes up because ^^^^^^^^^^^^^^^^ is < 0, it goes: 0,1,4,9,16,25 which goes up by -> 1,3,5,7,9 which goes up by -> 2,2,2,2
		//and x starts from 1 - radius*2 and increase the same way as y

		//line gradient based drawing point draw
		float startX = static_cast<float>(radius * sin(0.5 * M_PI - th1));
		float endX = static_cast<float>(radius * sin(0.5 * M_PI - th2));
		if (endX == startX) { //gradient will be undefined, so hitbox wont be drawn. use angle method to solve this problem.
			//angle draw could just always be used instead but it calculates asin() every loop which I think makes it slower
			DrawArc_wAngle(renderer, centreX, centreY, radius, th1, th2);
			return;
		}
		float startY = static_cast<float>(radius * sin(th1));
		float endY = static_cast<float>(radius * sin(th2));

		float gradient = (endY - startY) / (endX - startX);
		int f = endX < startX ? -1 : 1;
		float c = startY - gradient * startX;

		float fm = f * gradient;
		float fc = f * c;

		int32_t x = radius - 1; //distance in x from circle centerX
		int32_t y = 0; //distance in y from circle centerY;

		int32_t yError = 1;
		int32_t xError = 1 - radius * 2;
		int32_t error = xError;

		while (x >= y) {
			if (fm * x + fc - f * -y > 0) SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
			if (fm * -y + fc - f * -x > 0) SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
			if (fm * -x + fc - f * y > 0) SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
			if (fm * y + fc - f * x > 0) SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
			if (fm * x + fc - f * y > 0) SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
			if (fm * -y + fc - f * x > 0) SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);
			if (fm * -x + fc - f * -y > 0) SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
			if (fm * y + fc - f * -x > 0) SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);

			if (error >= 0) {
				error += xError;
				xError += 2;
				--x;
			}
			else {
				error += yError;
				yError += 2;
				++y;
			}
		}
	}
	static void DrawArc_wAngle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius, double th1, double th2)
	{
		//normalise so values become in range 0 - 2*M_PI
		while (th1 < 0) {
			th1 += 2 * M_PI;
		}
		while (th1 > 2 * M_PI) {
			th1 -= 2 * M_PI;
		}
		while (th2 < 0) {
			th2 += 2 * M_PI;
		}
		while (th2 > 2 * M_PI) {
			th2 -= 2 * M_PI;
		}

		int inRange = 1;
		if (th1 > th2) {//i.e. -30 to 30 -> 330 to 30 -> 30 - 330
			double temp = th2;
			th2 = th1;
			th1 = temp;
			inRange = 0;
		}
		//functions based off of x^2 + y^2 = r^2;
		//rearrange for x^2 + y^2 - r^2 = 0
		//check x^2 + y^2 - r^2, if < 0 : increase y, else if >=0 : decrease x;
		//draws from far right of circle up until x == y (45 degrees) and draws reflections in the other 7 octants to complete the circle

		//error vals used to calculate x^2 + y^2 -r^2 in advance
		//i.e. when y goes up because ^^^^^^^^^^^^^^^^ is < 0, it goes: 0,1,4,9,16,25 which goes up by -> 1,3,5,7,9 which goes up by -> 2,2,2,2
		//and x starts from 1 - radius*2 and increase the same way as y

		int32_t x = radius - 1; //distance in x from circle centerX
		int32_t y = 0; //distance in y from circle centerY;

		int32_t yError = 1;
		int32_t xError = 1 - radius * 2;
		int32_t error = xError;

		double th;
		double p5mpi = 0.5 * M_PI;
		while (x >= y) {
			//r = sqrt(x * x + y * y); //LEFT OFF HERE: NOT SURE IF REPLACING WITH RADIUS IS GOOD? I USED THIS BEFORE AND IT WENT BAD, BUT THAT MIGHTVE BEEN BEFORE THIS CODE EVEN WORKED AT ALL
			//OPTIMIZATION: REPLACE 0.5*M_PI with dTheta, which will be set to equal 0.5*M_PI, so it doesnt need to be recalculated
			double ang = asin(static_cast<double>(y) / radius); //if using radius and not r, move this to ++y bit cos thats the only time this'll change
			th = 2 * M_PI - ang;
			if ((th > th1 && th < th2) == inRange) SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
			th -= p5mpi;
			if ((th > th1 && th < th2) == inRange) SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
			th -= p5mpi;
			if ((th > th1 && th < th2) == inRange) SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
			th -= p5mpi;
			if ((th > th1 && th < th2) == inRange) SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
			th = ang;
			if ((th > th1 && th < th2) == inRange) SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
			th += p5mpi;
			if ((th > th1 && th < th2) == inRange) SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);
			th += p5mpi;
			if ((th > th1 && th < th2) == inRange) SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
			th += p5mpi;
			if ((th > th1 && th < th2) == inRange) SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);

			if (error >= 0) {
				error += xError;
				xError += 2;
				--x;
			}
			else {
				error += yError;
				yError += 2;
				++y;
			}
		}
	}
};