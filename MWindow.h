#ifndef MWINDOW_H
#define MWINDOW_H

#include <SFML/Graphics.hpp>
#include "guts/Parser.h"
#include "gui/button.h"
#include "gui/text.h"
#include "gui/checkbox.h"
#include "graph/Grid.h"


class MWindow
{
private:
    sf::RenderWindow window;
    sf::Font* inFont;
    sf::RenderTexture pic;
    sf::Sprite graphs;

    Grid grid;
    Vector2ld* firstCorner;
    sf::VertexArray zoomBox;

    sf::Thread* iterThread;      // Thread for the iteration

    bool interrupted;

    unsigned int* numIterations;
    unsigned int prevNumIterations; // The last number of iterations we did
    float* colorMult;

    uint16_t** numIters;        // Stores per-pixel the number of iterations taken for that pixel to leave the set (or when we stopped)

    unsigned int Iterate(cx* pos, cx* startPos = NULL);  // Iterates startPos^2 + pos; returns the number of iterations at which it terminated.
                                                         // Warning: calls delete on startPos and pos, so pass in a copy of anything you want to keep.

    void IterateGraph(); // Iterates across the entire graph, updating each point

    sf::Color Colorgen(unsigned int seed);
    sf::Color HSVtoRGBOp(int hue);

public:
    MWindow(); // Default constructor: Will not create a useable window

    MWindow(sf::Font* f, sf::Vector2i wTopLeft, sf::Vector2u wSize, Vector2ld gTopLeft, Vector2ld gBotRight,
            unsigned int* numIt, float* cMult);

    MWindow(const MWindow& target); // Move Constructor

    virtual ~MWindow();

    void Create(sf::Font* f, sf::Vector2i wTopLeft, sf::Vector2u wSize, Vector2ld gTopLeft, Vector2ld gBotRight,
                unsigned int* numIt, float* cMult);

    void UpdateGraph();

    int PollEvent(sf::Event& event, Vector2ld** topLeft = NULL, Vector2ld** botRight = NULL, bool checkBox = false);
    void SetActive(bool isActive);

    void Draw();
};

#endif // MWINDOW_H
