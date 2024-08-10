#pragma once
#include "SFML/Graphics.hpp"
#include <SFML/System/Vector2.hpp>


class RoundedRect : public sf::Shape
{
public:
    RoundedRect()
    {
        setSize(10, 10);
        setRadius(3);
    }

    RoundedRect(double width, double height, double radius)
    {
        setSize(width, height);
        setRadius(radius);
    }

    RoundedRect(double width, double height, double radiusTopLeft, double radiusTopRight, double radiusBottomRight, double radiusBottomLeft)
    {
        setSize(width, height);
        setRadius(radiusTopLeft, radiusTopRight, radiusBottomRight, radiusBottomLeft);
    }
    
    void setRadius(double radius)
    {
        setRadius(radius, radius, radius, radius);
    }

    void setRadius(double top, double bottom)
    {
        setRadius(top, top, bottom, bottom);
    }

    void setRadius(double topLeft, double topRight, double bottomRight, double bottomLeft)
    {
        double array[] = {topLeft, topRight, bottomRight, bottomLeft};
        if (std::equal(std::begin(array), std::end(array), std::end(this->cornerRadii)))
            return;

        std::copy(std::begin(array), std::end(array), std::begin(this->cornerRadii));

        for (int i = 0; i < 4; i++) // auto calculate corner resolution based on corner radius
        {
            cornerRes[i] = std::max(std::min(std::sqrt(this->cornerRadii[i] * 4), 16.0), 1.0);
        }

        this->cornerResSums[0] = 0;
        this->cornerResSums[1] = this->cornerRes[0];
        this->cornerResSums[2] = this->cornerResSums[1] + this->cornerRes[1];
        this->cornerResSums[3] = this->cornerResSums[2] + this->cornerRes[2];

        this->totalPoints = this->cornerResSums[3] + this->cornerRes[3] + 1;

        update();
    }

    void setSize(double width, double height)
    {
        if (this->width == width && this->height == height)
            return;

        this->width = width;
        this->height = height;
        update();
    }

    void setWidth(double width)
    {
        setSize(width, this->height);
    }

    void setHeight(double height)
    {
        setSize(this->width, height);
    }

    mutable double angle = 180.0;
    sf::Vector2f getPoint(std::size_t index) const override
    {
        static const double PI = 3.14159265358979323846;

        if (index == 0) // if this is the first point start the angle over
            angle = 180.0;

        if (index >= this->totalPoints) // if the index is out of bounds return 0,0
            return sf::Vector2f(0, 0);

        // which corner are we in?
        int centerIndex = this->indexToCorner(index);
        sf::Vector2f center = this->cornerCenters[centerIndex];
        double radius = this->cornerRadiiClamped[centerIndex];

        // if the radius is too small just return the center as a single point
        if (radius <= 0.01f)
        {
            angle -= 90.0;
            return center;
        }

        // get the point along the arc defined by this corner
        double angleRad = angle * PI / 180.0;
        double Cos = cos(angleRad);
        double Sin = sin(angleRad);
        sf::Vector2f vector = sf::Vector2f(
            radius * Cos + center.x,
            -radius * Sin + center.y
        );

        // decrement the angle
        double cornerRes = this->cornerRes[centerIndex];
        double deltaAngle = 90.0 / cornerRes;
        angle -= deltaAngle;

        return vector;
    }

    std::size_t getPointCount() const override
    {
        return totalPoints;
    }
    

private:
    int totalPoints;
    double cornerRadii[4];
    double cornerRadiiClamped[4];
    double cornerRes[4];
    double cornerResSums[4];

    double width;
    double height;

    sf::Vector2f cornerCenters[4];

    virtual void update()
    {
        std::copy(std::begin(cornerRadii), std::end(cornerRadii), std::begin(cornerRadiiClamped));
        for (int i = 0; i < 4; i++)
        {
            cornerRadiiClamped[i] = std::min(cornerRadii[i], std::min(width, height) / 2);
        }
        calcCornerCenters();
        Shape::update();
    }



    void calcCornerCenters()
    {
        sf::Vector2f array[4] =
        {
            sf::Vector2f(cornerRadiiClamped[0], cornerRadiiClamped[0]), // top left
            sf::Vector2f(width - cornerRadiiClamped[1], cornerRadiiClamped[1]), // top right
            sf::Vector2f(width - cornerRadiiClamped[2], height - cornerRadiiClamped[2]), // bottom right
            sf::Vector2f(cornerRadiiClamped[3], height - cornerRadiiClamped[3]), // bottom left
        };
        std::copy(std::begin(array), std::end(array), std::begin(cornerCenters));
    }

    int indexToCorner(uint16_t index) const
    {
        if (index >= totalPoints)
            return 0;

        return index >= cornerResSums[3] ? 3 : index >= cornerResSums[2] ? 2 : index >= cornerResSums[1] ? 1 : 0;
    }
};