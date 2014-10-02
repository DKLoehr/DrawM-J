#include "runner.h"
#include <time.h>

Runner::Runner(sf::RenderWindow* w, sf::Font* font, sf::RenderTexture* p) :
    window(w),
    inFont(font),
    pic(p)
{
    Init();
}


void Runner::Init() {
    numIterations = 0;

    pic->clear(sf::Color::White);
    graphs.setPosition(0, HEIGHT_OFFSET);
    graphs.setTexture(pic->getTexture());

    grid = Grid(window, sf::Vector2i(0, HEIGHT_OFFSET), (sf::Vector2i)window->getSize(),
                Vector2ld(-2.05, .75), Vector2ld(-1.15, 1.15));

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
    iterations = InputBox(window, inFont, 5, 5, 350, 15, "Number of Iterations"); // 1

    okIterations = Button(window, inFont, iterations.GetPosition().x + iterations.GetSize().x + 210, iterations.GetPosition().y,
                        108, 15, "Save Changes"); // 0
    elements.push_back(&okIterations);

    elements.push_back(&iterations); // Finally push back iterations so that it's at the end of the vector

    /** End GUI Creations **/

    for(int iii = 0; iii < elements.size(); iii++) {
        elements[iii]->SetActive(false);
    }
    activeBox = 1; // Start out highlighting the input box
    elements[activeBox]->SetActive(true);
}

void Runner::HandleEvents() {
    sf::Event event;
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
            }
        } else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
            StepActiveElement(!(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                                sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)));
        } else if(event.type == sf::Event::MouseButtonPressed) {
            if(event.mouseButton.y < HEIGHT_OFFSET) { // Above the graphs
                ActivateButtons(event);
            } else { // In one of the graphs
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
                    UpdateGraph(Vector2ld(TLx, TLy), Vector2ld(BRx, BRy));
                    delete firstCorner; // Get rid of the first corner's location since it is no longer relevant (second corner will just go out of scope)
                    firstCorner = NULL; // Cease looking at unallocated memory
                }
            }
        }
    }
}

int Runner::Iterate(cx pos, cx startPos) {
    if(abs(pos) > 2)    // If we're starting outside our circle of radius 2, we're already done
        return 0;       // Took 0 iterations to get ourside the circle, so return 0

    long double a = startPos.real(), b = startPos.imag(), c = pos.real(), d = pos.imag(), k1 = 0, k2 = 0;
    for(int iii = 0; iii < numIterations; iii++) {
        //startPos = startPos * startPos + pos;/*
        k1 = a*a;
        k2 = b*b;

        b = (a+b) * (a+b)-k1-k2 + d; // b uses a, but a doesn't use b, so calculate b first
        a = k1 - k2 + c;
        if(a*a + b*b > 4)    // If we've gone outside our circle of radius 2, we're done
            return iii + 1; // Return the number of iterations it took to get outside the circle
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
    UpdateGraph(grid.GetGraphTopLeft(), grid.GetGraphBotRight());
}

void Runner::ActivateButtons(sf::Event event) {
    switch(activeBox) {
    case 0: // Save Changes for iteration number
        UpdateIterations();
        break;
    default:
        if(event.type == sf::Event::MouseButtonPressed)
            elements[activeBox]->OnClick(event.mouseButton.x, event.mouseButton.y);
        else
            elements[activeBox]->OnEnter();
    }
}

void Runner::UpdateGraph(Vector2ld topLeft, Vector2ld botRight) {
    std::clock_t start;
    start = std::clock();

    if(topLeft != grid.GetGraphTopLeft() || botRight != grid.GetGraphBotRight()) // If either of the corners is different (i.e. we're zooming)
        ClearPic(); // Changing the graph entirely, so just clear it first
    grid.SetRangeCorners(topLeft, botRight);
    int winSizeX = window->getSize().x,
        winSizeY = window->getSize().y - HEIGHT_OFFSET;
    long double pixelDeltaX = (botRight.x - topLeft.x) / winSizeX, // Distance on the graph between the pixels on the window
                pixelDeltaY = (topLeft.y - botRight.y) / winSizeY;
    Vector2ld graphCoords = topLeft;
    for(int iii = 0; iii < winSizeY; iii++) {         // Iterate vertically
        for(int jjj = 0; jjj < winSizeX; jjj++) {     // Iterate horizontally
            sf::Vertex loc(sf::Vector2f(jjj, iii),
                           Colorgen(Iterate(cx(graphCoords.x, graphCoords.y))));
            pic->draw(&loc, 1, sf::Points);
            graphCoords.x = graphCoords.x + pixelDeltaX; // Move one pixel to the right
        }
        graphCoords.x = topLeft.x;                      // Reset x coordinate
        graphCoords.y = graphCoords.y - pixelDeltaY;    // Move one pixel down
        window->draw(graphs); // Draw the updated graph to the screen after each horizontal line of pixels
        pic->display();       // Update our graph with the newest points
        window->display();
    }
    double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    std::cout << duration << "\n";
}

void Runner::ClearPic() {
    pic->clear(sf::Color::White);   // Clear the canvas (pic) to be fully white
}

inline sf::Color Runner::Colorgen(int seed) {
    if(seed > numIterations) // Didn't go out from the circle, so it's in the set as far as we know
        return sf::Color::Black;
    return HSVtoRGBOp((seed * 7) % 360); // Loop the colors
}

void Runner::Draw() {
    window->clear(sf::Color::White); // Clear in preparation for drawing new stuff

    /// Draw GUI elements
    for(int iii = 0; iii < elements.size(); iii++) { // Draw each GUI element (textboxes, buttons, checkboxes)
        elements[iii]->Draw();
    }

    /// Draw graph elements
    window->draw(graphs); // Draw the updated graph to the screen

    if(firstCorner != NULL) // Draw the box if we're tracking the first corner
        window->draw(box);

    pic->display(); // Update our graph with the newest points

    window->display(); // Display everything we've drawn on the screen
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
