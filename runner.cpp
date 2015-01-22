#include "runner.h"
#include <time.h>

Runner::Runner(sf::RenderWindow* w, sf::Font* font) :
    window(w),
    inFont(font)
{
    Init();
}

Runner::~Runner() {
    for(int iii = windows.size() - 1; iii >= 0; iii--) {
        delete windows[iii];
        windows.pop_back();
    }
}


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

    windows = std::vector<MWindow*>(1);
    windows[0] = new MWindow(inFont, sf::Vector2i(0, 89), sf::Vector2u(300, 246), Vector2ld(-2.05, 1.15), Vector2ld(.75, -1.0015),
                              &numIterations, &colorMult);
    activeWindow = 0;

    window->setPosition(sf::Vector2i(0, 0)); // Start out highlighting the main window
}

void Runner::HandleEvents() {
    sf::Event event;
    Vector2ld *newTopLeft = NULL, *newBotRight = NULL;
    for(int iii = 0; iii < windows.size(); iii++) {     // Basic events for all MWindows
        while(windows[iii]->PollEvent(event, &newTopLeft, &newBotRight, (iii == activeWindow))) {
            if(newTopLeft != NULL) {
                windows.push_back(new MWindow(inFont, sf::Vector2i(300, 300), sf::Vector2u(300, 246), *newTopLeft, *newBotRight,
                                                   &numIterations, &colorMult));
                activeWindow = windows.size() - 1;
                windows[activeWindow]->UpdateGraph();
                std::printf("(%f, %f), (%f, %f) \n", (double)(newTopLeft->x), (double)(newTopLeft->y), (double)(newBotRight->x), (double)(newBotRight->y));
                delete(newTopLeft);
                delete(newBotRight);
                newTopLeft = NULL;
                newBotRight = NULL;
            }
            if(event.type == sf::Event::GainedFocus) {  // This is the new active window
                activeWindow = iii;
            } else if(event.type == sf::Event::TextEntered) {
                elements[activeBox]->OnTextEntered(event.text.unicode);
            } else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
                StepActiveElement(!(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                                    sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)));
            } else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) {
                ActivateButtons(event);
            } else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape &&
                      sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                window->close(); // Enda the program
            }
        }
    }
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
    UpdateGraph();
}

void Runner::UpdateColor() {
    colorMult = (float)atof(colorNum.GetText().c_str()); // Set the color multiplier to the number specified in the box
    UpdateGraph();
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

void Runner::UpdateGraph() {
    windows[activeWindow]->UpdateGraph();
}

void Runner::Draw() {
    //while(window->isOpen()) {
        window->clear(sf::Color::White); // Clear in preparation for drawing new stuff

        /// Draw GUI elements
        for(int iii = 0; iii < elements.size(); iii++) { // Draw each GUI element (textboxes, buttons, checkboxes)
            elements[iii]->Draw();
        }

        window->display(); // Display everything we've drawn on the screen

        for(int iii = 0; iii < windows.size(); iii++) {
            windows[iii]->Draw();
        }
    //}
}

// Converts a string with a number in it to an integer containing that number
inline int Runner::ToInt(std::string str) {
    int ret = 0;
    for(int i = str.length() - 1; i >= 0; i--) {
        ret += pow(10, str.length() - 1 - i) * (str[i] - '0');
    }
    return ret;
}
