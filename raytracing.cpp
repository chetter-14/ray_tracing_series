#include <iostream>
#include <cmath>
#include <vector>

#if defined __linux__ || defined __APPLE__
// Compiled for Linux
#else
#define M_PI 3.141592653589793 
#define INFINITY 1e8
#endif

template<typename T>
class Vec3 
{
public:
	T x, y, z;	
	Vec3() : x(T(0)), y(T(0)), z(T(0)) { }
	Vec3(T val) : x(val), y(val), z(val) { }
	Vec3(T x, T y, T z) 
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vec3& normalize()
	{
		T len = length();
		if (len > 0) 
		{
			T invlen = 1 / len;
			x *= invlen; 
			y *= invlen, 
			z *= invlen;
		}
		return *this;
	}

	T length2() const { return x * x + y * y + z * z; }
	T length() const { return sqrt(length2()); }
};

typedef Vec3<float> Vec3f;


class Sphere 
{
public:
	Vec3f center;
	float radius, radius2;
	Vec3f surfaceColor, emissionColor;
	float transparency, reflection;

	Sphere(const Vec3f& c, const float& r, const Vec3f& sc, const float& transp = 0, const float& refl = 0, const Vec3f& ec = 0)
		: center(c), radius(r), radius2(r * r), surfaceColor(sc), emissionColor(ec), transparency(transp), reflection(refl)
	{ }

};


void render(const std::vector<Sphere>& spheres)
{
	unsigned int width = 640, height = 480;
	Vec3f *image = new Vec3f[width * height], *pixel = image;
	float invWidth = 1 / float(width), invHeight = 1 / float(height);
	float fov = 30, aspectratio = width / float(height);
	float angle = tan(M_PI * 0.5 * fov / 180);
	// Trace rays
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++, ++pixel) {
			float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
			float yy = (1 - 2 * ((x + 0.5) * invHeight)) * angle;
			Vec3f raydir(xx, yy, -1);
			raydir.normalize();

		}
	}
}


int main()
{
	std::vector<Sphere> spheres;
	spheres.emplace_back(Sphere(Vec3f(5.0, -1, -15), 2,	 Vec3f(0.90, 0.76, 0.46),	1, 0.0));

	// light
	spheres.emplace_back(Sphere(Vec3f(0.0, 20, -30), 3, Vec3f(0.0, 0.0, 0.0),		0, 0.0, Vec3f(3)));

	render(spheres);
	return 0;
}