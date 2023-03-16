#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include "config.hpp"
#include <cmath>

int main() {
    
    sf::RenderWindow window ( sf::VideoMode( WIDTH, HIGHT ), "test window");
    window.setFramerateLimit(MAX_FPS);
    window.setVerticalSyncEnabled( false ); // Vsync Disabled

    sf::Shader shader;
    shader.loadFromFile( "raytracing.frag", sf::Shader::Fragment );
    shader.setUniform( "u_resolution", sf::Vector2f( WIDTH, HIGHT ) ); 
 
    sf::RenderTexture OutTexture;
    OutTexture.create( WIDTH, HIGHT); // using WIDTH*2 in order to fit screan size, 0.6 fixing resolution  
    sf::Sprite OutSprite = sf::Sprite(OutTexture.getTexture());
 
    sf::Clock clock;
    sf::Time time; 

    sf::Mouse::setPosition( sf::Vector2i( WIDTH/2, HIGHT/2 ), window );
    sf::Vector2i mouse_pos = sf::Mouse::getPosition();
    sf::Vector2i current_mouse_pos;
    int mouse_x = WIDTH/2;
    int mouse_y = HIGHT/2;
    bool ControlKeys[CONTROL_KEYS_NUM] = { false };
    
    //window.setMouseCursorVisible( false );

    sf::Vector3f camera_positoin( 0.f, 0.f, 0.f );

    while( window.isOpen() ) { // main loop

        sf::Event event;

        while ( window.pollEvent(event) ) {   // event loop

            switch ( event.type ) {

                case ( sf::Event::Closed ):
                    window.close();
                    break;
                case ( sf::Event::MouseMoved ):
                    
                    current_mouse_pos = sf::Mouse::getPosition();
                    mouse_x += ( current_mouse_pos.x - mouse_pos.x );
                    mouse_y += ( current_mouse_pos.y - mouse_pos.y );
                    mouse_pos = current_mouse_pos;
                    //sf::Mouse::setPosition( sf::Vector2i( WIDTH/2, HIGHT/2 ), window ); bugs 
                    break;
                case ( sf::Event::KeyPressed ):

                    switch( event.key.code ) {
                        
                        case( sf::Keyboard::W ): 

                            camera_positoin.x += WALKING_PACE;

                            break;

                        case( sf::Keyboard::A ): 

                            camera_positoin.y -= WALKING_PACE;

                            break;
                        
                        case( sf::Keyboard::S ): 

                            camera_positoin.x -= WALKING_PACE;

                            break;

                        case( sf::Keyboard::D ): 

                            camera_positoin.y += WALKING_PACE;

                            break;

                        case( sf::Keyboard::Up ): 

                            camera_positoin.z -= WALKING_PACE;

                            break;

                        case( sf::Keyboard::Down ): 

                            camera_positoin.z +=WALKING_PACE;

                            break;

                        default:
                            break;
                    }

                    break;
                
                default:
                    break;
            }

        }
        float res_mouse_x =  float(mouse_x) / WIDTH - 0.5f; 
        float res_mouse_y =  float(mouse_y) / HIGHT - 0.5f;
        window.setActive();
        shader.setUniform("u_mouse",  sf::Vector2f(res_mouse_x * MOUSE_SENSITIVITY, res_mouse_y * MOUSE_SENSITIVITY ) );

        time = clock.getElapsedTime();
        float u_time = time.asSeconds();
        shader.setUniform("u_time",  u_time);                 
        shader.setUniform("u_camera_pos", camera_positoin );
        /*for ( int i = 0; i < MOVING_KEYS; i++ ) {
            
            if ( ControlKeys[i] == true ) std::cout << " key is pressed " << std::endl;

        }*/

		window.draw(OutSprite, &shader);
        /*
            Shader goes here
        */

        window.display();

    }

    return 0;
}