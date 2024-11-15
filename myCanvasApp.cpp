#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include "Button.h"

enum class ToolMode { Brush, Eraser };

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "My Paint App", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);


    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("Resources/background.png")) {
        return -1; // Выход из программы в случае ошибки
    }
    sf::Sprite backgroundSprite(backgroundTexture);

    // Канвас для рисования
    sf::RectangleShape myCanvas(sf::Vector2f(840, 620));
    myCanvas.setFillColor(sf::Color::White);
    myCanvas.setPosition(sf::Vector2f(390, 50));

    // Загружаем изображение для кастомного курсора
    sf::Image cursorImage;
    if (!cursorImage.loadFromFile("Resources/cursor.png")) {
        return -1;
    }
    sf::Cursor cursor;
    cursor.loadFromPixels(cursorImage.getPixelsPtr(), cursorImage.getSize(), sf::Vector2u(0, 0));
    window.setMouseCursor(cursor);


    // Вектор для хранения всех линий (каждая линия — вектор сегментов)
    std::vector<std::vector<sf::RectangleShape>> allLines;
    std::vector<sf::RectangleShape> currentLineSegments;

    // Переменная для хранения предыдущей позиции мыши
    sf::Vector2f lastMousePos;
    bool isDrawing = false;

    // Инструмент по умолчанию - кисть
    ToolMode currentTool = ToolMode::Brush;




    // Размер кисти
    float brushSize = 20.0f;

    // Ползунок для выбора толщины
    sf::RectangleShape slider(sf::Vector2f(290, 10));
    slider.setFillColor(sf::Color(50, 50, 50));
    slider.setPosition(50, 520);

    // Индикатор положения на ползунке
    sf::RectangleShape sliderIndicator(sf::Vector2f(10, 20));
    sliderIndicator.setFillColor(sf::Color(0, 138, 230));

    // Расчет позиции индикатора по отношению к brushSize
    float minBrushSize = 1.0f;
    float maxBrushSize = 65.0f;

    // Масштабируем положение индикатора на ползунке в зависимости от brushSize
    float indicatorPosition = slider.getPosition().x + ((brushSize - minBrushSize) / (maxBrushSize - minBrushSize)) * slider.getSize().x;
    sliderIndicator.setPosition(indicatorPosition, slider.getPosition().y - 5);



    // Текст для отображения текущей толщины
    sf::Font font;
    font.loadFromFile("Resources/alphbeta.ttf");
    sf::Text brushSizeText;
    brushSizeText.setFont(font);
    brushSizeText.setCharacterSize(32);
    brushSizeText.setFillColor(sf::Color::Black);
    brushSizeText.setPosition(slider.getPosition().x, slider.getPosition().y - 45);

    // <------- Creating Buttons ------->

    Button brushButton(140, 40, 50, 570, "Brush", "Resources/brush.png", font);
    brushButton.setButtonColor("pressedColor");
    Button eraseButton(140, 40, 200, 570, "Erase", "Resources/eraser.png", font);
    Button saveButton(140, 40, 200, 630, "Save", "Resources/save.png", font);
    Button deleteButton(140, 40, 50, 630, "Delete", "Resources/trash.png", font);


    sf::Color brushColor = sf::Color::Black; // Начальный цвет кисти

    // Цвета палитры
    std::vector<sf::Color> paletteColors = {
        sf::Color::Red, 
        sf::Color::Green, 
        sf::Color::Blue,
        sf::Color::Yellow, 
        sf::Color::Magenta, 
        sf::Color::Cyan,
        sf::Color(255, 165, 0), // Оранжевый
        sf::Color(128, 0, 128), // Фиолетовый
        sf::Color::Black, 
        sf::Color::White
    };

    // Вектор для хранения квадратов палитры
    std::vector<sf::RectangleShape> paletteSquares;
    sf::Vector2f palettePosition(50, 50); // Начальная позиция палитры
    float squareSize = 20; // Размер квадрата палитры
    float padding = 10;    // Расстояние между квадратами

    // Создаем квадраты палитры
    for (size_t i = 0; i < paletteColors.size(); ++i)
    {
        sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
        square.setFillColor(paletteColors[i]);
        square.setPosition(palettePosition.x + i * (squareSize + padding), palettePosition.y);
        paletteSquares.push_back(square);
    }


    // Окно для отображения выбранного цвета кисти
    sf::RectangleShape selectedColorDisplay(sf::Vector2f(290, squareSize));
    selectedColorDisplay.setFillColor(brushColor);
    selectedColorDisplay.setPosition(palettePosition.x, palettePosition.y + squareSize + padding * 2);



    while (window.isOpen())
    {

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) 
            {
                window.close();
            }
                


            // Проверка нажатия мыши на кнопки
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                // Проверяем, наведена ли мышь на квадрат палитры
                for (size_t i = 0; i < paletteSquares.size(); ++i)
                {
                    if (paletteSquares[i].getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
                    {
                        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                        {
                            brushColor = paletteSquares[i].getFillColor(); // Устанавливаем цвет кисти при нажатии
                            selectedColorDisplay.setFillColor(brushColor);
                        }
                    }
                }


                // <----- SAVE BUTTON CLICK PROCESSING ---->

                if (saveButton.isPressed(mousePos)) {
                    saveButton.setButtonColor("pressedColor");
                    saveButton.draw(window);
                    window.display();

                    std::thread([&saveButton]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(150));
                        saveButton.setButtonColor("normalColor");
                        }).detach();


                    // Save the image code
                    sf::RenderTexture renderTexture;
                    renderTexture.create(myCanvas.getSize().x, myCanvas.getSize().y);
                    renderTexture.clear(sf::Color::White);

                    // Shift all elements so that they correspond to the beginning of the RenderTexture
                    for (const auto& line : allLines)
                    {
                        for (auto segment : line)
                        {
                            segment.move(-myCanvas.getPosition());
                            renderTexture.draw(segment);
                        }
                    }

                    renderTexture.display();
                    sf::Image image = renderTexture.getTexture().copyToImage();
                    image.saveToFile("drawing.png");

                }

                

                // <----- DELETE BUTTON CLICK PROCESSING ---->

                if (deleteButton.isPressed(mousePos)) 
                {
                    deleteButton.setButtonColor("pressedColor");
                    deleteButton.draw(window);
                    window.display();

                    std::thread([&deleteButton]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(150));
                        deleteButton.setButtonColor("normalColor");
                        }).detach();

                // Clearing Canvas
                    myCanvas.setFillColor(sf::Color::White);
                    currentLineSegments.clear();
                    allLines.clear();

                // Rerender the window, button and canvas

                    deleteButton.draw(window);
                    window.draw(myCanvas);
                    window.display();

                }

                // <----- BRUSH TOOL BUTTON CLICK PROCESSING ---->


                if (brushButton.isPressed(mousePos))
                {
                    currentTool = ToolMode::Brush;

                    brushButton.setButtonColor("pressedColor");
                    eraseButton.setButtonColor("normalColor");

                 
                }


                // <----- ERASE TOOL BUTTON CLICK PROCESSING ---->

                else if (eraseButton.isPressed(mousePos))
                {
                    currentTool = ToolMode::Eraser;

                    brushButton.setButtonColor("normalColor");
                    eraseButton.setButtonColor("pressedColor");

                }

                // Проверка нажатия на ползунок
                else if (slider.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    float newPos = mousePos.x;
                    if (newPos < slider.getPosition().x) newPos = slider.getPosition().x;
                    if (newPos > slider.getPosition().x + slider.getSize().x) newPos = slider.getPosition().x + slider.getSize().x;

                    sliderIndicator.setPosition(newPos - sliderIndicator.getSize().x, sliderIndicator.getPosition().y);

                    brushSize = 1 + (newPos - slider.getPosition().x) / 4.5;
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
                    lineSegment.setFillColor(currentTool == ToolMode::Brush ? brushColor : sf::Color::White);


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






        
        window.draw(backgroundSprite);
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

        // Рисуем палитру
        for (const auto& square : paletteSquares)
        {
            window.draw(square);
        }

        // Рисуем окно с выбранным цветом
        window.draw(selectedColorDisplay);

        window.draw(slider);
        window.draw(sliderIndicator);
        window.draw(brushSizeText);

        // Rendering all buttons
        brushButton.draw(window);
        eraseButton.draw(window);
        saveButton.draw(window);
        deleteButton.draw(window);

        window.display();
    }

    return 0;
}