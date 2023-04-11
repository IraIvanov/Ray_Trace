#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include "config.hpp"
#include <cmath>
#include <random>

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
    sf::Sprite OutSpriteFlipped = sf::Sprite(OutTexture.getTexture());
    OutSpriteFlipped.setScale( 1, -1 );
    OutSpriteFlipped.setPosition( 0, HIGHT );

    sf::RenderTexture FinalTexture;
    FinalTexture.create( WIDTH, HIGHT );
    sf::Sprite FinalSprite = sf::Sprite(FinalTexture.getTexture());
    sf::Sprite FinalSpriteFlipped = sf::Sprite(FinalTexture.getTexture());
    FinalSpriteFlipped.setScale( 1, -1 );
    FinalSpriteFlipped.setPosition( 0, HIGHT );

    sf::Clock clock;
    sf::Time time; 

    sf::Mouse::setPosition( sf::Vector2i( WIDTH/2, HIGHT/2 ), window );
    sf::Vector2i mouse_pos = sf::Mouse::getPosition();
    sf::Vector2i current_mouse_pos;
    int mouse_x = WIDTH/2;
    int mouse_y = HIGHT/2;
    int FrameStill = 1;
    //bool ControlKeys[CONTROL_KEYS_NUM] = { false };

    // setting random 
    std::random_device rd;
    std::mt19937 e2(rd());
	std::uniform_real_distribution<> dist(0.0f, 1.0f);
    //window.setMouseCursorVisible( false );
    
    sf::Vector3f camera_positoin( 0.f, 0.f, 0.f );

    window.setActive(true);
    while( window.isOpen() ) { // main loop

        sf::Event event;

        while ( window.pollEvent(event) ) {   // event loop

            switch ( event.type ) {

                case ( sf::Event::Closed ):
                    window.close();
                    break;
                case ( sf::Event::MouseMoved ):
                    
                    //mouse_x += event.mouseMove.x - WIDTH/2;
                    //mouse_y += event.mouseMove.y - HIGHT/2; 
                    current_mouse_pos = sf::Mouse::getPosition();
                    mouse_x += ( current_mouse_pos.x - mouse_pos.x );
                    mouse_y += ( current_mouse_pos.y - mouse_pos.y );
                    mouse_pos = current_mouse_pos;
                    //sf::Mouse::setPosition( sf::Vector2i( WIDTH/2, HIGHT/2 ), window ); //bugs 
                    break;
                case ( sf::Event::KeyPressed ):

                    switch( event.key.code ) {
                        
                        case( sf::Keyboard::W ): 

                            camera_positoin.x += WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case( sf::Keyboard::A ): 

                            camera_positoin.y -= WALKING_PACE;
                            FrameStill = 1;

                            break;
                        
                        case( sf::Keyboard::S ): 

                            camera_positoin.x -= WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case( sf::Keyboard::D ): 

                            camera_positoin.y += WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case( sf::Keyboard::Up ): 

                            camera_positoin.z -= WALKING_PACE;
                            FrameStill = 1;

                            break;

                        case( sf::Keyboard::Down ): 

                            camera_positoin.z +=WALKING_PACE;
                            FrameStill = 1;

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
        shader.setUniform("u_sample_part", 1.0f / FrameStill) ;
        shader.setUniform("u_seed1", sf::Vector2f((float)dist(e2), (float)dist(e2)) * 999.0f);
		shader.setUniform("u_seed2", sf::Vector2f((float)dist(e2), (float)dist(e2)) * 999.0f);

        if ( FrameStill % 2 == 1 ) {

            shader.setUniform("u_sample", OutTexture.getTexture() );
            FinalTexture.draw( OutSpriteFlipped, &shader );
            window.draw( FinalSprite );

        } else {

            shader.setUniform("u_sample", FinalTexture.getTexture() );
            OutTexture.draw( OutSpriteFlipped, &shader );
            window.draw( OutSprite );

        }


		//window.draw(OutSprite, &shader);
        /*
            Shader goes here
        */

        window.display();
        FrameStill++;

    }

    return 0;
}