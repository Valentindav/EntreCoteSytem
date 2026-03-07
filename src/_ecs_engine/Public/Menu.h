#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <DirectXMath.h>

class Entity;

class Menu
{
public:
    std::string name = "";
    bool active = true;

    std::vector<Entity*> entityList;
    std::unordered_map<std::string, Entity*> entityMap;

    void Start() {};
    void SetActive(bool _active);

    void addButton(
        std::string _Id,
        DirectX::XMFLOAT3 _pos,
        DirectX::XMFLOAT3 _size,
        std::string _name,
        std::vector<std::string> _ListenerList,
        std::string _hover = "",
        bool _animated = false,
        int _FrameCount = 0,
        int _currentFrame = 0,
        DirectX::XMFLOAT2 _FrameSize = { 0, 0 },
        bool _looping = false,
        float _frameRate = 0
    );

    void addImage(
        std::string _Id,
        DirectX::XMFLOAT3 _pos,
        DirectX::XMFLOAT3 _size,
        std::string _name,
        bool _animated = false,
        int _FrameCount = 0,
        int _currentFrame = 0,
        DirectX::XMFLOAT2 _FrameSize = { 0, 0 },
        bool _looping = false,
        float _frameRate = 0
    );

    void addText(
        std::string _Id,
        DirectX::XMFLOAT3 _pos,
        DirectX::XMFLOAT3 _size,
        std::string _name,
        DirectX::XMFLOAT3 _Color,
        std::string _Font,
        std::string _text,
        int _FontSize
    );

    Entity* getElement(std::string _Id);

    void Clear();
};