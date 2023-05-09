#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "../include/ImGui/imgui-SFML.h"
#include "../include/ImGui/imgui.h"

#include "../include/ImGui/imgui_demo.cpp"

#include "../include/config.hpp"
#include <cmath>
#include <iostream>
#include <random>

#define MAX_RADIUS 10
#define MAX_COORD 100
#define MAX_SMOOTH 100

int main() {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HIGHT), "RayTrace",
                            sf::Style::Fullscreen,
                            sf::ContextSettings(0, 0, 0));
    ImGui::SFML::Init(window);
    window.setFramerateLimit(MAX_FPS);
    window.setVerticalSyncEnabled(false);

    glEnable(GL_MULTISAMPLE_ARB);

    window.setMouseCursorVisible(false);

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

    sf::Mouse::setPosition(sf::Vector2i(WIDTH / 2, HIGHT / 2), window);
    sf::Vector2i mouse_pos = sf::Mouse::getPosition();
    sf::Vector2i current_mouse_pos;

    int mouse_x = WIDTH / 2;
    int mouse_y = HIGHT / 2;
    int FrameStill = 1;
    int mouse_move_x = 0;
    int mouse_move_y = 0;

    bool switcherLcontrol = true;
    bool switcherEscape = false;

    float Smooth = 0;
    int FPS = 120;

    // setting random

    std::random_device rd;
    std::mt19937 e2(rd());
    std::uniform_real_distribution<> dist(0.0f, 1.0f);

    // setting scene

    float sun_brightness = 0.02;

    sf::Glsl::Vec3 ulight_pos = sf::Glsl::Vec3(0.4f, -0.75f, 0.8f);

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
    boxes_pos[1] = sf::Glsl::Vec3(4.f, -10.f, 0.f);

    boxes_size[0] = sf::Glsl::Vec3(1.f, 1.f, 1.f);
    boxes_size[1] = sf::Glsl::Vec3(1.f, 1.f, 1.f);

    boxes_col[0] = sf::Glsl::Vec4(0.4, 0.6, 0.8, 1.f);
    boxes_col[1] = sf::Glsl::Vec4(0.6, 0.4, 0.5, 0.8);

    sf::Glsl::Vec3 planes_norm[PLANES_SIZE];
    sf::Glsl::Vec4 planes_col[PLANES_SIZE];

    int planes_num = 1;

    for (int i = 0; i < PLANES_SIZE; i++) {

        planes_norm[i] = sf::Glsl::Vec3(0.f, 0.f, 0.f);
        planes_col[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
    }

    planes_norm[0] = sf::Glsl::Vec3(0.f, 0.f, -1.f);
    planes_col[0] = sf::Glsl::Vec4(0.5, 0.5, 0.5, 1.f);

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

    int cyl_num = 0;

    for (int i = 0; i < DEFAULT_SIZE; i++) {

        cyl_down_point[i] = sf::Glsl::Vec3(0.f, 0.f, 0.f);
        cyl_up_point[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
        cyl_col[i] = sf::Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
    }

    cyl_down_point[0] = sf::Glsl::Vec3(4.f, 8.f, 1.f);
    cyl_up_point[0] = sf::Glsl::Vec4(4.f, 8.f, -2.f, 0.5);
    cyl_col[0] = sf::Glsl::Vec4(0.1, 0.6, 0.7, 1.f);

    sf::Vector3f camera_position(0.f, 0.f, 0.f);
    sf::Vector3f camera_movement(0.f, 0.f, 0.f);

    float SphereRadius = 1;
    int SphereCoord[3] = {};
    float SphereParam = 1;
    float antiSphereParam = -1;
    int SphereStatus = 2;
    float SphereIntention = 50;
    float SphereColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

    float BoxLen = 1;
    float BoxHei = 1;
    float BoxWi = 1;
    int BoxCoord[3] = {};
    // float BoxParam = 1;
    // float antiBoxParam = -1;
    // int BoxStatus = 2;
    // float BoxIntention = 50;
    float BoxColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

    float CylRadius = 1;
    int CylX[2] = {};
    int CylY[2] = {};
    int CylZ[2] = {};
    float CylColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

    int NormCoord[3] = {};
    float PlaneColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

    float ConeUpRadius = 1;
    float ConeDownRadius = 2;
    int ConeX[2] = {};
    int ConeY[2] = {};
    int ConeZ[2] = {};
    float ConeColor[3] = {(float)204 / 255, (float)77 / 255, (float)5 / 255};

    window.setActive(true);

    window.setMouseCursorVisible(switcherEscape);

    while (window.isOpen()) { // main loop

        sf::Event event;

        while (window.pollEvent(event)) { // event loop

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

                    sf::Mouse::setPosition(sf::Vector2i(WIDTH / 2, HIGHT / 2),
                                           window);
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

                    // example of an spawn logic
                    /*

                    case ( sf::Keyboard::Num1 ):

                        spheres_col[spheres_num] = sf::Glsl::Vec4( 0.1f,
                    0.7f, 0.5f, 1.f ); spheres_pos[spheres_num] =
                    sf::Glsl::Vec4( -camera_positoin.x, -camera_positoin.y,
                    camera_positoin.z, 1.f ); spheres_num = (spheres_num +
                    1) % DEFAULT_SIZE; FrameStill = 1;

                        break;

                    */

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

                    if (switcherEscape == false) {

                        // on pause
                        switcherEscape = true;
                        window.setMouseCursorVisible(switcherEscape);

                    } else {

                        // leave pause
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

        ImGui::Begin("Settings Window");

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Appearance")) {
                ImGui::CollapsingHeader("Help");
                ImGui::MenuItem("Switch Theme");
                // switch to light theme

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Data")) {
                ImGui::MenuItem("FPS Show");
                ImGui::MenuItem("Resolution Shpw");

                // FPS show on/off

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Credits")) {
                ImGui::MenuItem("AWESOMESLAYER");
                // Type our names
                ImGui::EndMenu();
            }
        }

        if (ImGui::CollapsingHeader("Objects Settings")) {
            if (ImGui::CollapsingHeader("Spheres")) {

                ImGui::InputInt("Number", &spheres_num, 0, DEFAULT_SIZE);
                ImGui::SliderFloat("Radius", &SphereRadius, 0, MAX_RADIUS);
                ImGui::InputInt3("Coord", SphereCoord);

                ImGui::RadioButton("Haze", &SphereStatus, 1);
                ImGui::SameLine();
                ImGui::RadioButton("Light", &SphereStatus, 2);
                ImGui::SameLine();
                ImGui::RadioButton("Reflection", &SphereStatus, 3);
                if (SphereStatus == 1) {
                    ImGui::SliderFloat("Haze", &SphereParam, 0, 1);
                } else if (SphereStatus == 3) {
                    ImGui::SliderFloat("Reflection", &antiSphereParam, 0, 1.99);
                    SphereParam = -antiSphereParam;
                } else if (SphereStatus == 2) {
                    ImGui::SliderFloat("Intention", &SphereIntention, 0, 100);
                }
                ImGui::ColorEdit3("Color", SphereColor);
            }
            if (ImGui::CollapsingHeader("Boxes")) {
                ImGui::InputInt("Number", &boxes_num, 0, DEFAULT_SIZE);
                ImGui::SliderFloat("Size", &BoxLen, 0, MAX_RADIUS);
                ImGui::SliderFloat("Height", &BoxHei, 0, MAX_RADIUS);
                ImGui::SliderFloat("Width", &BoxWi, 0, MAX_RADIUS);
                ImGui::InputInt3("Coord", BoxCoord);

                // ImGui::RadioButton("Haze", &BoxStatus, 1);
                // ImGui::SameLine(); ImGui::RadioButton("Light",
                // &BoxStatus, 2); ImGui::SameLine();
                // ImGui::RadioButton("Reflection", &BoxStatus, 3);
                // if(BoxStatus == 1)
                // {
                //     ImGui::SliderFloat("Haze", &BoxParam, 0, 1);
                // }
                // else if(BoxStatus == 3)
                // {
                //     ImGui::SliderFloat("Reflection", &antiBoxParam,
                //     0, 1.99); BoxParam = -antiBoxParam;
                // }
                // else if(BoxStatus == 2)
                // {
                //     ImGui::SliderFloat("Intention", &BoxIntention, 0,
                //     100);
                // }
                ImGui::ColorEdit3("Color", BoxColor);
            }
            if (ImGui::CollapsingHeader("Cylindres")) {
                ImGui::InputInt("Number", &cyl_num, 0, DEFAULT_SIZE);
                ImGui::SliderFloat("Radius", &CylRadius, 0, MAX_RADIUS);
                ImGui::InputInt2("Up and Down X", CylX);
                ImGui::InputInt2("Up and Down Y", CylY);
                ImGui::InputInt2("Up and Down Z", CylZ);

                ImGui::ColorEdit3("Color", CylColor);
            }
            if (ImGui::CollapsingHeader("Planes")) {
                ImGui::InputInt("Number", &planes_num, 0, PLANES_SIZE);
                ImGui::InputInt3("Norm Coord", NormCoord);
                ImGui::ColorEdit3("Color", PlaneColor);
            }
            if (ImGui::CollapsingHeader("Cones")) {
                ImGui::InputInt("Number", &cones_num, 0, DEFAULT_SIZE);
                ImGui::SliderFloat("Up Radius", &ConeUpRadius, 0, MAX_RADIUS);
                ImGui::SliderFloat("Down Radius", &ConeDownRadius, 0,
                                   MAX_RADIUS);
                ImGui::InputInt2("Up and Down X", ConeX);
                ImGui::InputInt2("Up and Down Y", ConeY);
                ImGui::InputInt2("Up and Down Z", ConeZ);
                ImGui::ColorEdit3("Color", ConeColor);
            }
        }
        if (ImGui::CollapsingHeader("Graphic Settings")) {
            ImGui::SliderFloat("Smooth", &Smooth, 0, MAX_SMOOTH);
            ImGui::InputInt("FPS", &FPS);
        }
        ImGui::End();

        // ImGui::ShowDemoWindow();

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
        // boxes_col[boxes_num].w = BoxParam;

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

        planes_norm[planes_num].x = NormCoord[0];
        planes_norm[planes_num].y = NormCoord[1];
        planes_norm[planes_num].z = NormCoord[2];
        planes_col[planes_num].x = PlaneColor[0];
        planes_col[planes_num].y = PlaneColor[1];
        planes_col[planes_num].z = PlaneColor[2];

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
        // window.setMouseCursorVisible(switcherEscape);
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

        window.display();
        FrameStill++;
    }

    ImGui::SFML::Shutdown(window);
    return 0;
}