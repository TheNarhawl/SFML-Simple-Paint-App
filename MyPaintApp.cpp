#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <windows.h>
#include "Button.h"

enum class ToolMode { Brush, Eraser };
void saveImage(const sf::Image& image);

int main()
{
    // <------ MAIN WINDOW ------>
    sf::RenderWindow window(sf::VideoMode(1280, 720), "MyPaintApp", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("Resources/background.png")) return -1;
    sf::Sprite backgroundSprite(backgroundTexture);

    // Custom cursor
    sf::Image cursorImage;
    if (!cursorImage.loadFromFile("Resources/cursor.png")) return -1;
    sf::Cursor cursor;
    cursor.loadFromPixels(cursorImage.getPixelsPtr(), cursorImage.getSize(), sf::Vector2u(0, 0));
    window.setMouseCursor(cursor);

    // Global font
    sf::Font font;
    font.loadFromFile("Resources/alphbeta.ttf");


    // <------- DRAWING CANVAS -------->
    sf::RectangleShape myCanvas(sf::Vector2f(840, 620));
    myCanvas.setFillColor(sf::Color::White);
    myCanvas.setPosition(sf::Vector2f(390, 50));

    // Store all lines in a vector (each line is a vector of segments)
    std::vector<std::vector<sf::RectangleShape>> allLines;
    std::vector<sf::RectangleShape> currentLineSegments;

    // Store mouse last position
    sf::Vector2f lastMousePos;
    bool isDrawing = false;


    // <---- BRUSH SETTINGS ----->
    float brushSize = 20.0f;
    ToolMode currentTool = ToolMode::Brush; // brush by default


    // <---- SLIDER ----->
    sf::RectangleShape slider(sf::Vector2f(290, 10));
    slider.setFillColor(sf::Color(50, 50, 50));
    slider.setPosition(50, 520);

    // Position indicator on slider
    sf::RectangleShape sliderIndicator(sf::Vector2f(10, 20));
    sliderIndicator.setFillColor(sf::Color(0, 138, 230));

    // Calculation of indicator position in relation to brushSize
    float minBrushSize = 1.0f;
    float maxBrushSize = 65.0f;
    float indicatorPosition = slider.getPosition().x + ((brushSize - minBrushSize) / (maxBrushSize - minBrushSize)) * slider.getSize().x;
    sliderIndicator.setPosition(indicatorPosition, slider.getPosition().y - 5);

    // Brush size text
    sf::Text brushSizeText;
    brushSizeText.setFont(font);
    brushSizeText.setCharacterSize(32);
    brushSizeText.setFillColor(sf::Color::Black);
    brushSizeText.setPosition(slider.getPosition().x, slider.getPosition().y - 45);



    // <------- CREATING BUTTONS ------->
    Button brushButton(140, 40, 50, 570, "Brush", "Resources/brush.png", font);
    brushButton.setButtonColor("pressedColor");
    Button eraseButton(140, 40, 200, 570, "Erase", "Resources/eraser.png", font);
    Button saveButton(140, 40, 200, 630, "Save", "Resources/save.png", font);
    Button deleteButton(140, 40, 50, 630, "Delete", "Resources/trash.png", font);

    

    // <-------- PALETTE PARAMETERS --------->
    std::vector<sf::Color> paletteColors = {
        sf::Color::Red, 
        sf::Color::Green, 
        sf::Color::Blue,
        sf::Color::Yellow, 
        sf::Color::Magenta, 
        sf::Color::Cyan,
        sf::Color(255, 165, 0), // Orange
        sf::Color(128, 0, 128), // Purple
        sf::Color::Black, 
        sf::Color::White
    };

    // Brush initial Color
    sf::Color brushColor = sf::Color::Black;

    // Palette squares storage
    std::vector<sf::RectangleShape> paletteSquares;
    sf::Vector2f palettePosition(50, 50);
    float squareSize = 20;
    float padding = 10;

    // Drawing palette squares
    for (size_t i = 0; i < paletteColors.size(); ++i)
    {
        sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
        square.setFillColor(paletteColors[i]);
        square.setPosition(palettePosition.x + i * (squareSize + padding), palettePosition.y);
        paletteSquares.push_back(square);
    }

    // Drawing picked color
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
               
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                // <----- PALETTE PROCESSING --->
                for (size_t i = 0; i < paletteSquares.size(); ++i)
                {
                    if (paletteSquares[i].getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
                    {
                        // Set the color if clicked
                        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                        {
                            brushColor = paletteSquares[i].getFillColor(); 
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

                    std::thread([image]() {
                        saveImage(image);
                        }).detach();

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


                // <------ BRUSH SIZE SLIDER PROCESSING ------>
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


        // Displaying the brush size
        brushSizeText.setString("Brush size: " + std::to_string(static_cast<int>(brushSize)));


        // <--------- PAINT LOGICS ---------->
        if (isDrawing && sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            // Getting mouse cords
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            // If mouse is inside canvas
            if (myCanvas.getGlobalBounds().contains(mousePos))
            {
                // Check if mouse actually moved
                if (mousePos != lastMousePos)
                {
                    // Draw using segment lines
                    sf::RectangleShape lineSegment;
                    lineSegment.setFillColor(currentTool == ToolMode::Brush ? brushColor : sf::Color::White);

                    /*      Math
                        delta - is the vector of the difference between the current and previous mouse position
                        length - calculates the distance between points (the length of a line segment) using Pythagoras' theorem.
                        setOrigin - sets the center of the rectangle (along the brush thickness).
                        setRotation - calculates the angle of the line using the arctangent (atan2) converted from radians to degrees.
                        setPosition - sets the start point of the segment to lastMousePos.

                        The segment is added to the list ofcurrent line segments (currentLineSegments).
                        The last mouse position is updated (lastMousePos).
                    */ 

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

        // If drawing is active but the left mouse button is released, the process of drawing finishes.
        else if (isDrawing && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            if (!currentLineSegments.empty())
            {
                allLines.push_back(currentLineSegments);
            }
            isDrawing = false;
        }


        // Rendering canvas and paintings
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

        // Rendering palette
        for (const auto& square : paletteSquares)
        {
            window.draw(square);
        }

        window.draw(selectedColorDisplay);

        // Rendering slider
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


// Using windows.h to save the painting
void saveImage(const sf::Image& image) {
    wchar_t filename[MAX_PATH] = L"";
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = L"PNG Files\0*.png\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"png";

    if (GetSaveFileNameW(&ofn)) {
        std::wcout << L"Выбранный файл: " << filename << std::endl;
        std::string filePath(filename, filename + wcslen(filename));
        image.saveToFile(filePath);
    }
    else {
        std::cout << "Операция отменена пользователем." << std::endl;
    }

    std::cin.get();
}
