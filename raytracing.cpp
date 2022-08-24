#include <iostream>

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
};


int main()
{
	Vec3<double> vect;
	return 0;
}