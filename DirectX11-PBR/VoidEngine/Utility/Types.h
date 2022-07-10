#pragma once

struct Pixel
{
	unsigned char r, g, b, a;
};
struct vec2i
{
	int x;
	int y;
	vec2i() { x = 0; y = 0; };
	vec2i(int x, int y) :
		x(x), y(y) {}
	bool operator==(const vec2i& v) const {
		return x == v.x && y == v.y;
	}
	bool operator!=(const vec2i& v) const {
		return !(x == v.x && y == v.y);
	}
	bool operator<(const vec2i& v)  const {
		return x < v.x || (x == v.x && y < v.y);
	}
};

struct vec3i
{
	int x;
	int y;
	int z;
	vec3i() { x = 0; y = 0; z = 0; };
	vec3i(int x, int y, int z) :
		x(x), y(y), z(z) {}
	bool operator==(const vec3i& v) const {
		return x == v.x && y == v.y && z == v.z;
	}
	bool operator!=(const vec3i& v) const {
		return !(x == v.x && y == v.y && z == v.z);
	}
	bool operator<(const vec3i& v)  const {
		return x < v.x&& y < v.y&& z < v.z;
	}
};

struct vec2f
{
	float x;
	float y;
	vec2f() { x = 0.0f; y = 0.0f; };
	vec2f(float x, float y) :
		x(x), y(y) {}
	bool operator==(const vec2f& v) const {
		return x == v.x && y == v.y;
	}
	bool operator!=(const vec2f& v) const {
		return !(x == v.x && y == v.y);
	}
	bool operator<(const vec2f& v)  const {
		return x < v.x&& y < v.y;
	}
	void operator+=(const vec2f v) {
		x += v.x; y += v.y;
	}
	void operator-=(const vec2f v) {
		x -= v.x; y -= v.y;
	}
};

struct vec3f
{
	float x;
	float y;
	float z;
	vec3f() { x = 0.0f; y = 0.0f; z = 0.0f; };
	vec3f(float x, float y, float z) :
		x(x), y(y), z(z) {}
	bool operator==(const vec3f& v) const {
		return x == v.x && y == v.y && z == v.z;
	}
	bool operator!=(const vec3f& v) const {
		return !(x == v.x && y == v.y && z == v.z);
	}
	bool operator<(const vec3f& v)  const {
		return x < v.x&& y < v.y&& z < v.z;
	}
	vec3f operator+(const vec3f& v)  const {
		return vec3f(x + v.x, y + v.y, z + v.z);
	}
	vec3f operator+(const float v)  const {
		return vec3f(x + v, y + v, z + v);
	}
};

struct vec4f
{
	float x;
	float y;
	float z;
	float w;
	vec4f() { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; };
	vec4f(float x, float y, float z, float w) :
		x(x), y(y), z(z), w(w) {}
	bool operator==(const vec4f& v) const {
		return x == v.x && y == v.y && z == v.z && w == v.w;
	}
	bool operator!=(const vec4f& v) const {
		return !(x == v.x && y == v.y && z == v.z && w == v.w);
	}
	bool operator<(const vec4f& v)  const {
		return x < v.x&& y < v.y&& z < v.z&& w < v.w;
	}
};