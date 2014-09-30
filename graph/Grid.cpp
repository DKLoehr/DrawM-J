#include "Grid.h"
#include <string>
#include <cmath>

Grid::Grid() {

}

Grid::Grid(sf::RenderWindow* w, sf::Vector2i topLeft, sf::Vector2i botRight,
           sf::Vector2f xRange, sf::Vector2f yRange) :
           m_w(w)
{
    m_origWin = m_w->getSize();

    m_position = topLeft;
    m_size = botRight - topLeft;

    SetRange(xRange, yRange);
}

void Grid::SetPosition(sf::Vector2i pos) {
    m_position = pos;
}

void Grid::SetPosition(int xPos, int yPos) {
    m_position = sf::Vector2i(xPos, yPos);
}

void Grid::SetRange(sf::Vector2f xRange, sf::Vector2f yRange) {
    m_xRange = xRange;
    m_yRange = yRange;
    m_center = sf::Vector2f((m_xRange.x + m_xRange.y) / 2,
                            (m_yRange.x + m_yRange.y) / 2);
}

void Grid::SetRangeCorners(sf::Vector2f topLeft, sf::Vector2f botRight) {
    SetRange(sf::Vector2f(topLeft.x, botRight.x),
             sf::Vector2f(botRight.y, topLeft.y));
}

sf::Vector2f Grid::WindowToGraph(sf::Vector2f wLoc) {
    double x = wLoc.x, y = wLoc.y;
    if(x < m_position.x) x = m_position.x;
    if(x > m_position.x + m_size.x) x = m_position.x + m_size.x;
    if(y < m_position.y) y = m_position.y;
    if(y > m_position.y + m_size.y) x = m_position.y + m_size.y;
    x -= (m_position.x + m_size.x / 2);
    y = -(y - (m_position.y + m_size.y / 2));
    x /= m_size.x / 2 / (m_center.x - m_xRange.x);
    y /= m_size.y / 2 / (m_center.y - m_yRange.x);
    x += m_center.x;
    y += m_center.y;
    return sf::Vector2f(x, y);
}

sf::Vector2f Grid::WindowToGraph(double xPos, double yPos) {
    return WindowToGraph(sf::Vector2f(xPos, yPos));
}

sf::Vector2f Grid::GraphToWindow(sf::Vector2f gLoc) {
    double x = gLoc.x - m_center.x;
    double y = gLoc.y - m_center.y;
    x *= m_size.x / 2 / (m_center.x - m_xRange.x);
    y *= m_size.y / 2 / (m_center.y - m_yRange.x);
    x += m_position.x + m_size.x / 2;
    y += m_position.y + m_size.y / 2;
    y += 2 * (m_size.y / 2 + m_position.y - y);
    if(x < m_position.x) x = m_position.x;
    if(x > m_position.x + m_size.x) x = m_position.x + m_size.x;
    if(y < m_position.y) y = m_position.y;
    if(y > m_position.y + m_size.y) x = m_position.y + m_size.y;
    return sf::Vector2f(x, y);
}

sf::Vector2f Grid::GraphToWindow(double xPos, double yPos) {
    return GraphToWindow(sf::Vector2f(xPos, yPos));
}

sf::Vector2f Grid::GraphToPic(sf::Vector2f gLoc) {
    /*sf::Vector2f coords = GraphToWindow(gLoc);
    coords -= sf::Vector2f(0, m_origWin.y); // subtract original window size
    coords.y *= -1;
    return coords;*/
    return GraphToWindow(gLoc) - sf::Vector2f(0, m_position.y);
}

sf::Vector2f Grid::GraphToPic(double xPos, double yPos) {
    return GraphToPic(sf::Vector2f(xPos, yPos));
}

sf::Vector2f Grid::GetGraphTopLeft() {
    return sf::Vector2f(m_xRange.x, m_yRange.y);
}

sf::Vector2f Grid::GetGraphBotRight() {
    return sf::Vector2f(m_xRange.y, m_yRange.x);
}
