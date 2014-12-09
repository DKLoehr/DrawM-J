#ifndef GRID_H
#define GRID_H

#include <SFML/Graphics.hpp>

typedef sf::Vector2<long double> Vector2ld;

/**
* This class represents the graph onto which you might draw functions.
* It consists of an x (horizontal) and y (vertical) axis, with either evenly spaced tick marks or full lines,
* as well as labels on the axes to indicate scale.
**/

class Grid
{
private:
    /// The window to which we draw
    sf::RenderWindow* m_w;
    /// The font we use for the axis labels
    sf::Font m_f;

    /// The original size of the window
    sf::Vector2u m_origWin;


    /** The following are all relative to the WINDOW, not the graph **/
    /// The coordinates of the top-left corner of the grid on the window
    sf::Vector2i m_position;
    /// Height and width of the grid
    sf::Vector2i m_size;


    /** The following are all relative to the GRAPH, not the window **/
    /// The coordinates of the center of the grid (usually (0,0))
    Vector2ld m_center;
    /// How far from the center we go in the x and y directions
    Vector2ld m_xRange;
    Vector2ld m_yRange;

public:
    /// Default constructor: Will not create a usable grid!
    Grid();

    Grid(sf::RenderWindow* w, sf::Vector2i topLeft, sf::Vector2i botRight,
         Vector2ld xRange, Vector2ld yRange);

    /// Move the grid to a new position on the window
    void SetPosition(sf::Vector2i pos);
    void SetPosition(int xPos, int yPos);


    /** Functions for modifying the window and changing settings -- Primarily for the user **/

    /// Set the range shown in the window for x and y;
    void SetRange(Vector2ld xRange, Vector2ld yRange);
    /// Set the range by giving the graph locations of the top left and bottom right corners
    void SetRangeCorners(Vector2ld topLeft, Vector2ld botRight);


    /** Functions for converting from graph coordinates to window coordinates and vice-versa **/

    /// Given the location on the window, returns the coordinates of the point on the graph
    Vector2ld WindowToGraph(Vector2ld wLoc) const;
    Vector2ld WindowToGraph(double xPos, double yPos) const;
    /// Given the location on the graph, returns the coordinates of the pixel on the window
    Vector2ld GraphToWindow(Vector2ld gLoc) const;
    Vector2ld GraphToWindow(double xPos, double yPos) const;
    /// Given the location on the graph, returns the coordinates of the pixel on the canvas
    Vector2ld GraphToPic(Vector2ld gLoc) const;
    Vector2ld GraphToPic(double xPos, double yPos) const;


    /** Functions for getting information from the grid **/
    /// Returns the graph locations of the top right and bottom left corners of the graph, respectively
    Vector2ld GetGraphTopLeft() const;
    Vector2ld GetGraphBotRight() const;
    /// As above, but returns a pointer to the Vector instead
    Vector2ld* GetGraphTopLeftP() const;
    Vector2ld* GetGraphBotRightP() const;
};

#endif // GRID_H
