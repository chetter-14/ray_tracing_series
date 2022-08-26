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

// an object that represents a set of 3 elements
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

// type for a set of 3 floats
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

	// the description of this method is in README
	bool intersect(const Vec3f& raySourcePoint, const Vec3f& rayVector, float& t0, float& t1) const
	{
		Vec3f l = center - raySourcePoint;
		float tca = l.dot(rayVector);
		if (tca < 0) return false;
		float d2 = l.dot(l) - tca * tca;
		if (d2 > radius2) return false;
		float thc = sqrt(radius2 - d2);
		t0 = tca - thc;
		t1 = tca + thc;

		return true;
	}

};

Vec3f trace(const Vec3f& raySourcePoint, const Vec3f& rayVector, const std::vector<Sphere>& spheres, const int& depth)
{
	float tnear = INFINITY;
	const Sphere* sphere = NULL;

	for (unsigned int i = 0; i < spheres.size(); i++) {
		float t0 = INFINITY, t1 = INFINITY;
		if (spheres[i].intersect(raySourcePoint, rayVector, t0, t1)) {
			if (t0 < 0) t0 = t1;	// if raySourcePoint is inside the sphere (?)
			if (t0 < tnear) {
				tnear = t0;
				sphere = &spheres[i];
			}
		}
	}
	if (!sphere) return Vec3f(2);
	Vec3f surfaceColor = 0;
	// the point where the ray hits a sphere
	Vec3f hitPoint = raySourcePoint + rayVector * tnear;
	// normal vector
	Vec3f hitNormalVector = hitPoint - sphere->center;
	hitNormalVector.normalize();
	
	// float bias = 1e-4;
	bool inside = false;
	if (rayVector.dot(hitNormalVector) > 0) {
		hitNormalVector = -hitNormalVector;
		inside = true;
	}

	// no transparent objects for now
	for (unsigned int i = 0; i < spheres.size(); i++) {
		if (spheres[i].emissionColor.x > 0) {
			// it's a light
			Vec3f transmission = 1;
			Vec3f lightDirection = spheres[i].center - hitPoint;
			lightDirection.normalize();
			for (unsigned int j = 0; j < spheres.size(); j++) {
				if (i != j) {
					float t0, t1;
					if (spheres[j].intersect(hitPoint, lightDirection, t0, t1)) {
						transmission = 0;
						break;
					}
				}
			}
			surfaceColor += sphere->surfaceColor * transmission 
				* std::max(float(0), hitNormalVector.dot(lightDirection)) * spheres[i].emissionColor;
		}
	}
	return surfaceColor + sphere->emissionColor;
}

// render all the pixels
void render(const std::vector<Sphere>& spheres)
{
	unsigned int width = 640, height = 480;								// properties of the image
	Vec3f *image = new Vec3f[width * height], *pixel = image;
	
	float invWidth = 1 / float(width), invHeight = 1 / float(height);	// used later to convert coordinates to NDC space
	float fov = 30, aspectratio = width / float(height);
	float angleCoef = tan(M_PI * 0.5 * fov / 180);						// coefficient that allows to zoom in/out
	// Trace rays
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++, ++pixel) {

			// PixelNDC_x = (Pixel_x + 0.5) / ImageWidth	(Raster space -> NDC space , range[0, ImageWidth]	-> range[0, 1])
			// PixelScreen_x = 2 * PixelNDC_x - 1			(NDC space -> Screen space , range[0, 1]			-> range[-1, 1])
			// for pixels to remain squares multiply PixelScreen_x 

			float cameraPixelX = (2 * ((x + 0.5) * invWidth) - 1) * angleCoef * aspectratio;
			float cameraPixelY = (1 - 2 * ((y + 0.5) * invHeight)) * angleCoef;
			Vec3f vector3D(cameraPixelX, cameraPixelY, -1);				// vector in 3d space (it goes from (0, 0, 0) )
			vector3D.normalize();
			*pixel = trace(Vec3f(0), vector3D, spheres, 0);
		}
	}
	std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (unsigned int i = 0; i < width * height; i++) {
		ofs << (unsigned char)(std::min(float(1), image[i].x) * 255)
			<< (unsigned char)(std::min(float(1), image[i].y) * 255)
			<< (unsigned char)(std::min(float(1), image[i].z) * 255);
	}
	ofs.close();
	delete[] image;
}


int main()
{
	std::vector<Sphere> spheres;
	spheres.emplace_back(Sphere(Vec3f(-3.0, 0.0, -20), 1,	 Vec3f(0.0, 1.0, 0.0),	1, 0.0));
	spheres.emplace_back(Sphere(Vec3f(3.0, 0.0, -20), 2,	 Vec3f(0.0, 0.0, 1.0),	1, 0.0));


	// light
	spheres.emplace_back(Sphere(Vec3f(0.0, 10.0, -20), 3, Vec3f(0.0, 0.0, 0.0),		0, 0.0, Vec3f(1)));

	render(spheres);
	return 0;
}