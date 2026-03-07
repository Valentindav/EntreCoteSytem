#pragma once
#include "Public/Menu.h"
#include <vector>
#include <string>

class MenuManager
{
protected:
    Menu* m_currentMenu = nullptr;
    std::vector<Menu*> menuList;
    std::vector<std::string> menuToLoad;

public:
    MenuManager() = default;
    ~MenuManager();

    void addMenuToLoad(std::string name) { menuToLoad.push_back(name); }
    std::vector<std::string> getMenuToLoad() { return menuToLoad; }

    void loadMenuToLoad();
    void Start();
    Menu* getMenu(std::string name);
    Menu* getCurrentMenu() { return m_currentMenu; }

    void forceActiveMenu(std::string name);
    void forceUnActiveMenu(std::string name);

    void loadAllMenu();
    void switchMenu(std::string name);

    void loadMenu(std::string menuName);
};