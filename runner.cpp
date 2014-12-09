#include "runner.h"
#include <time.h>

Runner::Runner(sf::RenderWindow* w, sf::Font* font) :
    window(w),
    inFont(font)
{
    Init();
}

Runner::~Runner() {}


void Runner::Init() {
    numIterations = 0;
    prevNumIterations = 0;
    colorMult = 7;

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

    windows.push_back(MWindow(inFont, sf::Vector2i(0, 100), sf::Vector2i(100, 200), Vector2ld(-2.05, 1.15), Vector2ld(.75, -1.15),
                              &numIterations, &prevNumIterations, &colorMult));
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
                  event.type == sf::Event::MouseButtonPressed && event.mouseButton.y) {
            ActivateButtons(event);
        } else if(event.type == sf::Event::MouseMoved) {
            SetActiveElement(event.mouseMove.x, event.mouseMove.y);
        } else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
            StepActiveElement(!(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                                sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)));
        }
    }
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

void Runner::Draw() {
    window->clear(sf::Color::White); // Clear in preparation for drawing new stuff

    /// Draw GUI elements
    for(int iii = 0; iii < elements.size(); iii++) { // Draw each GUI element (textboxes, buttons, checkboxes)
        elements[iii]->Draw();
    }

    /// Draw graph elements
    window->draw(graphs); // Draw the updated graph to the screen
    window->display(); // Display everything we've drawn on the screen
}
