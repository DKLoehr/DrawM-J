#include "MWindow.h"

MWindow::MWindow() {
    numIterations = NULL;
    prevNumIterations = NULL;
    colorMult = NULL;
    numIters = NULL;
};

MWindow::MWindow(sf::Font* f, sf::Vector2i wTopLeft, sf::Vector2i wBotRight, Vector2ld gTopLeft, Vector2ld gBotRight,
                 unsigned int* numIt, unsigned int* pNumIt, float* cMult)
{
    Create(f, wTopLeft, wBotRight, gTopLeft, gBotRight, numIt, pNumIt, cMult);
}

MWindow::MWindow(const MWindow& target) {
    if(target.numIterations == NULL) { // Looks like this window used the default constructor.
        numIterations = NULL;
        prevNumIterations = NULL;
        colorMult = NULL;
        numIters = NULL;
    }
    else {
        Create(target.inFont, target.window.getPosition(), target.window.getPosition() + sf::Vector2i(target.window.getSize().x, target.window.getSize().y),
               target.grid.GetGraphTopLeft(), target.grid.GetGraphBotRight(), target.numIterations, target.prevNumIterations, target.colorMult);
    }
}

MWindow::~MWindow()
{
    delete(firstCorner);

}

void MWindow::Create(sf::Font* f, sf::Vector2i wTopLeft, sf::Vector2i wBotRight, Vector2ld gTopLeft, Vector2ld gBotRight,
                     unsigned int* numIt, unsigned int* pNumIt, float* cMult)
{
    inFont = f;

    window.create(sf::VideoMode(wBotRight.x - wTopLeft.x, wBotRight.y - wTopLeft.y), "");
    window.setPosition(wTopLeft);

    if(!pic.create(window.getSize().x, window.getSize().y)) {
        std::cout << "Error creating RenderTexture\n";
        exit(-2);
    }

    pic.clear(sf::Color::White);
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

    numIters = NULL;
}
unsigned int MWindow::Iterate(cx* pos, cx* startPos) {
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

    ///bool check = (*numIterations > *prevNumIterations);
    unsigned int winSizeX = window.getSize().x,
                 winSizeY = window.getSize().y;
    Vector2ld topLeft = grid.GetGraphTopLeft(),
              botRight = grid.GetGraphBotRight();
    long double pixelDeltaX = (botRight.x - topLeft.x) / winSizeX, // Distance on the graph between the pixels on the window
                pixelDeltaY = (topLeft.y - botRight.y) / winSizeY;
    Vector2ld graphCoords = grid.GetGraphTopLeft();

    unsigned int xLoc = 0, yLoc = 0;/// iters = 0;
    for(unsigned int iii = winSizeY; iii != 0; iii--) {         // Iterate vertically
        for(unsigned int jjj = winSizeX; jjj != 0 ; jjj--) {    // Iterate horizontally
            /**if(check && ((numIters[xLoc][yLoc] > prevNumIterations) && !moreIters ||  // In the set and doing fewer iterations, so ignore
                        (numIters[xLoc][yLoc] <= prevNumIterations) && moreIters)) {   // Not in the set and doing more iterations, so ignore
                graphCoords.x = graphCoords.x + pixelDeltaX; // Move one pixel to the right
                xLoc = xLoc + 1;
                continue;
            }*/
            ///iters = Iterate(new cx(graphCoords.x, graphCoords.y));
            sf::Vertex loc(sf::Vector2f(xLoc, yLoc),
                           Colorgen(Iterate(new cx(graphCoords.x, graphCoords.y))));
            ///numIters[xLoc][yLoc] = iters;
            pic.draw(&loc, 1, sf::Points);
            graphCoords.x = graphCoords.x + pixelDeltaX; // Move one pixel to the right
            xLoc = xLoc + 1;
        }
        /**if(sf::Keyboard::isKeyPressed(sf::Keyboard::K)) { // K key is kill switch -- stop iterating
            interrupted = true;
            return;
        }*/
        yLoc = yLoc + 1;
        xLoc = 0;
        graphCoords.x = topLeft.x;       // Reset x coordinate
        graphCoords.y = graphCoords.y - pixelDeltaY;    // Move one pixel down
        window.draw(graphs); // Draw the updated graph to the screen after each horizontal line of pixels
        pic.display();       // Update our graph with the newest points
        window.display();
    }

    ///interrupted = false;
    *prevNumIterations = *numIterations;

    // FOR DEBUGGING
    std::cout << (std::clock() - start) / (double)CLOCKS_PER_SEC << "\n";
}

void MWindow::PollEvent(sf::Event& event) {
    window.pollEvent(event);
}

inline sf::Color MWindow::Colorgen(unsigned int seed) {
    if(seed > *numIterations) // Didn't go out from the circle, so it's in the set as far as we know
        return sf::Color::Black;
    return HSVtoRGBOp((int)(seed * *colorMult) % 360); // Loop the colors
}

void MWindow::Draw() {
    window.clear(sf::Color::White);
    window.draw(graphs);
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