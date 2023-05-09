#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <cmath>
#include <iostream>
#include <random>

#include "../include/ImGui/imgui-SFML.h"
#include "../include/ImGui/imgui.h"
#include "../include/ImGui/imgui_demo.cpp"
#include "../include/config.hpp"

#define MAX_RADIUS 10
#define MAX_COORD 100
#define MAX_AA 32

#define ImGuiFPSFlags                                             \
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | \
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | \
        ImGuiWindowFlags_NoMove
#define ImGuiSettingsFlags                                   \
    ImGuiWindowFlags_MenuBar | \
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize

int main() {
    // window and graphics settings

    sf::RenderWindow window(sf::VideoMode(WIDTH, HIGHT), "RayTrace",
                            sf::Style::Fullscreen,
                            sf::ContextSettings(0, 0, 0));
    window.setFramerateLimit(MAX_FPS);
    window.setVerticalSyncEnabled(false);
    glEnable(GL_MULTISAMPLE_ARB);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    // shader settings

    sf::Shader shader;

    shader.loadFromFile("./src/raycast.frag", sf::Shader::Fragment);
    shader.setUniform("u_resolution", sf::Vector2f(WIDTH, HIGHT));

    sf::RenderTexture OutTexture;
    OutTexture.create(WIDTH, HIGHT);
    sf::Sprite OutSprite = sf::Sprite(OutTexture.getTexture());
    sf::Sprite OutSpriteFlipped = sf::Sprite(OutTexture.getTexture());
    OutSpriteFlipped.setScale(1, -1);
    OutSpriteFlipped.setPosition(0, HIGHT);

    sf::RenderTexture FinalTexture;
    FinalTexture.create(WIDTH, HIGHT);
    sf::Sprite FinalSprite = sf::Sprite(FinalTexture.getTexture());
    sf::Sprite FinalSpriteFlipped = sf::Sprite(FinalTexture.getTexture());
    FinalSpriteFlipped.setScale(1, -1);
    FinalSpriteFlipped.setPosition(0, HIGHT);

    sf::Clock clock;
    sf::Time time;

    // cursor settings

    bool MouseCursorVisible = false;
    sf::Mouse::setPosition(sf::Vector2i(WIDTH / 2, HIGHT / 2), window);
    sf::Vector2i mouse_pos = sf::Mouse::getPosition();
    sf::Vector2i current_mouse_pos;

    int mouse_x = WIDTH / 2;
    int mouse_y = HIGHT / 2;
    int mouse_move_x = 0;
    int mouse_move_y = 0;

    // camera settings

    sf::Vector3f camera_position(0.f, 0.f, 0.f);
    sf::Vector3f camera_movement(0.f, 0.f, 0.f);

    int FrameStill = 1;

    bool switcherLcontrol = true;
    bool switcherEscape = false;
    bool switcherVsync = false;

    int Smooth = 0;
    int CurrentFPS = 0;
    int FPS = 120;
    int Resolution[2] = {1920, 1080};

    bool show_fps = false;
    bool theme_switcher = false;
    bool show_smooth = false;
    bool show_resolution = false;
    bool quit = false;
    
    int style_idx = 0;

    // random settings

    std::random_device rd;
    std::mt19937 e2(rd());
    std::uniform_real_distribution<> dist(0.0f, 1.0f);

    // scene settings

    float sun_brightness = 0.02;

    sf::Glsl::Vec3 ulight_pos = sf::Glsl::Vec3(0.4f, -0.75f, 0.8f);

    // sphere settings

    sf::Glsl::Vec4 spheres_pos[DEFAULT_SIZE];
    sf::Glsl::Vec4 spheres_col[DEFAULT_SIZE];

    int spheres_num = 0;

    for (int i = 0; i < 10; i++) {
        spheres_pos[i] = sf::Glsl::Vec4(0.f, -4.f + -2.f * i, 0.f, 1.f);
        spheres_col[i] = sf::Glsl::Vec4(0.4, 0.5, 0.8, 0.1 * i);
    }

    spheres_pos[10] = sf::Glsl::Vec4(4.f, -4.f, 2.f, 0.75f);
    spheres_col[10] = sf::Glsl::Vec4(1.f, 0.2, 0.1, 1.f);

    spheres_pos[11] = sf::Glsl::Vec4(4.f, 0.f, 0.f, 1.f);
    spheres_col[11] = sf::Glsl::Vec4(1.f, 1.f, 1.f, -2.f);

    spheres_pos[12] = sf::Glsl::Vec4(-4.f, 0.f, 0.f, 1.f);
    spheres_col[12] = sf::Glsl::Vec4(1.f, 1.f, 1.f, 0.f);

    spheres_pos[13] = sf::Glsl::Vec4(10.f, 0.f, 0.f, 1.f);
    spheres_col[13] = sf::Glsl::Vec4(1.f, 1.f, 1.f, -1.5);

    for (int i = 14; i < DEFAULT_SIZE; i++) {
        spheres_pos[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
        spheres_col[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
    }

    // boxes settings

    sf::Glsl::Vec3 boxes_pos[DEFAULT_SIZE];
    sf::Glsl::Vec3 boxes_size[DEFAULT_SIZE];
    sf::Glsl::Vec4 boxes_col[DEFAULT_SIZE];

    int boxes_num = 0;

    for (int i = 0; i < DEFAULT_SIZE; i++) {
        boxes_pos[i] = sf::Glsl::Vec3(0.f, 0.f, 0.f);
        boxes_col[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
        boxes_size[i] = sf::Glsl::Vec3(0.f, 0.f, 0.f);
    }

    boxes_pos[0] = sf::Glsl::Vec3(4.f, -4.f, 0.f);
    boxes_size[0] = sf::Glsl::Vec3(1.f, 1.f, 1.f);
    boxes_col[0] = sf::Glsl::Vec4(0.4, 0.6, 0.8, 1.f);

    boxes_pos[1] = sf::Glsl::Vec3(4.f, -10.f, 0.f);
    boxes_size[1] = sf::Glsl::Vec3(1.f, 1.f, 1.f);
    boxes_col[1] = sf::Glsl::Vec4(0.6, 0.4, 0.5, 0.8);

    // plane settings

    sf::Glsl::Vec3 planes_norm[PLANES_SIZE];
    sf::Glsl::Vec4 planes_col[PLANES_SIZE];

    int planes_num = 1;

    for (int i = 0; i < PLANES_SIZE; i++) {
        planes_norm[i] = sf::Glsl::Vec3(0.f, 0.f, 0.f);
        planes_col[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
    }

    planes_norm[0] = sf::Glsl::Vec3(0.f, 0.f, -1.f);
    planes_col[0] = sf::Glsl::Vec4(0.5, 0.5, 0.5, 1.f);

    // cones settings

    sf::Glsl::Vec4 cones_down_point[DEFAULT_SIZE];
    sf::Glsl::Vec4 cones_up_point[DEFAULT_SIZE];
    sf::Glsl::Vec4 cones_col[DEFAULT_SIZE];

    int cones_num = 0;

    for (int i = 0; i < DEFAULT_SIZE; i++) {
        cones_down_point[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
        cones_up_point[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
        cones_col[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
    }

    cones_down_point[0] = sf::Glsl::Vec4(4.f, 4.f, 1.f, 1.f);
    cones_up_point[0] = sf::Glsl::Vec4(4.f, 4.f, -2.f, 0.f);
    cones_col[0] = sf::Glsl::Vec4(1.f, 0.4, 0.6, 0.f);

    sf::Glsl::Vec3 cyl_down_point[DEFAULT_SIZE];
    sf::Glsl::Vec4 cyl_up_point[DEFAULT_SIZE];
    sf::Glsl::Vec4 cyl_col[DEFAULT_SIZE];

    // cylinders settings

    int cyl_num = 0;

    for (int i = 0; i < DEFAULT_SIZE; i++) {
        cyl_down_point[i] = sf::Glsl::Vec3(0.f, 0.f, 0.f);
        cyl_up_point[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
        cyl_col[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
    }

    cyl_down_point[0] = sf::Glsl::Vec3(4.f, 8.f, 1.f);
    cyl_up_point[0] = sf::Glsl::Vec4(4.f, 8.f, -2.f, 0.5);
    cyl_col[0] = sf::Glsl::Vec4(0.1, 0.6, 0.7, 1.f);

    float SphereRadius = 1;
    float SphereCoord[3] = {};
    float SphereParam = 1;
    float antiSphereParam = -1;
    int SphereStatus = 2;
    float SphereIntention = 0;
    float SphereColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

    float BoxLen = 1;
    float BoxHei = 1;
    float BoxWi = 1;
    float BoxCoord[3] = {};
    float BoxParam = 1;
    float antiBoxParam = -1;
    int BoxStatus = 2;
    float BoxIntention = 0;
    float BoxColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

    float CylRadius = 1;
    float CylX[2] = {};
    float CylY[2] = {};
    float CylZ[2] = {};
    float CylParam = 1;
    float antiCylParam = -1;
    int CylStatus = 2;
    float CylIntention = 0;
    float CylColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

    float NormCoord[3] = {};
    float PlaneParam = 1;
    float antiPlaneParam = -1;
    int PlaneStatus = 2;
    float PlaneIntention = 0;
    float PlaneColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

    float ConeUpRadius = 1;
    float ConeDownRadius = 2;
    float ConeX[2] = {};
    float ConeY[2] = {};
    float ConeZ[2] = {};
    float ConeParam = 1;
    float antiConeParam = -1;
    int ConeStatus = 2;
    float ConeIntention = 0;
    float ConeColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

   

    sf::Clock fps_clock;
    int frame_counter = 0;
    sf::Time delta_time;

    ImGui::SFML::Init(window);

    while (window.isOpen()) {  // main lopp

        sf::Event event;

        while (window.pollEvent(event)) {  // event loop

            ImGui::SFML::ProcessEvent(window, event);

            switch (event.type) {
                case (sf::Event::Closed):
                    window.close();
                    break;
                case (sf::Event::MouseMoved):

                    if (switcherEscape)
                        break;

                    current_mouse_pos = sf::Mouse::getPosition();
                    mouse_move_x = (current_mouse_pos.x - mouse_pos.x);
                    mouse_move_y = (current_mouse_pos.y - mouse_pos.y);
                    mouse_x += mouse_move_x;
                    mouse_y += mouse_move_y;

                    mouse_pos = current_mouse_pos;

                    if (abs(current_mouse_pos.x - WIDTH / 2) > 10) {
                        sf::Mouse::setPosition(
                            sf::Vector2i(WIDTH / 2, HIGHT / 2), window);
                        mouse_pos = sf::Vector2i(WIDTH / 2, HIGHT / 2);
                    }

                    if (mouse_move_x != 0 || mouse_move_y != 0)
                        FrameStill = 1;

                    break;

                case (sf::Event::KeyPressed):

                    switch (event.key.code) {
                        case (sf::Keyboard::W):

                            camera_movement.x = WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case (sf::Keyboard::A):

                            camera_movement.y = -WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case (sf::Keyboard::S):

                            camera_movement.x = -WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case (sf::Keyboard::D):

                            camera_movement.y = WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case (sf::Keyboard::Up):

                            camera_movement.z = -WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case (sf::Keyboard::Down):

                            camera_movement.z = WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case (sf::Keyboard::LControl):

                            if (switcherLcontrol == false) {
                                shader.loadFromFile("./src/raytracing.frag",
                                                    sf::Shader::Fragment);
                                shader.setUniform("u_resolution",
                                                  sf::Vector2f(WIDTH, HIGHT));
                                switcherLcontrol = true;
                                FrameStill = 1;

                            } else {
                                shader.loadFromFile("./src/raycast.frag",
                                                    sf::Shader::Fragment);
                                shader.setUniform("u_resolution",
                                                  sf::Vector2f(WIDTH, HIGHT));
                                switcherLcontrol = false;
                            }

                            break;

                        case (sf::Keyboard::Escape):

                            if (switcherEscape == false) {  // on pause
                                switcherEscape = true;
                                window.setMouseCursorVisible(switcherEscape);

                            } else {  // leave pause
                                switcherEscape = false;
                                window.setMouseCursorVisible(switcherEscape);
                            }

                        default:
                            break;
                    }

                    break;

                default:
                    break;
            }
        }

        ImGui::SFML::Update(window, clock.restart());

        float res_mouse_x = (float(mouse_x) / WIDTH - 0.5f) * MOUSE_SENSITIVITY;
        float res_mouse_y = (float(mouse_y) / HIGHT - 0.5f) * MOUSE_SENSITIVITY;

        window.setActive();
        shader.setUniform("u_mouse", sf::Vector2f(res_mouse_x, res_mouse_y));

        camera_position.x += camera_movement.x * cos(res_mouse_x) -
                             camera_movement.y * sin(res_mouse_x);
        camera_position.y += camera_movement.x * sin(res_mouse_x) +
                             camera_movement.y * cos(res_mouse_x);
        camera_position.z += camera_movement.z;

        camera_movement.x = 0.f;
        camera_movement.y = 0.f;
        camera_movement.z = 0.f;

        time = clock.getElapsedTime();
        float u_time = time.asSeconds();

        if(switcherEscape)
        {
            ImGui::SetNextWindowSize(
                ImVec2(598, 555));                  //, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(0, 0));  //, ImGuiCond_FirstUseEver);
            ImGui::Begin("Settings Window", NULL, ImGuiSettingsFlags);

            
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Appearance")) {
                    if (ImGui::BeginMenu("Switch Theme")) {
                        ImGui::Combo("Theme", &style_idx,
                                    "Dark\0Light\0Classic\0\0");
                        switch (style_idx) {
                            case 0:
                                ImGui::StyleColorsDark();
                                break;
                            case 1:
                                ImGui::StyleColorsLight();
                                break;
                            case 2:
                                ImGui::StyleColorsClassic();
                                break;
                        }
                        ImGui::EndMenu();
                    }
                    if(ImGui::BeginMenu("Switch Font"))
                    {
                        ImGui::ShowFontSelector("Fonts##Selector");
                        ImGui::EndMenu();
                    }

                    ImGui::MenuItem("Quit", NULL, &quit);
                    if(quit)
                        window.close();
                        
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Data")) {
                    ImGui::MenuItem("FPS Show", NULL, &show_fps);
                    ImGui::MenuItem("Resolution Show", NULL, &show_resolution);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Credits")) {
                    if (ImGui::BeginMenu("AWES0MESLAYER")) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
                                        "MOST HATED");
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("OLE G")) {
                        ImGui::TextColored(ImVec4(0.7f, 0.5f, 1.0f, 0.8f),
                                        "EGG FRYER");
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("IraIvanov")) {
                        ImGui::TextColored(ImVec4(0.9f, 0.35f, 0.34f, 0.99f),
                                        "$ad boy");
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::TextColored(
                ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                "Welcome to the Settings of Ray_Tracing Project v2.28!");
            ImGui::Separator();
            ImGui::Text("You can modify and add objects using this settings:");

            if (ImGui::CollapsingHeader("Objects Settings")) {
                if (ImGui::TreeNode("Spheres")) {
                    if (spheres_num > DEFAULT_SIZE)
                        spheres_num = 0;
                    ImGui::InputInt("Number", &spheres_num);
                    if (spheres_num > DEFAULT_SIZE)
                        spheres_num = 0;
                    SphereRadius = spheres_pos[spheres_num].w;
                    ImGui::SliderFloat("Radius", &SphereRadius, 0, MAX_RADIUS);
                    SphereCoord[0] = spheres_pos[spheres_num].x;
                    SphereCoord[1] = spheres_pos[spheres_num].y;
                    SphereCoord[2] = spheres_pos[spheres_num].z;
                    ImGui::InputFloat3("Coord", SphereCoord);

                    ImGui::RadioButton("Haze", &SphereStatus, 1);
                    ImGui::SameLine();
                    ImGui::RadioButton("Light", &SphereStatus, 2);
                    ImGui::SameLine();
                    ImGui::RadioButton("Reflection", &SphereStatus, 3);
                    SphereParam = spheres_col[spheres_num].w;
                    if (SphereStatus == 1) {
                        ImGui::SliderFloat("Haze", &SphereParam, 0, 1);
                    } else if (SphereStatus == 3) {
                        ImGui::SliderFloat("Reflection", &antiSphereParam, 0, 1.99);
                        SphereParam = -antiSphereParam;
                    } else if (SphereStatus == 2) {
                        ImGui::SliderFloat("Intention", &SphereIntention, 0, 100);
                    }
                    SphereColor[0] = spheres_col[spheres_num].x;
                    SphereColor[1] = spheres_col[spheres_num].y;
                    SphereColor[2] = spheres_col[spheres_num].z;
                    ImGui::ColorEdit3("Color", SphereColor);
                    ImGui::SeparatorText("Sun Light Slider:");
                    ImGui::SliderFloat("Light", &sun_brightness, 0, 1);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Boxes")) {
                    if (boxes_num > DEFAULT_SIZE)
                        boxes_num = 0;
                    ImGui::InputInt("Number", &boxes_num);
                    if (boxes_num > DEFAULT_SIZE)
                        boxes_num = 0;
                    BoxLen = boxes_size[boxes_num].x;
                    BoxHei = boxes_size[boxes_num].y;
                    BoxWi = boxes_size[boxes_num].z;
                    ImGui::SliderFloat("Size", &BoxLen, 0, MAX_RADIUS);
                    ImGui::SliderFloat("Height", &BoxHei, 0, MAX_RADIUS);
                    ImGui::SliderFloat("Width", &BoxWi, 0, MAX_RADIUS);
                    BoxCoord[0] = boxes_pos[boxes_num].x;
                    BoxCoord[1] = boxes_pos[boxes_num].y;
                    BoxCoord[2] = boxes_pos[boxes_num].z;

                    ImGui::InputFloat3("Coord", BoxCoord);

                    ImGui::RadioButton("Haze", &BoxStatus, 1);
                    ImGui::SameLine();
                    ImGui::RadioButton("Light", &BoxStatus, 2);
                    ImGui::SameLine();
                    ImGui::RadioButton("Reflection", &BoxStatus, 3);
                    BoxParam = boxes_col[boxes_num].w;
                    if (BoxStatus == 1) {
                        ImGui::SliderFloat("Haze", &BoxParam, 0, 1);
                    } else if (BoxStatus == 3) {
                        ImGui::SliderFloat("Reflection", &antiBoxParam, 0, 1.99);
                        BoxParam = -antiBoxParam;
                    } else if (BoxStatus == 2) {
                        ImGui::SliderFloat("Intention", &BoxIntention, 0, 1);
                    }
                    BoxColor[0] = boxes_col[boxes_num].x;
                    BoxColor[1] = boxes_col[boxes_num].y;
                    BoxColor[2] = boxes_col[boxes_num].z;
                    ImGui::ColorEdit3("Color", BoxColor);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Cylindres")) {
                    if (cyl_num > DEFAULT_SIZE)
                        cyl_num = 0;
                    ImGui::InputInt("Number", &cyl_num);
                    if (cyl_num > DEFAULT_SIZE)
                        cyl_num = 0;
                    CylRadius = cyl_up_point[cyl_num].w;
                    ImGui::SliderFloat("Radius", &CylRadius, 0, MAX_RADIUS);
                    CylX[0] = cyl_up_point[cyl_num].x;
                    CylX[1] = cyl_down_point[cyl_num].x;
                    CylY[0] = cyl_up_point[cyl_num].y;
                    CylY[1] = cyl_down_point[cyl_num].y;
                    CylZ[0] = cyl_up_point[cyl_num].z;
                    CylZ[1] = cyl_down_point[cyl_num].z;

                    ImGui::InputFloat2("Up and Down X", CylX);
                    ImGui::InputFloat2("Up and Down Y", CylY);
                    ImGui::InputFloat2("Up and Down Z", CylZ);

                    CylParam = cyl_col[cyl_num].w;
                    if (CylStatus == 1) {
                        ImGui::SliderFloat("Haze", &CylParam, 0, 1);
                    } else if (CylStatus == 3) {
                        ImGui::SliderFloat("Reflection", &antiCylParam, 0, 1.99);
                        CylParam = -antiCylParam;
                    } else if (CylStatus == 2) {
                        ImGui::SliderFloat("Intention", &CylIntention, 0, 1);
                    }

                    CylColor[0] = cyl_col[cyl_num].x;
                    CylColor[1] = cyl_col[cyl_num].y;
                    CylColor[2] = cyl_col[cyl_num].z;
                    ImGui::ColorEdit3("Color", CylColor);

                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Planes")) {
                    if (planes_num > PLANES_SIZE)
                        planes_num = 0;
                    ImGui::InputInt("Number", &planes_num);
                    if (planes_num > PLANES_SIZE)
                        planes_num = 0;
                    NormCoord[0] = planes_norm[planes_num].x;
                    NormCoord[1] = planes_norm[planes_num].y;
                    NormCoord[2] = planes_norm[planes_num].z;
                    ImGui::InputFloat3("Norm Coord", NormCoord);

                    PlaneParam = planes_col[planes_num].w;
                    if (PlaneStatus == 1) {
                        ImGui::SliderFloat("Haze", &PlaneParam, 0, 1);
                    } else if (PlaneStatus == 3) {
                        ImGui::SliderFloat("Reflection", &antiPlaneParam, 0, 1.99);
                        PlaneParam = -antiPlaneParam;
                    } else if (PlaneStatus == 2) {
                        ImGui::SliderFloat("Intention", &PlaneIntention, 0, 1);
                    }

                    PlaneColor[0] = planes_col[planes_num].x;
                    PlaneColor[1] = planes_col[planes_num].y;
                    PlaneColor[2] = planes_col[planes_num].z;
                    ImGui::ColorEdit3("Color", PlaneColor);

                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Cones")) {
                    if (cones_num > DEFAULT_SIZE)
                        cones_num = 0;
                    ImGui::InputInt("Number", &cones_num);
                    if (cones_num > DEFAULT_SIZE)
                        cones_num = 0;
                    ConeUpRadius = cones_up_point[cones_num].w;
                    ConeDownRadius = cones_down_point[cones_num].w;
                    ImGui::SliderFloat("Up Radius", &ConeUpRadius, 0, MAX_RADIUS);
                    ImGui::SliderFloat("Down Radius", &ConeDownRadius, 0,
                                    MAX_RADIUS);

                    ConeX[0] = cones_up_point[cones_num].x;
                    ConeY[0] = cones_up_point[cones_num].y;
                    ConeZ[0] = cones_up_point[cones_num].z;
                    ConeX[1] = cones_down_point[cones_num].x;
                    ConeY[1] = cones_down_point[cones_num].y;
                    ConeZ[1] = cones_down_point[cones_num].z;
                    ImGui::InputFloat2("Up and Down X", ConeX);
                    ImGui::InputFloat2("Up and Down Y", ConeY);
                    ImGui::InputFloat2("Up and Down Z", ConeZ);

                    ConeParam = cones_col[cones_num].w;
                    if (ConeStatus == 1) {
                        ImGui::SliderFloat("Haze", &ConeParam, 0, 1);
                    } else if (ConeStatus == 3) {
                        ImGui::SliderFloat("Reflection", &antiConeParam, 0, 1.99);
                        ConeParam = -antiConeParam;
                    } else if (ConeStatus == 2) {
                        ImGui::SliderFloat("Intention", &ConeIntention, 0, 1);
                    }

                    ConeColor[0] = cones_col[cones_num].x;
                    ConeColor[1] = cones_col[cones_num].y;
                    ConeColor[2] = cones_col[cones_num].z;
                    ImGui::ColorEdit3("Color", ConeColor);
                    ImGui::TreePop();
                }
            }
            ImGui::Separator();
            ImGui::Text("For modifying graphic settings you can use:");
            if (ImGui::CollapsingHeader("Graphic Settings")) {
                ImGui::SliderInt("AntiAliasing", &Smooth, 0, MAX_AA);  // add
                if (FPS < 0)
                    FPS = 10;
                ImGui::InputInt("MaxFPS", &FPS, 10, 0);
                ImGui::InputInt2("Resolution W X H", Resolution);
                Resolution[0] = fabs(Resolution[0]);
                Resolution[1] = fabs(Resolution[1]);
                ImGui::Checkbox("VSYNC", &switcherVsync);
            }
            ImGui::End();
            

            if (show_fps) {
                ImGui::SetNextWindowSize(
                    ImVec2(1000, 1000));  //, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowPos(
                    ImVec2(1840, 5));  //, ImGuiCond_FirstUseEver);
                ImGui::Begin("FPS", NULL, ImGuiFPSFlags);
                ImGui::Text("FPS: %d", frame_counter);
                ImGui::End();
            }
            if (show_resolution) {
                ImGui::SetNextWindowSize(ImVec2(1000, 1000));
                ImGui::SetNextWindowPos(ImVec2(1815, 20));
                ImGui::Begin("Resolution", NULL, ImGuiFPSFlags);
                ImGui::Text("%d x %d", Resolution[0], Resolution[1]);
                ImGui::End();
            }

            spheres_pos[spheres_num].w = SphereRadius;
            spheres_pos[spheres_num].x = SphereCoord[0];
            spheres_pos[spheres_num].y = SphereCoord[1];
            spheres_pos[spheres_num].z = SphereCoord[2];
            spheres_col[spheres_num].x = SphereColor[0];
            spheres_col[spheres_num].y = SphereColor[1];
            spheres_col[spheres_num].z = SphereColor[2];
            spheres_col[spheres_num].w = SphereParam;

            boxes_pos[boxes_num].x = BoxCoord[0];
            boxes_pos[boxes_num].y = BoxCoord[1];
            boxes_pos[boxes_num].z = BoxCoord[2];
            boxes_size[boxes_num].x = BoxLen;
            boxes_size[boxes_num].y = BoxWi;
            boxes_size[boxes_num].z = BoxHei;
            boxes_col[boxes_num].x = BoxColor[0];
            boxes_col[boxes_num].y = BoxColor[1];
            boxes_col[boxes_num].z = BoxColor[2];
            boxes_col[boxes_num].w = BoxParam;

            cyl_down_point[cyl_num].x = CylX[0];
            cyl_down_point[cyl_num].y = CylY[0];
            cyl_down_point[cyl_num].z = CylZ[0];
            cyl_up_point[cyl_num].x = CylX[1];
            cyl_up_point[cyl_num].y = CylY[1];
            cyl_up_point[cyl_num].z = CylZ[1];
            cyl_up_point[cyl_num].w = CylRadius;
            cyl_col[cyl_num].x = CylColor[0];
            cyl_col[cyl_num].y = CylColor[1];
            cyl_col[cyl_num].z = CylColor[2];
            cyl_col[cyl_num].w = CylParam;

            planes_norm[planes_num].x = NormCoord[0];
            planes_norm[planes_num].y = NormCoord[1];
            planes_norm[planes_num].z = NormCoord[2];
            planes_col[planes_num].x = PlaneColor[0];
            planes_col[planes_num].y = PlaneColor[1];
            planes_col[planes_num].z = PlaneColor[2];
            planes_col[planes_num].w = PlaneParam;

            cones_up_point[cones_num].x = ConeX[0];
            cones_up_point[cones_num].y = ConeY[0];
            cones_up_point[cones_num].z = ConeZ[0];
            cones_up_point[cones_num].w = ConeUpRadius;
            cones_down_point[cones_num].x = ConeX[1];
            cones_down_point[cones_num].y = ConeY[1];
            cones_down_point[cones_num].z = ConeZ[1];
            cones_down_point[cones_num].w = ConeDownRadius;
            cones_col[cones_num].x = ConeColor[0];
            cones_col[cones_num].y = ConeColor[1];
            cones_col[cones_num].z = ConeColor[2];
            cones_col[cones_num].w = ConeParam;

            
            window.setVerticalSyncEnabled(switcherVsync);

            //window.setSize(sf::Vector2u(Resolution[0], Resolution[1])); set size with params
            //settings.antialiasingLevel = Smooth;
            //window.setamtianilacing ???? do it b urself -> AA level in Smooth int!
        }

        shader.setUniform("u_time", u_time);
        shader.setUniform("u_camera_pos", camera_position);
        shader.setUniform("u_sample_part", 1.0f / FrameStill);
        shader.setUniform(
            "u_seed1", sf::Vector2f((float)dist(e2), (float)dist(e2)) * 999.0f);
        shader.setUniform(
            "u_seed2", sf::Vector2f((float)dist(e2), (float)dist(e2)) * 999.0f);
        shader.setUniform("sun_brightness", sun_brightness);
        shader.setUniformArray("spheres_pos", spheres_pos, DEFAULT_SIZE);
        shader.setUniformArray("spheres_col", spheres_col, DEFAULT_SIZE);
        shader.setUniformArray("boxes_pos", boxes_pos, DEFAULT_SIZE);
        shader.setUniformArray("boxes_col", boxes_col, DEFAULT_SIZE);
        shader.setUniformArray("boxes_size", boxes_size, DEFAULT_SIZE);
        shader.setUniformArray("planes_norm", planes_norm, PLANES_SIZE);
        shader.setUniformArray("planes_col", planes_col, PLANES_SIZE);
        shader.setUniformArray("cones_down_point", cones_down_point,
                               DEFAULT_SIZE);
        shader.setUniformArray("cones_up_point", cones_up_point, DEFAULT_SIZE);
        shader.setUniformArray("cones_col", cones_col, DEFAULT_SIZE);
        shader.setUniformArray("cyl_down_point", cyl_down_point, DEFAULT_SIZE);
        shader.setUniformArray("cyl_up_point", cyl_up_point, DEFAULT_SIZE);
        shader.setUniformArray("cyl_col", cyl_col, DEFAULT_SIZE);
        shader.setUniform("ulight_pos", ulight_pos);

        if (switcherLcontrol == true) {
            if (FrameStill % 2 == 1) {
                shader.setUniform("u_sample", OutTexture.getTexture());
                FinalTexture.draw(OutSpriteFlipped, &shader);
                window.draw(FinalSprite);
            } else {
                shader.setUniform("u_sample", FinalTexture.getTexture());
                OutTexture.draw(OutSpriteFlipped, &shader);
                window.draw(OutSprite);
            }
        } else {
            shader.setUniform("u_sample", OutTexture.getTexture());
            window.draw(OutSprite, &shader);
        }

        ImGui::SFML::Render(window);

        if (!MouseCursorVisible) {
            window.setMouseCursorVisible(MouseCursorVisible);
            MouseCursorVisible = true;
        }

        window.display();

        delta_time += fps_clock.restart();
        frame_counter++;
        if (delta_time.asSeconds() >= 1.f) {
            std::cout << "FPS: " << frame_counter << std::endl;

            frame_counter = 0;
            delta_time = sf::seconds(0.f);
        }
        FrameStill++;
    }

    ImGui::SFML::Shutdown(window);
    return 0;
}