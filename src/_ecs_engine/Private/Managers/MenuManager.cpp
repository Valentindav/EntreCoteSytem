#include "Private/Managers/MenuManager.h"
#include "Public/Menu.h"

#include <filesystem>
#include <fstream>
#include <Helpers/nlohmann.hpp>

using namespace DirectX;

MenuManager::~MenuManager()
{
    for (Menu* menu : menuList)
    {
        menu->Clear();
        delete menu;
    }
    menuList.clear();
}

void MenuManager::Start()
{
    loadAllMenu();

    for (Menu* object : menuList) {
        object->SetActive(false);
    }
}

void MenuManager::loadMenuToLoad()
{
    for (const std::string& object : menuToLoad)
    {
        loadMenu(object);
    }
    menuToLoad.clear();
}

Menu* MenuManager::getMenu(std::string name)
{
    for (Menu* object : menuList)
    {
        if (object->name == name)
        {
            return object;
        }
    }
    return nullptr;
}

void MenuManager::forceUnActiveMenu(std::string name)
{
    for (Menu* object : menuList)
    {
        if (object->name == name)
        {
            object->SetActive(false);
        }
    }
}

void MenuManager::forceActiveMenu(std::string name)
{
    for (Menu* object : menuList)
    {
        if (object->name == name)
        {
            object->SetActive(true);
        }
    }
}

void MenuManager::switchMenu(std::string name)
{
    if (m_currentMenu && m_currentMenu->name == name
        && m_currentMenu->active) return;

    if (m_currentMenu != nullptr) {
        m_currentMenu->SetActive(false);
    }

    for (Menu* object : menuList)
    {
        if (object->name == name)
        {
            m_currentMenu = object;
            m_currentMenu->SetActive(true);
            return;
        }
    }
}

void MenuManager::loadAllMenu()
{
    std::string folderPath = "../../res/Ui/UiMenu";

    if (!std::filesystem::exists(folderPath)) return;

    for (const auto& entry : std::filesystem::directory_iterator(folderPath))
    {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        std::string menuName = entry.path().stem().string();
        loadMenu(menuName);
    }
}

void MenuManager::loadMenu(std::string menuName)
{
    if (getMenu(menuName) != nullptr) return;

    std::string filePath = "../../res/Ui/UiMenu/" + menuName + ".json";

    if (!std::filesystem::exists(filePath)) return;

    std::ifstream file(filePath);
    if (!file.is_open()) return;

    nlohmann::json data;
    file >> data;

    Menu* menu = new Menu();
    menu->name = menuName;
    menuList.push_back(menu);

    // --------------------
    //      BUTTONS
    // --------------------
    if (data.contains("buttons")) {
        for (auto& btn : data["buttons"]) {
            XMFLOAT3 pos{ btn["pos"]["x"], btn["pos"]["y"], btn["pos"]["z"] };
            XMFLOAT3 size{ btn["size"]["x"], btn["size"]["y"], btn["size"]["z"] };
            XMFLOAT2 frameSize{ btn["frameSize"]["x"], btn["frameSize"]["y"] };

            std::vector<std::string> listeners;
            if (btn.contains("listenerList")) {
                for (auto& l : btn["listenerList"])
                    listeners.push_back(l.get<std::string>());
            }

            menu->addButton(
                btn["id"], pos, size,
                btn["name"].get<std::string>(),
                listeners,
                btn.value("hover", ""),
                btn.value("animated", false),
                btn.value("frameCount", 0),
                btn.value("currentFrame", 0),
                frameSize,
                btn.value("looping", false),
                btn.value("frameRate", 0.0f)
            );
        }
    }

    // --------------------
    //       IMAGES
    // --------------------
    if (data.contains("images")) {
        for (auto& img : data["images"]) {
            XMFLOAT3 pos{ img["pos"]["x"], img["pos"]["y"], img["pos"]["z"] };
            XMFLOAT3 size{ img["size"]["x"], img["size"]["y"], img["size"]["z"] };
            XMFLOAT2 frameSize{ img["frameSize"]["x"], img["frameSize"]["y"] };

            menu->addImage(
                img["id"], pos, size,
                img["name"].get<std::string>().c_str(),
                img.value("animated", false),
                img.value("frameCount", 0),
                img.value("currentFrame", 0),
                frameSize,
                img.value("looping", false),
                img.value("frameRate", 0.0f)
            );
        }
    }

    // --------------------
    //        TEXTS
    // --------------------
    if (data.contains("texts")) {
        for (auto& txt : data["texts"]) {
            XMFLOAT3 pos{ txt["pos"]["x"], txt["pos"]["y"], txt["pos"]["z"] };
            XMFLOAT3 size{ txt["size"]["x"], txt["size"]["y"], txt["size"]["z"] };
            XMFLOAT3 color{ txt["color"]["r"], txt["color"]["g"], txt["color"]["b"] };

            menu->addText(
                txt["id"], pos, size,
                txt.value("name", "TextObject"),
                color,
                txt["fontPath"].get<std::string>(),
                txt["text"].get<std::string>(),
                (int)txt.value("fontSize", 24.0f)
            );
        }
    }

    menu->SetActive(false);
}