#pragma once
#include <iostream>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
struct Vector2D;
struct Angle {
	double theta;
	bool radians;
	inline void bindIfNeeded() {
		if (radians && (theta > M_PI || theta < -M_PI)) bindNPItoPI();
		else if (!radians && (theta > 180 || theta < -180)) bindN180to180();
	}
	Angle() : theta(0), radians(true) {}
	Angle(double th) {
		theta = th;
		this->radians = true;
		bindIfNeeded();
	}
	Angle(double th, bool radians) {
		theta = th;
		this->radians = radians;
		bindIfNeeded();
	}
	Angle(Vector2D v);
	inline double asDegrees() const {
		if (radians) return theta / M_PI * 180;
		else return theta;
	}
	inline double asRadians() const {
		if (!radians) return theta / 180 * M_PI;
		else return theta;
	}
	template<typename T>
	Angle& add(T v) {
		theta += v;
		bindIfNeeded();
		return *this;
	}
	template<typename T>
	Angle& multiply(T v) {
		theta *= v;
		bindIfNeeded();
		return *this;
	}
	template<typename T>
	Angle& divide(T v) {
		theta /= v;
		bindIfNeeded();
		return *this;
	}
	void bindNPItoPI() {
		//theta = asin(sin(theta));
		while (theta < -M_PI) {
			theta += 2 * M_PI;
		}
		while (theta > M_PI) {
			theta -= 2 * M_PI;
		}
	}
	void bindN180to180() {
		int m = static_cast<int>(theta) / 360;
		theta -= m * 360;
		while (theta < -180) {
			theta += 360;
		}
		while (theta > 180) {
			theta -= 360;
		}
	}

	void bindZto2PI() {
		while (theta < 0) {
			theta += 2 * M_PI;
		}
		while (theta > 2 * M_PI) {
			theta -= 2 * M_PI;
		}
	}
	void bind0to360() {
		while (theta < 0) {
			theta += 360;
		}
		while (theta > 360) {
			theta -= 360;
		}
	}

	//th1 MUST be lower than th2
	void bindTo(double th1, double th2) {
		double incr = th2 - th1;
		while (theta < th1) {
			theta += incr;
		}
		while (theta > th2) {
			theta -= incr;
		}
	}
	bool between(Angle th1, Angle th2) const {
		if (th1 <= th2) {
			if (th1 <= theta && theta <= th2) {
				return true;
			}
		}
		else {
			if (theta >= th1 || theta <= th2) {
				return true;
			}
		}
		return false;
	}

	operator double() const {
		return theta;
	}
	bool operator<(const Angle& th2) {
		return theta < th2.theta;
	}

	bool operator<=(const Angle& th2) {
		return theta <= th2.theta;
	}
	bool operator>=(const Angle& th2) {
		return theta >= th2.theta;
	}
	bool operator<=(const double& th2) {
		return theta <= th2;
	}
	bool operator>=(const double& th2) {
		return theta >= th2;
	}
	Angle operator+(const Angle& th2) {
		return Angle(*this).add(th2);
	}
	Angle operator-(const Angle& th2) {
		return Angle(*this).add(-th2);
	}
};

struct Vector2D {
	float x;
	float y;

	Vector2D();
	Vector2D(float x, float y);
	Vector2D(int x, int y);
	Vector2D(double angle);
	Vector2D(double x, double y);
	Vector2D& zero();
	inline float distance(const Vector2D& v) const {
		return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y));
	}
	Vector2D direction(const Vector2D& v) const {
		Vector2D direction;
		direction.x = v.x - x;
		direction.y = v.y - y;
		direction.normalise();
		return direction;
	}
	Vector2D& normalise() {
		float mag = this->magnitude();
		if (mag > 1) {
			this->x /= mag;
			this->y /= mag;
		}
		return *this;
	}
	Vector2D normalised() {
		Vector2D v = { x,y };
		v.normalise();
		return v;
	}
	inline float magnitude() {
		return sqrt(x * x + y * y);
	}
	Angle toAngle();
	//bool represents degrees by default
	Vector2D& rotate(Angle angle, Vector2D* center);

	Vector2D& add(const Vector2D& v);
	Vector2D& subtract(const Vector2D& v);
	Vector2D& multiply(const Vector2D& v);
	Vector2D& divide(const Vector2D& v);

	friend Vector2D operator+(Vector2D& v, const Vector2D& v2);
	friend Vector2D operator-(Vector2D& v, const Vector2D& v2);
	friend Vector2D operator*(Vector2D& v, const Vector2D& v2);
	friend Vector2D operator/(Vector2D& v, const Vector2D& v2);
	Vector2D operator*(const int& i);
	Vector2D operator*(const float& d);
	Vector2D operator/(const int& i);
	Vector2D operator/(const float& i);
	Vector2D& operator+=(const Vector2D& v);
	Vector2D& operator-=(const Vector2D& v);
	Vector2D& operator*=(const Vector2D& v);
	Vector2D& operator/=(const Vector2D& v);
	Vector2D& operator*=(const int& i);
	Vector2D& operator/=(const int& i);

	friend bool operator==(const Vector2D& v1, const Vector2D& v2);
	friend bool operator!=(const Vector2D& v1, const Vector2D& v2);
	static std::string to_string(const Vector2D& x);
	
	friend std::ostream& operator<<(std::ostream& os, const Vector2D& v);
};
