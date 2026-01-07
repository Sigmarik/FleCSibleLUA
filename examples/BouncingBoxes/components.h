namespace ecs
{
struct Position
{
    float x = 0;
    float y = 0;
};

struct BoxCollider
{
    float sizeX = 50;
    float sizeY = 50;
};

struct Velocity
{
    float x = 0;
    float y = 0;
};

struct Gravity
{
    float value = 9.815f * 50.0f;
};
}
