#pragma once
#include <vector>
#include <unordered_map>
#include "Entity.h"
#include <ECS_Engine.h>

struct EntitySnapshot
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 rotation;
    MeshHandle meshHandle;
    std::string meshName;
    bool alive; 
};

struct FrameSnapshot
{
    std::unordered_map<int, EntitySnapshot> entities; 
};

class Replay
{
public:
    enum class State { Idle, Recording, Replaying };

    State state = State::Idle;

    void StartRecording(int playerId);

    void StopRecording();

    void RecordFrame();

    void StartReplay();

    bool ReplayFrame();

    bool HasRecording() const { return !m_frames.empty(); }

    DirectX::XMFLOAT3 GetCurrentPlayerPos();
private:
    int m_playerId = -1;
    std::vector<FrameSnapshot> m_frames;
    int m_replayIndex = 0;
};