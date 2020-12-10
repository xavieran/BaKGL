#pragma once


namespace BAK {

constexpr unsigned TILE_SIZE = 64000;
constexpr unsigned RES_X = 2048;
constexpr unsigned RES_Y = 2048;
constexpr unsigned MAP_SIZE_X = 96 * 4;
constexpr unsigned MAP_SIZE_Y = 96 * 4;

Vector2D TransformLoc(
    const Vector2D& loc,
    const Vector2D& centre,
    double scale)
{   
    auto translated = Vector2D{
        loc.GetX() - centre.GetX(), 
        loc.GetY() - centre.GetY()};
    return (translated * scale) * (static_cast<double>(MAP_SIZE_X) / TILE_SIZE);
}

Vector2D TransformLoc2(
    const Vector2D& loc,
    const Vector2D& centre,
    double scale)
{   
    auto translated = Vector2D{
        loc.GetX() + centre.GetX(), 
        loc.GetY() + centre.GetY()};
    return (translated * scale) * (static_cast<double>(MAP_SIZE_X) / TILE_SIZE);
}

Vector2D ScaleRad(
    const Vector2D& vec,
    double scale)
{
    auto result = Vector2D{vec};
    result *= scale;
    result *= (static_cast<double>(MAP_SIZE_X) / TILE_SIZE);
    return result;
}

}
