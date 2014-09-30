#include "button.h"
#include <iostream>

Button::Button() {

}

Button::Button(sf::RenderWindow* window, sf::Font* font, int x, int y, int width, int height, std::string str) :
        GUI(window, font, x, y, width, height)
{
    m_text.setString(str);

    m_rectangle.setPosition(x, y);
    m_text.setPosition(x, y - 2);
}

void Button::SetActive(bool active) {
    isActive = active;
    DrawWhite();
    if(isActive) {
        m_rectangle.setFillColor(sf::Color::White);
    } else {
        m_rectangle.setFillColor(sf::Color(150, 150, 150));
    }
    Draw();
}

void Button::SetPosition(sf::Vector2f newPos) {
    SetPosition(newPos.x, newPos.y);
}

void Button::SetPosition(double x, double y) {
    m_rectangle.setPosition(x, y);
    m_text.setPosition(x, y - 2);
}

bool Button::OnEnter() {
    return true;
}

bool Button::OnClick(double xP, double yP) {
    double xScale = m_w->getSize().x / m_wSize.x, yScale = m_w->getSize().y / m_wSize.y;

    if((m_position.x - 3) * xScale < xP && xP < (m_position.x + m_size.x + 3) * xScale &&
       (m_position.y - 3) * yScale < yP && yP < (m_position.y + m_size.y + 2) * yScale) {
       return true;
    } else {
        return false;
    }
}

void Button::OnTextEntered(char n) {
    return;
}
