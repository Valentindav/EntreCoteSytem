#include "Rail.h"
#include <cstdlib> 
#include <ctime> 
#include "WallScript.h"

float RandRange(float min, float max)
{
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

Rail::Rail()
{
    srand(time(NULL));

    for (int i = 0; i < 1; ++i)
        AppendStraight();
}

DirectX::XMFLOAT3 Rail::GetPoint(size_t index)
{
    EnsureGenerated((int)index);
    int local = (int)index - m_firstIndex;
    if (local < 0 || local >= (int)m_points.size())
        return { 0.f, 0.f, 0.f };
    return m_points[local];
}

void Rail::EnsureGenerated(int upToIndex)
{
    while (LastIndex() < upToIndex)
        AppendNext();
}

void Rail::PruneUpTo(int belowIndex)
{
    while (!m_points.empty() && m_firstIndex < belowIndex)
    {
        if (m_points.size() >= 2)
        {
            const auto& a = m_points[0];
            const auto& b = m_points[1];
            float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
            m_distanceOffset += std::sqrt(dx * dx + dy * dy + dz * dz);
        }
        m_points.pop_front();
        m_tangents.pop_front();
        ++m_firstIndex;
    }
    m_distTableDirty = true;
}

void Rail::Build(int subdivisions)
{
    m_distTable.clear();
    if (m_points.empty()) return;

    m_distTable.reserve(m_points.size() * subdivisions);
    float acc = m_distanceOffset;
    m_distTable.push_back({ m_firstIndex, acc });

    for (int i = 1; i < (int)m_points.size(); ++i)
    {
        const auto& a = m_points[i - 1];
        const auto& b = m_points[i];
        float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
        acc += std::sqrt(dx * dx + dy * dy + dz * dz);
        m_distTable.push_back({ m_firstIndex + i, acc });
    }
    m_distTableDirty = false;
}

DirectX::XMFLOAT3 Rail::GetPositionAtDistance(float dist)
{
    EnsureDistTable();
    if (m_distTable.empty()) return {};

    int lo = 0, hi = (int)m_distTable.size() - 1;
    while (lo < hi - 1)
    {
        int mid = (lo + hi) / 2;
        if (m_distTable[mid].cumDist <= dist) lo = mid; else hi = mid;
    }

    float d0 = m_distTable[lo].cumDist;
    float d1 = m_distTable[hi].cumDist;
    float t = (d1 > d0) ? (dist - d0) / (d1 - d0) : 0.f;

    int localLo = std::max<float>(0, std::min<float>(m_distTable[lo].absIndex - m_firstIndex, (int)m_points.size() - 1));
    int localHi = std::max<float>(0, std::min<float>(m_distTable[hi].absIndex - m_firstIndex, (int)m_points.size() - 1));

    const auto& a = m_points[localLo];
    const auto& b = m_points[localHi];
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
}

DirectX::XMFLOAT3 Rail::GetTangentAtDistance(float dist)
{
    EnsureDistTable();
    if (m_distTable.empty()) return { 0.f, 0.f, 1.f };

    int lo = 0, hi = (int)m_distTable.size() - 1;
    while (lo < hi - 1)
    {
        int mid = (lo + hi) / 2;
        if (m_distTable[mid].cumDist <= dist) lo = mid; else hi = mid;
    }

    float d0 = m_distTable[lo].cumDist;
    float d1 = m_distTable[hi].cumDist;
    float t = (d1 > d0) ? (dist - d0) / (d1 - d0) : 0.f;

    int localLo = std::max<float>(0, std::min<float>(m_distTable[lo].absIndex - m_firstIndex, (int)m_tangents.size() - 1));
    int localHi = std::max<float>(0, std::min<float>(m_distTable[hi].absIndex - m_firstIndex, (int)m_tangents.size() - 1));

    const auto& a = m_tangents[localLo];
    const auto& b = m_tangents[localHi];
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
}

int Rail::GetIndexAtDistance(float dist)
{
    EnsureDistTable();
    if (m_distTable.empty()) return m_firstIndex;

    for (int i = 1; i < (int)m_distTable.size(); ++i)
        if (m_distTable[i].cumDist >= dist)
            return m_distTable[i].absIndex;

    return m_distTable.back().absIndex;
}

DirectX::XMFLOAT3 Rail::GetTangentAtIndex(int absIndex)
{
    int local = absIndex - m_firstIndex;
    if (local < 0 || local >= (int)m_tangents.size())
        return { 0.f, 0.f, 1.f };
    return m_tangents[local];
}

void Rail::AppendStraight()
{
    m_points.push_back(m_curPos);
    m_tangents.push_back(m_curDir);
    m_curPos.z += segmentLength;
    ++m_totalAppended;
    m_distTableDirty = true;
}

void Rail::AppendNext()
{
    m_curYaw += RandRange(-maxYawDelta, maxYawDelta);
    m_curPitch += RandRange(-maxPitchDelta, maxPitchDelta);
    if (m_curPitch > 0.75f) m_curPitch = 0.75f;
    if (m_curPitch < -0.75f) m_curPitch = -0.75f;

    m_curDir.x = std::cos(m_curPitch) * std::sin(m_curYaw);
    m_curDir.y = 0.0f;
    m_curDir.z = std::cos(m_curPitch) * std::cos(m_curYaw);
    float len = std::sqrt(m_curDir.x * m_curDir.x + m_curDir.y * m_curDir.y + m_curDir.z * m_curDir.z);
    if (len > 1e-4f) { m_curDir.x /= len; m_curDir.y /= len; m_curDir.z /= len; }

    m_points.push_back(m_curPos);
    m_tangents.push_back(m_curDir);

    m_curPos.x += m_curDir.x * segmentLength;
    m_curPos.y += m_curDir.y * segmentLength;
    m_curPos.z += m_curDir.z * segmentLength;

    ++m_totalAppended;
    m_distTableDirty = true;
}

void Rail::EnsureDistTable()
{
    if (m_distTableDirty) Build();
}

void Rail::UpdateTiles(float distance)
{
    int currentIdx = GetIndexAtDistance(distance);
    EnsureGenerated(currentIdx + lookaheadPts);

    int from = (m_lastTileSpawned < 0) ? FirstIndex() : m_lastTileSpawned + 1;
    int upTo = currentIdx + lookaheadPts;
    if (upTo > LastIndex()) upTo = LastIndex();

    int spawned = 0;
    for (int i = from; i <= upTo && spawned < spawnBudget; ++i, ++spawned)
    {
        DirectX::XMFLOAT3 pt = GetPoint(i);
        DirectX::XMFLOAT3 tan = GetTangentAtIndex(i);

        float tLen = std::sqrt(tan.x * tan.x + tan.y * tan.y + tan.z * tan.z);
        float yaw = 0.f, pitch = 0.f;
        if (tLen > 1e-4f)
        {

            tan.x /= tLen; tan.y /= tLen; tan.z /= tLen;
            yaw = std::atan2(tan.x, tan.z);
            pitch = -std::asin(std::max<float>(-1.f, std::min<float>(1.f, tan.y)));
        }

        Entity* tile = nullptr;
        Entity* tileRight = nullptr;
        Entity* tileLeft = nullptr;
        if (!m_pool.empty() && m_pool.size() >= 3.f)
        {
            tile = m_pool.back();
            m_pool.pop_back();

            tileRight = m_pool.back();
            m_pool.pop_back();

            tileLeft = m_pool.back();
            m_pool.pop_back();
        }
       else
        {
           tile = Geometry::Cube();
           MaterialComponent* mat = ECS_ECS->AddComponents<MaterialComponent>(tile);
           mat->SetColor(tileColor);

           tileRight = Geometry::Cube();
           MaterialComponent* matRight = ECS_ECS->AddComponents<MaterialComponent>(tileRight);
           matRight->SetColor(tileColor);

           tileLeft = Geometry::Cube();
           MaterialComponent* matLeft = ECS_ECS->AddComponents<MaterialComponent>(tileLeft);
           matLeft->SetColor(tileColor);
        }

        DirectX::XMFLOAT3 up = { 0.f, 1.f, 0.f };
        DirectX::XMFLOAT3 right = {
            up.y * tan.z - up.z * tan.y,
            up.z * tan.x - up.x * tan.z,
            up.x * tan.y - up.y * tan.x
        };
        float rLen = std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
        if (rLen > 1e-4f) { right.x /= rLen; right.y /= rLen; right.z /= rLen; }

        float wallHeight = 4.0f;

        tile->transform.SetWorldPosition(pt);
        tile->transform.SetWorldScale({ tileWidth, tileHeight, segmentLength*2 });
        tile->transform.SetWorldRotation({ pitch, yaw, 0.f });

        tileRight->transform.SetWorldPosition({pt.x + right.x * (tileWidth * 0.5f),pt.y + wallHeight * 0.5f,pt.z + right.z * (tileWidth * 0.5f)});
        tileRight->transform.SetWorldScale({ tileHeight, wallHeight, segmentLength*2 });
        tileRight->transform.SetWorldRotation({ pitch, yaw, 0.f }); 

        tileLeft->transform.SetWorldPosition({pt.x - right.x * (tileWidth * 0.5f),pt.y + wallHeight * 0.5f,pt.z - right.z * (tileWidth * 0.5f)});
        tileLeft->transform.SetWorldScale({ tileHeight, wallHeight, segmentLength*2 });
        tileLeft->transform.SetWorldRotation({ pitch, yaw, 0.f });

        m_tiles.push_back({ i, tile });
        if (i - m_lastWallIndex >= minWallDistance)
        {
            int randomValue = rand() % 100;
            if (randomValue < wallSpawnChance)
            {
                Entity* wall = Geometry::Cube();

                MaterialComponent* wallMat = ECS_ECS->AddComponents<MaterialComponent>(wall);
                wallMat->SetColor({ 0.9f, 0.3f, 0.1f, 1.0f }); 

                DirectX::XMFLOAT3 wallPos = {pt.x + tan.x * segmentLength * 0.5f,pt.y + 2.0f, pt.z + tan.z * segmentLength * 0.5f};

                wall->transform.SetWorldPosition(wallPos);
                wall->transform.SetWorldScale({ tileWidth * 0.8f, 3.0f, 0.5f });  
                wall->transform.SetWorldRotation({ pitch, yaw, 0.f });

                ScriptComponent* wallScript = ECS_ECS->AddComponents<ScriptComponent>(wall);
                wallScript->SetScript<WallScript>();

                m_lastWallIndex = i;
            }
        }
        m_lastTileSpawned = i;
    }

    int pruneBelow = currentIdx - pruneBackPts;
    while (!m_tiles.empty() && m_tiles.front().ptIndex < pruneBelow)
    {
        if (m_tiles.front().entity)
            m_pool.push_back(m_tiles.front().entity);
        m_tiles.pop_front();
    }

    PruneUpTo(pruneBelow);
}