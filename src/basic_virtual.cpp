#include<iostream>
#include<vector>
#include<array>
#include<unordered_set>
#include<format>
#include<algorithm>
#include<optional>
#include<deque>
#include<queue>
#include<tuple>
#include<string>
#include<unordered_map>
#include<fstream>
#include<chrono>
#include<list>

#include<json.hpp>

//imgui相关
#include "imgui.h"
#include "imgui_impl_win32.h"  // 平台后端
#include "imgui_impl_dx11.h"   // 渲染后端
#include <d3d11.h>             // DX11 底层
#include <windows.h>           // Win32 底层

#include <wrl/client.h>
#include "WICTextureLoader.h"
#include "SpriteBatch.h"
#include "CommonStates.h"
#include <dxgi.h>


class IStaticActor;
class IMobileActor;
template<typename SubClass>
class StaticActor;
template<typename SubClass>
class MobileActor;
class IStaticActor;
class IMobileActor;
class Game;
class Draw;

class SingleTower;
class GroupAttackTower;
class SlowTower;
class CenterTower;

class MeleeMobile;
class RangedMobile;
class DefenseMobile;
class ExplosionMobile;

class ISpecialEffect;
template<typename ApplyActor,typename ReceiveActor>
class SpecialEffect;
class Effective;

class Resist;
class ResistList;

class _AttributeId;
template<typename SubClass>
class AttributeId;



template<typename T>
class TypeId{
    static constexpr int value=-1;
};

template<>class TypeId<SingleTower>{ public: static constexpr int value=1; };

template<>class TypeId<GroupAttackTower>{ public: static constexpr int value=2; };

template<>class TypeId<SlowTower>{ public: static constexpr int value=3; };

template<>class TypeId<CenterTower>{ public: static constexpr int value=4; };

template<>class TypeId<MeleeMobile>{ public: static constexpr int value=1; };

template<>class TypeId<RangedMobile>{ public: static constexpr int value=2; };

template<>class TypeId<DefenseMobile>{ public: static constexpr int value=3; };

template<>class TypeId<ExplosionMobile>{ public: static constexpr int value=4; };


template<typename T>
void erase_basedSwap(std::vector<T>& vec,size_t index){
    std::swap(vec[index],vec[vec.size()-1]);
    vec.pop_back();
}
template<typename T>
void erase_basedSwap(std::deque<T>& vec,size_t index){
    std::swap(vec[index],vec[vec.size()-1]);
    vec.pop_back();
}

enum class ModType:uint8_t{
    add1,
    mul,
    add2
};

enum class AttackType:uint8_t{
    Phys=0,//物理
    Magic=1,//魔法
    Pure=2,//真实
    Count
};
class _AttributeId{
    public:

    static constexpr uint8_t hp=static_cast<uint8_t>(AttackType::Count);
    static constexpr uint8_t attackNum=hp+1;
    static constexpr uint8_t attackScope=hp+2;
    static constexpr uint8_t attackCount=hp+3;
    static constexpr uint8_t attackSpeed=hp+4;
    static constexpr uint8_t attackType=hp+5;
    static constexpr uint8_t cost=hp+6;
    static constexpr uint8_t totalCost=hp+7;
    static constexpr uint8_t costRate=hp+8;
    static constexpr uint8_t moveSpeed=hp+9;
    static constexpr uint8_t rasistCount=hp+9;
};


template<typename SubClass>
class AttributeId:public _AttributeId{};

template<>
class AttributeId<GroupAttackTower>:public _AttributeId{
    public:
    static constexpr uint8_t GroupArrackScope=hp+10;
};
template<>
class AttributeId<SlowTower>:public _AttributeId{
    public:
    static constexpr uint8_t attackSlowMul=hp+10;
    static constexpr uint8_t moveSlowMul=hp+11;
    static constexpr uint8_t attackSlowTime=hp+12;
    static constexpr uint8_t moveSlowTime=hp+13;
};
template<>
class AttributeId<ExplosionMobile>:public _AttributeId{
    public:
    static constexpr uint8_t explosionNum=hp+10;
};


enum class TimeType:uint8_t{
    Attack,
    Move,
    Skill1,
    Skill2,
    Count//计数用
};

class Timers{
    public:
    float timers[static_cast<uint8_t>(TimeType::Count)];

    Timers(){
        memset(timers,0,sizeof(timers));
    }

    float get(TimeType timeType){
        return timers[static_cast<uint8_t>(timeType)];
    }

    void set(TimeType timeType,float value){
        timers[static_cast<uint8_t>(timeType)]=value;
    }

    float& operator[](TimeType timeType){
        return timers[static_cast<uint8_t>(timeType)];
    }

    const float& operator[](TimeType timeType)const{
        return timers[static_cast<uint8_t>(timeType)];
    }
};
class ISpecialEffect{
    public:
    int Id;//作用属性id
    ModType modType;
    float value;
    float endTime;
    //持有施加方的list与迭代器
    std::list<ISpecialEffect*>::iterator applyIterator;
    std::list<ISpecialEffect*>::iterator receiveIterator;
    ISpecialEffect(int Id,ModType modType,float value,float endTime):Id(Id),value(value),endTime(endTime){
        this->Id=Id;
        this->modType=modType;
        this->value=value;
        this->endTime=endTime;

    };
    virtual std::list<ISpecialEffect*>::iterator check(){return {};};
    virtual void removeApply(){};
    virtual void removeReceive(){};
};

template<typename ApplyActor,typename ReceiveActor>
class SpecialEffect:public ISpecialEffect{
    public:
    ApplyActor* applyActorPtr;
    ReceiveActor* receiveActorPtr;
    SpecialEffect(int Id,ModType modType,float value,float endTime,ApplyActor* applyActorPtr,ReceiveActor* receiveActorPtr):ISpecialEffect(Id,modType,value,endTime),applyActorPtr(applyActorPtr),receiveActorPtr(receiveActorPtr){
        applyActorPtr->applyEffectList.push_back(this);
        receiveActorPtr->receiveEffectList.push_back(this);
        applyIterator=applyActorPtr->applyEffectList.end();
        receiveIterator=receiveActorPtr->receiveEffectList.end();

        receiveActorPtr->nowAttributeList[this->Id];

        auto& tempEffective=receiveActorPtr->nowAttributeList[this->Id];
        switch(this->modType){
            case ModType::add1:
                tempEffective.add1+=this->value;
                break;
            case ModType::mul:
                tempEffective.mul*=this->value;
                break;
            case ModType::add2:
                tempEffective.add2+=this->value;
                break;
        }
        tempEffective.checkFlag=false;
    };

    ~SpecialEffect(){
        auto& tempEffective=receiveActorPtr->nowAttributeList[this->Id];
        switch(this->modType){
            case ModType::add1:
            tempEffective.add1-=this->value;
            break;
            case ModType::mul:
            tempEffective.mul/=this->value;
            break;
            case ModType::add2:
            tempEffective.add2-=this->value;
            break;
        }
        tempEffective.checkFlag=false;
    }

    std::list<ISpecialEffect *>::iterator remove(){//从两边列表中删除 很危险
        applyActorPtr->applyEffectList.erase(applyIterator);
        auto tempIterator=receiveActorPtr->receiveEffectList.erase(receiveIterator);
        delete this;
        return tempIterator;
    }

    void removeApply(){
        applyActorPtr->applyEffectList.erase(applyIterator);
        delete this;
    }

    void removeReceive(){
        receiveActorPtr->receiveEffectList.erase(receiveIterator);
    }


    virtual std::list<ISpecialEffect*>::iterator check()override{//接受方进行检测
        Game* gamePtr=receiveActorPtr->gamePtr;
        if(endTime!=-1&&gamePtr->nowTime>endTime){//应当结束状态
            return remove();
        }
        return ++receiveIterator;
    };
};

class Effective{
    public:
    float add1;//绝对值
    float mul;//倍率
    float add2;//最后加算
    float value;//最终值
    bool checkFlag;

    Effective(){//直接覆盖?
        add1=0;
        mul=1;
        add2=0;
        value=0.0;
        checkFlag=false;
    }

    void updateValue(float normalData){
        value=(normalData+add1)*mul+add2;
        checkFlag=true;

        //等价y=ax+b 可表述任意一次单变量线性变换
    }

};

class ResistList;

class Resist{
    public:
    ResistList* resistedList;
    std::list<Resist*>::iterator resistIterator;
    bool state;
    Resist():state(false){};

    void removeResist();
};

class ResistList{
    public:
    std::list<Resist*> resistList;
    int resistNum;
    int residualResistNum;
    ResistList():resistNum(0),residualResistNum(0){};
    void push(Resist& resist){
        if(residualResistNum<=0){
            return;
        }
        residualResistNum-=1;
        resist.resistedList=this;
        resist.state=true;
        resistList.push_back(&resist);
        resist.resistIterator=std::prev(resistList.end());
    }

    void pop(){
        if(resistList.size()==0){
            return;
        }
        auto tempResist=*std::prev(resistList.end());
        tempResist->removeResist();
    }

    void clear(){
        if(resistList.size()==0){
            return;
        }
        while(resistList.size()!=0){
            auto tempResist=*std::prev(resistList.end());
            tempResist->removeResist();
        }
    }
};

void Resist::removeResist(){
    if(resistedList==nullptr){
        return;
    }
    if(state){
        state=false;
        resistedList->residualResistNum+=1;
        resistedList->resistList.erase(resistIterator);
        resistedList=nullptr;
    }
}

template<typename T>
class Attribute{
    public:
    static std::vector<std::vector<float>> attributeList;

    static float getCost(int startRank,int endRank){
        float result=0.0;
        if(startRank<0||endRank>=attributeList.size()){
            return -1;
        }
        for(int i=startRank;i<endRank+1;i++){
            result+=attributeList[i][9];
        }
        return result;

    }
    static float getCost(int startRank,int endRank,T* ptr){
        float result=0.0;
        if(startRank<0||endRank>=attributeList.size()){
            return -1;
        }
        for(int i=startRank;i<endRank+1;i++){
            result+=attributeList[i][9];
        }
        return result;
    }
};



template<typename T>
class ActorPool{
    public:
    static std::deque<T>Pool;
};

template<typename T>std::deque<T>ActorPool<T>::Pool;
template<typename T>std::vector<std::vector<float>>Attribute<T>::attributeList;

class IStaticActor{//仅用作基类指针
    public:

    std::vector<std::array<int,2>> scopeList;//攻击范围覆盖的格子 非直接坐标而是偏移量[dx,dy] 获取真实坐标是x+dx,y+dy

    std::list<ISpecialEffect*> applyEffectList;//所拥有的特殊状态         对方析构时会将其结束时间置0
    std::list<ISpecialEffect*> receiveEffectList;//所施加的特殊状态       在自身析构时将造成的所有异常状态全部结束

    std::vector<Effective> nowAttributeList;

    ResistList resistList;
    Game* gamePtr;

    Timers timeManger;

    int subclassPoolIndex;
    int poolIndex;
    int aliveListIndex;
    int mapListIndex;

    int rankNum;
    int owner;
    float x,y;
    float hp;

    int typeId;
    virtual ~IStaticActor()=default;
    virtual void dead(){};
    virtual bool setRank(int rankOffest){return true;};
    virtual float getValue(int attributeId){ return 0.0f; };
    virtual void applyEffect(IStaticActor* staticActorPtr){};
    virtual void applyEffect(IMobileActor* mobileActorPtr){};
    virtual void checkEffect(){};
    virtual void setScope(){};
    virtual void beHurted(AttackType attackType,float attackNum){};
    virtual void getAttackGoal(std::array<std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>,2>& goalList){};
    virtual void attack(){};
    virtual void move(std::pair<float,float> goalPos){};
    virtual void skill1(){};
    virtual void skill2(){};
};

class IMobileActor{
    public:
    std::vector<std::array<int,2>> path;

    std::vector<std::array<int,2>> scopeList;//攻击范围覆盖的格子 非直接坐标而是偏移量[dx,dy] 获取真实坐标是x+dx,y+dy

    std::list<ISpecialEffect*> applyEffectList;//所拥有的特殊状态         对方析构时会将其结束时间置0
    std::list<ISpecialEffect*> receiveEffectList;//所施加的特殊状态       在自身析构时将造成的所有异常状态全部结束

    std::vector<Effective> nowAttributeList;

    Game* gamePtr;

    Resist resistState;//自身阻挡状态

    Timers timeManger;


    int subclassPoolIndex;
    int poolIndex;
    int aliveListIndex;
    int mapListIndex;

    int pathIndex;
    int rankNum;
    int owner;
    float x,y;
    float hp;


    int typeId;
    virtual ~IMobileActor()=default;
    virtual void dead(){};
    virtual bool setRank(int rankOffest){return true;};
    virtual float getValue(int attributeId){ return 0.0f; };
    virtual void applyEffect(IStaticActor* staticActorPtr){};
    virtual void applyEffect(IMobileActor* mobileActorPtr){};
    virtual void checkEffect(){};
    virtual void setScope(){};
    virtual void beHurted(AttackType attackType,float attackNum){};
    virtual void getAttackGoal(std::array<std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>,2>& goalList){};
    virtual void attack(){};
    virtual void move(std::pair<float,float> goalPos){};
    virtual void followPath(){};
    virtual void getPath(float goalX,float goalY,std::vector<std::array<int,2>>& resPath){};
    virtual void skill1(){};
    virtual void skill2(){};
};


class Game{
    public:
    std::vector<std::vector<uint8_t>> basicMap;//0为障碍物 1为空地
    std::unordered_map<std::string,std::vector<std::vector<uint8_t>>> mapTable;//存地图列表
    std::vector<std::vector<std::vector<int>>> staticActorMap;//存每个格子的固定单位索引列表
    std::vector<std::vector<std::vector<int>>> mobileActorMap;//存每个格子的移动单位索引列表

    //以下均用,且仅用于转为张量传入CNN
    std::deque<std::pair<std::vector<float>,std::vector<int64_t>>>tensor;//存近frameNum次操作帧 [(张量,形状)]
    std::deque<std::pair<std::vector<int>,std::vector<int64_t>>>resultOut;//存对应操作帧进行的操作[( (操作类型,操作对象,操作参数),形状)]

    std::vector<IStaticActor*> staticActorPool;
    std::vector<IMobileActor*> mobileActorPool;

    std::vector<std::vector<int>> aliveStaticList;
    std::vector<std::vector<int>> aliveMobileList;
    std::vector<int> eraseStaticActorSet;//后续进行sort与unique来处理 从大到小来进行
    std::vector<int> eraseMobileActorSet;//

    std::vector<float> nowCost;//根据owner作为索引来划分
    std::vector<float> costMax;
    std::vector<float> costMin;
    std::vector<float> costSpeed;
    std::vector<float> returnCostMul;//返回时消耗费用的倍率
    std::vector<float> moveCostMul;//移动单位消耗费用的倍率

    Draw* mainDrawPtr;
    
    int ownerCount;
    int frameNum=5;
    int mapIndex;
    float nowTime;
    float timeStep=0.1f;


    bool cnnSwitch;


    Game(std::string mapJsonPath="assets/MapData/Map.json",int mapIndex=0,std::string staticActorJsonPath="assets/ActorAttribute/StaticActor.json",std::string mobileActorJsonPath="assets/ActorAttribute/MobileActor.json",int ownerCount=2){//初始地图,仅01
        using json = nlohmann::json;
        std::ifstream mapFile(mapJsonPath);
        auto openFlag=mapFile.is_open();
        if(!openFlag){
            throw std::runtime_error("Failed to open the mapJsonPath");
        }
        this->mapTable=json::parse(mapFile).get<std::unordered_map<std::string, std::vector<std::vector<uint8_t>>>>();;

        this->basicMap=this->mapTable[std::to_string(mapIndex)];

        setBaseAttribute(staticActorJsonPath,mobileActorJsonPath);

        int h=int(basicMap.size());
        int w=int(basicMap[0].size());
        staticActorMap.resize(h,std::vector<std::vector<int>>(w));
        mobileActorMap.resize(h,std::vector<std::vector<int>>(w));

        this->ownerCount=ownerCount;
        nowCost.resize(ownerCount,0.0f);
        costMax.resize(ownerCount,1000.0f);
        costMin.resize(ownerCount,0.0f);
        costSpeed.resize(ownerCount,0.01);
        returnCostMul.resize(ownerCount,0.5f);
        moveCostMul.resize(ownerCount,0.7f);
        aliveStaticList.resize(ownerCount);
        aliveMobileList.resize(ownerCount);

        nowTime=0.0f;
        cnnSwitch=false;
        mainDrawPtr=nullptr;
        this->mapIndex=mapIndex;

    }
    void setMap(int index){
        this->basicMap=this->mapTable[std::to_string(index)];

    }

    void setBaseAttribute(std::string staticActorJsonPath="assets/ActorAttribute/StaticActor.json",std::string mobileActorJsonPath="assets/ActorAttribute/MobileActor.json"){
        using json = nlohmann::json;
        std::ifstream staticActorAttributeFile(staticActorJsonPath);
        std::ifstream mobileActorAttributeFile(mobileActorJsonPath);
        auto staticActorAttribute=json::parse(staticActorAttributeFile);
        auto mobileActorAttribute=json::parse(mobileActorAttributeFile);

        Attribute<SingleTower>::attributeList=std::move(staticActorAttribute["SingleTower"].get<std::vector<std::vector<float>>>());
        Attribute<GroupAttackTower>::attributeList=std::move(staticActorAttribute["GroupAttackTower"].get<std::vector<std::vector<float>>>());
        Attribute<SlowTower>::attributeList=std::move(staticActorAttribute["SlowTower"].get<std::vector<std::vector<float>>>());
        Attribute<CenterTower>::attributeList=std::move(staticActorAttribute["CenterTower"].get<std::vector<std::vector<float>>>());

        Attribute<MeleeMobile>::attributeList=std::move(mobileActorAttribute["MeleeMobile"].get<std::vector<std::vector<float>>>());
        Attribute<RangedMobile>::attributeList=std::move(mobileActorAttribute["RangedMobile"].get<std::vector<std::vector<float>>>());
        Attribute<DefenseMobile>::attributeList=std::move(mobileActorAttribute["DefenseMobile"].get<std::vector<std::vector<float>>>());
        Attribute<ExplosionMobile>::attributeList=std::move(mobileActorAttribute["ExplosionMobile"].get<std::vector<std::vector<float>>>());

    }

    void creatTensor(){
        //基础属性
        //对每个阵营 需要存储 当前费用(标量) 当前费用速率(标量) 当前操作类型(标量) 当前操作 静态和动态单位的布局 平均剩余生命值 平均攻击力 平均攻击范围 平均攻击速度
    }

    bool creatStaticActor(float x,float y,int owner,int StaticActorType);

    bool creatMobileActor(float x,float y,int owner,int MobileActorType);


    void eraseStaticActor(int index){
        eraseStaticActorSet.push_back(index);
        auto actorPtr=staticActorPool[index];
        nowCost[actorPtr->owner]+=actorPtr->getValue(11)*returnCostMul[actorPtr->owner];
        nowCost[actorPtr->owner]=std::min<float>(nowCost[actorPtr->owner],costMax[actorPtr->owner]);
        solveDeadActor();
    }

    void eraseMobileActor(int index){
        eraseMobileActorSet.push_back(index);
        auto actorPtr=mobileActorPool[index];
        nowCost[actorPtr->owner]+=actorPtr->getValue(11)*returnCostMul[actorPtr->owner];
        nowCost[actorPtr->owner]=std::min<float>(nowCost[actorPtr->owner],costMax[actorPtr->owner]);
        solveDeadActor();
    }


    void solveDeadActor(){
        std::sort(eraseStaticActorSet.begin(),eraseStaticActorSet.end(),std::greater<int>());
        eraseStaticActorSet.erase(std::unique(eraseStaticActorSet.begin(),eraseStaticActorSet.end()),eraseStaticActorSet.end());//去重
        for(auto index:eraseStaticActorSet){
            staticActorPool[index]->dead();
        }
        eraseStaticActorSet.clear();

        std::sort(eraseMobileActorSet.begin(),eraseMobileActorSet.end(),std::greater<int>());
        eraseMobileActorSet.erase(std::unique(eraseMobileActorSet.begin(),eraseMobileActorSet.end()),eraseMobileActorSet.end());//去重

        for(auto index:eraseMobileActorSet){
            mobileActorPool[index]->dead();

        }
        eraseMobileActorSet.clear();

    }



    void tick();

    void draw();

};



template<typename SubClass>
class StaticActor:public IStaticActor{//用于实现通用方法的模板类
    public:
    StaticActor(Game* gamePtr,int owner,float x,float y,SubClass* subclassPtr){

        this->gamePtr=gamePtr;
        this->owner=owner;
        this->x=x;
        this->y=y;
        this->hp=Attribute<SubClass>::attributeList[0][AttributeId<SubClass>::hp];

        typeId=TypeId<SubClass>::value;

        rankNum=0;
        //setRank由子类构造方法调用
        nowAttributeList.resize(Attribute<SubClass>::attributeList[0].size());

        subclassPoolIndex=ActorPool<SubClass>::Pool.size();
        poolIndex=gamePtr->staticActorPool.size();
        mapListIndex=gamePtr->staticActorMap[int(y)][int(x)].size();
        aliveListIndex=gamePtr->aliveStaticList[owner].size();

        gamePtr->staticActorPool.push_back(this);
        gamePtr->staticActorMap[int(y)][int(x)].push_back(poolIndex);
        gamePtr->aliveStaticList[owner].push_back(poolIndex);
    };

    virtual ~StaticActor(){
        resistList.clear();
        auto start=applyEffectList.begin();
        while(start!=applyEffectList.end()){
            auto&temp =*start;
            temp->endTime=0;//将自身所施加的异常状态置0
            ++start;
        }
        start=receiveEffectList.begin();
        while(start!=receiveEffectList.end()){
            auto& temp =*start;
            temp->removeApply();//将自身所受的异常状态从施加方删除
            ++start;
        }
    }

    virtual void dead()override{
        this->_dead(static_cast<SubClass*>(this));
    };

    void _dead(SubClass* subclassPtr){
        //删除在staticActorMap[y][x]中的索引
        gamePtr->staticActorPool[gamePtr->staticActorMap[int(this->y)][int(this->x)][gamePtr->staticActorMap[int(this->y)][int(this->x)].size()-1]]->mapListIndex=this->mapListIndex;
        erase_basedSwap(gamePtr->staticActorMap[int(this->y)][int(this->x)],this->mapListIndex);

        //删除在aliveActorPool[owner]中的索引
        gamePtr->staticActorPool[gamePtr->aliveStaticList[owner][gamePtr->aliveStaticList[owner].size()-1]]->aliveListIndex=this->aliveListIndex;
        erase_basedSwap(gamePtr->aliveStaticList[owner],this->aliveListIndex);
        //删除对象本身(函数中有对该数组的引用作为参数，所以pop不会有空引用问题)
        erase_basedSwap(ActorPool<SubClass>::Pool,subclassPoolIndex);
    }

    virtual bool setRank(int rankOffest=0)override{
        return _setRank(static_cast<SubClass*>(this),rankOffest);
    }

    bool _setRank(SubClass* thisPtr,int rankOffest=0){
        auto& attributeList=Attribute<SubClass>::attributeList;
        if(rankOffest!=0){
            if(rankNum+rankOffest>attributeList.size()-1||rankNum+rankOffest<0){
                throw std::runtime_error(std::format("[StaticActor][setRank]the rankNum={} rankOffest={} and rankMax is {}",rankNum,rankOffest,attributeList.size()));
                return false;
            }
            float costOffest=0.0;
            if(rankOffest>0){
                for(int i=1;i<=rankOffest;i++){//统计[rankNum+1,rankNum+rankOffest]区间内的费用消耗
                    costOffest-=attributeList[rankNum+i][9];
                }
            }
            else{//降级返费
                for(int i=0;i<-rankOffest;i++){//统计[rankNum,rankNum+|rankOffest|+1]区间内的费用消耗
                    costOffest+=attributeList[rankNum-i][9]*gamePtr->returnCostMul[this->owner];
                }
            }
            if(gamePtr->nowCost[this->owner]+costOffest<gamePtr->costMin[this->owner]){//费用低于临界
                // throw std::runtime_error(std::format("[StaticActor][setRank]the nowCost={} costOffest={} and rankMax={} the rankMin={}",gamePtr->nowCost[this->owner],costOffest,gamePtr->costMax[this->owner],gamePtr->costMin[this->owner]));
                return false;
            }
            else{
                if(gamePtr->nowCost[this->owner]+costOffest>gamePtr->costMax[this->owner]){
                    gamePtr->nowCost[this->owner]=gamePtr->costMax[this->owner];//置为最大值
                }
                else{gamePtr->nowCost[this->owner]+=costOffest;
                }//合法情况
            }
            rankNum+=rankOffest;
        }
        this->hp=Attribute<SubClass>::attributeList[rankNum][AttributeId<SubClass>::hp];

        for(auto& tempAttribute:nowAttributeList){
            tempAttribute.checkFlag=false;
        }

        return true;
    };

    virtual float getValue(int attributeId)override{
        auto& tempData=this->nowAttributeList[attributeId];
        if(!tempData.checkFlag){
            tempData.updateValue(Attribute<SubClass>::attributeList[rankNum][attributeId]);
            if(attributeId==AttributeId<SubClass>::attackScope)this->setScope();
        }
        return tempData.value;
    };

    virtual void applyEffect(IStaticActor* staticActorPtr)override{
        return;
    };

    virtual void applyEffect(IMobileActor* mobileActorPtr)override{
        return;
    };

    virtual void checkEffect()override{
        auto start=this->receiveEffectList.begin();

        while(start!=this->receiveEffectList.end()){
            auto& effect=*start;
            start=effect->check();//检测/析构
        }
    }

    virtual void setScope()override{//静态,有超出边界不加入
        scopeList.clear();
        auto scope=this->getValue(AttributeId<SubClass>::attackScope);
        int r=int(scope);
        for(int dx=-r;dx<=r;dx++){
            int dy_max=int(sqrt(scope*scope-dx*dx));
            for(int dy=-dy_max;dy<=dy_max;dy++){
                if(x+dx<0||x+dx>=gamePtr->basicMap[0].size()||y+dy<0||y+dy>=gamePtr->basicMap.size()){
                    continue;
                }
                scopeList.push_back({dx,dy});
            }
        }
    }

    virtual void beHurted(AttackType attackType,float attackNum)override{
        switch(attackType){
            case AttackType::Phys:
                attackNum=attackNum-Attribute<SubClass>::attributeList[rankNum][0];
                attackNum=std::max<float>(attackNum,0.0f);
                break;
            case AttackType::Magic:
                attackNum=attackNum*(1-Attribute<SubClass>::attributeList[rankNum][1]);
                attackNum=std::max<float>(attackNum,0.0f);
                break;
            case AttackType::Pure:
                break;
            default:
                throw std::runtime_error(std::format("[ERROR][StaticActor] the attackType is {}",static_cast<int>(attackType)));
        }
        this->hp-=attackNum;
    }

    virtual void getAttackGoal(std::array<std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>,2>& goalList)override{
        auto&  mobileAttackQueue=goalList[1];

        auto attackSpeed=this->getValue(AttributeId<SubClass>::attackSpeed);
        auto attackScope=this->getValue(AttributeId<SubClass>::attackScope);
        auto attackCount=this->getValue(AttributeId<SubClass>::attackCount);

        auto& mobileActorMap=gamePtr->mobileActorMap;

        for(auto [x,y]:scopeList){
            for(auto index:mobileActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->mobileActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<attackScope*attackScope){
                    mobileAttackQueue.emplace(d2,index);
                    while(mobileAttackQueue.size()>attackCount){
                        mobileAttackQueue.pop();
                    }
                }
            }
        }

        int residualAttackCount=attackCount-int64_t(mobileAttackQueue.size());
        if(residualAttackCount==0){//攻击数量足够
            return;
        }

        auto&  staticAttackQueue=goalList[0];
        auto& staticActorMap=gamePtr->staticActorMap;

        for(auto [x,y]:scopeList){
            for(auto index:staticActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<attackScope*attackScope){
                    staticAttackQueue.emplace(d2,index);
                    while(staticAttackQueue.size()>residualAttackCount){
                        staticAttackQueue.pop();
                    }
                }
            }
        }
    };


    virtual void attack()override{

        auto& nowTime=gamePtr->nowTime;
        auto attackSpeed=this->getValue(AttributeId<SubClass>::attackSpeed);

        auto& lastAttackTime=this->timeManger[TimeType::Attack];

        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }
        auto attackNum=this->getValue(AttributeId<SubClass>::attackNum);
        auto attackType=static_cast<AttackType>(this->getValue(AttributeId<SubClass>::attackType));

        auto& eraseMobileActorSet=gamePtr->eraseMobileActorSet;
        auto& eraseStaticActorSet=gamePtr->eraseStaticActorSet;
        std::array<std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>,2> goalList;
        getAttackGoal(goalList);//距离,索引

        while(goalList[1].size()>0){//优先级为 先攻击移动单位，再攻击固定单位 其次，优先攻击最近
            auto [d,index]=goalList[1].top();
            goalList[1].pop();
            auto& actor=gamePtr->mobileActorPool[index];
            if(actor->hp<=0){
                continue;
            }
            actor->beHurted(attackType,attackNum);
            this->applyEffect(actor);
            if(actor->hp<=0){
                eraseMobileActorSet.push_back(index);
            }
        }

        while(goalList[0].size()>0){
            auto [d,index]=goalList[0].top();
            goalList[0].pop();
            auto& actor=gamePtr->staticActorPool[index];
            if(actor->hp<=0){
                continue;
            }
            actor->beHurted(attackType,attackNum);
            this->applyEffect(actor);
            if(actor->hp<=0){
                eraseStaticActorSet.push_back(index);
            }
        }

        this->timeManger[TimeType::Attack]=nowTime;
    }


    virtual void move(std::pair<float,float> goalPos)override{
        gamePtr->staticActorPool[gamePtr->staticActorMap[int(this->y)][int(this->x)][gamePtr->staticActorMap[int(this->y)][int(this->x)].size()-1]]->mapListIndex=this->mapListIndex;
        erase_basedSwap(gamePtr->staticActorMap[int(this->y)][int(this->x)],this->mapListIndex);
        this->x=goalPos.first;
        this->y=goalPos.second;
        this->setScope();
        this->mapListIndex=gamePtr->staticActorMap[int(this->y)][int(this->x)].size();
        gamePtr->staticActorMap[int(this->y)][int(this->x)].push_back(poolIndex);
    };

    virtual void skill1()override{};
    virtual void skill2()override{};

};
template<typename SubClass>
class MobileActor:public IMobileActor{
    public:


    MobileActor(Game* gamePtr,int owner,float x,float y,SubClass* subclassPtr){

        this->gamePtr=gamePtr;
        this->owner=owner;
        this->x=x;
        this->y=y;

        this->hp=Attribute<SubClass>::attributeList[0][AttributeId<SubClass>::hp];

        typeId=TypeId<SubClass>::value;
        rankNum=0;
        nowAttributeList.resize(Attribute<SubClass>::attributeList[0].size());

        subclassPoolIndex=ActorPool<SubClass>::Pool.size();
        poolIndex=gamePtr->mobileActorPool.size();
        mapListIndex=gamePtr->mobileActorMap[int(y)][int(x)].size();
        aliveListIndex=gamePtr->aliveMobileList[owner].size();

        gamePtr->mobileActorPool.push_back(this);
        gamePtr->mobileActorMap[int(y)][int(x)].push_back(poolIndex);
        gamePtr->aliveMobileList[owner].push_back(poolIndex);
    }

    virtual ~MobileActor(){
        this->resistState.removeResist();
        auto start=applyEffectList.begin();
        while(start!=applyEffectList.end()){
            auto&temp =*start;
            temp->endTime=0;//将自身所施加的异常状态置0
            ++start;
        }
        start=receiveEffectList.begin();
        while(start!=receiveEffectList.end()){
            auto& temp =*start;
            temp->removeApply();//将自身所受的异常状态从施加方删除
            ++start;
        }
        this->resistState.removeResist();
    }
    virtual void dead()override{
        this->_dead(static_cast<SubClass*>(this));
    };

    void _dead(SubClass* subclassPtr){
        //删除在staticActorMap[y][x]中的索引
        gamePtr->mobileActorPool[gamePtr->mobileActorMap[int(this->y)][int(this->x)][gamePtr->mobileActorMap[int(this->y)][int(this->x)].size()-1]]->mapListIndex=this->mapListIndex;
        erase_basedSwap(gamePtr->mobileActorMap[int(this->y)][int(this->x)],this->mapListIndex);

        //删除在aliveActorPool[owner]中的索引
        gamePtr->mobileActorPool[gamePtr->aliveMobileList[owner][gamePtr->aliveMobileList[owner].size()-1]]->aliveListIndex=this->aliveListIndex;
        erase_basedSwap(gamePtr->aliveMobileList[owner],this->aliveListIndex);
        //删除对象本身(函数中有对该数组的引用作为参数，所以pop不会有空引用问题)
        erase_basedSwap(ActorPool<SubClass>::Pool,subclassPoolIndex);
    }

    virtual bool setRank(int rankOffest=0)override{
        return _setRank(static_cast<SubClass*>(this),rankOffest);
    }

    bool _setRank(SubClass* thisPtr,int rankOffest=0){
        auto& attributeList=Attribute<SubClass>::attributeList;
        if(rankOffest!=0){
            if(rankNum+rankOffest>attributeList.size()-1||rankNum+rankOffest<0){
                throw std::runtime_error(std::format("[StaticActor][setRank]the rankNum={} rankOffest={} and rankMax is {}",rankNum,rankOffest,attributeList.size()));
                return false;
            }
            float costOffest=0.0;
            if(rankOffest>0){
                for(int i=1;i<=rankOffest;i++){//统计[rankNum+1,rankNum+rankOffest]区间内的费用消耗
                    costOffest-=attributeList[rankNum+i][9];
                }
            }
            else{//降级返费
                for(int i=0;i<-rankOffest;i++){//统计[rankNum,rankNum+|rankOffest|+1]区间内的费用消耗
                    costOffest+=attributeList[rankNum-i][9]*gamePtr->returnCostMul[this->owner];
                }
            }
            if(gamePtr->nowCost[this->owner]+costOffest<gamePtr->costMin[this->owner]){//费用低于临界
                // throw std::runtime_error(std::format("[StaticActor][setRank]the nowCost={} costOffest={} and rankMax={} the rankMin={}",gamePtr->nowCost[this->owner],costOffest,gamePtr->costMax[this->owner],gamePtr->costMin[this->owner]));
                return false;
            }
            else{
                if(gamePtr->nowCost[this->owner]+costOffest>gamePtr->costMax[this->owner]){
                    gamePtr->nowCost[this->owner]=gamePtr->costMax[this->owner];//置为最大值
                }
                else{gamePtr->nowCost[this->owner]+=costOffest;}//合法情况
            }
            rankNum+=rankOffest;
        }
        this->hp=Attribute<SubClass>::attributeList[rankNum][AttributeId<SubClass>::hp];

        for(auto& tempAttribute:nowAttributeList){
            tempAttribute.checkFlag=false;
        }

        return true;
    };

    virtual float getValue(int attributeId)override{
        auto& tempData=this->nowAttributeList[attributeId];
        if(!tempData.checkFlag){
            tempData.updateValue(Attribute<SubClass>::attributeList[rankNum][attributeId]);
            if(attributeId==AttributeId<SubClass>::attackScope)this->setScope();
        }
        return tempData.value;
    };

    virtual void applyEffect(IStaticActor* staticActorPtr)override{
        return;
    };

    virtual void applyEffect(IMobileActor* mobileActorPtr)override{
        return;
    };

    virtual void checkEffect()override{
       auto start=this->receiveEffectList.begin();

        while(start!=this->receiveEffectList.end()){
            auto& effect=*start;
            start=effect->check();//检测/析构
        }
    }

    virtual void setScope()override{//动态,在调用时检测边界
        scopeList.clear();
        auto scope=this->getValue(AttributeId<SubClass>::attackScope);
        int r=int(scope);
        for(int dx=-r;dx<=r;dx++){
            int dy_max=int(sqrt(scope*scope-dx*dx));
            for(int dy=-dy_max;dy<=dy_max;dy++){
                scopeList.push_back({dx,dy});
            }
        }
    }

    virtual void beHurted(AttackType attackType,float attackNum)override{
        switch(attackType){
            case AttackType::Phys:
                attackNum=attackNum-Attribute<SubClass>::attributeList[rankNum][0];
                attackNum=std::max<float>(attackNum,0.0f);
                break;
            case AttackType::Magic:
                attackNum=attackNum*(1-Attribute<SubClass>::attributeList[rankNum][1]);
                attackNum=std::max<float>(attackNum,0.0f);
                break;
            case AttackType::Pure:
                break;
            default:
                throw std::runtime_error(std::format("[ERROR][StaticActor] the attackType is {}",static_cast<int>(attackType)));
        }
        this->hp-=attackNum;
    }

    virtual void getAttackGoal(std::array<std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>,2>& goalList)override{
        auto&  mobileAttackQueue=goalList[1];

        auto attackSpeed=this->getValue(AttributeId<SubClass>::attackSpeed);
        auto attackScope=this->getValue(AttributeId<SubClass>::attackScope);
        auto attackCount=this->getValue(AttributeId<SubClass>::attackCount);

        auto& mobileActorMap=gamePtr->mobileActorMap;

        std::vector<std::array<int,2>> scopeList;//存真实攻击范围
        for(auto [x,y]:this->scopeList){//动态获取实时攻击范围
            if(this->x+x<0||this->x+x>=gamePtr->basicMap[0].size()||this->y+y<0||this->y+y>=gamePtr->basicMap.size()){
                continue;
            }
            scopeList.push_back({x,y});
        }

        for(auto [x,y]:scopeList){
            for(auto index:mobileActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->mobileActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<attackScope*attackScope){
                    mobileAttackQueue.emplace(d2,index);
                    while(mobileAttackQueue.size()>attackCount){
                        mobileAttackQueue.pop();
                    }
                }
            }
        }

        int residualAttackCount=attackCount-int64_t(mobileAttackQueue.size());
        if(residualAttackCount==0){//攻击数量足够
            return;
        }

        auto&  staticAttackQueue=goalList[0];
        auto& staticActorMap=gamePtr->staticActorMap;

        for(auto [x,y]:scopeList){
            for(auto index:staticActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<attackScope*attackScope){
                    staticAttackQueue.emplace(d2,index);
                    while(staticAttackQueue.size()>residualAttackCount){
                        staticAttackQueue.pop();
                    }
                }
            }
        }
    };

    virtual void attack()override{
        int attackCount=int(this->getValue(AttributeId<SubClass>::attackCount));//取整

        auto& nowTime=gamePtr->nowTime;
        auto attackSpeed=this->getValue(AttributeId<SubClass>::attackSpeed);

        auto& lastAttackTime=this->timeManger[TimeType::Attack];

        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }
        auto attackNum=this->getValue(AttributeId<SubClass>::attackNum);
        auto attackType=static_cast<AttackType>(this->getValue(AttributeId<SubClass>::attackType));

        auto& mobileActorMap=gamePtr->mobileActorMap;
        auto& eraseMobileActorSet=gamePtr->eraseMobileActorSet;
        std::array<std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>,2> goalList;
        getAttackGoal(goalList);//距离,索引

        while(goalList[1].size()>0){//优先级为 先攻击移动单位，再攻击固定单位 其次，优先攻击最近
            auto [d,index]=goalList[1].top();
            goalList[1].pop();
            auto& actor=gamePtr->mobileActorPool[index];
            if(actor->hp<=0){
                continue;
            }
            actor->beHurted(attackType,attackNum);
            this->applyEffect(actor);
            if(actor->hp<=0){
                eraseMobileActorSet.push_back(index);
            }
        }

        while(goalList[0].size()>0){
            auto [d,index]=goalList[0].top();
            goalList[0].pop();
            auto& actor=gamePtr->staticActorPool[index];
            if(actor->hp<=0){
                continue;
            }
            actor->beHurted(attackType,attackNum);
            this->applyEffect(actor);
            if(actor->hp<=0){
                gamePtr->eraseStaticActorSet.push_back(index);
            }
        }

        this->timeManger[TimeType::Attack]=nowTime;

    }

    virtual void move(std::pair<float,float> goalPos)override{
        this->path.clear();
        gamePtr->mobileActorPool[gamePtr->mobileActorMap[int(this->y)][int(this->x)][gamePtr->mobileActorMap[int(this->y)][int(this->x)].size()-1]]->mapListIndex=this->mapListIndex;
        erase_basedSwap(gamePtr->mobileActorMap[int(this->y)][int(this->x)],this->mapListIndex);
        this->x=goalPos.first;
        this->y=goalPos.second;
        this->setScope();
        this->mapListIndex=gamePtr->mobileActorMap[int(this->y)][int(this->x)].size();
        gamePtr->mobileActorMap[int(this->y)][int(this->x)].push_back(poolIndex);
        this->resistState.removeResist();
        for(auto index:gamePtr->staticActorMap[int(this->y)][int(this->x)]){//阻挡相关
                if(this->resistState.state==true){
                    break;
                }
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner!=this->owner){
                    actor->resistList.push(this->resistState);
                    break;
                }
            }
    }

    virtual void followPath()override{//移动以0.1s为单位
        if(path.size()!=0&&pathIndex<path.size()-1&&resistState.state==false){//有路&&未走完&&未阻挡
            float dx=path[pathIndex][0]-this->x;
            float dy=path[pathIndex][1]-this->y;
            float d=sqrt(dx*dx+dy*dy);
            float newX=this->x+dx/d*this->getValue(AttributeId<SubClass>::moveSpeed)*gamePtr->timeStep;
            float newY=this->y+dy/d*this->getValue(AttributeId<SubClass>::moveSpeed)*gamePtr->timeStep;
            //暂时没有合适的方案来修复速度过快导致的越界/进入障碍物的问题 保留，等待以后修复
            // //修正x
            // if(gamePtr->basicMap[int(newY)][int(newX)]==1){
            //    newX;
            // }

            if(int(newX)!=int(this->x)||int(newY)!=int(this->y)){
                gamePtr->mobileActorPool[gamePtr->mobileActorMap[int(this->y)][int(this->x)][gamePtr->mobileActorMap[int(this->y)][int(this->x)].size()-1]]->mapListIndex=this->mapListIndex;
                erase_basedSwap(gamePtr->mobileActorMap[int(this->y)][int(this->x)],this->mapListIndex);

                this->mapListIndex=gamePtr->mobileActorMap[int(newY)][int(newX)].size();
                gamePtr->mobileActorMap[int(newY)][int(newX)].push_back(poolIndex);
            }
            this->x=newX;
            this->y=newY;
            for(auto index:gamePtr->staticActorMap[int(this->y)][int(this->x)]){//阻挡相关
                if(this->resistState.state==true){
                    break;
                }
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner!=this->owner){
                    actor->resistList.push(this->resistState);
                    break;
                }
            }
            if(d<=this->getValue(AttributeId<SubClass>::moveSpeed)*gamePtr->timeStep){
                pathIndex++;
            }
        }
    }

    virtual void getPath(float goalX,float goalY,std::vector<std::array<int,2>>&resPath)override{
        this->resistState.removeResist();//主动清除被阻挡状态
        // this->getPath_dfs(goalX,goalY,resPath);
        this->aStar(goalX,goalY,resPath);
        this->pathIndex=0;
    }

    void aStar(float goalX,float goalY,std::vector<std::array<int,2>>& resPath){
        int startX=int(this->x);
        int startY=int(this->y);
        struct PairHash {
            inline std::size_t operator()(const std::pair<int,int>& v) const {
                return std::hash<int>()(v.first) ^ (std::hash<int>()(v.second) << 1);
            }
        };



        std::priority_queue<std::tuple<float,int,int>,std::vector<std::tuple<float,int,int>>,std::greater<std::tuple<float,int,int>>> openSet;//f,g,x,y
        std::unordered_set<std::pair<int,int>,PairHash> closedSet;
        std::vector<std::vector<std::pair<int,int>>> parent(gamePtr->basicMap.size(),std::vector<std::pair<int,int>>(gamePtr->basicMap[0].size(),{-1,-1}));
        std::vector<std::vector<int>> g(gamePtr->basicMap.size(),std::vector<int>(gamePtr->basicMap[0].size(),-1));
        int dx[4]={0,1,0,-1};
        int dy[4]={1,0,-1,0};
        bool findFlag=false;
        g[startY][startX]=0;
        openSet.push({abs(goalX-startX)+abs(goalY-startY),startX,startY});

        while(openSet.size()!=0&&!findFlag){
            auto [f,tempX,tempY]=openSet.top();
            closedSet.insert({tempX,tempY});
            openSet.pop();
            for(int i=0;i<4;i++){
                int nextX=tempX+dx[i];
                int nextY=tempY+dy[i];
                auto tempLenth=g[tempY][tempX]+1;
                if(nextX==goalX&&nextY==goalY){
                    parent[nextY][nextX]={tempX,tempY};
                    g[nextY][nextX]=tempLenth;
                    findFlag=true;
                    break;
                }

                if(nextX<0||nextX>=gamePtr->basicMap[0].size()||nextY<0||nextY>=gamePtr->basicMap.size()||gamePtr->basicMap[nextY][nextX]==0||!(closedSet.count({nextX,nextY})==0||(closedSet.count({nextX,nextY})!=0&&g[nextY][nextX]>tempLenth))){
                    continue;
                }
                g[nextY][nextX]=tempLenth;
                parent[nextY][nextX]={tempX,tempY};
                openSet.push(std::tuple<float,int,int>(tempLenth+abs(goalX-nextX)+abs(goalY-nextY),nextX,nextY));
            }
        }
        auto goalLenth=g[goalY][goalX];
        resPath.resize(goalLenth);
        auto tempPos=std::array<int,2>{int(goalX),int(goalY)};
        for(int _th=goalLenth-1;_th>=0;_th--){
            resPath[_th]=tempPos;
            auto partentPos=parent[tempPos[1]][tempPos[0]];
            tempPos=std::array<int,2>{partentPos.first,partentPos.second};
        }
    };

    void getPath_dfs(float goalX,float goalY,std::vector<std::array<int,2>>& resPath){
        int minStep=1000000;
        std::vector<std::vector<bool>>visited(gamePtr->basicMap.size(),std::vector<bool>(gamePtr->basicMap[0].size(),false));
        dfs(x,y,goalX,goalY,minStep,0,visited,resPath,resPath);
    }

    void dfs(int nowX,int nowY,int goalX,int goalY,int minStep,int moveStep,std::vector<std::vector<bool>>&visited,std::vector<std::array<int,2>>tempPath,std::vector<std::array<int,2>>&resPath){
        if(nowX==goalX&&nowY==goalY){
            if(moveStep<minStep){
                resPath=tempPath;
                minStep=moveStep;
            }
            return;
        }
        auto& basicMap=gamePtr->basicMap;//暂时先用basicMap来判断,之后可以改成专门的地图数据结构
        if(nowX<0||nowX>=basicMap[0].size()||nowY<0||nowY>=basicMap.size()||basicMap[nowY][nowX]==0||visited[nowY][nowX]){
            return;
        }
        int dx[4]={0,1,0,-1};
        int dy[4]={1,0,-1,0};
        visited[nowY][nowX]=true;

        for(int i=0;i<4;i++){
            int nextX=nowX+dx[i];
            int nextY=nowY+dy[i];
            if(nextX<0||nextX>=basicMap[0].size()||nextY<0||nextY>=basicMap.size()||basicMap[nextY][nextX]==0||visited[nextY][nextX]){//不绕开敌方防御塔,可能被阻挡 路径完全自定义
                continue;
            }
            std::array<int,2> pos{nextX,nextY};
            tempPath.push_back(pos);
            dfs(nextX,nextY,goalX,goalY,minStep,moveStep+1,visited,tempPath,resPath);
         }
        visited[nowY][nowX]=false;
    }

    virtual void skill1()override{};
    virtual void skill2()override{};

};

class SingleTower:public StaticActor<SingleTower>{//单体攻击
    public:
    SingleTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y,this){

    }
};

class GroupAttackTower:public StaticActor<GroupAttackTower>{//群攻
    public:
    std::vector<std::array<int,2>> groupAttackArea;//溅射攻击范围

    GroupAttackTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y,this){

    }

    void set_groupAttackArea(){
        groupAttackArea.clear();
        auto groupAttackScope=this->getValue(AttributeId<GroupAttackTower>::GroupArrackScope);
        int r=int(groupAttackScope);
        for(int dx=-r;dx<=r;dx++){
            int dy_max=int(sqrt(groupAttackScope*groupAttackScope-dx*dx));
            for(int dy=-dy_max;dy<=dy_max;dy++){
                groupAttackArea.push_back({dx,dy});
            }
        }
    }
    virtual void attack()override{

        auto& nowTime=gamePtr->nowTime;
        auto attackSpeed=this->getValue(AttributeId<GroupAttackTower>::attackSpeed);
        auto groupAttackScope=this->getValue(AttributeId<GroupAttackTower>::GroupArrackScope);


        auto& lastAttackTime=this->timeManger[TimeType::Attack];

        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }
        auto attackNum=this->getValue(AttributeId<GroupAttackTower>::attackNum);
        auto attackType=static_cast<AttackType>(this->getValue(AttributeId<GroupAttackTower>::attackType));

        auto& eraseMobileActorSet=gamePtr->eraseMobileActorSet;
        auto& eraseStaticActorSet=gamePtr->eraseStaticActorSet;
        std::array<std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>,2> goalList;
        getAttackGoal(goalList);//距离,索引

        std::vector<int> mobileGroupAttackList;
        std::vector<int> staticGroupAttackList;

        while(goalList[1].size()>0){
            auto [d,index]=goalList[1].top();
            auto& mainActor=gamePtr->mobileActorPool[index];
            for(auto& [x,y]:groupAttackArea){
                if(mainActor->x+x<0||mainActor->x+x>=gamePtr->basicMap[0].size()||mainActor->y+y<0||mainActor->y+y>=gamePtr->basicMap.size()){
                    continue;
                }
                for(auto& index_map:gamePtr->mobileActorMap[int(mainActor->y+y)][int(mainActor->x+x)]){
                    auto& actor=gamePtr->mobileActorPool[index_map];
                    if(actor->owner==owner){
                        continue;
                    }
                    float d2=abs(mainActor->x+x-actor->x)*abs(mainActor->x+x-actor->x)+abs(mainActor->y+y-actor->y)*abs(mainActor->y+y-actor->y);
                    if(d2<groupAttackScope*groupAttackScope){
                        staticGroupAttackList.push_back(index_map);
                    }
                }
            }
            goalList[1].pop();
        }
        while(goalList[0].size()>0){
            auto [d,index]=goalList[0].top();
            auto& mainActor=gamePtr->staticActorPool[index];
            for(auto& [x,y]:groupAttackArea){
                if(mainActor->x+x<0||mainActor->x+x>=gamePtr->basicMap[0].size()||mainActor->y+y<0||mainActor->y+y>=gamePtr->basicMap.size()){
                    continue;
                }
                for(auto& index_map:gamePtr->staticActorMap[int(mainActor->y+y)][int(mainActor->x+x)]){
                    auto& actor=gamePtr->staticActorPool[index_map];
                    if(actor->owner==owner){
                        continue;
                    }
                    float d2=abs(mainActor->x+x-actor->x)*abs(mainActor->x+x-actor->x)+abs(mainActor->y+y-actor->y)*abs(mainActor->y+y-actor->y);
                    if(d2<groupAttackScope*groupAttackScope){
                        staticGroupAttackList.push_back(index_map);
                    }
                }
            }
            goalList[0].pop();
        }
        // std::sort(mobileGroupAttackList.begin(),mobileGroupAttackList.end());
        // mobileGroupAttackList.erase(std::unique(mobileGroupAttackList.begin(),mobileGroupAttackList.end()),mobileGroupAttackList.end());
        // std::sort(staticGroupAttackList.begin(),staticGroupAttackList.end());
        // staticGroupAttackList.erase(std::unique(staticGroupAttackList.begin(),staticGroupAttackList.end()),staticGroupAttackList.end());
        //不去重 交集算受到两次溅射伤害


        for(auto i=0ULL;i<mobileGroupAttackList.size();i++){
            auto index=mobileGroupAttackList[i];
            auto& actor=gamePtr->mobileActorPool[index];
            if(actor->hp<=0){
                continue;
            }
            actor->beHurted(attackType,attackNum);
            this->applyEffect(actor);
            if(actor->hp<=0){
                eraseMobileActorSet.push_back(index);
            }
        }
        for(auto i=0ULL;i<staticGroupAttackList.size();i++){
            auto index=staticGroupAttackList[i];
            auto& actor=gamePtr->staticActorPool[index];
            if(actor->hp<=0){
                continue;
            }
            actor->beHurted(attackType,attackNum);
            this->applyEffect(actor);
            if(actor->hp<=0){
                eraseStaticActorSet.push_back(index);
            }
        }
    }

};
class SlowTower:public StaticActor<SlowTower>{//减速
    public:
    SlowTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y,this){
    }

    virtual void applyEffect(IStaticActor* staticActorPtr) override{
        auto attackSlowMul=this->getValue(AttributeId<SlowTower>::attackSlowMul);
        auto attackSlowTime=this->getValue(AttributeId<SlowTower>::attackSlowTime);
        auto moveSlowMul=this->getValue(AttributeId<SlowTower>::moveSlowMul);
        auto moveSlowTime=this->getValue(AttributeId<SlowTower>::moveSlowTime);
        auto ptr=new SpecialEffect{7,ModType::mul,attackSlowMul,gamePtr->nowTime+attackSlowTime,this,staticActorPtr};
        return;
    };

    virtual void applyEffect(IMobileActor* mobileActorPtr)override{
        auto attackSlowMul=this->getValue(AttributeId<SlowTower>::attackSlowMul);
        auto attackSlowTime=this->getValue(AttributeId<SlowTower>::attackSlowTime);
        auto moveSlowMul=this->getValue(AttributeId<SlowTower>::moveSlowMul);
        auto moveSlowTime=this->getValue(AttributeId<SlowTower>::moveSlowTime);
        auto ptr=new SpecialEffect{7,ModType::mul,attackSlowMul,gamePtr->nowTime+attackSlowTime,this,mobileActorPtr};
        auto ptr1=new SpecialEffect{7,ModType::mul,moveSlowMul,gamePtr->nowTime+moveSlowTime,this,mobileActorPtr};
        return;
    };

};
class CenterTower:public StaticActor<CenterTower>{//枢纽
    public:
    CenterTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y,this){

    }

    void attack() override{
        return;
    }
};



class MeleeMobile:public MobileActor<MeleeMobile>{//近战
    public:
    MeleeMobile(Game* gamePtr,int owner,float x,float y):MobileActor(gamePtr,owner,x,y,this){

    }

};
class RangedMobile:public MobileActor<RangedMobile>{//远程
    public:
    RangedMobile(Game* gamePtr,int owner,float x,float y):MobileActor(gamePtr,owner,x,y,this){
    }

};
class DefenseMobile:public MobileActor<DefenseMobile>{//防御
    public:
    DefenseMobile(Game* gamePtr,int owner,float x,float y):MobileActor(gamePtr,owner,x,y,this){
    }

};
class ExplosionMobile:public MobileActor<ExplosionMobile>{//自爆
    public:
    float explosionNum;

    ExplosionMobile(Game* gamePtr,int owner,float x,float y):MobileActor(gamePtr,owner,x,y,this){
    }


    void beHurted(AttackType attackType,float attackNum) override{
        MobileActor::beHurted(attackType,attackNum);
        auto explosionNum=this->getValue(AttributeId<ExplosionMobile>::explosionNum);
        if(this->hp<=0){
            this->nowAttributeList[AttributeId<ExplosionMobile>::explosionNum].add2=this->getValue(AttributeId<ExplosionMobile>::explosionNum)-this->getValue(AttributeId<ExplosionMobile>::attackNum);
            this->nowAttributeList[AttributeId<ExplosionMobile>::explosionNum].checkFlag=false;
            this->timeManger[TimeType::Attack]=-1000.0f;

            this->nowAttributeList[AttributeId<ExplosionMobile>::attackScope].mul=3;
            this->nowAttributeList[AttributeId<ExplosionMobile>::attackScope].checkFlag=false;

            this->nowAttributeList[AttributeId<ExplosionMobile>::attackCount].add1=997;
            this->nowAttributeList[AttributeId<ExplosionMobile>::attackCount].checkFlag=false;

            this->attack();
        }
    }

};


class Texture2D
{
public:
    Texture2D() = default;

    Texture2D(ID3D11Device* device, const wchar_t* path)
    {
        Load(device, path);
    }

    bool Load(ID3D11Device* device, const wchar_t* path)
    {
        HRESULT hr = DirectX::CreateWICTextureFromFile(
            device,
            path,
            nullptr,
            &m_srv
        );

        return SUCCEEDED(hr);
    }

    ID3D11ShaderResourceView* GetSRV() const
    {
        return m_srv.Get();
    }

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
};

class Draw{
public:
    Game* gamePtr;

    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;
    HWND hwnd;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
    int windowWidth,windowHeight;

    std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
    std::unique_ptr<DirectX::CommonStates> commonStates;

    Texture2D emptyTex;
    Texture2D wallTex;
    Texture2D singleTowerTex;
    Texture2D groupAttackTowerTex;
    Texture2D slowTowerTex;
    Texture2D centerTowerTex;
    Texture2D meleeMobileTex;
    Texture2D rangedMobileTex;
    Texture2D defenseMobileTex;
    Texture2D explosionMobileTex;

    float cellSize=64.0f;
    float spriteScale=1.0f;
    float lastFrameTime=0;
    float accumLastFrameTime=0;

    //输入缓存
    std::array<float,4> creatStaticActorInput{};
    std::array<float,4> creatMobileActorInput{};
    std::array<float,2> moveStaticActorInput{};
    std::array<float,2> moveMobileActorInput{};
    std::array<float,2> getPathInput{};
    float setRankStaticActorInput=0;
    float setRankMobileActorInput=0;



    Draw(Game* gamePtr,ID3D11Device* device,ID3D11DeviceContext* context,IDXGISwapChain* swapChain,HWND hwnd,int windowWidth,int windowHeight):gamePtr(gamePtr),device(device),context(context),swapChain(swapChain),hwnd(hwnd),windowWidth(windowWidth),windowHeight(windowHeight){
        spriteBatch=std::make_unique<DirectX::SpriteBatch>(context);
        commonStates=std::make_unique<DirectX::CommonStates>(device);

        emptyTex=Texture2D(device,L"assets/Png/Empty.png");
        wallTex=Texture2D(device,L"assets/Png/Wall.png");
        singleTowerTex=Texture2D(device,L"assets/Png/SingleTower.png");
        groupAttackTowerTex=Texture2D(device,L"assets/Png/GroupAttackTower.png");
        slowTowerTex=Texture2D(device,L"assets/Png/SlowTower.png");
        centerTowerTex=Texture2D(device,L"assets/Png/CenterTower.png");
        meleeMobileTex=Texture2D(device,L"assets/Png/MeleeMobile.png");
        rangedMobileTex=Texture2D(device,L"assets/Png/RangedMobile.png");
        defenseMobileTex=Texture2D(device,L"assets/Png/DefenseMobile.png");
        explosionMobileTex=Texture2D(device,L"assets/Png/ExplosionMobile.png");


        createRTV();

        auto[tw,th]=getTextureSize(wallTex);
        if(tw>0) spriteScale=cellSize/tw;
    }

    std::pair<float,float> getTextureSize(Texture2D& tex){
        Microsoft::WRL::ComPtr<ID3D11Resource> resource;
        tex.GetSRV()->GetResource(&resource);
        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
        resource.As(&tex2D);
        D3D11_TEXTURE2D_DESC desc;
        tex2D->GetDesc(&desc);
        return {float(desc.Width),float(desc.Height)};
    }

    void createRTV(){
        rtv.Reset();
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        swapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),&backBuffer);
        device->CreateRenderTargetView(backBuffer.Get(),nullptr,&rtv);
    }

    void resizeRTV(int newWidth,int newHeight){
        windowWidth=newWidth;
        windowHeight=newHeight;
        context->OMSetRenderTargets(0,nullptr,nullptr);
        rtv.Reset();
        swapChain->ResizeBuffers(0,newWidth,newHeight,DXGI_FORMAT_UNKNOWN,0);
        createRTV();
    }

    Texture2D* getTextureForType(int typeId,bool isStatic){
        if(isStatic){
            if(typeId==1)return &singleTowerTex;
            if(typeId==2)return &groupAttackTowerTex;
            if(typeId==3)return &slowTowerTex;
            if(typeId==4)return &centerTowerTex;
        }else{
            if(typeId==1)return &meleeMobileTex;
            if(typeId==2)return &rangedMobileTex;
            if(typeId==3)return &defenseMobileTex;
            if(typeId==4)return &explosionMobileTex;
        }
        return nullptr;
    }

    void renderMap(){
        auto& bm=gamePtr->basicMap;
        for(size_t y=0;y<bm.size();y++){
            for(size_t x=0;x<bm[y].size();x++){
                if(bm[y][x]==0){
                    spriteBatch->Draw(wallTex.GetSRV(),DirectX::XMFLOAT2(float(x)*cellSize,float(y)*cellSize),nullptr,DirectX::Colors::White,0.0f,DirectX::XMFLOAT2(0,0),spriteScale);
                }
                else{
                     spriteBatch->Draw(emptyTex.GetSRV(),DirectX::XMFLOAT2(float(x)*cellSize,float(y)*cellSize),nullptr,DirectX::Colors::White,0.0f,DirectX::XMFLOAT2(0,0),spriteScale);
                }
            }
        }
    }

    void renderActors(){
        for(auto actor:gamePtr->staticActorPool){
            if(actor->hp<=0)continue;
            auto* tex=getTextureForType(actor->typeId,true);
            if(tex){
                spriteBatch->Draw(tex->GetSRV(),DirectX::XMFLOAT2(actor->x*cellSize,actor->y*cellSize),nullptr,DirectX::Colors::White,0.0f,DirectX::XMFLOAT2(0,0),spriteScale);
            }
        }
        for(auto actor:gamePtr->mobileActorPool){
            if(actor->hp<=0)continue;
            auto* tex=getTextureForType(actor->typeId,false);
            if(tex){
                spriteBatch->Draw(tex->GetSRV(),DirectX::XMFLOAT2(actor->x*cellSize,actor->y*cellSize),nullptr,DirectX::Colors::White,0.0f,DirectX::XMFLOAT2(0,0),spriteScale);
            }
        }
    }

    void run(){
        float nowTime=std::chrono::duration<float>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        float deltaTime=nowTime-lastFrameTime;
        lastFrameTime=nowTime;
        accumLastFrameTime+=deltaTime;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // float clearColor[4]={0.15f,0.15f,0.2f,1.0f};
        float clearColor[4]={255.0f,255.0f,255.0f,1.0f};
        context->ClearRenderTargetView(rtv.Get(),clearColor);

        D3D11_VIEWPORT vp={0,0,(float)windowWidth,(float)windowHeight,0,1};
        context->RSSetViewports(1,&vp);

        context->OMSetRenderTargets(1,rtv.GetAddressOf(),nullptr);

        spriteBatch->Begin(DirectX::SpriteSortMode_Deferred,commonStates->NonPremultiplied());
        renderMap();
        renderActors();

        spriteBatch->End();

        RenderBasicVirtualUI();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swapChain->Present(1,0);
    }

void RenderBasicVirtualUI(bool* p_open = nullptr){
    if (!gamePtr)
        return;

    if (!ImGui::Begin("Basic Virtual Game Control", p_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    

    ImGui::Text("Game Runtime Overview");
    ImGui::Separator();

    ImGui::Text("Time: %.1f", gamePtr->nowTime);
    ImGui::Text("CNN Switch: %s", gamePtr->cnnSwitch ? "On" : "Off");

    bool& cnnEnabled = gamePtr->cnnSwitch;
    if (ImGui::Checkbox("Enable CNN Switch", &cnnEnabled))
    {
        cnnEnabled=true;
    }

    ImGui::Separator();
    ImGui::Text("Map Size: %dx%d", gamePtr->basicMap[0].size(), gamePtr->basicMap.size());
    ImGui::Text("Static Actors: %d", gamePtr->staticActorPool.size());
    ImGui::Text("Mobile Actors: %d", gamePtr->mobileActorPool.size());

    if (ImGui::TreeNode("Resource Flow"))
    {
        for (int i = 0; i <gamePtr->ownerCount; ++i)
        {
            ImGui::BulletText("Owner %d: Cost = %.1f, Speed = %.2f", i,gamePtr->nowCost[i], gamePtr->costSpeed[i]);
        }
        ImGui::TreePop();
    }

    

    ImGui::InputFloat4("creatStaticActorInput",&creatStaticActorInput[0]);
    ImGui::Text(gamePtr->basicMap[creatStaticActorInput[1]][creatStaticActorInput[0]]?"empty":"wall");
    bool creatStaticActorFlag=false;
    auto clickFlag=ImGui::Button("Create Static Actor");
    if(clickFlag){
        creatStaticActorFlag=gamePtr->creatStaticActor(creatStaticActorInput[0],creatStaticActorInput[1],creatStaticActorInput[2],creatStaticActorInput[3]);
        creatStaticActorInput.fill(0);
    }

    ImGui::Dummy(ImVec2(0,10));
    ImGui::InputFloat4("creatMobileActorInput",&creatMobileActorInput[0]);
    ImGui::Text(gamePtr->basicMap[creatMobileActorInput[1]][creatMobileActorInput[0]]?"empty":"wall");
    bool creatMobileActorFlag=false;
    if(ImGui::Button("Create Mobile Actor")){
        creatMobileActorFlag=gamePtr->creatMobileActor(creatMobileActorInput[0],creatMobileActorInput[1],creatMobileActorInput[2],creatMobileActorInput[3]);
        creatMobileActorInput.fill(0);
    }

    ImGui::Separator();
    ImGui::TextWrapped("This UI panel is built with Dear ImGui to inspect basic_virtual.cpp runtime state, following the README style of fast iteration and tool-oriented tools.");
    

    if (ImGui::TreeNode("ownerActor")){
        for(int i=0;i<gamePtr->ownerCount;i++){
            for(auto index:gamePtr->aliveStaticList[i]){
                auto& actor=gamePtr->staticActorPool[index];
                void* tempId=static_cast<void*>(&actor);
                if(ImGui::TreeNode(std::format("actorData&func##actor{}",tempId).c_str())){
                    ImGui::Text("typeId:%d hp:%f rankNum:%d timer:Attack%f Move%f %f %f %f %f",actor->typeId,actor->hp,actor->rankNum,actor->timeManger[TimeType::Attack],actor->timeManger[TimeType::Move],actor->timeManger[TimeType::Skill1],actor->timeManger[TimeType::Skill2]);
                    if(ImGui::TreeNode("setRank()")){
                        ImGui::InputFloat("setRankInput",&setRankStaticActorInput);
                        if(ImGui::Button("setRankClick")){
                            actor->setRank(setRankStaticActorInput);
                            setRankStaticActorInput=0;
                        }
                        ImGui::TreePop();
                    }
                    if(ImGui::TreeNode("move()")){
                        ImGui::InputFloat2("moveInput",&moveStaticActorInput[0]);
                        if(ImGui::Button("moveClick")){
                            actor->move({moveStaticActorInput[0],moveStaticActorInput[1]});
                            moveStaticActorInput.fill(0);
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            for(auto index:gamePtr->aliveMobileList[i]){
                auto& actor=gamePtr->mobileActorPool[index];
                void* tempId=static_cast<void*>(&actor);
                if(ImGui::TreeNode(std::format("actorData&func##actor{}",tempId).c_str())){
                    ImGui::Text("typeId:%d hp:%f rankNum:%d timer:Attack%f Move%f %f %f %f %f",actor->typeId,actor->hp,actor->rankNum,actor->timeManger[TimeType::Attack],actor->timeManger[TimeType::Move],actor->timeManger[TimeType::Skill1],actor->timeManger[TimeType::Skill2]);
                    if(ImGui::TreeNode("setRank()")){
                        ImGui::InputFloat("setRankInput",&setRankMobileActorInput);
                        if(ImGui::Button("setRankClick")){
                            actor->setRank(setRankStaticActorInput);
                            setRankStaticActorInput=0;
                        }
                        ImGui::TreePop();
                    }
                    if(ImGui::TreeNode("move()")){
                        ImGui::InputFloat2("moveInput",&moveStaticActorInput[0]);
                        if(ImGui::Button("moveClick")){
                            actor->move({moveStaticActorInput[0],moveStaticActorInput[1]});
                            moveStaticActorInput.fill(0);
                        }
                        ImGui::TreePop();
                    }
                    if(ImGui::TreeNode("getPath()")){
                        ImGui::InputFloat2("getPathInput",&getPathInput[0]);
                        if(ImGui::Button("getPathClick")){
                            actor->getPath(getPathInput[0],getPathInput[1],actor->path);
                            getPathInput.fill(0);
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
        }
        ImGui::TreePop();
    }
    ImGui::End();
    }

};



void SetGameCnnSwitch(Game* gamePtr,bool enabled){
    if(gamePtr)gamePtr->cnnSwitch=enabled;
}


// Out-of-line Game method definitions (moved for dependency ordering)

bool Game::creatStaticActor(float x,float y,int owner,int StaticActorType){
        if(!basicMap[y][x])return false;
        switch(StaticActorType){
            // case 0:
            //     basicTowerPool.emplace_back(StaticActor(this,owner,x,y));
            //     break;
            case 1:
                if(nowCost[owner]-Attribute<SingleTower>::attributeList[0][9]<costMin[owner]){
                    return false;
                }
                ActorPool<SingleTower>::Pool.emplace_back(this,owner,x,y);
                break;
            case 2:
                if(nowCost[owner]-Attribute<GroupAttackTower>::attributeList[0][9]<costMin[owner]){
                    return false;
                }
                ActorPool<GroupAttackTower>::Pool.emplace_back(this,owner,x,y);
                break;
            case 3:
                if(nowCost[owner]-Attribute<SlowTower>::attributeList[0][9]<costMin[owner]){
                    return false;
                }
                ActorPool<SlowTower>::Pool.emplace_back(this,owner,x,y);
                break;
            case 4:
                if(nowCost[owner]-Attribute<CenterTower>::attributeList[0][9]<costMin[owner]){
                    return false;
                }
                ActorPool<CenterTower>::Pool.emplace_back(this,owner,x,y);
                break;
            default:
                throw std::runtime_error(std::format("[ERROR][Game] the StaticActorType is {}",StaticActorType));
        }
        return true;
    }

bool Game::creatMobileActor(float x,float y,int owner,int MobileActorType){
        if(!basicMap[y][x])return false;
        switch(MobileActorType){
            // case 0:
            //     basicTowerPool.emplace_back(StaticActor(this,owner,x,y));
            //     break;
            case 1:
                if(nowCost[owner]-Attribute<MeleeMobile>::attributeList[0][9]<costMin[owner]){
                    return false;
                }
                ActorPool<MeleeMobile>::Pool.emplace_back(this,owner,x,y);
                break;
            case 2:
                if(nowCost[owner]-Attribute<RangedMobile>::attributeList[0][9]<costMin[owner]){
                    return false;
                }
                ActorPool<RangedMobile>::Pool.emplace_back(this,owner,x,y);
                break;
            case 3:
                if(nowCost[owner]-Attribute<DefenseMobile>::attributeList[0][9]<costMin[owner]){
                    return false;
                }
                ActorPool<DefenseMobile>::Pool.emplace_back(this,owner,x,y);
                break;
            case 4:
                if(nowCost[owner]-Attribute<ExplosionMobile>::attributeList[0][9]<costMin[owner]){
                    return false;
                }
                ActorPool<ExplosionMobile>::Pool.emplace_back(this,owner,x,y);
                break;
            default:
                throw std::runtime_error(std::format("[ERROR][Game] the StaticActorType is {}",MobileActorType));
        }
        return true;
    }

void Game::tick(){
            //之后绑定操作按钮 暂时先不处理创建逻辑

            //移动->攻击->删除
            for(auto& actor:staticActorPool){
                actor->checkEffect();
            }
            for(auto& actor:mobileActorPool){
                actor->checkEffect();
            }

            for(int i=0;i<mobileActorPool.size();i++){
                mobileActorPool[i]->followPath();
            }

            for(int i=0;i<staticActorPool.size();i++){
                staticActorPool[i]->attack();
            }
            for(int i=0;i<mobileActorPool.size();i++){
                mobileActorPool[i]->attack();

            }

            this->solveDeadActor();

            costSpeed=std::vector<float>(costSpeed.size(),timeStep);//每0.1s产生的费用
            for(auto actor:ActorPool<CenterTower>::Pool){
                costSpeed[actor.owner]+=actor.getValue(AttributeId<CenterTower>::costRate);
            }

            for(auto i=0;i<nowCost.size();i++){
                nowCost[i]+=costSpeed[i];
                if(nowCost[i]>costMax[i]){
                    nowCost[i]=costMax[i];
                }
            }

            nowTime+=timeStep;
        }

void Game::draw(){
        mainDrawPtr->run();

    }

