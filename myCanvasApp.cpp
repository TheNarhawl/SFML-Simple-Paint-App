#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

enum class ToolMode { Brush, Eraser };

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Draw and Erase");

    // Канвас для рисования
    sf::RectangleShape myCanvas(sf::Vector2f(800, 620));
    myCanvas.setFillColor(sf::Color::White);
    myCanvas.setPosition(sf::Vector2f(430, 50));
    myCanvas.setOutlineColor(sf::Color(50, 50, 50));
    myCanvas.setOutlineThickness(3);

    // Вектор для хранения всех линий (каждая линия — вектор сегментов)
    std::vector<std::vector<sf::RectangleShape>> allLines;
    std::vector<sf::RectangleShape> currentLineSegments;

    // Переменная для хранения предыдущей позиции мыши
    sf::Vector2f lastMousePos;
    bool isDrawing = false;

    // Инструмент по умолчанию - кисть
    ToolMode currentTool = ToolMode::Brush;

    // Размер кисти
    float brushSize = 5.0f;

    // Ползунок для выбора толщины
    sf::RectangleShape slider(sf::Vector2f(200, 10));
    slider.setFillColor(sf::Color::Black);
    slider.setPosition(50, 650);

    // Индикатор положения на ползунке
    sf::CircleShape sliderIndicator(10);
    sliderIndicator.setFillColor(sf::Color::Red);
    sliderIndicator.setPosition(slider.getPosition().x + (brushSize - 1) * 20, slider.getPosition().y - 5);

    // Текст для отображения текущей толщины
    sf::Font font;
    font.loadFromFile("arial.ttf"); // Путь к шрифту
    sf::Text brushSizeText;
    brushSizeText.setFont(font);
    brushSizeText.setCharacterSize(20);
    brushSizeText.setFillColor(sf::Color::Black);
    brushSizeText.setPosition(50, 620);

    // Кнопка "Кисть"
    sf::RectangleShape brushButton(sf::Vector2f(100, 40));
    brushButton.setFillColor(sf::Color(200, 200, 200));
    brushButton.setPosition(50, 550);
    sf::Text brushText("Brush", font, 20);
    brushText.setFillColor(sf::Color::Black);
    brushText.setPosition(brushButton.getPosition().x + 10, brushButton.getPosition().y + 5);

    // Кнопка "Ластик"
    sf::RectangleShape eraserButton(sf::Vector2f(100, 40));
    eraserButton.setFillColor(sf::Color(200, 200, 200));
    eraserButton.setPosition(160, 550);
    sf::Text eraserText("Eraser", font, 20);
    eraserText.setFillColor(sf::Color::Black);
    eraserText.setPosition(eraserButton.getPosition().x + 10, eraserButton.getPosition().y + 5);

    // Кнопка "Сохранить"
    sf::Text saveText("Save", font, 20);
    saveText.setFillColor(sf::Color::Black);
    saveText.setPosition(50, 600);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // Проверка нажатия мыши на кнопки
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                // Обработка нажатия на кнопку "Сохранить"
                if (saveText.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    // Создаем RenderTexture точно под размер канваса
                    sf::RenderTexture renderTexture;
                    renderTexture.create(myCanvas.getSize().x, myCanvas.getSize().y);
                    renderTexture.clear(sf::Color::White);

                    // Сдвигаем все элементы так, чтобы они соответствовали началу RenderTexture
                    for (const auto& line : allLines)
                    {
                        for (auto segment : line)
                        {
                            segment.move(-myCanvas.getPosition());  // Смещаем элементы к началу RenderTexture
                            renderTexture.draw(segment);
                        }
                    }

                    // Сохраняем RenderTexture в изображение
                    renderTexture.display();
                    sf::Image image = renderTexture.getTexture().copyToImage();
                    image.saveToFile("drawing.png");
                }

                // Обработка выбора кисти
                if (brushButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    currentTool = ToolMode::Brush;
                    brushButton.setFillColor(sf::Color(100, 100, 100));
                    eraserButton.setFillColor(sf::Color(200, 200, 200));
                }
                // Обработка выбора ластика
                else if (eraserButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    currentTool = ToolMode::Eraser;
                    eraserButton.setFillColor(sf::Color(100, 100, 100));
                    brushButton.setFillColor(sf::Color(200, 200, 200));
                }
                // Проверка нажатия на ползунок
                else if (slider.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    float newPos = mousePos.x;
                    if (newPos < slider.getPosition().x) newPos = slider.getPosition().x;
                    if (newPos > slider.getPosition().x + slider.getSize().x) newPos = slider.getPosition().x + slider.getSize().x;

                    sliderIndicator.setPosition(newPos - sliderIndicator.getRadius(), sliderIndicator.getPosition().y);

                    brushSize = 1 + (newPos - slider.getPosition().x) / 20;  // диапазон от 1 до 10
                }
                else if (myCanvas.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    currentLineSegments.clear();
                    isDrawing = true;
                    lastMousePos = window.mapPixelToCoords(mousePos);
                }
            }
        }

        brushSizeText.setString("Brush size: " + std::to_string(static_cast<int>(brushSize)));

        if (isDrawing && sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            if (myCanvas.getGlobalBounds().contains(mousePos))
            {
                if (mousePos != lastMousePos)
                {
                    sf::RectangleShape lineSegment;
                    lineSegment.setFillColor(currentTool == ToolMode::Brush ? sf::Color::Black : sf::Color::White);

                    sf::Vector2f delta = mousePos - lastMousePos;
                    float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                    lineSegment.setSize(sf::Vector2f(length, brushSize));
                    lineSegment.setOrigin(0, brushSize / 2);
                    lineSegment.setRotation(atan2(delta.y, delta.x) * 180 / 3.14159f);
                    lineSegment.setPosition(lastMousePos);

                    currentLineSegments.push_back(lineSegment);
                    lastMousePos = mousePos;
                }
            }
        }
        else if (isDrawing && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            if (!currentLineSegments.empty())
            {
                allLines.push_back(currentLineSegments);
            }
            isDrawing = false;
        }

        window.clear(sf::Color::White);
        window.draw(myCanvas);

        for (const auto& line : allLines)
        {
            for (const auto& segment : line)
            {
                window.draw(segment);
            }
        }

        for (const auto& segment : currentLineSegments)
        {
            window.draw(segment);
        }

        window.draw(slider);
        window.draw(sliderIndicator);
        window.draw(brushSizeText);

        // Рисуем кнопки и их текст
        window.draw(brushButton);
        window.draw(brushText);
        window.draw(eraserButton);
        window.draw(eraserText);
        window.draw(saveText);

        window.display();
    }

    return 0;
}