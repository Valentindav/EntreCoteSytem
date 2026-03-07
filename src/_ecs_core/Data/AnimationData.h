#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <DirectXMath.h>

struct Keyframe {
    float TimePos;
    DirectX::XMFLOAT3 Translation;
    DirectX::XMFLOAT3 Scale;
    DirectX::XMFLOAT4 RotationQuat;
};

struct BoneAnimation {
    std::vector<Keyframe> Keyframes;

    DirectX::XMFLOAT4X4 Interpolate(float t) const;
};

struct AnimationClip {
    float GetClipStartTime() const;
    float GetClipEndTime() const;
    void Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms) const;

    std::vector<BoneAnimation> BoneAnimations;
};

struct SkeletonData {
    std::vector<int> BoneHierarchy;
    std::vector<DirectX::XMFLOAT4X4> BoneOffsets;
    std::unordered_map<std::string, AnimationClip> Animations;

    void GetFinalTransforms(const std::string& clipName, float timePos, std::vector<DirectX::XMFLOAT4X4>& finalTransforms) const;
};