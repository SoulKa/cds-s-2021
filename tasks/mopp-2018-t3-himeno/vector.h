template <typename T>
class Vector3 {
    public:

        T x;
        T y;
        T z;

        Vector3() {}
        Vector3( T x, T y, T z ) : x(x), y(y), z(z) {}
};

template <typename T>
class Vector4 {
    public:

        T x;
        T y;
        T z;
        T d;

        Vector4() {}
        Vector4( T x, T y, T z, T d ) : x(x), y(y), z(z), d(d) {}
};