#include "Grid.h"
#include <string>
#include <cmath>

Grid::Grid() {

}

Grid::Grid(sf::RenderWindow* w, sf::Vector2i topLeft, sf::Vector2i botRight,
           Vector2ld xRange, Vector2ld yRange) :
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

void Grid::SetRange(Vector2ld xRange, Vector2ld yRange) {
    m_xRange = xRange;
    m_yRange = yRange;
    m_center = Vector2ld((m_xRange.x + m_xRange.y) / 2,
                            (m_yRange.x + m_yRange.y) / 2);
}

void Grid::SetRangeCorners(Vector2ld topLeft, Vector2ld botRight) {
    SetRange(Vector2ld(topLeft.x, botRight.x),
             Vector2ld(botRight.y, topLeft.y));
}

Vector2ld Grid::WindowToGraph(Vector2ld wLoc) {
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
    return Vector2ld(x, y);
}

Vector2ld Grid::WindowToGraph(double xPos, double yPos) {
    return WindowToGraph(Vector2ld(xPos, yPos));
}

Vector2ld Grid::GraphToWindow(Vector2ld gLoc) {
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
    return Vector2ld(x, y);
}

Vector2ld Grid::GraphToWindow(double xPos, double yPos) {
    return GraphToWindow(Vector2ld(xPos, yPos));
}

Vector2ld Grid::GraphToPic(Vector2ld gLoc) {
    /*Vector2ld coords = GraphToWindow(gLoc);
    coords -= Vector2ld(0, m_origWin.y); // subtract original window size
    coords.y *= -1;
    return coords;*/
    return GraphToWindow(gLoc) - Vector2ld(0, m_position.y);
}

Vector2ld Grid::GraphToPic(double xPos, double yPos) {
    return GraphToPic(Vector2ld(xPos, yPos));
}

Vector2ld Grid::GetGraphTopLeft() {
    return Vector2ld(m_xRange.x, m_yRange.y);
}

Vector2ld Grid::GetGraphBotRight() {
    return Vector2ld(m_xRange.y, m_yRange.x);
}
