#ifndef RUNNER_H
#define RUNNER_H

#include <SFML/Graphics.hpp>
#include "guts/Parser.h"
#include "gui/button.h"
#include "gui/text.h"
#include "gui/checkbox.h"
#include "graph/Grid.h"

#define HEIGHT_OFFSET 40        // The number of pixels between the top of the window and the top of the graphs

class Runner
{
private:
    int activeBox;
    int numIterations;

    sf::RenderWindow* window;   // The window to which we draw
    sf::Font* inFont;           // The font to use for all text (VeraMono)
    sf::RenderTexture* pic;     // The canvas to which we draw graphed points
    sf::Sprite graphs;          // The sprite which we use to draw pic to the screen

    parser::Tree* fct;          // The tree we use to evaluate our expression

    Grid grid;                  // An invisible set of axes used to keep track of our current graph location

    sf::Vector2f* firstCorner;

    std::vector<GUI*> elements;  // An alternate way of accessing each gui element
    const int elementsSize = 2; // Initial size of elements (without any variables)

    Button okIterations;  // 0
    InputBox iterations;  // 1

    void Init(); // Initialize the class

    int Iterate(cx pos); // Iterates the given point; returns the number of iterations at which

    void SetActiveElement(double x, double y);  // Determines what activeBox should be based on the mouse coordinates; x and y are window coords to test
    void StepActiveElement(bool increment);     // Increases activeBox by 1 if true, decreases if false, keeping it in valid bounds
    void UpdateIterations();                    // Update numIterations according to what's in the box
    void UpdateGraph(sf::Vector2f topLeft, sf::Vector2f botRight); // Update the graph given the positions of the top left and bottom right corner
    void ActivateButtons(sf::Event event);      // Activate buttons depending on activeBox and/or the event; event is the event that activated a button

    void ClearPic(); // Clear all points drawn to pic

    sf::Color Colorgen(int seed); // Returns a color based on the input; expected input is the result of of Iterate

    /// Helpful functions
    int ToInt(std::string str);
    sf::Color HSVtoRGB(int hue, double sat, double val); // Convert HSV color values to RGB color values
    sf::Color HSVtoRGBOp(int hue); // Optimized version of HSVto RGB for our purposes

public:
    Runner(sf::RenderWindow* w, sf::Font* font, sf::RenderTexture* p); // Constructor

    void HandleEvents();    // Perform all the tasks necessary to run

    void Draw();            // Draw everything to the screen
};

#endif // RUNNER_H
