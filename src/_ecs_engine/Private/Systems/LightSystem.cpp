#include "Private/Systems/LightSystem.h"

#include "Private/_engine.h"

void LightSystem::AddComponent(Component* _comp)
{
    if (_comp->GetType() == ComponentType::Light)
    {
        m_lights.push_back(static_cast<LightComponent*>(_comp));
    }
}

void LightSystem::Update()
{
    m_activeLights.clear();

    for (LightComponent* light : m_lights)
    {
		Entity* owner = light->GetOwner();

        TransformComponent* transform = &owner->transform;

        LightData data;
        data.Type = static_cast<int>(light->m_type);

        data.Color.x = light->m_color.x * light->m_intensity;
        data.Color.y = light->m_color.y * light->m_intensity;
        data.Color.z = light->m_color.z * light->m_intensity;

        DirectX::XMFLOAT3 pos = transform->GetWorldPosition();
        data.Position = pos;

        if (light->m_type == LightType::Directional)
        {
            data.Direction = light->m_dirDirection;

            // Optionnel : Si la rotation de l'entité contrôle la direction :
            // data.Direction = transform->GetForward(); 
        }
        else if (light->m_type == LightType::Point)
        {
            data.Range = light->m_pointRange;
            data.Attenuation = light->m_pointAttenuation;
        }
        else if (light->m_type == LightType::Spot)
        {
            data.Direction = light->m_spotDirection;
            data.Range = light->m_spotRange;
            data.SpotInnerAngle = light->m_spotInnerAngle;
            data.SpotOuterAngle = light->m_spotOuterAngle;
            data.Attenuation = light->m_spotAttenuation;
        }

        m_activeLights.push_back(data);
    }

    LightManager* lightManager = ECS_ENGINE->GetLightManager();
    if (lightManager)
    {
        lightManager->UpdateLights(m_activeLights);
    }

    m_lights.clear();
}