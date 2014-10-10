#include "runner.h"
#include <time.h>

Runner::Runner(sf::RenderWindow* w, sf::RenderWindow* j, sf::Font* font, sf::RenderTexture* p, sf::RenderTexture* jP) :
    window(w),
    jWindow(j),
    inFont(font),
    pic(p),
    jPic(jP)
{
    Init();
}

Runner::~Runner() {
    for(int iii = 0; iii < WIN_SIZE_X; iii++) {
        delete numIters[iii];
    }
    delete numIters;
}


void Runner::Init() {
    numIterations = 0;
    prevNumIterations = 0;
    colorMult = 7;

    pic->clear(sf::Color::White);
    graphs.setPosition(0, HEIGHT_OFFSET);
    graphs.setTexture(pic->getTexture());

    jPic->clear(sf::Color::White);
    jGraphs.setPosition(0, 0);
    jGraphs.setTexture(jPic->getTexture());

    numIters = new uint16_t*[WIN_SIZE_X];

    for(int iii = 0; iii < WIN_SIZE_X; iii++) {
        numIters[iii] = new uint16_t[WIN_SIZE_Y];
        if(numIters[iii] == NULL) {
            std::cout << "ERROR!\n";
            throw -1;
        }
    }

    grid = Grid(window, sf::Vector2i(0, HEIGHT_OFFSET), (sf::Vector2i)window->getSize(),
                Vector2ld(-2, 1), Vector2ld(-1, 1));

    firstCorner = NULL;
    box = sf::VertexArray(sf::LinesStrip, 5);
    box[0] = sf::Vertex(sf::Vector2f(0, 0), sf::Color(100, 100, 100));
    box[1] = box[0];
    box[2] = box[0];
    box[3] = box[0];
    box[4] = box[0];

    fct = new parser::Tree("Z^2 + C");

    elements = std::vector<GUI*>(0);

    /** GUI Creation **/
    iterations = InputBox(window, inFont, 5, 5, 73, 15, "Number of Iterations"); // 0
    elements.push_back(&iterations);

    okIterations = Button(window, inFont, iterations.GetPosition().x + iterations.GetSize().x + 210, iterations.GetPosition().y,
                        108, 15, "Save Changes"); // 1
    elements.push_back(&okIterations);

    colorNum = InputBox(window, inFont, okIterations.GetPosition().x + okIterations.GetSize().x + 150, 5, 73, 15, "Color Multiple"); // 2
    elements.push_back(&colorNum);

    okColor = Button(window, inFont, colorNum.GetPosition().x + colorNum.GetSize().x + 150, colorNum.GetPosition().y,
                        108, 15, "Save Changes"); // 3
    elements.push_back(&okColor);

    /** End GUI Creations **/

    for(int iii = 0; iii < elements.size(); iii++) {
        elements[iii]->SetActive(false);
    }
    activeBox = 0; // Start out highlighting the input box
    elements[activeBox]->SetActive(true);
    window->setPosition(sf::Vector2i(0, 0)); // Start out with the main window active rather than the julia set one
    SetUpGraph();
}

void Runner::SetUpGraph() {
    ClearPic();
    Vector2ld TL(-2.05, 1.15),
              BR(.75, -1.15);
    grid.SetRangeCorners(TL, BR);
    unsigned int winSizeX = window->getSize().x,
                 winSizeY = window->getSize().y - HEIGHT_OFFSET;
    long double pixelDeltaX = (BR.x - TL.x) / winSizeX, // Distance on the graph between the pixels on the window
                pixelDeltaY = (TL.y - BR.y) / winSizeY;
    Vector2ld graphCoords = TL;
    unsigned int xLoc = 0, yLoc = 0, iters = 0;
    for(unsigned int iii = winSizeY; iii != 0; iii--) {         // Iterate vertically
        for(unsigned int jjj = winSizeX; jjj != 0 ; jjj--) {     // Iterate horizontally
            iters = Iterate(new cx(graphCoords.x, graphCoords.y));
            sf::Vertex loc(sf::Vector2f(xLoc, yLoc),
                           Colorgen(iters));
            numIters[xLoc][yLoc] = iters;
            pic->draw(&loc, 1, sf::Points);
            graphCoords.x = graphCoords.x + pixelDeltaX; // Move one pixel to the right
            xLoc = xLoc + 1;
        }
        yLoc = yLoc + 1;
        xLoc = 0;
        graphCoords.x = TL.x;                      // Reset x coordinate
        graphCoords.y = graphCoords.y - pixelDeltaY;    // Move one pixel down
    }
    prevNumIterations = numIterations;
}

void Runner::HandleEvents() {
    sf::Event event;
    while(jWindow->pollEvent(event)); // For internal handling of events
    while(window->pollEvent(event)) {
        if(event.type == sf::Event::Closed ||
           (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
        {
            window->close();
        } else if(event.type == sf::Event::TextEntered) {
            elements[activeBox]->OnTextEntered(event.text.unicode);
        } else if((event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) ||
                  (event.type == sf::Event::MouseButtonPressed && event.mouseButton.y < HEIGHT_OFFSET)) {
            ActivateButtons(event);
        } else if(event.type == sf::Event::MouseMoved) {
            if(event.mouseMove.y < HEIGHT_OFFSET) // Upper part of the screen
                SetActiveElement(event.mouseMove.x, event.mouseMove.y);
            else { // In the graph
                    box[1].position = sf::Vector2f(event.mouseMove.x, box[0].position.y);
                    box[2].position = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
                    box[3].position = sf::Vector2f(box[0].position.x, event.mouseMove.y);
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        Vector2ld graphCoords(grid.WindowToGraph(event.mouseMove.x, event.mouseMove.y));
                        UpdateJulia(cx(graphCoords.x, graphCoords.y));
                    }
            }
        } else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
            StepActiveElement(!(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                                sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)));
        } else if(event.type == sf::Event::MouseButtonPressed) {
            if(event.mouseButton.y < HEIGHT_OFFSET) { // Above the graphs
                ActivateButtons(event);
            } else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { // In one of the graphs, not Julia Set
                if(firstCorner == NULL) {   // Aren't currently selecting a rectangle
                    firstCorner = new Vector2ld(grid.WindowToGraph(event.mouseButton.x, event.mouseButton.y).x, // Graph coordinates of the first corner
                                                grid.WindowToGraph(event.mouseButton.x, event.mouseButton.y).y);
                    box[0].position = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
                    box[1] = box[0];
                    box[2] = box[0];
                    box[3] = box[0];
                    box[4] = box[0];
                }
                else { // We've already had the first corner selected
                    Vector2ld secondCorner(grid.WindowToGraph(event.mouseButton.x, event.mouseButton.y).x, // Graph coordinates of the second corner
                                           grid.WindowToGraph(event.mouseButton.x, event.mouseButton.y).y);
                    Vector2ld clickTL(min(firstCorner->x, secondCorner.x), max(firstCorner->y, secondCorner.y)), // Graph coordinates of the top-left corner
                              clickBR(max(firstCorner->x, secondCorner.x), min(firstCorner->y, secondCorner.y)); // Graph coords of the bottom-right corner
                    long double BRx = 0, BRy = 0, TLx = 0, TLy = 0; // Coordinates of the corners of the window after the zoom
                    if(!sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) { // The left control key is not pressed, so zoom in
                        BRx = clickBR.x; // Our corners are just the places we clicked
                        BRy = clickBR.y;
                        TLx = clickTL.x;
                        TLy = clickTL.y;
                    } else { // The left control key is pressed, so zoom out
                        Vector2ld currentBR = grid.GetGraphBotRight(),
                                  currentTL = grid.GetGraphTopLeft();
                        // Result of hard math. Take my word for it.
                        BRx = (currentBR.x - currentTL.x) / (clickBR.x - clickTL.x) * (currentBR.x - clickBR.x) + currentBR.x,
                        BRy = (currentBR.y - currentTL.y) / (clickBR.y - clickTL.y) * (currentBR.y - clickBR.y) + currentBR.y,
                        TLx = BRx - (currentBR.x - currentTL.x) * (currentBR.x - currentTL.x) / (clickBR.x - clickTL.x),
                        TLy = BRy - (currentBR.y - currentTL.y) * (currentBR.y - currentTL.y) / (clickBR.y - clickTL.y);
                    }
                    Vector2ld* newTL = new Vector2ld(TLx, TLy);
                    Vector2ld* newBR = new Vector2ld(BRx, BRy);
                    UpdateGraph(newTL, newBR);
                    delete newTL;
                    delete newBR;
                    delete firstCorner; // Get rid of the first corner's location since it is no longer relevant (second corner will just go out of scope)
                    firstCorner = NULL; // Cease looking at unallocated memory
                }
            } else { // In graph, Julia set
                Vector2ld graphCoords(grid.WindowToGraph(event.mouseButton.x, event.mouseButton.y));
                UpdateJulia(cx(graphCoords.x, graphCoords.y));
            }
        }
    }
}

unsigned int Runner::Iterate(cx* pos, cx* startPos) {
    if(abs(*pos) > 2)   // If we're starting outside our circle of radius 2, we're already done
        return 0;       // Took 0 iterations to get ourside the circle, so return 0

    if(startPos == NULL)
        startPos = new cx(0,0);

    long double a = startPos->real(), b = startPos->imag(), c = pos->real(), d = pos->imag(), k1 = a * a, k2 = b * b;

    delete startPos; // We've gotten what we needed from pos and startPos, so get rid of them to avoid memory leaks
    delete pos;
    for(unsigned int iii = numIterations; iii != 0; iii--) {
        //startPos = startPos * startPos + pos; // This is equivalent to the below code (except without pointers), but a lot slower
        b = (a + b) * (a + b) - k1 - k2 + d; // b uses a, but a doesn't use b, so calculate b first
        a = k1 - k2 + c;

        k1 = a * a;
        k2 = b * b;
        if(k1 + k2 > 4)    // If we've gone outside our circle of radius 2, we're done
            return numIterations - iii + 1; // Return the number of iterations it took to get outside the circle
    }
    return numIterations + 1;   // If we didn't return in the for loop, then we're still inside the circle; return number of iterations performed
}

void Runner::SetActiveElement(double x, double y) {
    for(int iii = 0; iii < elements.size(); iii++) {
        if(elements[iii]->OnClick(x, y)) {              // True iff x and y are in that element's box; will toggle checkboxes, so must toggle back later
            if(activeBox != iii) {                      // If the active box has changed
                elements[activeBox]->SetActive(false);  // Deactivate the previously-active box
                activeBox = iii;                        // Update which box is active
                elements[activeBox]->SetActive(true);   // Activate the newly-active box
            }
            elements[iii]->OnClick(x, y);               // For checkboxes; toggle them back
            break;                                      // Figured out which box is active, so we can stop looking now
        }
        elements[iii]->OnClick(x, y);                   // For checkboxes; toggle them back
    }
}

void Runner::StepActiveElement(bool increment) {
    elements[activeBox]->SetActive(false);          // Deactivate previously-active box
    if(increment)                                   // If we're going forward (down or to the right)
        activeBox = ++activeBox % elements.size();  // Increment by one modularly
    else
        activeBox = (activeBox + elements.size() - 1) % elements.size(); // Decrement by one modularly
    elements[activeBox]->SetActive(true);           // Activate the newly-active box
}

void Runner::UpdateIterations() {
    numIterations = ToInt((std::string)iterations.GetText()); // Set numIterations to the number specified in the box
    if(numIterations > (1 << 16) - 1)   // Cap to 2^16 - 1 both to prevent absurd iteration times and to keep it storable in numIters
        numIterations = (1 << 16) - 1;
    UpdateGraph(grid.GetGraphTopLeftP(), grid.GetGraphBotRightP());
}

void Runner::UpdateColor() {
    colorMult = (float)atof(colorNum.GetText().c_str()); // Set the color multiplier to the number specified in the box
    std::cout << colorMult << "\n";
    unsigned int winSizeX = window->getSize().x,
                 winSizeY = window->getSize().y - HEIGHT_OFFSET;
    for(unsigned int iii = 0; iii < winSizeY; iii++) {         // Iterate vertically
        for(unsigned int jjj = 0; jjj < winSizeX ; jjj++) {    // Iterate horizontally
            if(numIters[jjj][iii] <= numIterations) {
                sf::Vertex loc(sf::Vector2f(jjj, iii), Colorgen(numIters[jjj][iii]));
                pic->draw(&loc, 1, sf::Points);
            }
        }
        window->draw(graphs); // Draw the updated graph to the screen after each horizontal line of pixels
        pic->display();       // Update our graph with the newest points
        window->display();
    }
}

void Runner::ActivateButtons(sf::Event event) {
    switch(activeBox) {
    case 1: // Save Changes for iteration number
        UpdateIterations();
        break;
    case 3: // Save Changes for iteration number
        UpdateColor();
        break;
    default:
        if(event.type == sf::Event::MouseButtonPressed)
            elements[activeBox]->OnClick(event.mouseButton.x, event.mouseButton.y);
        else
            elements[activeBox]->OnEnter();
    }
}

void Runner::UpdateGraph(Vector2ld* topLeft, Vector2ld* botRight) {
    std::clock_t start; // FOR DEBUGGING
    start = std::clock(); // FOR DEBUGGING

    bool moreIters = (numIterations >= prevNumIterations);
    bool check = (*topLeft == grid.GetGraphTopLeft() || *botRight == grid.GetGraphBotRight()) && // Neither of the corners has changed position (not zooming)
                 !interrupted; // And we weren't interrupted last time

    if(!check && !interrupted) // We're zooming
        ClearPic(); // Changing the graph entirely, so just clear it first

    grid.SetRangeCorners(*topLeft, *botRight);
    unsigned int winSizeX = window->getSize().x,
                 winSizeY = window->getSize().y - HEIGHT_OFFSET;
    long double pixelDeltaX = (botRight->x - topLeft->x) / winSizeX, // Distance on the graph between the pixels on the window
                pixelDeltaY = (topLeft->y - botRight->y) / winSizeY;
    Vector2ld graphCoords = *topLeft;
    unsigned int xLoc = 0, yLoc = 0, iters = 0;
    for(unsigned int iii = winSizeY; iii != 0; iii--) {         // Iterate vertically
        for(unsigned int jjj = winSizeX; jjj != 0 ; jjj--) {    // Iterate horizontally
            if(check && ((numIters[xLoc][yLoc] > prevNumIterations) && !moreIters ||  // In the set and doing fewer iterations, so ignore
                        (numIters[xLoc][yLoc] <= prevNumIterations) && moreIters)) {   // Not in the set and doing more iterations, so ignore
                graphCoords.x = graphCoords.x + pixelDeltaX; // Move one pixel to the right
                xLoc = xLoc + 1;
                continue;
            }
            iters = Iterate(new cx(graphCoords.x, graphCoords.y));
            sf::Vertex loc(sf::Vector2f(xLoc, yLoc),
                           Colorgen(iters));
            numIters[xLoc][yLoc] = iters;
            pic->draw(&loc, 1, sf::Points);
            graphCoords.x = graphCoords.x + pixelDeltaX; // Move one pixel to the right
            xLoc = xLoc + 1;
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::K)) { // K key is kill switch -- stop iterating
            interrupted = true;
            return;
        }
        yLoc = yLoc + 1;
        xLoc = 0;
        graphCoords.x = topLeft->x;                      // Reset x coordinate
        graphCoords.y = graphCoords.y - pixelDeltaY;    // Move one pixel down
        window->draw(graphs); // Draw the updated graph to the screen after each horizontal line of pixels
        pic->display();       // Update our graph with the newest points
        window->display();
    }

    interrupted = false;
    prevNumIterations = numIterations;

    // FOR DEBUGGING
    std::cout << (std::clock() - start) / (double)CLOCKS_PER_SEC << "\n";
}

void Runner::UpdateJulia(cx pos) {
    unsigned int winSizeX = jWindow->getSize().x,
                 winSizeY = jWindow->getSize().y;
    long double pixelDeltaX = 4.0 / winSizeX, // Distance on the graph between the pixels on the window
                pixelDeltaY = 4.0 / winSizeY;
    Vector2ld graphCoords = Vector2ld(-2.0, 2.0);
    unsigned int xLoc = 0, yLoc = winSizeY;
    for(unsigned int iii = winSizeY; iii != 0; iii--) {         // Iterate vertically
        for(unsigned int jjj = winSizeX; jjj != 0 ; jjj--) {     // Iterate horizontally
            cx *cPos = new cx(0, 0);
            *cPos = pos;
            sf::Vertex loc(sf::Vector2f(xLoc, yLoc),
                           Colorgen(Iterate(cPos, new cx(graphCoords.x, graphCoords.y))));
            jPic->draw(&loc, 1, sf::Points);
            graphCoords.x = graphCoords.x + pixelDeltaX; // Move one pixel to the right
            xLoc = xLoc + 1;
        }
        yLoc = yLoc - 1;
        xLoc = 0;
        graphCoords.x = -2;                      // Reset x coordinate
        graphCoords.y = graphCoords.y - pixelDeltaY;    // Move one pixel down
        jWindow->draw(jGraphs); // Draw the updated graph to the screen after each horizontal line of pixels
        jPic->display();       // Update our graph with the newest points
        jWindow->display();
    }
}

void Runner::ClearPic() {
    pic->clear(sf::Color::White);   // Clear the canvas (pic) to be fully white
    Draw();
}

inline sf::Color Runner::Colorgen(unsigned int seed) {
    if(seed > numIterations) // Didn't go out from the circle, so it's in the set as far as we know
        return sf::Color::Black;
    return HSVtoRGBOp((int)(seed * colorMult) % 360); // Loop the colors
}

void Runner::Draw() {
    window->clear(sf::Color::White); // Clear in preparation for drawing new stuff
    jWindow->clear(sf::Color::White);

    /// Draw GUI elements
    for(int iii = 0; iii < elements.size(); iii++) { // Draw each GUI element (textboxes, buttons, checkboxes)
        elements[iii]->Draw();
    }

    /// Draw graph elements
    window->draw(graphs); // Draw the updated graph to the screen
    jWindow->draw(jGraphs);

    if(firstCorner != NULL) // Draw the box if we're tracking the first corner
        window->draw(box);

    pic->display(); // Update our graph with the newest points
    jPic->display();

    window->display(); // Display everything we've drawn on the screen
    jWindow->display();
}

// Converts a string with a number in it to an integer containing that number
inline int Runner::ToInt(std::string str) {
    int ret = 0;
    for(int i = str.length() - 1; i >= 0; i--) {
        ret += pow(10, str.length() - 1 - i) * (str[i] - '0');
    }
    return ret;
}

// Convert HSV color values to RGB color values
sf::Color Runner::HSVtoRGB(int hue, double sat, double val) {
    while(hue < 0) hue += 360;
    hue %= 360;

    if(sat < 0) sat = 0.0;
    if(sat > 1) sat = 1.0;

    if(val < 0) val = 0.0;
    if(val > 1) val = 1.0;

    int h = hue / 60;
    double f = double(hue) / 60 - h;
    double p = val * (1 - sat);
    double q = val * (1 - sat * f);
    double t = val * (1 - sat * (1 - f));

    switch(h)
    {
        default:
        case 0:
        case 6: return sf::Color(val * 255, t * 255, p * 255);
        case 1: return sf::Color(q * 255, val * 255, p * 255);
        case 2: return sf::Color(p * 255, val * 255, t * 255);
        case 3: return sf::Color(p * 255, q * 255, val * 255);
        case 4: return sf::Color(t * 255, p * 255, val * 255);
        case 5: return sf::Color(val * 255, p * 255, q * 255);
    }
}

// Optimizations: assume sat and val always == 1
inline sf::Color Runner::HSVtoRGBOp(int hue) {
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
