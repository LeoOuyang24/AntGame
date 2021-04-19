#ifndef FRIENDLYASSEMBLERS_H_INCLUDED
#define FRIENDLYASSEMBLERS_H_INCLUDED

#include "entities.h"

struct ObjectAssembler
{
    AnimationWrapper* const sprite;
    const glm::vec2 dimen;
    const std::string name;
    const bool movable = false;
    const bool friendly = false;
    ObjectAssembler( std::string name_, const glm::vec2& rect_,AnimationWrapper& anime, bool mov = false, bool friendly_ = true);
    ~ObjectAssembler();
    virtual Object* assemble();
};

struct UnitAssembler : public ObjectAssembler
{
    const double maxHealth = 0;
    float speed = .1;
    UnitAssembler( std::string name_,const glm::vec2& rect_, AnimationWrapper& anime, bool mov, double maxHealth_, float speed,bool friendly_ = false);
    virtual Object* assemble();
    Unit* NPCUnitAssemble(); //creates a unit with a bunch of default components. Does not add unitAttackComponent since that may be unique to each assembler
};

struct ProjectileAssembler : public ObjectAssembler
{
    const double damage = 0;
    float speed = .1;

    ProjectileAssembler(double damage_, std::string name_,const glm::vec2& rect_, AnimationWrapper& anime, float speed, bool friendly_ = false);
    virtual Object* assemble(Object& shooter, const glm::vec2& point, const glm::vec2& target);
};

class HitboxAssembler : public ProjectileAssembler //assembles a hitbox, which is basically a projectile that becomes inactive after a while.
{

    class HitboxRender : public AnimationComponent, public ComponentContainer<HitboxRender> //really more of a debug component than anything. Renders the hitboxes and if they have collided with something
    {
        bool collided = false;
    public:
        HitboxRender(Entity& entity);
        void collide(Entity& other);
        void update();
    };
    ProjectileComponent::ProjCollideFunc projCollideFunc = nullptr;
    const int duration;
    public:
        class HitboxComponent : public ProjectileComponent, public ComponentContainer<HitboxComponent> //projectile component that becomes inacive after some time
        {
            int duration = 0;//in milliseconds
            bool active = false;
            DeltaTime timer;
        public:
            HitboxComponent(int duration, float damage, bool friendly, const glm::vec2& target,
                                        const glm::vec4& rect,Object& entity, ProjectileComponent::ProjCollideFunc func);
            bool getActive();
            void collide(Entity& other);
            void activate();
            void deactivate();
            void update();
            ~HitboxComponent();
        };
        HitboxAssembler(int duration, float damage, std::string name, bool friendly, const glm::vec2& dimen, ProjectileComponent::ProjCollideFunc func);
        Object* assemble(Object& shooter, const glm::vec2& point, const glm::vec2& target);
};


class HitboxAttack : public ProjectileAttack
{
    Object* hitbox = nullptr;
    void doAttack(Object* attacker, const glm::vec2& target);
public:
    HitboxAttack(HitboxAssembler& ass, int endLag, double range, AnimationWrapper* attackAnime_ = nullptr, AnimationSequencer* sequencer_ = nullptr);
};




typedef std::vector<UnitAssembler*> UnitBucket;

extern UnitBucket allUnits; //vector of all units and structures
extern UnitBucket allStructures;
extern UnitBucket allShopItems;

void initAssemblers();

UnitAssembler* getRandomAssembler(UnitBucket& bucket);
void addUnitToBucket(UnitAssembler& ass, UnitBucket& bucket);

#endif // FRIENDLYASSEMBLERS_H_INCLUDED
