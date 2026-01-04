#include "MenuManager.hpp"
#include <sstream>
std::unordered_map<
    Menu,
    std::unordered_map<int, Action>,
    MenuHash
> MenuManager::actions;

Menu MenuManager::currentMenu{Menu::MAIN};
int MenuManager::optionSelected;
std::unordered_map<Menu, int, MenuHash> MenuManager::optionCounts={
    {Menu::MAIN, 4},
    {Menu::PAUSE, 4},
    {Menu::GAME_LOSE, 2},
    {Menu::GAME_WON, 2}
};

std::unordered_map<Menu, std::vector<std::string>, MenuHash> 
MenuManager::buttonNames=
{
    {
        Menu::MAIN,{
            "PLAY",
            "READ THIS!",
            "CREDITS",
            "QUIT"
        }
    },
    {
        Menu::PAUSE,{
            "RESUME",
            "READ THIS!",
            "BACK TO MENU",
            "QUIT"
        }
    },
    {
        Menu::GAME_LOSE,{
            "BACK TO MENU",
            "QUIT"
        }
    },
    {
        Menu::GAME_WON,{
            "BACK TO MENU",
            "QUIT"
        }
    }
};

std::unordered_map<
        Menu,
        std::unordered_map<int, Action>,
        MenuHash
    > actions;

SDLTexturePtr MenuManager::cursorImage{nullptr, SDL_DestroyTexture};
std::pair<int, int> MenuManager::cursorImageWH;
std::string MenuManager::displayTxt;

// Actions
void play(GameState& state){
    state = GameState::GAMEPLAY;
    return;
}
void back_to_menu(GameState& state){
    state = GameState::MAINMENU;
    MenuManager::setMenu(Menu::MAIN);
}
void show_instructions(GameState& state){
    MenuManager::setMenu(Menu::INSTRUCTIONS);
    MenuManager::set_displayTxt(
R"(HELLO
WORLD)");
}
void show_credits(GameState& state){
    MenuManager::setMenu(Menu::CREDITS);
    MenuManager::set_displayTxt(
R"(HELLO
WORLDD)");
}
void MenuManager::Init(SDL_Renderer& r){
    loadCursorImage("Textures/Red_triangle.svg", r);
    currentMenu = Menu::MAIN;

    // Binding actions
    bind(Menu::MAIN, 0, play);
    bind(Menu::PAUSE, 2, back_to_menu);
    bind(Menu::INSTRUCTIONS, 0, back_to_menu);
    bind(Menu::CREDITS, 0, back_to_menu);
    bind(Menu::MAIN, 1, show_instructions);
    bind(Menu::MAIN, 2, show_credits);

}

std::tuple<SDL_Color, SDL_Color, SDL_Color, SDL_Color> 
MenuManager::menuColors = {
    SDL_Color{14, 23, 126, 255},   // Background
    SDL_Color{6, 11, 81, 255},   // Foregroud
    SDL_Color{255, 255, 255, 255},    // Font High
    SDL_Color{142, 142, 142, 255}    // Font Low
};

std::unordered_map<Menu, std::string, MenuHash> MenuManager::titles ={
    {Menu::MAIN, "OPTIONS"},
    {Menu::PAUSE, "OPTIONS"},
    {Menu::GAME_LOSE, "YOU DIED :("},
    {Menu::GAME_WON, "YOU WON :)"},
    {Menu::INSTRUCTIONS, "CONTROLS"},
    {Menu::CREDITS, "CREDITS"}
};

void MenuManager::set_displayTxt(std::string x){
    displayTxt = x;
}
bool MenuManager::handleEvents(GameState& state){
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            return true;
        if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
        {
            if (event.key.keysym.scancode == SDL_SCANCODE_UP)
            {
                std::cout<<"UP\n";
                moveUp();
            }else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN)
            {
                std::cout<<"DOWN\n";
                moveDown();
            }else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN ||
                event.key.keysym.scancode == SDL_SCANCODE_KP_ENTER)
            {
                if(buttonNames.count(currentMenu) &&
                    buttonNames[currentMenu][optionSelected]=="QUIT"){
                    return true;
                }
                std::cout<<"SELECTED OPTION "<<optionSelected<<std::endl;
                select(state);
            }
        }
    }
    return false;
}

void MenuManager::loadCursorImage(const char* filePath, SDL_Renderer& renderer)
{
    SDL_Texture* raw = IMG_LoadTexture(&renderer, filePath);
    if (!raw) {
        std::cerr << "Failed to load Cursor texture: "
                  << filePath << " | " << IMG_GetError() << "\n";
        return;
    }

    cursorImage = SDLTexturePtr(raw, SDL_DestroyTexture);

    int width = 0, height = 0;
    if (SDL_QueryTexture(raw, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "Failed to query texture: "
                  << SDL_GetError() << "\n";
        return;
    }
    cursorImageWH = std::make_pair(width, height);
}

void MenuManager::renderMenu(SDL_Renderer& renderer, const std::pair<int, int>& screenWH){
    auto background = std::get<0>(menuColors);
    auto foreground = std::get<1>(menuColors);
    auto fontclrHig = std::get<2>(menuColors);
    auto fontclrLow = std::get<3>(menuColors);

    // Painting BackGround
    UIManager::drawFilledRectWithBorder(renderer,
        {0,0,screenWH.first, screenWH.second},
        background, background, 0
    );

    // Painting foreground
    int scale = 2;
    int w = 14 * UIManager::getGlyphSize().first * scale, h=0;
    if (buttonNames.count(currentMenu))
        h = buttonNames[currentMenu].size() * UIManager::getGlyphSize().second * scale;
    int x = screenWH.first/2 - w/2;
    int y = screenWH.second/2 - h/2;

    UIManager::drawFilledRectWithBorder(renderer,
        {x,y,w,h},
        foreground, foreground, 0
    );

    // Writing options
    x += UIManager::getGlyphSize().first * scale;
    std::string s;
    for (int i=0; h>0 && i<buttonNames[currentMenu].size(); i++){
        s = buttonNames[currentMenu][i];
        if (i==optionSelected)
            UIManager::renderText(renderer, s, x, y, scale, fontclrHig);
        else
            UIManager::renderText(renderer, s, x, y, scale, fontclrLow);
        y += UIManager::getGlyphSize().second * scale;
    }

    // display text for some menus
    scale = 2;
    x -= UIManager::getGlyphSize().first * scale;
    if(h==0){
        std::istringstream iss(displayTxt);
        std::string line;
        int lines = std::count(displayTxt.begin(), displayTxt.end(), '\n');
        h = lines * UIManager::getGlyphSize().second * scale;
        y = screenWH.second/2 - h/2;
        while (std::getline(iss, line)) {
            UIManager::renderText(renderer, line, x, y, scale, fontclrHig);
            y += UIManager::getGlyphSize().second * scale;
        }
    }

    // display title
    scale = 4;
    if(titles.count(currentMenu)){
        w = titles[currentMenu].size() * UIManager::getGlyphSize().first * scale;
        x = screenWH.first/2 - w/2;
        y = UIManager::getGlyphSize().second * 2;
        UIManager::renderText(renderer, titles[currentMenu], x, y, scale, fontclrHig);
    }
    SDL_RenderPresent(&renderer);

}