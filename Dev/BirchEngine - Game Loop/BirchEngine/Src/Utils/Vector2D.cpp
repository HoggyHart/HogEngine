#include "Vector2D.hpp"
#include <string>
#include <math.h>
#include <SDL.h>
Vector2D::Vector2D() {
	this->zero();
}

Vector2D::Vector2D(float x, float y) {
	this->x = x;
	this->y = y;
}
Vector2D::Vector2D(int x, int y) {
	this->x = static_cast<float>(x);
	this->y = static_cast<float>(y);
}
Vector2D::Vector2D(double angle) {
	this->x = static_cast<float>(cos(angle));
	this->y = static_cast<float>(sin(angle));
}
Vector2D::Vector2D(double x, double y)
{
	this->x = static_cast<float>(x);
	this->y = static_cast<float>(y);
}
Vector2D& Vector2D::zero() {
	x = 0.0f;
	y = 0.0f;
	return *this;
}
Angle Vector2D::toAngle() {
	double angle = this->magnitude() != 0 ? asin(this->y / this->magnitude()) : 0; //using y results capping at 0.5*M_PI
	//account for each extra 0.5pi degrees
	if (x < 0) angle = y < 0 ? -M_PI - angle : M_PI - angle;
	return Angle(angle);
}
Vector2D& Vector2D::add(const Vector2D& v) {
	this->x += v.x;
	this->y += v.y;
	return *this;
}
Vector2D& Vector2D::subtract(const Vector2D& v) {
	this->x -= v.x;
	this->y -= v.y;
	return *this;
}
Vector2D& Vector2D::multiply(const Vector2D& v) {
	this->x *= v.x;
	this->y *= v.y;
	return *this;
}
Vector2D& Vector2D::divide(const Vector2D& v) {
	this->x /= v.x;
	this->y /= v.y;
	return *this;
}

Vector2D operator+(Vector2D& v1, const Vector2D& v2) {
	return Vector2D(v1.x + v2.x, v1.y + v2.y);
}
Vector2D operator-(Vector2D& v1, const Vector2D& v2) {
	return Vector2D(v1.x - v2.x, v1.y - v2.y);
}
Vector2D operator*(Vector2D& v1, const Vector2D& v2) {
	return Vector2D(v1.x * v2.x, v1.y * v2.y);
}
Vector2D operator/(Vector2D& v1, const Vector2D& v2) {
	return Vector2D(v1.x / v2.x, v1.y / v2.y);
}
Vector2D Vector2D::operator*(const int& i) {
	return Vector2D(x * i, y * i);
}
Vector2D Vector2D::operator/(const int& i) {
	return Vector2D(x / i, y / i);
}
Vector2D Vector2D::operator*(const float& i) {
	return Vector2D(x * i, y * i);
}
Vector2D Vector2D::operator/(const float& i) {
	return Vector2D(x / i, y / i);
}

Vector2D& Vector2D::operator+=(const Vector2D& v) {
	return this->add(v);
}
Vector2D& Vector2D::operator-=(const Vector2D& v) {
	return this->subtract(v);
}
Vector2D& Vector2D::operator*=(const Vector2D& v) {
	return this->multiply(v);
}
Vector2D& Vector2D::operator/=(const Vector2D& v) {
	return this->divide(v);
}
Vector2D& Vector2D::operator*=(const int& i) {
	this->x *= i;
	this->y *= i;
	return *this;
}
Vector2D& Vector2D::operator/=(const int& i) {
	this->x /= i;
	this->y /= i;
	return *this;
}

bool operator==(const Vector2D& v1, const Vector2D& v2) {
	return v1.x == v2.x && v1.y == v2.y;
}
bool operator!=(const Vector2D& v1, const Vector2D& v2) {
	return !(v1 == v2);
}

Vector2D& Vector2D::rotate(Angle angle, Vector2D* center = nullptr) {
	float tmpX = x;

	if (!center) {
		x = x * static_cast<float>(cos(angle)) + y * static_cast<float>(sin(angle));
		y = y * static_cast<float>(cos(angle)) - tmpX * static_cast<float>(sin(angle));
	}
	else {
		x = (x - center->x) * static_cast<float>(cos(angle)) + (y - center->y) * static_cast<float>(sin(angle)) + center->x;
		y = (y - center->y) * static_cast<float>(cos(angle)) - (tmpX - center->x) * static_cast<float>(sin(angle)) + center->y;
	}
	return *this;
}
std::string Vector2D::to_string(const Vector2D& v) {
	return "(" + std::to_string(v.x) + "," + std::to_string(v.y) + ")";
}
std::ostream& operator<<(std::ostream& os, const Vector2D& v) {
	return os << "(" << v.x << "," << v.y << ")";
}

Angle::Angle(Vector2D v)
{
	theta = v.toAngle();
	radians = true;
}