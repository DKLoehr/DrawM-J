#include "MWindow.h"

MWindow::MWindow(sf::Font* f, sf::Vector2i wTopLeft, sf::Vector2i wBotRight, Vector2ld gTopLeft, Vector2ld gBotRight,
                 int* numIt, unsigned int* pNumIt, float* cMult):
            inFont(f)
{
    sf::RenderWindow window(sf::VideoMode(wBotRight.x - wTopLeft.x, wTopLeft.y - wBotRight.y), "");
    window.setPosition(wTopLeft);

    if(!pic.create(window.getSize().x, window.getSize().y)) {
        std::cout << "Error creating RenderTexture\n";
        exit(-2);
    }

    pic.clear(sf::Color::White);
    graphs = sf::Sprite();
    graphs.setPosition(0, 0);
    graphs.setTexture(pic.getTexture());

    grid = Grid(&window, sf::Vector2i(0, 0), sf::Vector2i(window.getSize()), gTopLeft, gBotRight);

    firstCorner = NULL;
    zoomBox = sf::VertexArray(sf::Lines, 5);
    zoomBox[0] = sf::Vertex(sf::Vector2f(0, 0), sf::Color(100, 100, 100));
    zoomBox[1] = zoomBox[0];
    zoomBox[2] = zoomBox[0];
    zoomBox[3] = zoomBox[0];
    zoomBox[4] = zoomBox[0];

    numIterations = numIt;
    prevNumIterations = pNumIt;
    colorMult = cMult;
}

MWindow::~MWindow()
{
    //dtor
}
