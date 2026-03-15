#ifndef AUDIO_DEFINE_H_INCLUDED
#define AUDIO_DEFINE_H_INCLUDED

// ============================================================
//  Primitive type aliases  (replaces _ecs_core define.h)
// ============================================================
#include <cstdint>
#include <iostream>
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using float32 = float;
using float64 = double;
using char16 = wchar_t;

// ============================================================
//  Container aliases  (replaces Containers/Vector.hpp etc.)
// ============================================================
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

template<typename T>
using Vector = std::vector<T>;

template<typename K, typename V>
using UnorderedMap = std::unordered_map<K, V>;

// ============================================================
//  Math  (replaces Core/Maths — uses DirectXMath + std)
// ============================================================
#include <DirectXMath.h>
#include <cmath>

namespace audio
{
    // --------------------------------------------------------
    //  Constants
    // --------------------------------------------------------
    constexpr float PI = 3.14159265358979323846f;

    // --------------------------------------------------------
    //  Scalar helpers
    // --------------------------------------------------------
    template<typename T>
    inline T Clamp(T value, T lo, T hi)
    {
        return (value < lo) ? lo : (value > hi) ? hi : value;
    }

    inline float32 Sqrt(float32 v) { return std::sqrt(v); }
    inline float32 Sin(float32 v) { return std::sin(v); }
    inline float32 Cos(float32 v) { return std::cos(v); }
    inline float32 Tan(float32 v) { return std::tan(v); }
    inline float32 Abs(float32 v) { return std::abs(v); }
    inline float32 Pow(float32 b, float32 e) { return std::pow(b, e); }

    // --------------------------------------------------------
    //  Vector3f32  — full 3D math replacing Core/Maths/Vector3
    // --------------------------------------------------------
    struct Vector3f32
    {
        float x, y, z;

        // --- Constructors ---
        Vector3f32() : x(0.f), y(0.f), z(0.f) {}
        Vector3f32(float x, float y, float z) : x(x), y(y), z(z) {}

        // --- DirectXMath interop ---
        explicit Vector3f32(const DirectX::XMFLOAT3& v) : x(v.x), y(v.y), z(v.z) {}
        DirectX::XMVECTOR  ToXMVector()  const { return DirectX::XMVectorSet(x, y, z, 0.f); }
        DirectX::XMFLOAT3  ToXMFloat3()  const { return { x, y, z }; }
        static Vector3f32  FromXMFloat3(const DirectX::XMFLOAT3& v) { return { v.x, v.y, v.z }; }

        // --- Arithmetic operators ---
        Vector3f32 operator+(const Vector3f32& o) const { return { x + o.x, y + o.y, z + o.z }; }
        Vector3f32 operator-(const Vector3f32& o) const { return { x - o.x, y - o.y, z - o.z }; }
        Vector3f32 operator*(float s)             const { return { x * s,   y * s,   z * s }; }
        Vector3f32 operator/(float s)             const { return { x / s,   y / s,   z / s }; }
        Vector3f32& operator+=(const Vector3f32& o) { x += o.x; y += o.y; z += o.z; return *this; }
        Vector3f32& operator-=(const Vector3f32& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
        Vector3f32& operator*=(float s) { x *= s;   y *= s;   z *= s;   return *this; }
        Vector3f32& operator/=(float s) { x /= s;   y /= s;   z /= s;   return *this; }
        Vector3f32  operator-()                   const { return { -x, -y, -z }; }

        // --- Comparison ---
        bool operator==(const Vector3f32& o) const { return x == o.x && y == o.y && z == o.z; }
        bool operator!=(const Vector3f32& o) const { return !(*this == o); }

        // --- Vector operations ---
        float DotProduct(const Vector3f32& o) const
        {
            return x * o.x + y * o.y + z * o.z;
        }

        Vector3f32 CrossProduct(const Vector3f32& o) const
        {
            return
            {
                y * o.z - z * o.y,
                z * o.x - x * o.z,
                x * o.y - y * o.x
            };
        }

        float Norm() const
        {
            return std::sqrt(x * x + y * y + z * z);
        }

        float NormSquared() const
        {
            return x * x + y * y + z * z;
        }

        Vector3f32 Normalize() const
        {
            float n = Norm();
            if (n < 1e-8f) return { 0.f, 0.f, 0.f };
            return { x / n, y / n, z / n };
        }

        // Distance to another vector
        float DistanceTo(const Vector3f32& o) const
        {
            return (*this - o).Norm();
        }

        // Linear interpolation
        static Vector3f32 Lerp(const Vector3f32& a, const Vector3f32& b, float t)
        {
            return { a.x + (b.x - a.x) * t,
                     a.y + (b.y - a.y) * t,
                     a.z + (b.z - a.z) * t };
        }

        // Zero / unit vectors
        static Vector3f32 Zero() { return { 0.f, 0.f,  0.f }; }
        static Vector3f32 Up() { return { 0.f, 1.f,  0.f }; }
        static Vector3f32 Forward() { return { 0.f, 0.f,  1.f }; }
        static Vector3f32 Right() { return { 1.f, 0.f,  0.f }; }
    };

    // Scalar * vector (commutative form)
    inline Vector3f32 operator*(float s, const Vector3f32& v) { return v * s; }

} 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <xaudio2.h>
#include <x3daudio.h>

#endif 