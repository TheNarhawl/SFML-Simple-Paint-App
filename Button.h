#pragma once

#include <SFML/Graphics.hpp>

	extern const sf::Color normalColor;
	extern const sf::Color pressedColor;

class Button
{
public:
	// Button constructor
	Button(int SizeX, int SizeY, int posX, int posY, const std::string& text, const std::string& iconPath, sf::Font& font);

	void draw(sf::RenderWindow& window);

	bool isPressed(const sf::Vector2i& mousePos) const;

	sf::Color getColor();

	void setButtonColor(std::string colorName);


private:
	sf::RectangleShape buttonShape;
	sf::Text buttonText;
	sf::Sprite iconSprite;
	sf::Texture iconTexture;
	bool isButtonPressed;
};

