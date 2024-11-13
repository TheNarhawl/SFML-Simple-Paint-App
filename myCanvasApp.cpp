#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "My Canvas App", sf::Style::Titlebar | sf::Style::Close);


    sf::RectangleShape myCanvas(sf::Vector2f(800, 620));
    myCanvas.setFillColor(sf::Color::White);
    myCanvas.setPosition(sf::Vector2f(430, 50));
    myCanvas.setOutlineColor(sf::Color(50, 50, 50));
    myCanvas.setOutlineThickness(3);

    std::vector<sf::CircleShape> points;

    sf::Image cursorImage;
    if (!cursorImage.loadFromFile("C:/cpp projects/myCanvasApp/Resources/cursor.png")) {
        return -1;
    }

    sf::Cursor cursor;
    cursor.loadFromPixels(cursorImage.getPixelsPtr(), cursorImage.getSize(), sf::Vector2u(0, 0));

    window.setMouseCursor(cursor);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

        }




        window.clear(sf::Color(200, 200, 200));
        window.draw(myCanvas);

        

        window.display();
    }

    return 0;
}