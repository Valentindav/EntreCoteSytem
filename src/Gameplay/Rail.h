#pragma once
#include <deque>
#include <vector>
#include <cmath>
#include <ECS_Engine.h>

class Rail
{
private:
    struct VisualTile { 
        int ptIndex;
        Entity* entity;
    };
    std::deque<DirectX::XMFLOAT3> m_points;
    std::deque<DirectX::XMFLOAT3> m_tangents;

    int m_firstIndex = 0;
    int m_totalAppended = 0;
    bool m_distTableDirty = true;
    float m_distanceOffset = 0.f;

    struct DistEntry { int absIndex; float cumDist; };
    std::vector<DistEntry> m_distTable;

    DirectX::XMFLOAT3 m_curPos = { 0.f, 0.f, 0.f };
    DirectX::XMFLOAT3 m_curDir = { 0.f, 0.f, 1.f };
    float m_curYaw = 0.f;
    float m_curPitch = 0.f;

    void AppendStraight();
    void AppendNext();
    void EnsureDistTable();

    std::deque<VisualTile> m_tiles;
    std::vector<Entity*>   m_pool;
    int m_lastTileSpawned = -1;

public:
    float segmentLength = 20.f;
    float maxYawDelta = 0.42f;
    float maxPitchDelta = 0.22f;
    float maxRollDelta = 0.28f;
    float minSpeed = 3.5f;
    float maxSpeed = 8.0f;

    int wallSpawnChance = 20;// 20 %
    int minWallDistance = 5;          
    int m_lastWallIndex = -999;

    int lookaheadPts = 8;  
    int pruneBackPts = 2;  
    int spawnBudget = 1;   
    float tileWidth = 50.f;
    float tileHeight = 0.08f;
    DirectX::XMFLOAT4 tileColor = { 0.0f, 0.0f, 0.6f, 1.0f };

    Rail();

    void EnsureGenerated(int upToIndex);
    void PruneUpTo(int belowIndex);
    void Build(int subdivisions = 12);

    DirectX::XMFLOAT3 GetPoint(size_t index);
    DirectX::XMFLOAT3 GetPositionAtDistance(float dist);
    DirectX::XMFLOAT3 GetTangentAtDistance(float dist);
    DirectX::XMFLOAT3 GetTangentAtIndex(int absIndex);
    int GetIndexAtDistance(float dist);

    size_t PointCount() const { return m_points.size(); }
    int FirstIndex() const { return m_firstIndex; }
    int LastIndex()  const { return m_firstIndex + (int)m_points.size() - 1; }

    void UpdateTiles(float distance);

};