#include "Shapes.h"
#include <iostream>

void cross(Vec& dest, Vec v1, Vec v2)
{
	dest.x = v1.y * v2.z - v1.z * v2.y; 
	dest.y = v1.z * v2.x - v1.x * v2.z; 
	dest.z = v1.x * v2.y - v1.y * v2.x;
}

Color genericShade(Vec& light, const Vec& cam, Ray& r, HitData& h)
{
	Color finalColor;

	//get the two vectors
	Vec normalN = h.lastNormal;
	Vec lightN = light - (r.o + (r.d * h.t)); //a vector between the point the camera ray hit and the light source

	//Normalize them
	lightN.Normalize();
	normalN.Normalize();

	//Calculate the angle between the point and the light
	float angle = lightN.Dot(normalN);

	if (angle < 0)
		angle = 0;

	float red   = h.color.r * angle + (50.0f / 255.0f) * h.color.r;
	float green = h.color.g * angle + (50.0f / 255.0f) * h.color.g;
	float blue  = h.color.b * angle + (50.0f / 255.0f) * h.color.b;

	//Check the light intensity
	if (red > 255)
		red = 255;
	if (green > 255)
		green = 255;
	if (blue > 255)
		blue = 255;

	finalColor = Color(red, green, blue);

	return finalColor;
}

Plane::Plane(Vec normal, float _d, Color color)
{
	n = normal;
	d = _d;
	c = color;
}

void Plane::test(Ray& ray, HitData& hit)
{	
	float t = (d - n.Dot(ray.o)) / (n.Dot(ray.d));

	//intersects	
	if (t > 0)
	{
		//Check if the intersection is closer to the monitor than the previously closest intersection
		//or if it is the first thing hit
		if (t < hit.t || hit.t == -1)
		{
			hit.t = t;
			hit.lastShape = this;
			hit.color = c;
			hit.lastNormal = n;
		}
	}
}

Color Plane::shade(Vec& light, const Vec& cam, Ray& r, HitData& h){return genericShade(light, cam, r, h);}

Vec Plane::normal(Vec& point)
{
	return n;
}

Sphere::Sphere(Vec _center, float _radius, Color _color)
{
	center = _center;
	radius = _radius;
	c      = _color;
}
void Sphere::test(Ray& ray, HitData& hit)
{
	float b       = ray.d.Dot(ray.o - center);
	float center2 = (ray.o - center).Dot(ray.o - center) - (radius*radius);

	//check if the roots are real
	float test = b*b - center2;
	if (test > 0)
	{
		//two different cases
		float t1 = -b + sqrt(test);
		float t2 = -b - sqrt(test);

		//Unnecessary checks below because t1 will never be smaller than t2
		if (t1 > 0 && t2 > 0)
		{
			if (t1 < t2)
			{
				//Check if the intersection is closer to the monitor than the previously closest intersection
				//or if it is the first thing hit
				if (t1 < hit.t || hit.t == -1)
				{
					Vec r = ray.o + ray.d*t1;
					//add data
					hit.t          = t1;
					hit.color      = c;
					hit.lastShape  = this;
					hit.lastNormal = normal(r);
				}
			}
			else
			{
				//Check if the intersection is closer to the monitor than the previously closest intersection
				//or if it is the first thing hit
				if (t2 < hit.t || hit.t == -1)
				{
					Vec r = ray.o + ray.d*t2;
					//add data
					hit.t          = t2;
					hit.color      = c;
					hit.lastShape  = this;
					hit.lastNormal = normal(r);
				}
			}
		}
		else if (t1 > 0)
		{
			//Check if the intersection is closer to the monitor than the previously closest intersection
			//or if it is the first thing hit
			if (t1 < hit.t || hit.t == -1)
			{
				Vec r = ray.o + ray.d*t1;

				//add data
				hit.t          = t1;
				hit.color      = c;
				hit.lastShape  = this;
				hit.lastNormal = normal(r);
			}
		}
		else if (t2 > 0)
		{
			//Check if the intersection is closer to the monitor than the previously closest intersection
			//or if it is the first thing hit
			if (t2 < hit.t || hit.t == -1)
			{
				Vec r = ray.o + ray.d*t2;

				//add data
				hit.t          = t2;
				hit.color      = c;
				hit.lastShape  = this;
				hit.lastNormal = normal(r);
			}
		}
	}
}

Color Sphere::shade(Vec& light, const Vec& cam, Ray& r, HitData& h){return genericShade(light, cam, r, h);}

Vec Sphere::normal(Vec &point)
{
	Vec normal = point - center;
	normal.Normalize();
	return normal;
}

Triangle::Triangle(Vec _p1, Vec _p2, Vec _p3, Color _color)
{
	p0 = _p1;
	p1 = _p2;
	p2 = _p3;
	c  = _color;

	Vec v1 = p0 - p1;
	Vec v2 = p0 - p2;
	cross(nor, v1, v2);
}
void Triangle::test(Ray& ray, HitData& hit)
{
	Vec p, q, t;

	//Create vectors as edges using points in the triangle
	edge1 = p1 - p0;
	edge2 = p2 - p0;

	cross(p, ray.d, edge2);

	float det = edge1.Dot(p);

	if (det > -0.000001 && det < 0.000001)
		return;

	float invDet = 1.0 / det;

	t = ray.o - p0;

	float u = t.Dot(p) * invDet;
	if (u < 0.0 || u > 1.0)
		return;

	cross(q, t, edge1);

	float v = ray.d.Dot(q) * invDet;

	if (v < 0.0 || v > 1.0)
		return;

	if (u + v > 1.0)
		return;

	//calculate the final hitpoint
	float tFloat = edge2.Dot(q) * invDet;

	//Check if the intersection is closer to the monitor than the previously closest intersection
	//or if it is the first thing hit
	if (tFloat < hit.t || hit.t == -1)
	{
		hit.t          = tFloat;
		hit.color      = c;
		hit.lastShape  = this;
		hit.lastNormal = nor;
	}
}
Color Triangle::shade(Vec& light, const Vec& cam, Ray& r, HitData& h){return genericShade(light, cam, r, h);}

// Center point, lenght U vector, length V vector, length W vector, color
OBB::OBB(Vec b, Vec b1, Vec b2, Vec b3, float Hu, float Hv, float Hw, Color _color)
{
	Bcenter = b;
	Bu      = b1;
	Bv      = b2;
	Bw      = b3;

	halfBu = Hu;
	halfBv = Hv;
	halfBw = Hw;

	c = _color;
}
OBB::OBB(Vec b, float Hu, float Hv, float Hw, Color _color)
{
	Bcenter = b;
	halfBu  = Hu;
	halfBv  = Hv;
	halfBw  = Hw;

	c = _color;
}

void OBB::test(Ray& ray, HitData& hit)
{
	float min = -100000;
	float max =  100000;

	//create a vector between the center of the ray and the center of the OBB
	Vec p = Bcenter - ray.o;

	//normalize the vectors of the OBB
	Bu.Normalize();
	Bv.Normalize();
	Bw.Normalize();

	float e = Bu.Dot(p);
	float f = Bu.Dot(ray.d);

	if (fabs(f) > 0.000001)
	{
		//calculate two possible hit locations
		float t1 = (e + halfBu) / f;
		float t2 = (e - halfBu) / f;

		//t1 should be the smaller one
		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}
		if (t1 > min)
			min = t1;
		if (t2 < max)
			max = t2;
		if (min > max)
			return;
		if (max < 0)
			return;
	}
	//check if the float is within acceptable ranges
	else if (-e - halfBu > 0 || -e + halfBu < 0)
		return;

	e = Bv.Dot(p);
	f = Bv.Dot(ray.d);
	if (fabs(f) > 0.000001)
	{
		float t1 = (e + halfBv) / f;
		float t2 = (e - halfBv) / f;

		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}
		if (t1 > min)
			min = t1;
		if (t2 < max)
			max = t2;
		if (min > max)
			return;
		if (max < 0)
			return;
	}
	else if (-e - halfBv > 0 || -e + halfBv < 0)
		return;

	e = Bw.Dot(p);
	f = Bw.Dot(ray.d);
	if (fabs(f) > 0.000001)
	{
		float t1 = (e + halfBw) / f;
		float t2 = (e - halfBw) / f;

		if (t1 > t2)
		{
			float temp = t1;
			t1 = t2;
			t2 = temp;
		}
		if (t1 > min)
			min = t1;
		if (t2 < max)
			max = t2;
		if (min > max)
			return;
		if (max < 0)
			return;
	}
	else if (-e - halfBw > 0 || -e + halfBw < 0)
		return;

	if (min > 0)
	{
		//Check if the intersection is closer to the monitor than the previously closest intersection
		//or if it is the first thing hit
		if (min < hit.t || hit.t == -1)
		{
			hit.t = min;
			hit.color = c;
			hit.lastShape = this;

			Vec r = ray.o + ray.d*min;
			hit.lastNormal = normal(r);
		}
	}
	else
	{
		//Check if the intersection is closer to the monitor than the previously closest intersection
		//or if it is the first thing hit
		if (max < hit.t || hit.t == -1)
		{
			hit.t = max;
			hit.color = c;
			hit.lastShape = this;

			Vec r = ray.o + ray.d*max;
			hit.lastNormal = normal(r);
		}
	}
}
Vec  OBB::normal(Vec& point)
{
	//These variables could be calculated in the constructor for increased performance

	Vec normalFaces[6];
	normalFaces[0] = Bu;
	normalFaces[1] = Bu * -1;
	normalFaces[2] = Bv;
	normalFaces[3] = Bv * -1;
	normalFaces[4] = Bw;
	normalFaces[5] = Bw * -1;

	Vec pointsOnPlanes[6];
	pointsOnPlanes[0] = Bcenter + (Bu * halfBu);
	pointsOnPlanes[1] = Bcenter - (Bu * halfBu);
	pointsOnPlanes[2] = Bcenter + (Bv * halfBv);
	pointsOnPlanes[3] = Bcenter - (Bv * halfBv);
	pointsOnPlanes[4] = Bcenter + (Bw * halfBw);
	pointsOnPlanes[5] = Bcenter - (Bw * halfBw);

	float res;
	for (int i = 0; i < 6; i++)
	{
		res = (point - pointsOnPlanes[i]).Dot(normalFaces[i]);
		if (res > -0.0001 && res < 0.0001)
			return normalFaces[i];
	}
}
Color OBB::shade(Vec& light, const Vec& cam, Ray& r, HitData& h){return genericShade(light, cam, r, h);}