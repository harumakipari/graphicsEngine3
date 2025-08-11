#ifndef MATH_HELPER_H
#define MATH_HELPER_H

// C++ 標準ライブラリ
#include <cmath>
#include <cfloat>
#include <random>

// 他ライブラリ
#include <DirectXMath.h>

namespace MathHelper
{
    static bool VectorContainsNanOrInfinite(DirectX::FXMVECTOR v)
    {
        DirectX::XMVECTOR isInvalid = DirectX::XMVectorOrInt(DirectX::XMVectorIsNaN(v), DirectX::XMVectorIsInfinite(v));
        return DirectX::XMVector4EqualInt(isInvalid, DirectX::XMVectorTrueInt());
    }

    static bool IsValidQuaternion(const DirectX::XMFLOAT4& q)
    {
        return std::isfinite(q.x) && std::isfinite(q.y) &&
            std::isfinite(q.z) && std::isfinite(q.w);
    }

    inline float ClampAngle(float angle)
    {
        const float PI = 3.14159265f;
        const float TWO_PI = PI * 2.0f;

        angle = std::fmod(angle, TWO_PI);
        if (angle > PI)
        {
            angle -= TWO_PI;
        }
        else if (angle < -PI)
        {
            angle += TWO_PI;
        }
        return angle;
    }



    inline float RandomRange(float min, float max)
    {
        if (min >= max) 
        {
            std::swap(min, max);
        }

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(min, max);
        return dist(gen);
    }

    //inline bool AlmostEqualRelative(float a, float b,float maxRelDiff = FLT_EPSILON)
    //{
    //    float diff = std::fabs(a - b);
    //    float absA = std::fabs(a), absB = std::fabs(b);
    //    float largest = std::max<float>(absA, absB);
    //    return diff <= largest * maxRelDiff;
    //}

    enum class RotationSequence 
    {
        zyx, zxy, xyz, xzy, yxz, yzx 
    };

    // クォータニオンから角度に変更する
    static DirectX::XMFLOAT3 QuaternionToEuler(DirectX::XMFLOAT4 quaternion, RotationSequence rotationSequenceToUse = RotationSequence::yxz)
    {
        int i = 0;
        int j = 0;
        int k = 0;
        // The first action in the function is to reverse the sequence if the rotation is intrinsic. As we only do intrinsic rotations, we have to make sure i,j and k are reversed
        // So if e.g. the sequence is ZXY, i should be 1, j should be 0 and k should be 2 as the sequence to use is yxz. 
        switch (rotationSequenceToUse)
        {
        case RotationSequence::zyx:
            i = 0;
            j = 1;
            k = 2;
            break;
        case RotationSequence::zxy:
            i = 1;
            j = 0;
            k = 2;
            break;
        case RotationSequence::xyz:
            i = 2;
            j = 1;
            k = 0;
            break;
        case RotationSequence::xzy:
            i = 1;
            j = 2;
            k = 0;
            break;
        case RotationSequence::yxz:
            i = 2;
            j = 0;
            k = 1;
            break;
        case RotationSequence::yzx:
            i = 0;
            j = 2;
            k = 1;
            break;
        }

        float sign = (float)static_cast<int>((i - j) * (j - k) * (k - i) / 2);

        float angles[3] = { 0.0f };

        float quat[4] = { quaternion.x, quaternion.y, quaternion.z, quaternion.w };
        float a = quat[3] - quat[j];
        float b = quat[i] + quat[k] * sign;
        float c = quat[j] + quat[3];
        float d = quat[k] * sign - quat[i];

        float n2 = a * a + b * b + c * c + d * d;

        // always not proper as we only support Tait-Bryan angles/rotations

        angles[1] = std::acos((2.0f * (a * a + b * b) / n2) - 1.0f);
        bool safe1 = abs(angles[1]) >= FLT_EPSILON;
        bool safe2 = abs(angles[1] - DirectX::XM_PI) >= FLT_EPSILON;
        if (safe1 && safe2)
        {
            float half_sum = std::atan2(b, a);
            float half_diff = std::atan2(-d, c);

            angles[0] = half_sum + half_diff;
            angles[2] = half_sum - half_diff;
        }
        else
        {
            // always intrinsic as we rotate a camera
            angles[0] = 0.0f;

            if (!safe1)
            {
                float half_sum = std::atan2(b, a);
                angles[2] = 2.0f * half_sum;
            }
            if (!safe2)
            {
                float half_diff = std::atan2(-d, c);
                angles[2] = -2.0f * half_diff;
            }
        }

        for (int index = 0; index < 3; index++)
        {
            if (angles[index] < -DirectX::XM_PI)
            {
                angles[index] += DirectX::XM_2PI;
            }
            else
            {
                if (angles[index] > DirectX::XM_PI)
                {
                    angles[index] -= DirectX::XM_2PI;
                }
            }
        }
        // always not proper as we only support Tait-Bryan angles/rotations
        angles[2] *= sign;
        angles[1] -= DirectX::XM_PIDIV2;
        // reversal, always intrinsic
        float tmp = angles[0];
        angles[0] = angles[2];
        angles[2] = tmp;

        // angle 1 is pitch, angle 0 is yaw and angle 2 is roll... 
        return { ClampAngle(angles[1]), ClampAngle(angles[0]), ClampAngle(angles[2]) };
    }


    // 各成分が 0 以上なら +1.0f、負なら -1.0f となるベクトルを返す関数
    // ビット演算を用いて高速に符号を判定する。
    static DirectX::XMVECTOR VectorSign(DirectX::FXMVECTOR v)
    {
        // 全成分が +1.0f のベクトルを用意する
        DirectX::XMVECTOR one = DirectX::XMVectorSplatOne();

        // 各成分の符号ビットだけを取り出すためのマスク（0x80000000）
        DirectX::XMVECTOR signMask = DirectX::XMVectorSplatSignMask();

        // v の各成分から符号ビットだけを抽出（AND演算でマスク適用）
        DirectX::XMVECTOR signBits = DirectX::XMVectorAndInt(v, signMask);

        // +1.0f（= 0x3F800000）に符号ビットを OR 演算で合成：
        // → 0xBF800000（= -1.0f）になる可能性がある
        // 結果として、v の各成分が負なら -1.0f、正なら +1.0f になる
        return DirectX::XMVectorOrInt(one, signBits);
    }
}

#endif //MATH_HELPER_H
