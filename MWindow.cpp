#include "MWindow.h"

MWindow::MWindow() {
    numIterations = NULL;
    prevNumIterations = 0;
    colorMult = NULL;
    numIters = NULL;
};

MWindow::MWindow(sf::Font* f, sf::Vector2i wTopLeft, sf::Vector2u wSize, Vector2ld gTopLeft, Vector2ld gBotRight,
                 unsigned int* numIt, float* cMult)
{
    Create(f, wTopLeft, wSize, gTopLeft, gBotRight, numIt, cMult);
}

MWindow::MWindow(const MWindow& target) {
    if(target.numIterations == NULL) { // Looks like this window used the default constructor.
        numIterations = NULL;
        prevNumIterations = 0;
        colorMult = NULL;
        numIters = NULL;
    }
    else {
        Create(target.inFont, target.window.getPosition(), target.window.getSize(),
               target.grid.GetGraphTopLeft(), target.grid.GetGraphBotRight(), target.numIterations, target.colorMult);
        prevNumIterations = target.prevNumIterations;
        graphs.setTexture(target.pic.getTexture());
        pic.draw(graphs);
        pic.display();
        graphs.setTexture(pic.getTexture());
    }
}

MWindow::~MWindow()
{
    delete(firstCorner);

}

void MWindow::Create(sf::Font* f, sf::Vector2i wTopLeft, sf::Vector2u wSize, Vector2ld gTopLeft, Vector2ld gBotRight,
                     unsigned int* numIt, float* cMult)
{
    inFont = f;

    window.create(sf::VideoMode(wSize.x, wSize.y), "");
    window.setPosition(wTopLeft);


    if(!pic.create(window.getSize().x, window.getSize().y)) {
        std::cout << "Error creating RenderTexture\n";
        exit(-2);
    }

    pic.clear(sf::Color::White);
    graphs.setPosition(0, 0);
    graphs.setTexture(pic.getTexture());

    grid = Grid(&window, sf::Vector2i(0, 0), sf::Vector2i(window.getSize()), gTopLeft, gBotRight);
    grid.SetRangeCorners(gTopLeft, gBotRight);

    firstCorner = NULL;
    zoomBox = sf::VertexArray(sf::LinesStrip, 5);
    zoomBox[0] = sf::Vertex(sf::Vector2f(0, 0), sf::Color(100, 100, 100));
    zoomBox[1] = zoomBox[0];
    zoomBox[2] = zoomBox[0];
    zoomBox[3] = zoomBox[0];
    zoomBox[4] = zoomBox[0];

    iterThread = new sf::Thread(&MWindow::IterateGraph, this);

    interrupted = true;
    numIterations = numIt;
    prevNumIterations = 0;
    colorMult = cMult;

    numIters = (uint16_t**)malloc(sizeof(uint16_t*) * window.getSize().x);
    for(int iii = 0; iii < window.getSize().x; iii++) {
        numIters[iii] = (uint16_t*)malloc(sizeof(uint16_t) * window.getSize().y);
    }
}

unsigned int MWindow::Iterate(cx* pos, cx* startPos) {
    if(numIterations == NULL) {
        std::cout << "NULL\n";
    }
    if(abs(*pos) > 2)   // If we're starting outside our circle of radius 2, we're already done
        return 0;       // Took 0 iterations to get ourside the circle, so return 0

    if(startPos == NULL)
        startPos = new cx(0,0);

    long double a = startPos->real(), b = startPos->imag(), c = pos->real(), d = pos->imag(), k1 = a * a, k2 = b * b;

    delete startPos; // We've gotten what we needed from pos and startPos, so get rid of them to avoid memory leaks
    delete pos;
    for(unsigned int iii = *numIterations; iii != 0; iii--) {
        //startPos = startPos * startPos + pos; // This is equivalent to the below code (except without pointers), but a lot slower
        b = (a + b) * (a + b) - k1 - k2 + d; // b uses a, but a doesn't use b, so calculate b first
        a = k1 - k2 + c;

        k1 = a * a;
        k2 = b * b;
        if(k1 + k2 > 4)    // If we've gone outside our circle of radius 2, we're done
            return *numIterations - iii + 1; // Return the number of iterations it took to get outside the circle
    }
    return *numIterations + 1;   // If we didn't return in the for loop, then we're still inside the circle; return number of iterations performed
}

void MWindow::IterateGraph() {
    std::clock_t start; // FOR DEBUGGING
    start = std::clock(); // FOR DEBUGGING

    bool moreIters = (*numIterations > prevNumIterations);
    unsigned int winSizeX = window.getSize().x,
                 winSizeY = window.getSize().y;
    Vector2ld topLeft = grid.GetGraphTopLeft(),
              botRight = grid.GetGraphBotRight();
    long double pixelDeltaX = (botRight.x - topLeft.x) / winSizeX, // Distance on the graph between the pixels on the window
                pixelDeltaY = (topLeft.y - botRight.y) / winSizeY;
    Vector2ld graphCoords = topLeft;
    graphCoords.y = botRight.y;
    int startNumIters = *numIterations;
    //pic.clear(sf::Color(0, 0, 0, 0));

    std::printf("(%f, %f), (%f, %f) \n", (double)(topLeft.x), (double)(topLeft.y), (double)(botRight.x), (double)(botRight.y));

    unsigned int iters = 0;
    for(unsigned int iii = winSizeY + 1; iii != 0; iii--) {         // Iterate vertically
        for(unsigned int jjj = winSizeX; jjj != 0 ; jjj--) {    // Iterate horizontally
            if(!interrupted && ((numIters[winSizeX - jjj][iii - 1] > prevNumIterations) && !moreIters ||    // In the set and doing fewer iterations, so ignore
                                (numIters[winSizeX - jjj][iii - 1] <= prevNumIterations) && moreIters)) {   // Not in the set and doing more iterations, so ignore
                graphCoords.x = graphCoords.x + pixelDeltaX; // Move one pixel to the right
                continue;
            }
            iters = Iterate(new cx(graphCoords.x, graphCoords.y));
            sf::Vertex loc(sf::Vector2f(winSizeX - jjj, iii - 1),
                           Colorgen(iters));
            numIters[winSizeX - jjj][iii - 1] = iters;
            pic.draw(&loc, 1, sf::Points);
            graphCoords.x = graphCoords.x + pixelDeltaX; // Move one pixel to the right
        }
        /*if(sf::Keyboard::isKeyPressed(sf::Keyboard::K)) { // K key is kill switch -- stop iterating
            interrupted = true;
            return;
        }*/
        graphCoords.x = topLeft.x;       // Reset x coordinate
        graphCoords.y = graphCoords.y + pixelDeltaY;    // Move one pixel down
    }

    pic.display();
    graphs.setTexture(pic.getTexture());

    interrupted = (*numIterations != startNumIters);
    prevNumIterations = *numIterations;

    // FOR DEBUGGING
    std::cout << (std::clock() - start) / (double)CLOCKS_PER_SEC << "\n";
}

inline sf::Color MWindow::Colorgen(unsigned int seed) {
    if(seed > *numIterations) // Didn't go out from the circle, so it's in the set as far as we know
        return sf::Color::Black;
    return HSVtoRGBOp((int)(seed * *colorMult) % 360); // Loop the colors
}

int MWindow::PollEvent(sf::Event& event, Vector2ld** topLeft, Vector2ld** botRight, bool checkBox) {
    int ret = window.pollEvent(event);
    if(!checkBox || !ret) {
        return ret;
    }
    if(event.type == sf::Event::MouseMoved) {
        if(firstCorner != NULL) {
            zoomBox[1].position = sf::Vector2f(event.mouseMove.x, zoomBox[0].position.y);
            zoomBox[2].position = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
            zoomBox[3].position = sf::Vector2f(zoomBox[0].position.x, event.mouseMove.y);
        } else {
            zoomBox[0].position = sf::Vector2f(0, 0);
            zoomBox[1].position = sf::Vector2f(0, 0);
            zoomBox[2].position = sf::Vector2f(0, 0);
            zoomBox[3].position = sf::Vector2f(0, 0);
        }
    } else if(event.type == sf::Event::MouseButtonPressed) {
        if(firstCorner == NULL) {   // Aren't currently selecting a rectangle
            firstCorner = new Vector2ld(grid.WindowToGraph(event.mouseButton.x, event.mouseButton.y).x, // Graph coordinates of the first corner
                                        grid.WindowToGraph(event.mouseButton.x, event.mouseButton.y).y);
            zoomBox[0].position = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
            zoomBox[1] = zoomBox[0];
            zoomBox[2] = zoomBox[0];
            zoomBox[3] = zoomBox[0];
            zoomBox[4] = zoomBox[0];
        } else {
            Vector2ld click = grid.WindowToGraph(event.mouseButton.x, event.mouseButton.y); // Window coordinates of the click
            *topLeft = new Vector2ld(min(click.x, firstCorner->x),
                                     max(click.y, firstCorner->y));
            *botRight = new Vector2ld(max(click.x, firstCorner->x),
                                      min(click.y, firstCorner->y));
            delete(firstCorner);
            firstCorner = NULL;
        }
    } else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        delete(firstCorner);
        firstCorner = NULL;
    }
    return ret;
}

void MWindow::UpdateGraph() {
    //iterThread->wait();
    //iterThread->launch();
    IterateGraph();
}

void MWindow::SetActive(bool isActive) {
    window.setActive(isActive);
}

void MWindow::Draw() {
    window.clear(sf::Color::White);
    //pic.display();
    //graphs.setTexture(pic.getTexture());
    window.draw(graphs);

    if(true || firstCorner != NULL) {
        window.draw(zoomBox);
    }

    window.display();
}

inline sf::Color MWindow::HSVtoRGBOp(int hue) {
    int h = hue / 60;
    float f = float(hue) / 60 - h;
    float q = 1 - f;

    switch(h)
    {
        default:
        case 0:
        case 6: return sf::Color(255, f * 255, 0);
        case 1: return sf::Color(q * 255, 255, 0);
        case 2: return sf::Color(0, 255, f * 255);
        case 3: return sf::Color(0, q * 255, 255);
        case 4: return sf::Color(f * 255, 0, 255);
        case 5: return sf::Color(255, 0, q * 255);
    }
}
