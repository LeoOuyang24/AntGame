#include "enemyAssemblers.h"
#include "animation.h"

int AnimationSequencer::getStateIndex(int frameStart,int* timeSince_)
{
    int timeSince = (SDL_GetTicks() - frameStart)%fullDuration; //normalize our time since our animation started
    int index = infoSize - 1;
    for (int i = 0; i < infoSize; ++i)
    {
      //  std::cout << info[index].x << " " << timeSince << "\n";
        if (timeSince - info[i].x <= 0)
        {
            index = std::max(i,0);
            break;
        }
        timeSince -= info[i].x;
    }
    if (timeSince_)
    {
        *timeSince_ = timeSince;
    }
    return index;
}

AnimationSequencer::AnimationSequencer(const std::vector<glm::vec2>& baseInfo)
{
    info = new glm::vec3[baseInfo.size()];
    infoSize = baseInfo.size();
    for (int i = 0; i < infoSize; ++i)
    {
        info[i] = {baseInfo[i].x,baseInfo[i].y,totalFrames};
        fullDuration += baseInfo[i].x;
        totalFrames += baseInfo[i].y;
    }
}

AnimationParameter AnimationSequencer::process(int frameStart)
{
    //std::cout << (SDL_GetTicks() - frameStart) << "\n";
    int timeSince = 0;
    int index = getStateIndex(frameStart,&timeSince);
    AnimationParameter param;
    param.fps = 1000.0*info[index].y/info[index].x;
    param.start = fmod((timeSince*info[index].y/info[index].x + info[index].z),totalFrames);
    //std::cout << timeSince << " " << param.start << " " << index << "\n";
    return param;
}

int AnimationSequencer::getStateIndex(int frameStart)
{
    return getStateIndex(frameStart,nullptr);
}

AnimationSequencer::~AnimationSequencer()
{
    delete[] info;
}

EvilMoonAssembler::EvilMoonAssembler() : UnitAssembler("evilMoon", {20,20}, basicEnemyAnime,true,100,.1)
{

}

Object* EvilMoonAssembler::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRect((new PathComponent(speed,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addRender((new AnimationComponent(*sprite,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new UnitAttackComponent(1,1000,1,100,true,*ent)));
    return ent;
}

ImgParameter TurtFrog::TurtFrogAttack::getParam(Object* attacker, const glm::vec2& pos)
{
    return {{},sequencer.process(attackTimer.getTime())};
}

void TurtFrog::TurtFrogAttack::doAttack(Object* attacker, const glm::vec2& pos)
{

    if (sequencer.getStateIndex(attackTimer.getTime()) == 2)
    {
        ForcesComponent* forces = attacker->getComponent<ForcesComponent>();
        if (forces)
        {
           // std::cout << atan2( attacker->getCenter().y - pos.y, attacker->getCenter().x - pos.x) << "\n";
            forces->addForce({atan2( attacker->getCenter().y - pos.y, attacker->getCenter().x - pos.x),DeltaTime::deltaTime/500.0*modData.range*-1});
        }
    }
}

TurtFrog::TurtFrogAttack::TurtFrogAttack(Entity& unit) : Attack(10,0,lungeRange,unit,&turtFrogAttack), sequencer({
                                                                                                                               {500,2},
                                                                                                                               {1000,1},
                                                                                                                               {500,2}
                                                                                                                               })
{
    //delay = 2000;
}

ImgParameter TurtFrog::TurtFrogAttack::attack(Object* attacker, const glm::vec2& pos)
{
    if (offCooldown())
    {
        if (!attackTimer.isSet())
        {
            attackTimer.set();
        }
        else if (attackTimer.timePassed(duration))
        {
            attackTimer.reset();
            coolDownTimer.set();
        }
        else
        {
            doAttack(attacker,pos);
            attacker->getComponent<MoveComponent>()->setSpeed(0);
        }
    }
    return getParam(attacker,pos);
}

bool TurtFrog::TurtFrogAttack::canAttack(Object* owner, Object* ptr)
{
    return Attack::canAttack(owner,ptr) || attackTimer.isSet();
}


TurtFrog::TurtFrog() : UnitAssembler("TurtFrog",{50,50},turtFrogWalk,true,100,.05,false)
{

}

Object* TurtFrog::assemble()
{
    Unit* ent = new Unit(movable);
    ent->addRender((new UnitAnimationComponent(*sprite,*ent)));
    ent->addRect((new PathComponent(speed,{0,0,dimen.x,dimen.y},*ent)));
    ent->addClickable((new ClickableComponent(name,*ent)));
    ent->addHealth(new HealthComponent(*ent,maxHealth));
    ent->addComponent(*(new UnitAttackComponent(1,100,1,100,true,*ent)));
    ent->addComponent(*(new ForcesComponent(*ent)));
    ent->getComponent<UnitAttackComponent>()->addAttack(*(new TurtFrogAttack(*ent)));
//    ent->addComponent();
    return ent;
}

EvilMoonAssembler evilMoonAssembler;
TurtFrog turtFrog;
