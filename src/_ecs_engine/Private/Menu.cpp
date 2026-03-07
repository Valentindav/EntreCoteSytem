#include "Public/Menu.h"
#include "Public/ECS.h"
#include "Public/Entity.h"

#include "Public/ECS_Components/UiButtonComponent.h"
#include "Public/ECS_Components/UiImageComponent.h"
#include "Public/ECS_Components/UiTextComponent.h"
#include "Public/ECS_Components/UiTransformComponent.h"
#include "Public/ECS_Components/UiAnimatorComponent.h"

#include "Private/Managers/ButtonListenerManager.h"
#include "Private/EngineCore.h"

using namespace DirectX;

void Menu::SetActive(bool _active)
{
    active = _active;
    ECS* ecs = ECS_ENGINE->GetECS();

    for (Entity* ent : entityList)
    {
        if (auto* img = ecs->GetComponent<UiImageComponent>(ent->GetId()))
            img->m_isVisible = _active;

        if (auto* txt = ecs->GetComponent<UiTextComponent>(ent->GetId()))
            txt->m_isVisible = _active;

        if (auto* txt = ecs->GetComponent<UiAnimatorComponent>(ent->GetId()))
            txt->m_isPlaying = _active;
    }
}

void Menu::addButton(
    std::string _Id, XMFLOAT3 _pos, XMFLOAT3 _size, std::string _name,
    std::vector<std::string> _ListenerList, std::string _hover,
    bool _animated, int _FrameCount, int _currentFrame,
    XMFLOAT2 _FrameSize, bool _looping, float _frameRate)
{
    ECS* ecs = ECS_ENGINE->GetECS();
    Entity* buttonEnt = ecs->CreateEntity();

    auto* transform = ecs->AddComponents<UiTransformComponent>(buttonEnt);
    transform->SetRect(_pos.x, _pos.y, _size.x / ECS_ENGINE->GetClientWidth(), _size.y / ECS_ENGINE->GetClientHeight());
    transform->m_depth = _pos.z;

    _name = "../../" + _name;
    if (_hover == "") _hover = _name;
    else _hover = "../../" + _hover;

    auto* buttonComp = ecs->AddComponents<UiButtonComponent>(buttonEnt);
    buttonComp->LoadButtonStates(_name, _hover);

    for (const std::string& listener : _ListenerList) {
        buttonComp->AddListener(ButtonListenerManager::GetButtonListener(listener));
    }

    if (_animated) {
        auto* animComp = ecs->AddComponents<UiAnimatorComponent>(buttonEnt);
        animComp->Play((int)_FrameSize.x, (int)_FrameSize.y, _frameRate, _FrameCount);
        animComp->m_loop = _looping;
        animComp->m_currentFrame = _currentFrame;
    }

    entityList.push_back(buttonEnt);
    entityMap[_Id] = buttonEnt;
}

void Menu::addImage(
    std::string _Id, XMFLOAT3 _pos, XMFLOAT3 _size, std::string _name,
    bool _animated, int _FrameCount, int _currentFrame,
    XMFLOAT2 _FrameSize, bool _looping, float _frameRate)
{
    ECS* ecs = ECS_ENGINE->GetECS();
    Entity* imageEnt = ecs->CreateEntity();

    auto* transform = ecs->AddComponents<UiTransformComponent>(imageEnt);
    transform->SetRect(_pos.x, _pos.y, _size.x / ECS_ENGINE->GetClientWidth(), _size.y / ECS_ENGINE->GetClientHeight());
    transform->m_depth = _pos.z;

    _name = "../../" + _name;
    auto* imageComp = ecs->AddComponents<UiImageComponent>(imageEnt);
    imageComp->LoadTexture(_name);
    imageComp->m_isVisible = active;

    if (_animated) {
        auto* animComp = ecs->AddComponents<UiAnimatorComponent>(imageEnt);
        animComp->Play((int)_FrameSize.x, (int)_FrameSize.y, _frameRate, _FrameCount);
        animComp->m_loop = _looping;
        animComp->m_currentFrame = _currentFrame;
    }

    entityList.push_back(imageEnt);
    entityMap[_Id] = imageEnt;
}

void Menu::addText(
    std::string _Id, XMFLOAT3 _pos, XMFLOAT3 _size, std::string _name,
    XMFLOAT3 _Color, std::string _Font, std::string _text, int _FontSize)
{
    ECS* ecs = ECS_ENGINE->GetECS();
    Entity* textEnt = ecs->CreateEntity();

    auto* transform = ecs->AddComponents<UiTransformComponent>(textEnt);
    transform->SetRect(_pos.x, _pos.y, _size.x, _size.y);
    transform->m_depth = _pos.z;

    auto* textComp = ecs->AddComponents<UiTextComponent>(textEnt);

    std::string fontPng = "../../" + _Font + ".png";
    std::string fontFnt = "../../" + _Font + ".fnt";

    textComp->LoadText(_text, fontPng, fontFnt);
    textComp->m_isVisible = active;

    entityList.push_back(textEnt);
    entityMap[_Id] = textEnt;
}

Entity* Menu::getElement(std::string _Id)
{
    if (entityMap.find(_Id) != entityMap.end()) {
        return entityMap[_Id];
    }
    return nullptr;
}

void Menu::Clear()
{
    entityList.clear();
    entityMap.clear();
}