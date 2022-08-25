#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>

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

	Vec3<T> operator*(const Vec3<T>& v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
	Vec3<T> operator+(const Vec3<T>& v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
	Vec3<T> operator-(const Vec3<T>& v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
	Vec3<T> operator-() const { return Vec3<T>(-x, -y, -z); }
	Vec3<T>& operator+=(const Vec3<T>& v) { x += v.x; y += v.y; z += v.z; return *this; }
	T length2() const { return x * x + y * y + z * z; }
	T length() const { return sqrt(length2()); }
	T dot(const Vec3<T>& v) const { return x * v.x + y * v.y + z * v.z; }
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

	bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const
	{
		Vec3f l = center - rayorig;
		float tca = l.dot(raydir);
		if (tca < 0) return false;
		float d2 = l.dot(l) - tca * tca;
		if (d2 > radius2) return false;
		float thc = sqrt(radius2 - d2);
		t0 = tca - thc;
		t1 = tca + thc;

		return true;
	}

};

Vec3f trace(const Vec3f& rayorig, const Vec3f& raydir, const std::vector<Sphere>& spheres, const int& depth)
{
	float tnear = INFINITY;
	const Sphere* sphere = NULL;

	for (unsigned int i = 0; i < spheres.size(); i++) {
		float t0 = INFINITY, t1 = INFINITY;
		if (spheres[i].intersect(rayorig, raydir, t0, t1)) {
			if (t0 < 0) t0 = t1;
			if (t0 < tnear) {
				tnear = t0;
				sphere = &spheres[i];
			}
		}
	}
	if (!sphere) return Vec3f(2);
	Vec3f surfaceColor = 0;
	Vec3f phit = rayorig + raydir * tnear;
	Vec3f nhit = phit - sphere->center;
	nhit.normalize();
	
	float bias = 1e-4;
	bool inside = false;
	if (raydir.dot(nhit) > 0) {
		nhit = -nhit;
		inside = true;
	}
	// no transparent object for now
	for (unsigned int i = 0; i < spheres.size(); i++) {
		if (spheres[i].emissionColor.x > 0) {
			// it's a light
			Vec3f transmission = 1;
			Vec3f lightDirection = spheres[i].center - phit;
			lightDirection.normalize();
			for (unsigned int j = 0; j < spheres.size(); j++) {
				if (i != j) {
					float t0, t1;
					if (spheres[j].intersect(phit, lightDirection, t0, t1)) {
						transmission = 0;
						break;
					}
				}
			}
			surfaceColor += sphere->surfaceColor * transmission * std::max(float(0), nhit.dot(lightDirection)) * spheres[i].emissionColor;
		}
	}

	return surfaceColor + sphere->emissionColor;
}


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
			*pixel = trace(Vec3f(0), raydir, spheres, 0);
		}
	}
	std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (unsigned int i = 0; i < width * height; i++) {
		ofs << (unsigned char)(std::min(float(0), image[i].x) * 255)
			<< (unsigned char)(std::min(float(0), image[i].y) * 255)
			<< (unsigned char)(std::min(float(0), image[i].z) * 255);
	}
	ofs.close();
	delete[] image;
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