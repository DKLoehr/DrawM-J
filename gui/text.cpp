#include <SFML/Graphics.hpp>
#include <string>
#include "text.h"
#include <iostream>
#include <stdlib.h>

InputBox::InputBox() {

}

InputBox::InputBox(sf::RenderWindow* window, sf::Font* font, int x, int y, int width, int height,
                   std::string cap):
    GUI(window, font, x, y, width, height)
{
    m_cap.setString(cap);

    m_rectangle.setOutlineColor(sf::Color(100,100,100));

    m_cap.setPosition(x, y - 2);
    m_rectangle.setPosition(x + cap.length() * 10, y);
    m_text.setPosition(x + cap.length() * 10, y - 2);
};


bool InputBox::IsValid(char n) {
    if(n == 8 || n == 32 || n == 33 || (n >= 40 && n <= 57) || (n >= 65 && n <= 90) || n == 94 ||
       (n >= 97 && n <= 122) || n == 127)
        return true;
    else
        return false;
}

void InputBox::SetActive(bool active) {
    isActive = active;
    DrawWhite();
    if(isActive) {
        m_rectangle.setFillColor(sf::Color::White);
    } else {
        m_rectangle.setFillColor(sf::Color(150, 150, 150));
    }
    Draw();
}

void InputBox::SetPosition(sf::Vector2f newPos) {
    SetPosition(newPos.x, newPos.y);
}

void InputBox::SetPosition(double x, double y) {
    std::string cap = m_cap.getString();
    m_cap.setPosition(x, y - 2);
    m_rectangle.setPosition(x + cap.length() * 10, y);
    m_text.setPosition(x + cap.length() * 10, y - 2);
}

bool InputBox::OnEnter() {
    return false;
}

bool InputBox::OnClick(double xP, double yP) {
    double xScale = m_w->getSize().x / m_wSize.x, yScale = m_w->getSize().y / m_wSize.y;
    int cap = (((std::string)m_cap.getString()).length() + 1) * 10;

    if((m_position.x - 3) * xScale < xP && xP < (m_position.x + m_size.x + cap + 5) * xScale &&
       (m_position.y - 3) * yScale < yP && yP < (m_position.y + m_size.y + 2) * yScale)
        return true;
    else
        return false;
}

void InputBox::OnTextEntered(char n) {
    if(IsValid(n)) { // n is a valid character
        std::string temp = m_text.getString();
        if(n != 8) { // Some character
            m_text.setString(temp.substr(0, temp.length()) + n);
        } else if(n == 8) { // Backspace
            m_text.setString(temp.substr(0, temp.length() - 1));
        }
    }
}

void InputBox::EnterText(char n) {
    if(IsValid(n)) { // n is a valid character
        std::string temp = m_text.getString();
        if(n != 8) { // Some character
            m_text.setString(temp.substr(0, temp.length()) + n);
        } else if(n == 8) { // Backspace
            m_text.setString(temp.substr(0, temp.length() - 1));
        }
    }
}

void InputBox::Draw() {
    m_w->draw(m_rectangle);
    std::string str = m_text.getString();
    std::string truncStr;
    if(str.length() * 10 > m_size.x) {
        truncStr = str.substr(str.length() - m_size.x / 10, std::string::npos);
        m_text.setString(truncStr);
    }
    m_w->draw(m_text);
    m_text.setString(str);
    m_w->draw(m_cap);
}

double InputBox::GetStringAsDouble() {
    return atof(((std::string)m_text.getString()).c_str());

}

sf::Vector2f InputBox::GetStringAsVector() {
    std::string str = m_text.getString();
    if(str.find('(') != std::string::npos)
        str = str.substr(1, std::string::npos);
    if(str.find(')') != std::string::npos)
        str = str.substr(0, str.length() - 1);
    std::string xStr = str.substr(0, str.find(','));
    std::string yStr = str.substr(str.find(',') +  1, str.length());
    return sf::Vector2f(atof(xStr.c_str()), atof(yStr.c_str()));
}

std::string InputBox::GetOrderedPairElement(bool first) {
    std::string str = m_text.getString();
    if(str.find('(') != std::string::npos)
        str = str.substr(1, std::string::npos);
    if(str.find(')') != std::string::npos)
        str = str.substr(0, str.length() - 1);
    std::string xStr = str.substr(0, str.find(','));
    std::string yStr = str.substr(str.find(',') +  1, str.length());
    if(first)
        return xStr;
    else
        return yStr;
}
