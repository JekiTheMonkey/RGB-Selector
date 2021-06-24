#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <exception>

// f fraction
float lerp(float a, float b, float f)
{
    if (f < 0 || f > 1)
        throw std::runtime_error("std::runtime_error exception: "
            "lerp() cannot work with following fraction " + std::to_string(f) + "\n");

    return a + f * (b - a);
}

// f fraction
sf::Color colorLerp(sf::Color a, sf::Color b, float f)
{
    return { static_cast<sf::Uint8>(lerp(a.r, b.r, f)),
             static_cast<sf::Uint8>(lerp(a.g, b.g, f)),
             static_cast<sf::Uint8>(lerp(a.b, b.b, f)) };
}

// a - top-left     d - top-right
// b - bottom-left  c - bottom-right 
template <typename T>
sf::Color bilinearInterp(sf::Color a, 
                         sf::Color b, 
                         sf::Color c, 
                         sf::Color d, 
                         sf::Vector2<T> p)
{
    sf::Color cTop = colorLerp(a, d, p.x);
    sf::Color cBot = colorLerp(b, c, p.x);
    sf::Color cUv = colorLerp(cTop, cBot, p.y);

    return cUv;
}

sf::Color rgb(double ratio)
{
    // we want to normalize ratio so that it fits in to 6 regions
    // where each region is 256 units long
    int normalized = int(ratio * 256 * 6);

    // find the distance to the start of the closest region
    int x = normalized % 256;

    sf::Uint8 red = 0, grn = 0, blu = 0;
    switch(normalized / 256)
    {
    case 0: red = 255;      grn = 0;        blu = x;       break; // red -> magenta
    case 1: red = 255 - x;  grn = 0;        blu = 255;     break; // magenta -> blue
    case 2: red = 0;        grn = x;        blu = 255;     break; // blue -> cyan
    case 3: red = 0;        grn = 255;      blu = 255 - x; break; // cyan -> green
    case 4: red = x;        grn = 255;      blu = 0;       break; // green -> yellow
    case 5: red = 255;      grn = 255 - x;  blu = 0;       break; // yellow -> red
    }

    return { red, grn, blu };
}

float distance = 5.f;

int positionToNumber(sf::Vector2i position)
{
    return static_cast<int>(static_cast<float>(position.y) / distance * 2);
}

int main()
{
    size_t lineSize = 192;
    sf::Vertex line[lineSize];
    sf::Color color(sf::Color::Red);
    double range = lineSize / 2;
    for (size_t n = 0; n < lineSize; n += 2)
    {
        line[n].position = sf::Vector2f(10.f, distance * n / 2);
        line[n].color = color;
        line[n+1].position = sf::Vector2f(50.f, distance * n / 2);
        line[n+1].color = color;

        color = rgb(n / 2 / range);
    }

    sf::FloatRect lineRect(
        line[0].position.x, 
        line[0].position.y, 
        line[1].position.x - line[0].position.x, 
        line[lineSize - 1].position.y - line[0].position.y);

    sf::RectangleShape lineIndicator(sf::Vector2f(45.f, 3.f));
    lineIndicator.setOutlineThickness(1.f);
    lineIndicator.setFillColor(sf::Color::Black);
    lineIndicator.setOrigin(lineIndicator.getSize() / 2.f);
    lineIndicator.setPosition(sf::Vector2f(30.f, lineRect.height));

    sf::Vertex canvas[4];
    canvas[0].position = sf::Vector2f(75.f, 0.f);
    canvas[0].color = sf::Color::White;
    canvas[1].position = sf::Vector2f(75.f, lineRect.height);
    canvas[1].color = sf::Color::Black;
    canvas[2].position = sf::Vector2f(475.f, lineRect.height);
    canvas[2].color = sf::Color::Black;
    canvas[3].position = sf::Vector2f(475.f, 0.f);
    canvas[3].color = sf::Color::Red;

    sf::FloatRect canvasRect(
        canvas[0].position.x, 
        canvas[0].position.y, 
        canvas[2].position.x - canvas[0].position.x, 
        canvas[2].position.y - canvas[0].position.y);

    float r = 6.f;
    sf::CircleShape canvasIndicator(r);
    canvasIndicator.setFillColor(sf::Color::Transparent);
    canvasIndicator.setOutlineThickness(3.f);
    canvasIndicator.setOrigin(sf::Vector2f(r, r));
    canvasIndicator.setPosition(canvas[3].position);
    sf::Color indicatorColor(sf::Color::Red);

    // Mouse position on palette. Default position is top right angle
    sf::Vector2f normilizedVec(1.f, 0.f);

    const int width = 500, height = distance * (lineSize - 1) / 2;
    sf::RenderWindow window(sf::VideoMode(width, height), "Color selector", sf::Style::Default);
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                if (event.type == sf::Event::MouseMoved)
                {
                    sf::Vector2i mousePosition(event.mouseMove.x, event.mouseMove.y);
                    
                    if (lineRect.contains(sf::Vector2f(mousePosition)))
                    {
                        int n = positionToNumber(mousePosition);

                        lineIndicator.setPosition(sf::Vector2f(
                            lineRect.left + lineRect.width / 2, mousePosition.y));
                        color = line[n].color;
                        canvas[3].color = color;
                    }

                    if (canvasRect.contains(sf::Vector2f(mousePosition)))
                    {
                        normilizedVec = sf::Vector2f(
                            (mousePosition.x - canvasRect.left) / canvasRect.width, 
                            (mousePosition.y - canvasRect.top) / canvasRect.height);
                        canvasIndicator.setPosition(sf::Vector2f(mousePosition));
                    }

                    indicatorColor = sf::Color(bilinearInterp(
                        canvas[0].color,
                        canvas[1].color,
                        canvas[2].color,
                        canvas[3].color,
                        normilizedVec));
                    window.setTitle("Selected RGB color: [" +
                        std::to_string(int(indicatorColor.r)) + "," +
                        std::to_string(int(indicatorColor.g)) + "," +
                        std::to_string(int(indicatorColor.b)) + "]");
                }
            }
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        window.clear();

        window.draw(line, lineSize, sf::TriangleStrip);
        window.draw(lineIndicator);
        window.draw(canvas, 4, sf::Quads);
        window.draw(canvasIndicator);

        window.display();
    }
}