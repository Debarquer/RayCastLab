#include "Shapes.h"
#include <iostream>

void cross(Vec& dest, Vec v1, Vec v2)
{
	dest.x = v1.y * v2.z - v1.z * v2.y; 
	dest.y = v1.z * v2.x - v1.x * v2.z; 
	dest.z = v1.x * v2.y - v1.y * v2.x;
}

void sub(Vec& dest, Vec v1, Vec v2)
{
	dest.x = v1.x - v2.x;
	dest.y = v1.y - v2.y;
	dest.z = v1.z - v2.z;
}

Color genericShade(Vec& light, const Vec& cam, Ray& r, HitData& h)
{
	Color finalColor;

	//get the two normalized vectors
	Vec normalN = h.lastNormal;
	Vec lightN = light - (r.o + (r.d * h.t));

	//Normalize them
	lightN.Normalize();
	normalN.Normalize();

	//Calulcuttalate the angle
	float angle = lightN.Dot(normalN);

	//Check if the angle is acceptable
	if (angle > 0)
	{
		float red   = h.color.r * angle + (50.0f / 255.0f) * h.color.r;
		float green = h.color.g * angle + (50.0f / 255.0f) * h.color.g;
		float blue  = h.color.b * angle + (50.0f / 255.0f) * h.color.b;
		if (red > 255)
			red = 255;
		if (green > 255)
			green = 255;
		if (blue > 255)
			blue = 255;
		finalColor = Color(red, green, blue);
	}
	else if (angle < 0)
	{
		float red   = (50.0f / 255.0f) * h.color.r;
		float green = (50.0f / 255.0f) * h.color.g;
		float blue  = (50.0f / 255.0f) * h.color.b;
		if (red > 255)
			red = 255;
		if (green > 255)
			green = 255;
		if (blue > 255)
			blue = 255;
		finalColor = Color(red, green, blue);
	}

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
		//Check if the intersection is closer to the monitor than the previously closest intersection (for that pixel)
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
	c =      _color;
}
void Sphere::test(Ray& ray, HitData& hit)
{
	float b       = ray.d.Dot(ray.o - center);
	float center2 = (ray.o - center).Dot(ray.o - center) - (radius*radius);

	//check if the roots are real
	float test = b*b - center2;
	if (test > 0)
	{
		float t1 = -b + sqrt(b*b - center2);
		float t2 = -b - sqrt(b*b - center2);
		//check for the closest one
		if (t1 > 0 && t2 > 0)
		{
			if (t1 < t2)
			{
				//check if it is closer than the previously closest
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
				//check if it is closer than the previously closest
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
			if (t2 < hit.t || hit.t == -1)
			{
				Vec r = ray.o + ray.d*t2;
				//add data
				hit.t          =         t2;
				hit.color      =     c;
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
	c = _color;

	Vec v1 = p0 - p1;
	Vec v2 = p0 - p2;
	cross(nor, v1, v2);
}
void Triangle::test(Ray& ray, HitData& hit)
{
	Vec p, q, t;

	//sub(edge1, p1, p0);
	//sub(edge2, p2, p0);
	edge1 = p1 - p0;
	edge2 = p2 - p0;

	cross(p, ray.d, edge2);

	float det = edge1.Dot(p);

	//Non culling
	if (det > -0.000001 && det < 0.000001)
		return;
	float invDet = 1.0 / det;

	//sub(t, ray.o, p0);
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

	float tFloat = edge2.Dot(q) * invDet;

	if (tFloat < hit.t || hit.t == -1)
	{
		hit.t = tFloat;
		hit.color = c;
		hit.lastShape = this;
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

	Vec p = Bcenter - ray.o;

	Bu.Normalize();
	Bv.Normalize();
	Bw.Normalize();

	float e = Bu.Dot(p);
	float f = Bu.Dot(ray.d);
	if (fabs(f) > 0.000001)
	{
		float t1 = (e + halfBu) / f;
		float t2 = (e - halfBu) / f;

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
	else if (-e - halfBw > 0 || -e + halfBw < 0)
		return;

	if (min > 0)
	{
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
		if (res > -0.000001 && res < 0.000001)
			return normalFaces[i];
	}
}
Color OBB::shade(Vec& light, const Vec& cam, Ray& r, HitData& h){return genericShade(light, cam, r, h);}