#ifndef RUNNER_H
#define RUNNER_H

#include <SFML/Graphics.hpp>
#include "guts/Parser.h"
#include "gui/button.h"
#include "gui/text.h"
#include "gui/checkbox.h"
#include "graph/Grid.h"
#include "MWindow.h"

#define HEIGHT_OFFSET 40        // The number of pixels between the top of the window and the top of the graphs
#define WIN_SIZE_X 1000
#define WIN_SIZE_Y 50

class Runner
{
private:
    int activeBox;
    int activeWindow;
    unsigned int numIterations;
    unsigned int prevNumIterations; // The last number of iterations we did
    float colorMult;

    sf::RenderWindow* window;   // The window to which we draw the M-set
    sf::Font* inFont;           // The font to use for all text (VeraMono)

    std::vector<MWindow> windows; // Vector to hold all the mini windows that are generated

    std::vector<GUI*> elements; // An alternate way of accessing each gui element
    const int elementsSize = 4; // Initial size of elements (without any variables)

    InputBox iterations;    // 0
    Button okIterations;    // 1
    InputBox colorNum;      // 2
    Button okColor;         // 3

    void Init(); // Initialize the class

    void SetActiveElement(double x, double y);  // Determines what activeBox should be based on the mouse coordinates; x and y are window coords to test
    void StepActiveElement(bool increment);     // Increases activeBox by 1 if true, decreases if false, keeping it in valid bounds
    void UpdateIterations();                    // Update numIterations according to what's in the box
    void UpdateColor();                         // Update the number by which we multiply the iterations for color
    void UpdateGraph();                         // Update the graph given the positions of the top left and bottom right corner

    void ActivateButtons(sf::Event event);      // Activate buttons depending on activeBox and/or the event; event is the event that activated a button

    int ToInt(std::string str);
public:
    Runner(sf::RenderWindow* w, sf::Font* font); // Constructor
    ~Runner();

    void HandleEvents();    // Perform all the tasks necessary to run

    void Draw();            // Draw everything to the screen
};

#endif // RUNNER_H
