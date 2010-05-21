class vdfloat2 {
public:
	typedef vdfloat2 self_type;
	typedef float value_type;

	void set(float x2, float y2) { x=x2; y=y2; }

	float&			operator[](int k)					{ return v[k]; }
	const float&	operator[](int k) const				{ return v[k]; }

	float		lensq() const							{ return x*x + y*y; }

	self_type	operator-() const						{ self_type a = {-x, -y}; return a; }

	self_type	operator+(const self_type& r) const		{ self_type a = {x+r.x, y+r.y}; return a; }
	self_type	operator-(const self_type& r) const		{ self_type a = {x-r.x, y-r.y}; return a; }

	self_type&	operator+=(const self_type& r)			{ x+=r.x; y+=r.y; return *this; }
	self_type&	operator-=(const self_type& r)			{ x-=r.x; y-=r.y; return *this; }

	self_type	operator*(const float s) const			{ self_type a = {x*s, x*s}; return a; }
	self_type&	operator*=(const float s)				{ x*=s; y*=s; return *this; }

	self_type	operator/(const float s) const			{ const float inv(float(1)/s); self_type a = {x*inv, y*inv}; return a; }
	self_type&	operator/=(const float s)				{ const float inv(float(1)/s); x*=inv; y*=inv; return *this; }

	self_type	operator*(const self_type& r) const		{ self_type a = {x*r.x, y*r.y}; return a; }
	self_type&	operator*=(const self_type& r)			{ x*=r.x; y*=r.y; return *this; }

	self_type	operator/(const self_type& r) const		{ self_type a = {x/r.x, y/r.y}; return a; }
	self_type&	operator/=(const self_type& r)			{ x/=r.x; y/=r.y; return *this; }

	union {
		struct {
			float x;
			float y;
		};
		float v[2];
	};
};

VDFORCEINLINE vdfloat2 operator*(const float s, const vdfloat2& v) { return v*s; }

///////////////////////////////////////////////////////////////////////////

class vdfloat3 {
public:
	typedef vdfloat3 self_type;
	typedef float value_type;

	void set(float x2, float y2, float z2) { x=x2; y=y2; z=z2; }

	float&			operator[](int k)					{ return v[k]; }
	const float&	operator[](int k) const				{ return v[k]; }

	float		lensq() const							{ return x*x + y*y + z*z; }

	vdfloat2	project() const							{ const float inv(float(1)/z); const vdfloat2 a = {x*inv, y*inv}; return a; }
	vdfloat2	as2d() const							{ const vdfloat2 a = {x, y}; return a; }

	self_type	operator-() const						{ const self_type a = {-x, -y, -z}; return a; }

	self_type	operator+(const self_type& r) const		{ const self_type a = {x+r.x, y+r.y, z+r.z}; return a; }
	self_type	operator-(const self_type& r) const		{ const self_type a = {x-r.x, y-r.y, z-r.z}; return a; }

	self_type&	operator+=(const self_type& r)			{ x+=r.x; y+=r.y; z+=r.z; return *this; }
	self_type&	operator-=(const self_type& r)			{ x-=r.x; y-=r.y; z-=r.z; return *this; }

	self_type	operator*(const float s) const			{ const self_type a = {x*s, y*s, z*s}; return a; }
	self_type&	operator*=(const float s)				{ x*=s; y*=s; z*=s; return *this; }

	self_type	operator/(const float s) const			{ const float inv(float(1)/s); const self_type a = {x*inv, y*inv, z*inv}; return a; }
	self_type&	operator/=(const float s)				{ const float inv(float(1)/s); x*=inv; y*=inv; z*=inv; return *this; }

	self_type	operator*(const self_type& r) const		{ self_type a = {x*r.x, y*r.y, z*r.z}; return a; }
	self_type&	operator*=(const self_type& r)			{ x*=r.x; y*=r.y; z*=r.z; return *this; }

	self_type	operator/(const self_type& r) const		{ self_type a = {x/r.x, y/r.y, z/r.z}; return a; }
	self_type&	operator/=(const self_type& r)			{ x/=r.x; y/=r.y; z/=r.z; return *this; }

	union {
		struct {
			float x;
			float y;
			float z;
		};
		float v[3];
	};
};

VDFORCEINLINE vdfloat3 operator*(const float s, const vdfloat3& v) { return v*s; }

///////////////////////////////////////////////////////////////////////////

class vdfloat4 {
public:
	typedef vdfloat4 self_type;
	typedef float value_type;

	void setzero() { x=y=z=w = 0; }
	void set(float x2, float y2, float z2, float w2) { x=x2; y=y2; z=z2; w=w2; }

	float&			operator[](int i) { return v[i]; }
	const float&	operator[](int i) const { return v[i]; }

	float		lensq() const							{ return x*x + y*y + z*z + w*w; }

	vdfloat3	project() const							{ const float inv(float(1)/w); const vdfloat3 a = {x*inv, y*inv, z*inv}; return a; }

	self_type	operator-() const						{ const self_type a = {-x, -y, -z, -w}; return a; }

	self_type	operator+(const self_type& r) const		{ const self_type a = {x+r.x, y+r.y, z+r.z, w+r.w}; return a; }
	self_type	operator-(const self_type& r) const		{ const self_type a = {x-r.x, y-r.y, z-r.z, w-r.w}; return a; }

	self_type&	operator+=(const self_type& r)			{ x+=r.x; y+=r.y; z+=r.z; w+=r.w; return *this; }
	self_type&	operator-=(const self_type& r)			{ x-=r.x; y-=r.y; z-=r.z; w-=r.w; return *this; }

	self_type	operator*(const float factor) const		{ const self_type a = {x*factor, y*factor, z*factor, w*factor}; return a; }
	self_type	operator/(const float factor) const		{ const float inv(float(1) / factor); const self_type a = {x*inv, y*inv, z*inv, w*inv}; return a; }

	self_type&	operator*=(const float factor)			{ x *= factor; y *= factor; z *= factor; w *= factor; return *this; }
	self_type&	operator/=(const float factor)			{ const float inv(float(1) / factor); x *= inv; y *= inv; z *= inv; w *= inv; return *this; }

	self_type	operator*(const self_type& r) const		{ self_type a = {x*r.x, y*r.y, z*r.z, w*r.w}; return a; }
	self_type&	operator*=(const self_type& r)			{ x*=r.x; y*=r.y; z*=r.z; w*=r.w; return *this; }

	self_type	operator/(const self_type& r) const		{ self_type a = {x/r.x, y/r.y, z/r.z, w*r.w}; return a; }
	self_type&	operator/=(const self_type& r)			{ x/=r.x; y/=r.y; z/=r.z; w/=r.w; return *this; }

	union {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
		float v[4];
	};
};

VDFORCEINLINE vdfloat4 operator*(const float s, const vdfloat4& v) { return v*s; }

///////////////////////////////////////////////////////////////////////////

class vdfloat2c : public vdfloat2 {
public:
	VDFORCEINLINE vdfloat2c(float x2, float y2) {x=x2; y=y2;}
	VDFORCEINLINE vdfloat2c(const float src[2]) {x=src[0]; y=src[1];}
};

class vdfloat3c : public vdfloat3 {
public:
	VDFORCEINLINE vdfloat3c(float x2, float y2, float z2) { x=x2; y=y2; z=z2; }
	VDFORCEINLINE vdfloat3c(const float src[3]) { x=src[0]; y=src[1]; z=src[2]; }
};

class vdfloat4c : public vdfloat4 {
public:
	VDFORCEINLINE vdfloat4c(float x2, float y2, float z2, float w2) { x=x2; y=y2; z=z2; w=w2; }
	VDFORCEINLINE vdfloat4c(const float src[4]) { x=src[0]; y=src[1]; z=src[2]; w=src[3]; }
};


///////////////////////////////////////////////////////////////////////////

namespace nsVDMath {
	VDFORCEINLINE float length(const vdfloat2& a) {
		return sqrtf(a.x*a.x + a.y*a.y);
	}

	VDFORCEINLINE float length(const vdfloat3& a) {
		return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
	}

	VDFORCEINLINE float length(const vdfloat4& a) {
		return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w);
	}

	VDFORCEINLINE vdfloat2 normalize(const vdfloat2& a) {
		return a / length(a);
	}

	VDFORCEINLINE vdfloat3 normalize(const vdfloat3& a) {
		return a / length(a);
	}

	VDFORCEINLINE vdfloat4 normalize(const vdfloat4& a) {
		return a / length(a);
	}

	VDFORCEINLINE float dot(const vdfloat2& a, const vdfloat2& b) {
		return a.x*b.x + a.y*b.y;
	}

	VDFORCEINLINE float dot(const vdfloat3& a, const vdfloat3& b) {
		return a.x*b.x + a.y*b.y + a.z*b.z;
	}

	VDFORCEINLINE float dot(const vdfloat4& a, const vdfloat4& b) {
		return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	}

	VDFORCEINLINE vdfloat3 cross(const vdfloat3& a, const vdfloat3& b) {
		const vdfloat3 r = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
		return r;
	}
};
