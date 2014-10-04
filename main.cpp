#include "runner.h"
//#include "ResourcePath.hpp" // For Macs; comment out if on Windows or Linux

int main() {
    sf::Font inFont;
    if(!inFont.loadFromFile("VeraMono.ttf")) // For Windows & Linux; comment out if on Mac
        return -1;
    //if(!inFont.loadFromFile(resourcePath() + "VeraMono.ttf")) // For Macs; comment out if on Windows or Linux
    //    return -1;

    sf::RenderWindow window(sf::VideoMode(1000, 750), "Draw M&J Revamped");
    window.setPosition(sf::Vector2i(0, 0));

    sf::RenderWindow jWindow(sf::VideoMode(500, 500), "Julia Set");
    jWindow.setPosition(sf::Vector2i(window.getSize().x + 16, 0));

    sf::RenderTexture pic;
    if(!pic.create(window.getSize().x, window.getSize().y - HEIGHT_OFFSET))
        return -2;

    sf::RenderTexture jPic;
    if(!jPic.create(jWindow.getSize().x, jWindow.getSize().y))
        return -3;

    Runner run = Runner(&window, &jWindow, &inFont, &pic, &jPic);

    while(window.isOpen()) {
        run.HandleEvents();
        run.Draw();
    }

    if(!jWindow.isOpen()) {
        jWindow.close();
    }
    return 0;
}
