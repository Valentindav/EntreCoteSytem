#include "AnimationData.h"

using namespace DirectX;

float AnimationClip::GetClipStartTime() const
{
    float startTime = 0.0f;
    bool foundFirst = false;

    for (size_t i = 0; i < BoneAnimations.size(); ++i)
    {
        if (!BoneAnimations[i].Keyframes.empty())
        {
            float firstKeyTime = BoneAnimations[i].Keyframes.front().TimePos;

            if (!foundFirst)
            {
                startTime = firstKeyTime;
                foundFirst = true;
            }
            else if (firstKeyTime < startTime)
            {
                startTime = firstKeyTime;
            }
        }
    }
    return startTime;
}

float AnimationClip::GetClipEndTime() const
{
    float endTime = 0.0f;

    for (size_t i = 0; i < BoneAnimations.size(); ++i)
    {
        if (!BoneAnimations[i].Keyframes.empty())
        {
            float lastKeyTime = BoneAnimations[i].Keyframes.back().TimePos;
            if (lastKeyTime > endTime)
            {
                endTime = lastKeyTime;
            }
        }
    }
    return endTime;
}

void AnimationClip::Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms) const
{
    boneTransforms.resize(BoneAnimations.size());
    for (size_t i = 0; i < BoneAnimations.size(); ++i)
    {
        boneTransforms[i] = BoneAnimations[i].Interpolate(t);
    }
}

XMFLOAT4X4 BoneAnimation::Interpolate(float t) const
{
    XMFLOAT4X4 result;

    if (t <= Keyframes.front().TimePos)
    {
        XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
        XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
        XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

        XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        XMStoreFloat4x4(&result, XMMatrixAffineTransformation(S, zero, Q, P));
        return result;
    }
    else if (t >= Keyframes.back().TimePos)
    {
        XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
        XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
        XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

        XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        XMStoreFloat4x4(&result, XMMatrixAffineTransformation(S, zero, Q, P));
        return result;
    }
    else
    {
        for (size_t i = 0; i < Keyframes.size() - 1; ++i)
        {
            if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
            {
                float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

                XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
                XMVECTOR s1 = XMLoadFloat3(&Keyframes[i + 1].Scale);

                XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
                XMVECTOR p1 = XMLoadFloat3(&Keyframes[i + 1].Translation);

                XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
                XMVECTOR q1 = XMLoadFloat4(&Keyframes[i + 1].RotationQuat);

                XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
                XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
                XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

                XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
                XMStoreFloat4x4(&result, XMMatrixAffineTransformation(S, zero, Q, P));
                return result;
            }
        }
    }

    XMStoreFloat4x4(&result, XMMatrixIdentity());
    return result;
}

void SkeletonData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms) const
{
    size_t numBones = BoneOffsets.size();
    finalTransforms.resize(numBones);

    std::vector<XMFLOAT4X4> localTransforms(numBones);
    const AnimationClip& clip = Animations.at(clipName);

    for (size_t i = 0; i < numBones; ++i)
    {
        localTransforms[i] = clip.BoneAnimations[i].Interpolate(timePos);
    }

    std::vector<XMFLOAT4X4> toRootTransforms(numBones);
    toRootTransforms[0] = localTransforms[0];

    for (size_t i = 1; i < numBones; ++i)
    {
        XMMATRIX local = XMLoadFloat4x4(&localTransforms[i]);

        int parentIndex = BoneHierarchy[i];
        XMMATRIX parent = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

        XMMATRIX toRoot = XMMatrixMultiply(local, parent);
        XMStoreFloat4x4(&toRootTransforms[i], toRoot);
    }

    for (size_t i = 0; i < numBones; ++i)
    {
        XMMATRIX offset = XMLoadFloat4x4(&BoneOffsets[i]);
        XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);

        XMMATRIX finalTransform = XMMatrixMultiply(offset, toRoot);

        XMStoreFloat4x4(&finalTransforms[i], XMMatrixTranspose(finalTransform));
    }
}