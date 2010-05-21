class vdint2 {
public:
	typedef vdint2 self_type;
	typedef int value_type;

	void set(int x2, int y2) { x=x2; y=y2; }

	int&		operator[](int k)					{ return v[k]; }
	const int&	operator[](int k) const				{ return v[k]; }

	int			lensq() const						{ return x*x + y*y; }
	int			len() const							{ return (int)sqrtf((float)(x*x + y*y)); }
	self_type	normalized() const					{ return *this / len(); }

	self_type	operator-() const					{ const self_type a = {-x, -y}; return a; }

	self_type	operator+(const self_type& r) const	{ const self_type a = {x+r.x, y+r.y}; return a; }
	self_type	operator-(const self_type& r) const	{ const self_type a = {x-r.x, y-r.y}; return a; }

	self_type&	operator+=(const self_type& r)		{ x+=r.x; y+=r.y; return *this; }
	self_type&	operator-=(const self_type& r)		{ x-=r.x; y-=r.y; return *this; }

	self_type	operator*(const int s) const		{ const self_type a = {x*s, x*s}; return a; }
	self_type&	operator*=(const int s)				{ x*=s; y*=s; return *this; }

	self_type	operator/(const int s) const		{ const self_type a = {x/s, y/s}; return a; }
	self_type&	operator/=(const int s)				{ x/=s; y/=s; return *this; }

	self_type	operator*(const self_type& r) const		{ self_type a = {x*r.x, y*r.y}; return a; }
	self_type&	operator*=(const self_type& r)			{ x*=r.x; y*=r.y; return *this; }

	self_type	operator/(const self_type& r) const		{ self_type a = {x/r.x, y/r.y}; return a; }
	self_type&	operator/=(const self_type& r)			{ x/=r.x; y/=r.y; return *this; }

	union {
		struct {
			int x;
			int y;
		};
		int v[2];
	};
};

VDFORCEINLINE vdint2 operator*(const int s, const vdint2& v) { return v*s; }

///////////////////////////////////////////////////////////////////////////

class vdint3 {
public:
	typedef vdint3 self_type;
	typedef int value_type;

	int&		operator[](int k)					{ return v[k]; }
	const int&	operator[](int k) const				{ return v[k]; }

	int			lensq() const						{ return x*x + y*y + z*z; }
	int			len() const							{ return (int)sqrtf((float)(x*x + y*y + z*z)); }
	self_type	normalized() const					{ return *this / len(); }

	vdint2	project() const						{ const int inv(int(1)/z); const vdint2 a = {x*inv, y*inv}; return a; }
	vdint2	as2d() const						{ const vdint2 a = {x, y}; return a; }

	self_type	operator-() const					{ const self_type a = {-x, -y, -z}; return a; }

	self_type	operator+(const self_type& r) const	{ const self_type a = {x+r.x, y+r.y, z+r.z}; return a; }
	self_type	operator-(const self_type& r) const	{ const self_type a = {x-r.x, y-r.y, z-r.z}; return a; }

	self_type&	operator+=(const self_type& r)		{ x+=r.x; y+=r.y; z+=r.z; return *this; }
	self_type&	operator-=(const self_type& r)		{ x-=r.x; y-=r.y; z-=r.z; return *this; }

	self_type	operator*(const int s) const		{ const self_type a = {x*s, y*s, z*s}; return a; }
	self_type&	operator*=(const int s)				{ x*=s; y*=s; z*=s; return *this; }

	self_type	operator/(const int s) const		{ const self_type a = {x/s, y/s, z/s}; return a; }
	self_type&	operator/=(const int s)				{ x /= s; y /= s; z /= s; return *this; }

	self_type	operator*(const self_type& r) const		{ self_type a = {x*r.x, y*r.y, z*r.z}; return a; }
	self_type&	operator*=(const self_type& r)			{ x*=r.x; y*=r.y; z*=r.z; return *this; }

	self_type	operator/(const self_type& r) const		{ self_type a = {x/r.x, y/r.y, z/r.z}; return a; }
	self_type&	operator/=(const self_type& r)			{ x/=r.x; y/=r.y; z/=r.z; return *this; }

	union {
		struct {
			int x;
			int y;
			int z;
		};
		int v[3];
	};
};

VDFORCEINLINE vdint3 operator*(const int s, const vdint3& v) { return v*s; }

///////////////////////////////////////////////////////////////////////////

class vdint4 {
public:
	typedef vdint4 self_type;
	typedef int value_type;

	int&			operator[](int i) { return v[i]; }
	const int&	operator[](int i) const { return v[i]; }

	int			lensq() const						{ return x*x + y*y + z*z + w*w; }
	int			len() const							{ return (int)sqrtf((float)(x*x + y*y + z*z + w*w)); }
	self_type	normalized() const					{ return *this / len(); }

	vdint3	project() const						{ const int inv(int(1)/w); const vdint3 a = {x*inv, y*inv, z*inv}; return a; }

	self_type	operator-() const					{ const self_type a = {-x, -y, -z, -w}; return a; }

	self_type	operator+(const self_type& r) const	{ const self_type a = {x+r.x, y+r.y, z+r.z, w+r.w}; return a; }
	self_type	operator-(const self_type& r) const	{ const self_type a = {x-r.x, y-r.y, z-r.z, w-r.w}; return a; }

	self_type&	operator+=(const self_type& r)		{ x+=r.x; y+=r.y; z+=r.z; w+=r.w; return *this; }
	self_type&	operator-=(const self_type& r)		{ x-=r.x; y-=r.y; z-=r.z; w-=r.w; return *this; }

	self_type	operator*(const int factor) const	{ const self_type a = {x*factor, y*factor, z*factor, w*factor}; return a; }
	self_type	operator/(const int factor) const	{ const self_type a = {x/factor, y/factor, z/factor, w/factor}; return a; }

	self_type&	operator*=(const int factor)		{ x *= factor; y *= factor; z *= factor; w *= factor; return *this; }
	self_type&	operator/=(const int factor)		{ x /= factor; y /= factor; z /= factor; w /= factor; return *this; }

	self_type	operator*(const self_type& r) const		{ self_type a = {x*r.x, y*r.y, z*r.z, w*r.w}; return a; }
	self_type&	operator*=(const self_type& r)			{ x*=r.x; y*=r.y; z*=r.z; w*=r.w; return *this; }

	self_type	operator/(const self_type& r) const		{ self_type a = {x/r.x, y/r.y, z/r.z, w*r.w}; return a; }
	self_type&	operator/=(const self_type& r)			{ x/=r.x; y/=r.y; z/=r.z; w/=r.w; return *this; }

	union {
		struct {
			int x;
			int y;
			int z;
			int w;
		};
		int v[4];
	};
};

VDFORCEINLINE vdint4 operator*(const int s, const vdint4& v) { return v*s; }

///////////////////////////////////////////////////////////////////////////

class vdint2c : vdint2 {
public:
	VDFORCEINLINE vdint2c(int x2, int y2) {x=x2; y=y2;}
	VDFORCEINLINE vdint2c(const int src[2]) {x=src[0]; y=src[1];}
};

class vdint3c : vdint3 {
public:
	VDFORCEINLINE vdint3c(int x2, int y2, int z2) { x=x2; y=y2; z=z2; }
	VDFORCEINLINE vdint3c(const int src[3]) { x=src[0]; y=src[1]; z=src[2]; }
};

class vdint4c : vdint4 {
public:
	VDFORCEINLINE vdint4c(int x2, int y2, int z2, int w2) { x=x2; y=y2; z=z2; w=w2; }
	VDFORCEINLINE vdint4c(const int src[4]) { x=src[0]; y=src[1]; z=src[2]; w=src[3]; }
};

///////////////////////////////////////////////////////////////////////////

namespace nsVDMath {
	VDFORCEINLINE int dot(const vdint2& a, const vdint2& b) {
		return a.x*b.x + a.y*b.y;
	}

	VDFORCEINLINE int dot(const vdint3& a, const vdint3& b) {
		return a.x*b.x + a.y*b.y + a.z*b.z;
	}

	VDFORCEINLINE int dot(const vdint4& a, const vdint4& b) {
		return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	}

	VDFORCEINLINE vdint3 cross(const vdint3& a, const vdint3& b) {
		const vdint3 r = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
		return r;
	}
};
