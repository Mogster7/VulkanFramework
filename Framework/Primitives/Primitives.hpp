#pragma once

struct Box
{
	glm::vec3 position;
	glm::vec3 halfExtent;
};

struct BoxUniform
{
	glm::vec3 position;
	float halfExtent;
};

struct Sphere
{
	float radius;
	glm::vec3 position;
};

struct Plane
{
	glm::vec3 normal;
	glm::vec3 position;
	float D;
	inline static float thickness = 0.02f;
};

struct Point
{
	glm::vec3 position;
};

struct Ray
{
	glm::vec3 position;
	glm::vec3 direction;
};

struct Triangle
{
	glm::vec3 positions[3];
};

static bool SphereSphere(Sphere& sphere1, Sphere& sphere2)
{
	float distance = glm::distance(sphere1.position, sphere2.position);
	return distance < (sphere1.radius + sphere2.radius);
}

static bool BoxSphere(Box& box, Sphere& sphere)
{
	glm::vec3 boxMin = box.position - box.halfExtent;
	glm::vec3 boxMax = box.position + box.halfExtent;
	glm::vec3 closestPoint =
	{
		glm::max(boxMin.x, glm::min(sphere.position.x, boxMax.x)),
		glm::max(boxMin.y, glm::min(sphere.position.y, boxMax.y)),
		glm::max(boxMin.z, glm::min(sphere.position.z, boxMax.z))
	};

	float distance = glm::distance(closestPoint, sphere.position);

	return distance < sphere.radius;
}

static bool BoxBox(Box& box1, Box& box2)
{
	glm::vec3 aMin = box1.position - box1.halfExtent;
	glm::vec3 aMax = box1.position + box1.halfExtent;
	glm::vec3 bMin = box2.position - box2.halfExtent;
	glm::vec3 bMax = box2.position + box2.halfExtent;

	return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
		(aMin.y <= bMax.y && aMax.y >= bMin.y) &&
		(aMin.z <= bMax.z && aMax.z >= bMin.z);
}

static bool PlaneBox(Plane& plane, Box& box)
{
	// Compute projection interval radius of b onto the line L = box.position + t * plane.normal
	float r = glm::dot(box.halfExtent, glm::abs(plane.normal));

	// Compute distance of box.position from plane
	float s = glm::dot(plane.normal, box.position) - plane.D;

	// Intersection occurs when distance is between [-r,+r]
	return glm::abs(s) <= r;
}

static bool PlaneSphere(Plane& plane, Sphere& sphere)
{
	glm::vec3 v = sphere.position - plane.position;
	float d = glm::dot(plane.normal, v) / glm::sqrt(glm::dot(plane.normal, plane.normal));
	return glm::abs(d) <= sphere.radius;
}

static bool PointBox(Point& point, Box& box)
{
	glm::vec3 min = box.position - box.halfExtent;
	glm::vec3 max = box.position + box.halfExtent;

	return point.position.x >= min.x && point.position.x <= max.x &&
		point.position.y >= min.y && point.position.y <= max.y &&
		point.position.z >= min.z && point.position.z <= max.z;
}

static bool PointSphere(Point& point, Sphere& sphere)
{
	return glm::distance(point.position, sphere.position) <= sphere.radius;
}

static bool PointPlane(Point& point, Plane& plane)
{
	glm::vec3 v = point.position - plane.position;
	float d = glm::dot(plane.normal, v) / glm::sqrt(glm::dot(plane.normal, plane.normal));
	return glm::abs(d) < plane.thickness;
}

static bool RayBox(Ray& ray, Box& box)
{
	glm::vec3 lb = box.position - box.halfExtent;
	glm::vec3 rt = box.position + box.halfExtent;

	glm::vec3 invDir = 1.0f / ray.direction;
	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
	float t1 = (lb.x - ray.position.x) * invDir.x;
	float t2 = (rt.x - ray.position.x) * invDir.x;
	float t3 = (lb.y - ray.position.y) * invDir.y;
	float t4 = (rt.y - ray.position.y) * invDir.y;
	float t5 = (lb.z - ray.position.z) * invDir.z;
	float t6 = (rt.z - ray.position.z) * invDir.z;

	float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
	float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0)
		return false;

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
		return false;

	return true;
}

static bool SolveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1)
{
	float discr = b * b - 4 * a * c;
	if (discr < 0) return false;
	else if (discr == 0) x0 = x1 = -0.5 * b / a;
	else {
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1) std::swap(x0, x1);

	return true;
}

static bool RaySphere(Ray& ray, Sphere& sphere)
{
	float t0, t1;

	glm::vec3 L = ray.position - sphere.position;
	float a = glm::dot(ray.direction, ray.direction);
	float b = 2 * glm::dot(ray.direction, L);
	float c = glm::dot(L, L) - sphere.radius * sphere.radius;

	if (!SolveQuadratic(a, b, c, t0, t1)) return false;

	if (t0 > t1) std::swap(t0, t1);

	if (t0 < 0)
	{
		t0 = t1;
		if (t0 < 0) return false;
	}

	return true;
}

static bool RayPlane(Ray& ray, Plane& plane)
{
	// assuming vectors are all normalized
	float denom = glm::dot(plane.normal, ray.direction);
	return (denom > plane.thickness || denom < -plane.thickness);
	/*	{
			glm::vec3 p0l0 = plane.position - ray.position;
			float t = glm::dot(p0l0, plane.normal) / denom;
			return (t >= 0);
		*/
//	}

	return false;
}

static bool TriangleBox(Triangle& tri, Box& box)
{

}

static bool TrianglePoint(Triangle& tri, Point& point)
{

}

static bool TriangleRay(Triangle& tri, Point& point)
{
	//// compute plane's normal
	//Vec3f v0v1 = v1 - v0;
	//Vec3f v0v2 = v2 - v0;
	//// no need to normalize
	//Vec3f N = v0v1.crossProduct(v0v2); // N 
	//float area2 = N.length();

	//// Step 1: finding P

	//// check if ray and plane are parallel ?
	//float NdotRayDirection = N.dotProduct(dir);
	//if (fabs(NdotRayDirection) < kEpsilon) // almost 0 
	//	return false; // they are parallel so they don't intersect ! 

	//// compute d parameter using equation 2
	//float d = N.dotProduct(v0);

	//// compute t (equation 3)
	//t = (N.dotProduct(orig) + d) / NdotRayDirection;
	//// check if the triangle is in behind the ray
	//if (t < 0) return false; // the triangle is behind 

	//// compute the intersection point using equation 1
	//Vec3f P = orig + t * dir;

	//// Step 2: inside-outside test
	//Vec3f C; // vector perpendicular to triangle's plane 

	//// edge 0
	//Vec3f edge0 = v1 - v0;
	//Vec3f vp0 = P - v0;
	//C = edge0.crossProduct(vp0);
	//if (N.dotProduct(C) < 0) return false; // P is on the right side 

	//// edge 1
	//Vec3f edge1 = v2 - v1;
	//Vec3f vp1 = P - v1;
	//C = edge1.crossProduct(vp1);
	//if (N.dotProduct(C) < 0)  return false; // P is on the right side 

	//// edge 2
	//Vec3f edge2 = v0 - v2;
	//Vec3f vp2 = P - v2;
	//C = edge2.crossProduct(vp2);
	//if (N.dotProduct(C) < 0) return false; // P is on the right side; 

	//return true; // this ray hits the triangle 
}