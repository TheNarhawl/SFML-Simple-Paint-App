#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>

enum class ToolMode { Brush, Eraser };

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "My Canvas App");

    // Создаем прямоугольник для фона
    sf::RectangleShape background(sf::Vector2f(window.getSize()));
    background.setFillColor(sf::Color(100, 100, 100));

    // Канвас для рисования
    sf::RectangleShape myCanvas(sf::Vector2f(840, 620));
    myCanvas.setFillColor(sf::Color::White);
    myCanvas.setPosition(sf::Vector2f(390, 50));


    // Определите базовый цвет кнопки
    sf::Color normalButtonColor(200, 200, 200);
    sf::Color pressedButtonColor(150, 150, 150);  // Более тёмный серый для нажатия


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
    sf::RectangleShape slider(sf::Vector2f(290, 10));
    slider.setFillColor(sf::Color(50, 50, 50));
    slider.setPosition(50, 650);

    // Индикатор положения на ползунке
    sf::CircleShape sliderIndicator(10);
    sliderIndicator.setFillColor(sf::Color::Red);

    sliderIndicator.setPosition(slider.getPosition().x + (brushSize - 1) * 20, slider.getPosition().y - 5);

    // Текст для отображения текущей толщины
    sf::Font font;
    font.loadFromFile("arial.ttf");
    sf::Text brushSizeText;
    brushSizeText.setFont(font);
    brushSizeText.setCharacterSize(20);
    brushSizeText.setFillColor(sf::Color::Black);
    brushSizeText.setPosition(50, 620);

    // Кнопка "Кисть"
    sf::RectangleShape brushButton(sf::Vector2f(140, 40));
    brushButton.setFillColor(normalButtonColor);
    brushButton.setPosition(50, 550);
    sf::Text brushText("Brush", font, 20);
    brushText.setFillColor(sf::Color::Black);
    brushText.setPosition(brushButton.getPosition().x + 44, brushButton.getPosition().y + 6);

    // Кнопка "Ластик"
    sf::RectangleShape eraserButton(sf::Vector2f(140, 40));
    eraserButton.setFillColor(normalButtonColor);
    eraserButton.setPosition(200, 550);
    sf::Text eraserText("Eraser", font, 20);
    eraserText.setFillColor(sf::Color::Black);
    eraserText.setPosition(eraserButton.getPosition().x + 39, eraserButton.getPosition().y + 6);


    // Кнопка "Сохранить"
    sf::RectangleShape saveButton(sf::Vector2f(290, 40));
    saveButton.setFillColor(normalButtonColor);
    saveButton.setPosition(50, 600);
    sf::Text saveText("Save", font, 20);
    saveText.setFillColor(sf::Color::Black);
    saveText.setPosition(saveButton.getPosition().x + 120, saveButton.getPosition().y + 6);

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

    sf::Color brushColor = sf::Color::Black; // Начальный цвет кисти

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
                window.close();

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

                // Обработка нажатия на кнопку "Сохранить"
                if (saveButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    // Устанавливаем цвет нажатия
                    saveButton.setFillColor(pressedButtonColor);
                    window.draw(saveButton);    // Обновляем отображение, чтобы цвет изменился сразу
                    window.draw(saveText);      // Рисуем текст на кнопке "Save"
                    window.display();           // Перерисовка окна для немедленного отображения эффекта

                    // Запускаем асинхронный таймер для временного изменения цвета
                    std::thread([&saveButton, normalButtonColor]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(150));  // Пауза на 150 мс
                        saveButton.setFillColor(normalButtonColor);  // Возвращаем нормальный цвет
                        }).detach();  // Отсоединяем поток, чтобы не блокировать основной поток

                        // Код для сохранения изображения
                        sf::RenderTexture renderTexture;
                        renderTexture.create(myCanvas.getSize().x, myCanvas.getSize().y);
                        renderTexture.clear(sf::Color::White);

                        // Смещаем все элементы так, чтобы они соответствовали началу RenderTexture
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

                // Обработка выбора кисти
                if (brushButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    currentTool = ToolMode::Brush;
                    brushButton.setFillColor(pressedButtonColor);
                    eraserButton.setFillColor(normalButtonColor);
                }
                // Обработка выбора ластика
                else if (eraserButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    currentTool = ToolMode::Eraser;
                    eraserButton.setFillColor(pressedButtonColor);
                    brushButton.setFillColor(normalButtonColor);
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


        window.clear(sf::Color::White);
        window.draw(background);
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

        // Рисуем кнопки и их текст
        window.draw(brushButton);
        window.draw(brushText);

        window.draw(eraserButton);
        window.draw(eraserText);

        window.draw(saveButton);
        window.draw(saveText);

        window.display();
    }

    return 0;
}