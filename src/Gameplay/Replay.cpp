#include "Replay.h"
#include <vector>
#include <unordered_map>
#include <ECS_Engine.h>

void Replay::StartRecording(int playerId)
{
    m_frames.clear();
    m_playerId = playerId;
    state = State::Recording;
}

void Replay::StopRecording()
{
    state = State::Idle;
}

void Replay::RecordFrame()
{
    if (state != State::Recording) return;

    FrameSnapshot snap;
    for (int i = 0; i < ECS_ECS->GetEntitiesSize(); ++i)
    {
        Entity* e = ECS_ECS->GetEntity(i);
        if (!e) continue;

        EntitySnapshot es;
        es.position = e->transform.GetWorldPosition();
        es.rotation = e->transform.GetWorldRotation();
        es.alive = true;
        MeshComponent* mesh = e->GetComponent<MeshComponent>();
        if (mesh)
        {
            es.meshHandle = mesh->GetMeshHandle();
            es.meshName = mesh->GetMeshName(); 
        }
        snap.entities[e->GetId()] = es;
    }
    m_frames.push_back(snap);


    if (m_frames.size() > 60 * 1000)
        m_frames.erase(m_frames.begin());
}

void Replay::StartReplay()
{
    if (m_frames.empty()) return;
    m_replayIndex = 0;
    state = State::Replaying;

    const FrameSnapshot& firstFrame = m_frames[0];
    for (const auto& pair : firstFrame.entities)
    {
        int recordedId = pair.first;
        const EntitySnapshot& es = pair.second;

        while (ECS_ECS->GetEntitiesSize() <= recordedId)
        {
            Entity* ghost = ECS_ECS->CreateEntity();
            MeshComponent* mesh = ECS_ECS->AddComponents<MeshComponent>(ghost);
            if (!es.meshName.empty())
                mesh->LoadMesh(es.meshName);
        }
    }
}

bool Replay::ReplayFrame()
{
    if (state != State::Replaying) return false;
    if (m_replayIndex >= (int)m_frames.size())
    {
        state = State::Idle;
        return false;
    }

    const FrameSnapshot& snap = m_frames[m_replayIndex];
    for (const auto& pair : snap.entities)
    {
        int id = pair.first;
        const EntitySnapshot& es = pair.second;

        Entity* e = ECS_ECS->GetEntity(id);
        if (!e) continue;

        e->transform.SetWorldPosition(es.position);
        e->transform.SetWorldRotation(es.rotation);

        MeshComponent* mesh = e->GetComponent<MeshComponent>();
        if (mesh && !es.meshName.empty())
            mesh->LoadMesh(es.meshName);
    }

    ++m_replayIndex;
    return true;
}

DirectX::XMFLOAT3 Replay::GetCurrentPlayerPos()
{
    if (m_replayIndex <= 0 || m_frames.empty()) return { 0,0,0 };

    int frameToRead = std::min<float>(m_replayIndex - 1, (int)m_frames.size() - 1);

    std::unordered_map<int, EntitySnapshot>::iterator it = m_frames[frameToRead].entities.find(m_playerId);
    if (it != m_frames[frameToRead].entities.end())
        return it->second.position;
    return { 0,0,0 };
}
