#include <SFML/Graphics.hpp>
#include "Button.h"
    // Button colors

    extern const sf::Color normalColor(200, 200, 200);
    extern const sf::Color pressedColor(150, 150, 150);
    

Button::Button(int SizeX, int SizeY, int posX, int posY, 
    const std::string& text, const std::string& iconPath, sf::Font& font)
{

    // Loading Button`s icon
    if (!iconTexture.loadFromFile(iconPath)) {
        throw std::runtime_error("Unable to load icon");
    }

    iconSprite.setTexture(iconTexture);


    // Button`s inner text controls
    buttonText.setString(text);
    buttonText.setFont(font);
    buttonText.setCharacterSize(20);
    buttonText.setFillColor(sf::Color::Black);
    buttonText.setPosition(posX + 15, posY + 6);


    // Button`s shape controls
    buttonShape.setSize(sf::Vector2f(SizeX, SizeY));
    buttonShape.setFillColor(normalColor);
    buttonShape.setPosition(posX, posY);

    buttonShape.setOutlineColor(sf::Color::Black);
    buttonShape.setOutlineThickness(2);


    isButtonPressed = false;

    // Button`s icon pos
    iconSprite.setPosition(posX + 100, posY + 5);
}

// Button render method
void Button::draw(sf::RenderWindow& window) {
    window.draw(buttonShape);
    window.draw(buttonText);
    window.draw(iconSprite);
}

// Method checks if the Button was pressed
bool Button::isPressed(const sf::Vector2i& mousePos) const {
    return buttonShape.getGlobalBounds().contains(sf::Vector2f(mousePos));
}


// Method returns you the color of Button
sf::Color Button::getColor() {
    return buttonShape.getFillColor();
}


// Method takes a color name and sets a button color
void Button::setButtonColor(std::string colorName) {
    if (colorName == "normalColor") 
    {
        buttonShape.setFillColor(normalColor);
        isButtonPressed = false;
    }
    else if (colorName == "pressedColor") 
    {
        buttonShape.setFillColor(pressedColor);
        isButtonPressed = false;
    }
};



