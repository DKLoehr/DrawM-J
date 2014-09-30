#include "runner.h"
//#include "ResourcePath.hpp" // For Macs; comment out if on Windows or Linux

int main() {
    sf::Font inFont;
    if(!inFont.loadFromFile("VeraMono.ttf")) // For Windows & Linux; comment out if on Mac
        return -1;
    //if(!inFont.loadFromFile(resourcePath() + "VeraMono.ttf")) // For Macs; comment out if on Windows or Linux
    //    return -1;

    sf::RenderWindow window(sf::VideoMode(1200, 724), "Complex Paint Revamped");
    window.setPosition(sf::Vector2i(0, 0));

    sf::RenderTexture pic;
    if(!pic.create(window.getSize().x, window.getSize().y - HEIGHT_OFFSET))
        return -2;

    Runner run = Runner(&window, &inFont, &pic);

    while(window.isOpen()) {
        run.HandleEvents();
        run.Draw();
    }
    return 0;
}
