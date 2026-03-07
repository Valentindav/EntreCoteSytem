//***************************************************************************************
// MathHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper math class.
//***************************************************************************************

#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <cstdint>

class MathHelper
{
public:
	// Returns random float in [0, 1).
	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	static float RandF(float a, float b)
	{
		return a + RandF()*(b-a);
	}

    static int Rand(int a, int b)
    {
        return a + rand() % ((b - a) + 1);
    }

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}
	 
	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b-a)*t;
	}

	static DirectX::XMVECTOR LerpVec(DirectX::XMVECTOR a,DirectX::XMVECTOR b,float t)
	{
		return DirectX::XMVectorLerp(a, b, t);
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}

	static float AngleFromXY(float x, float y);

	static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
	{
		return DirectX::XMVectorSet(
			radius*sinf(phi)*cosf(theta),
			radius*cosf(phi),
			radius*sinf(phi)*sinf(theta),
			1.0f);
	}

    static DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX M)
	{
        DirectX::XMMATRIX A = M;
        A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
        return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}

    static DirectX::XMFLOAT4X4 Identity4x4()
    {
        static DirectX::XMFLOAT4X4 I(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        return I;
    }

	static float ToRadians(float degrees)
	{
		return degrees * (Pi / 180.0f);
	}

	static float ToDegrees(float radians)
	{
		return radians * (180.0f / Pi);
	}

	static float NormalizeAngle(float angle)
	{
		while (angle < 0.0f) angle += 2.0f * Pi;
		while (angle >= 2.0f * Pi) angle -= 2.0f * Pi;
		return angle;
	}

	static float Distance(DirectX::XMVECTOR a, DirectX::XMVECTOR b)
	{

		DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(a, b);
		DirectX::XMVECTOR len = DirectX::XMVector3Length(diff);

		return DirectX::XMVectorGetX(len);
	}

	static DirectX::XMVECTOR Reflect(DirectX::XMVECTOR v,DirectX::XMVECTOR n)
	{
		return DirectX::XMVector3Reflect(v, n);
	}


    static DirectX::XMVECTOR RandUnitVec3();
    static DirectX::XMVECTOR RandHemisphereUnitVec3(DirectX::XMVECTOR n);

	static const float Infinity;
	static const float Pi;


};

inline DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
	return DirectX::XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& a, float scalar) {
	return DirectX::XMFLOAT3(a.x * scalar, a.y * scalar, a.z * scalar);
}

inline DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& v) {
	DirectX::XMVECTOR vec = DirectX::XMLoadFloat3(&v);
	vec = DirectX::XMVector3Normalize(vec);

	DirectX::XMFLOAT3 res;
	DirectX::XMStoreFloat3(&res, vec);
	return res;
}