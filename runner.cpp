#include "runner.h"

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
                sf::Vector2f(-2, 1), sf::Vector2f(-1.5, 1.5));

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
            else {
                ///In graphs, do something
            }
        } else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
            StepActiveElement(!(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                                sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)));
        } else if(event.type == sf::Event::MouseButtonPressed) {
            if(event.mouseButton.y < HEIGHT_OFFSET) { // Above the graphs
                ActivateButtons(event);
            } else { // In one of the graphs
                /** Eventually zoom in on place in between clicking locations **/
            }
        }
    }
}

int Runner::Iterate(cx pos) {
    if(abs(pos) > 2)    // If we're starting outside our circle of radius 2, we're already done
        return 0;       // Took 0 iterations to get ourside the circle, so return 0

    fct->setVar("C", pos); // Feed our current location into parser as the variable C (what we're subtracting from Z^2)
    pos = cx(0,0); // We've stored pos's value in the tree, so now we'll repurpose it to be what we're iterating

    for(int iii = 0; iii < numIterations; iii++) {
        fct->setVar("Z", pos); // Feed our current location into parser as the variable Z
        try {
            pos = fct->eval(); // Don't change position, so we can make it black first
        }
        catch (std::invalid_argument) { // Should mean we've "reached infinity", so we can stop
            return iii;
        }
        if(abs(pos) > 2)    // If we've gone outside our circle of radius 2, we're done
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

void Runner::UpdateGraph(sf::Vector2f topLeft, sf::Vector2f botRight) {
    grid.SetRangeCorners(topLeft, botRight);
    for(double iii = topLeft.x; iii < botRight.x; iii += ITERATION_DELTA) {        // Iterate horizontally, left to right
        for(double jjj = topLeft.y; jjj > botRight.y; jjj -= ITERATION_DELTA) {    // Iterate vertically, top-to-bottom
            sf::CircleShape loc = sf::CircleShape(1);
            loc.setPosition(grid.GraphToPic(iii,jjj));
            loc.setFillColor(Colorgen(Iterate(cx(iii, jjj))));
            pic->draw(loc);
            window->draw(graphs); // Draw the updated graph to the screen
            pic->display(); // Update our graph with the newest points
            window->display();
        }
    }
}

void Runner::ClearPic() {
    pic->clear(sf::Color::White);   // Clear the canvas (pic) to be fully white
}

sf::Color Runner::Colorgen(int seed) {
    if(seed > numIterations) // Didn't go out from the circle, so it's in the set as far as we know
        return sf::Color::Black;
    return HSVtoRGB((seed * 10) % 255, 1, 1); // Loop the colors
}

void Runner::Draw() {
    window->clear(sf::Color::White); // Clear in preparation for drawing new stuff

    /// Draw GUI elements
    for(int iii = 0; iii < elements.size(); iii++) { // Draw each GUI element (textboxes, buttons, checkboxes)
        elements[iii]->Draw();
    }

    /// Draw graph elements
    window->draw(graphs); // Draw the updated graph to the screen

    pic->display(); // Update our graph with the newest points

    window->display(); // Display everything we've drawn on the screen
}

// Converts a string with a number in it to an integer containing that number
int Runner::ToInt(std::string str) {
    int ret = 0;
    for(int i = str.length() - 1; i >= 0; i--) {
        ret += pow(10, str.length() - 1 - i) * (str[i] - '0');
    }
    return ret;
}

// Convert HSV color values to RGB color values
sf::Color Runner::HSVtoRGB(int hue, double sat, double val) {
    hue %= 360;
    while(hue < 0) hue += 360;

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
