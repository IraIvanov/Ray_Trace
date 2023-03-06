#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include "config.hpp"
#include <cmath>

int main() {
    
    sf::RenderWindow window ( sf::VideoMode( HIGHT, WIDTH ), "test window");
    window.setFramerateLimit(MAX_FPS);
    window.setVerticalSyncEnabled( false ); // Vsync Disabled

    sf::Shader shader;
    shader.loadFromFile( "raytracing.frag", sf::Shader::Fragment );
    shader.setUniform( "u_resolution", sf::Vector2f( WIDTH, HIGHT ) ); 
 
    sf::RenderTexture OutTexture;
    OutTexture.create( WIDTH/**2*/, HIGHT/**0.6 */); // using WIDTH*2 in order to fit screan size, 0.6 fixing resolution  
    sf::Sprite OutSprite = sf::Sprite(OutTexture.getTexture());
 
    sf::Clock clock;
    sf::Time time;
 
    sf::Vector2i mouse_pos( WIDTH/2, HIGHT/2 );
    sf::Vector2i current_mouse_pos( WIDTH/2, HIGHT/2);
    sf::Vector2f u_mouse ( WIDTH/2, HIGHT/2 );
    window.setMouseCursorVisible( false );

    while( window.isOpen() ) { // main loop

        sf::Event event;

        while ( window.pollEvent(event) ) {   // event loop

            switch ( event.type ) {

                case ( sf::Event::Closed ):
                    window.close();
                    break;
                case ( sf::Event::MouseMoved ):
                    current_mouse_pos = sf::Mouse::getPosition();
                    u_mouse.x += ( current_mouse_pos.x - WIDTH/2 );
                    u_mouse.y += ( current_mouse_pos.y - HIGHT/2 );
                    sf::Mouse::setPosition( mouse_pos, window );
                    break;    
                default:
                    break;
            }

        }
        u_mouse.x /= WIDTH;
        u_mouse.y /= HIGHT;
        shader.setUniform("u_mouse",  u_mouse );

        time = clock.getElapsedTime();
        float u_time = time.asSeconds();
        shader.setUniform("u_time",  u_time);
        window.setActive();
		window.draw(OutSprite, &shader);
        /*
            Shader goes here
        */

        window.display();

    }

    return 0;
}