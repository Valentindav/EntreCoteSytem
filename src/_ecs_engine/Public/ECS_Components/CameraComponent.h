#pragma once
#include "private/Component.h"

#include <RenderCamera.h>

#include "Private/EngineCore.h"
#include "Private/Window.h"

class CameraComponent : public Component
{
public:
	bool m_isActive = true;
	int m_renderOrder = 0;

	float m_viewX = 0.0f;
	float m_viewY = 0.0f;
	float m_viewWidth = 1.0f;
	float m_viewHeight = 1.0f;

	RenderCamera* m_renderCamera = nullptr;

	CameraComponent()
	{
		int clientWidth = ECS_ENGINE->GetClientWidth();
		int clientHeight = ECS_ENGINE->GetClientHeight();

		m_renderCamera = new RenderCamera(clientWidth, clientHeight);
	};

	~CameraComponent() {
		delete m_renderCamera;
		m_renderCamera = nullptr;
	}

private:

	virtual const ComponentType::Type GetType() { return ComponentType::Camera; }

	friend class CameraSystem;
};