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
    state = GameState::RESET;
    MenuManager::setMenu(Menu::NONE);  
    AudioManager::stopMusic(); 
    return;
}
void back_to_menu(GameState& state){
    state = GameState::MAINMENU;
    MenuManager::setMenu(Menu::MAIN);
    AudioManager::playMusic("Menu", -1);
}
void show_instructions(GameState& state){
    MenuManager::setMenu(Menu::INSTRUCTIONS);
    MenuManager::set_displayTxt(
R"(MENU CONTROLS:
UP, DOWN - MOVE
ENTER    - SELECT OPTION

IN GAME CONTROLS:
WASD + UP, DOWN - MOVE
RIGHT, LEFT     - TURN
MOUSE           - AIM, SHOOT
SPACE           - OPEN DOORS
1 2 3           - SWITCH WEAPONS
ESC             - PAUSE MENU
)");
}
void show_instructions_pause(GameState& state){
    MenuManager::setMenu(Menu::INSTRUCTIONS_DURING_PAUSE);
    MenuManager::set_displayTxt(
R"(MENU CONTROLS:
UP, DOWN - MOVE
ENTER    - SELECT OPTION

IN GAME CONTROLS:
WASD + UP, DOWN - MOVE
RIGHT, LEFT     - TURN
MOUSE           - AIM, SHOOT
SPACE           - OPEN DOORS
1 2 3           - SWITCH WEAPONS
ESC             - PAUSE MENU
)");
}
void show_credits(GameState& state){
    MenuManager::setMenu(Menu::CREDITS);
    MenuManager::set_displayTxt(
R"(DEVELOPMENT
WASI HUSAIN: PROGRAMMING AND
	            GAME DESIGN

ASSETS
SOME TEXTURES AND SOUND EFFECTS 
WERE OBTAINED FROM FREELY 
AVAILABLE ONLINE SOURCES 

DUE TO THE AGE AND AGGREGATION 
OF THESE RESOURCES, NOT ALL 
ORIGINAL CREATORS COULD BE 
IDENTIFIED

FULL CREDIT WILL BE PROVIDED UPON 
REQUEST)");
}
void resume(GameState& state){
    state = GameState::GAMEPLAY;
    MenuManager::setMenu(Menu::NONE);
    AudioManager::stopMusic();
}
void MenuManager::Init(SDL_Renderer& r){
    loadCursorImage("Textures/cursor.png", r);
    currentMenu = Menu::MAIN;

    // Binding actions
    bind(Menu::MAIN, 0, play);
    bind(Menu::PAUSE, 2, back_to_menu);
    bind(Menu::INSTRUCTIONS, 0, back_to_menu);
    bind(Menu::GAME_LOSE, 0, back_to_menu);
    bind(Menu::GAME_WON, 0, back_to_menu);
    bind(Menu::CREDITS, 0, back_to_menu);
    bind(Menu::MAIN, 1, show_instructions);
    bind(Menu::MAIN, 2, show_credits);
    bind(Menu::PAUSE, 0, resume);
    bind(Menu::PAUSE, 1, show_instructions_pause);
    bind(Menu::INSTRUCTIONS_DURING_PAUSE, 0, resume);

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
    {Menu::GAME_LOSE, "YOU DIED"},
    {Menu::GAME_WON, "YOU WON"},
    {Menu::INSTRUCTIONS, "CONTROLS"},
    {Menu::CREDITS, "CREDITS"},
    {Menu::INSTRUCTIONS_DURING_PAUSE, "CONTROLS"}
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
                //std::cout<<"UP\n";
                moveUp();
            }else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN)
            {
                //std::cout<<"DOWN\n";
                moveDown();
            }else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN ||
                event.key.keysym.scancode == SDL_SCANCODE_KP_ENTER)
            {
                if(buttonNames.count(currentMenu) &&
                    buttonNames[currentMenu][optionSelected]=="QUIT"){
                    return true;
                }
                //std::cout<<"SELECTED OPTION "<<optionSelected<<std::endl;
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
        if (i==optionSelected){
            UIManager::renderText(renderer, s, x, y, scale, fontclrHig);

            // Box where the cursor image must fit
            SDL_Rect box {
                x - UIManager::getGlyphSize().first * scale,
                y,
                UIManager::getGlyphSize().first * scale,
                UIManager::getGlyphSize().second * scale
            };

            // Original image size
            int imgW = cursorImageWH.first;
            int imgH = cursorImageWH.second;

            // Compute scale to preserve aspect ratio
            float scaleX = static_cast<float>(box.w) / imgW;
            float scaleY = static_cast<float>(box.h) / imgH;
            float imgScale = std::min(scaleX, scaleY);

            // Final rendered size
            int renderW = static_cast<int>(imgW * imgScale);
            int renderH = static_cast<int>(imgH * imgScale);

            // Center image inside the box
            SDL_Rect dst {
                box.x + (box.w - renderW) / 2,
                box.y + (box.h - renderH) / 2,
                renderW,
                renderH
            };

            // Render cursor image
            SDL_RenderCopy(&renderer, cursorImage.get(), nullptr, &dst);

        }
        else
            UIManager::renderText(renderer, s, x, y, scale, fontclrLow);
        y += UIManager::getGlyphSize().second * scale;
    }
    

    // display text for some menus
    scale = 1;
    x -= UIManager::getGlyphSize().first * scale + 60;
    if(h==0){
        std::istringstream iss(displayTxt);
        std::string line;
        int lines = std::count(displayTxt.begin(), displayTxt.end(), '\n');
        h = lines * UIManager::getGlyphSize().second * scale;
        y = screenWH.second/2 - h/2 + 60;
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

