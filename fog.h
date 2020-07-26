#ifndef FOG_H_INCLUDED
#define FOG_H_INCLUDED

class FogMaker //handles all things fog of war
{
    struct FogRequest //holds rect/polygon info
    {
        bool isRect = false; //whether or not this request is that of a rect
        glm::vec2 center = {0,0};
        double radius = 0;
        int sides = 0;
        glm::vec4 rect = {0,0,0,0};


    };
    std::vector<FogRequest> fogRequests;

public:
    const static int fogZ; //z at which to render fog
    void renderFog(); //renders fog and all vision spots as well as EVERY POLYGON REQUESTED UP TO THIS POINT
    void requestRectFog(const glm::vec4& rect); //places a request for a rectangular fog
    void requestPolyFog(const glm::vec2& center, double radius, int sides);
};

#endif // FOG_H_INCLUDED
