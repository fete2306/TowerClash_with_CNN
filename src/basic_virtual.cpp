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
#include <fstream>
#include "basic_virtual_ui.h"
#include<json.hpp>

template<typename SubClass>
class StaticActor;
template<typename SubClass>
class MobileActor;
class IStaticActor;
class IMobileActor;
class Game;

template<typename T>
void erase_basedSwap(std::vector<T>& vec,size_t index){
    swap(vec[index],vec[vec.size()-1]);
    vec.pop_back();
}
template<typename T>
void erase_basedSwap(std::deque<T>& vec,size_t index){
    swap(vec[index],vec[vec.size()-1]);
    vec.pop_back();
}

class specialEffect{
    public:
    int Id;//0攻击减速 //1移动减速 //2脆弱
    float value;
    float normalData;
    float endTime;
    specialEffect(int Id,float value,float normalData,float endTime):Id(Id),value(value),normalData(normalData),endTime(endTime){};

};

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

class IStaticActor{//仅用作基类指针
    public:
    std::vector<bool>* resistList;//阻挡状态的列表 在析构时清空对方被阻挡的状态
    std::vector<std::array<int,2>> scopeList;//攻击范围覆盖的格子 非直接坐标而是偏移量[dx,dy] 获取真实坐标是x+dx,y+dy

    std::vector<specialEffect*> effectedList;//所拥有的特殊状态         对方析构时会将其结束时间置0
    std::vector<specialEffect*> effectList;//所施加的特殊状态           在自身析构时将造成的所有异常状态全部结束
    
    Game* gamePtr;
    int subclassPoolIndex;
    int poolIndex;
    int aliveListIndex;
    int mapListIndex;

    int rankNum;
    int owner;
    float x,y;
    float hp;
    float attackNum;
    float scope;
    float attackCount;
    float attackSpeed;
    float lastAttackTime;
    int attackType;
    int cost;
    float costRate;
    float rasistCount;

    static float basicCost;
    virtual ~IStaticActor()=default;
    virtual void dead(){};
    virtual bool setRank(int rankOffest){};
    virtual void applyEffect(IStaticActor* staticActorPtr){};
    virtual void applyEffect(IMobileActor* mobileActorPtr){};
    virtual void checkEffect(){};
    virtual void setScope(){};
    virtual void beHurted(int attackType,float attackNum){};
    virtual void attack(){};
    virtual void move(std::pair<int,int> goalPos){};
    virtual float getTotalCost(){};
    virtual void skill1(){};
    virtual void skill2(){};
};

class IMobileActor{
    public:
    std::vector<std::array<int,2>> path;
    
    std::vector<bool>* resistedList;//阻挡方的列表
    
    std::vector<std::array<int,2>> scopeList;//攻击范围覆盖的格子 非直接坐标而是偏移量[dx,dy] 获取真实坐标是x+dx,y+dy
    
    std::vector<specialEffect*> effectedList;//所拥有的特殊状态         对方析构时会将其结束时间置0
    std::vector<specialEffect*> effectList;//所施加的特殊状态           在自身析构时将造成的所有异常状态全部结束
    Game* gamePtr;
    int subclassPoolIndex;
    int poolIndex;
    int aliveListIndex;
    int mapListIndex;
    
    int resistListIndex;

    int pathNum;
    float moveSpeed;
    int rankNum;
    int owner;
    float x,y;
    float hp;

    float attackNum;
    float scope;
    int attackCount;
    float attackSpeed;
    float lastAttackTime;
    int attackType;
    int cost;
    float costRate;
    virtual ~IMobileActor()=default;
    virtual void dead(){};
    virtual bool setRank(int rankOffest){};
    virtual void applyEffect(IStaticActor* staticActorPtr){};
    virtual void applyEffect(IMobileActor* mobileActorPtr){};
    virtual void checkEffect(){};
    virtual void setScope(){};
    virtual void beHurted(int attackType,float attackNum){};
    virtual void attack(){};
    virtual void move(){};
    virtual std::vector<std::array<int,2>> getPath(float goalX,float goalY){};
    virtual float getTotalCost(){};
    virtual void skill1(){};
    virtual void skill2(){};
};



template<typename SubClass>
class StaticActor:public IStaticActor{//用于实现通用方法的模板类
    public:
    StaticActor(Game* gamePtr,int owner,float x,float y,SubClass* subclassPtr):gamePtr(gamePtr),owner(owner),x(x),y(y){

        resistList=new std::vector<bool>;

        rankNum=0;
        //setRank由子类构造方法调用
        poolIndex=ActorPool<SubClass>::Pool.size();
        mapListIndex=gamePtr->staticActorMap[int(y)][int(x)].size();
        aliveListIndex=gamePtr->aliveStaticList[owner].size();

        gamePtr->staticActorPool.push_back(this);
        gamePtr->staticActorMap[int(y)][int(x)].push_back(poolIndex);
        gamePtr->aliveStaticList[owner].push_back(poolIndex);
    };

    virtual ~StaticActor(){
        for(int i=0;i<resistList->size();i++){
            resistList->at(i)=false;
        }
        delete resistList;
        for(int i=0;i<effectList.size();i++){
            effectList[i]->endTime=0;
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
                    costOffest+=attributeList[rankNum-i][9]*gamePtr->returnCostMul;
                }
            }
            if(gamePtr->nowCost[this->owner]+costOffest<gamePtr->costMin[this->owner]){//内存低于临界
                throw std::runtime_error(std::format("[StaticActor][setRank]the nowCost={} costOffest={} and rankMax={} the rankMin={}",gamePtr->nowCost[this->owner],costOffest,gamePtr->costMax[this->owner],gamePtr->costMin[this->owner]));
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
        float lastScope=scope;
        this->hp=attributeList[rankNum][3];
        this->attackNum=attributeList[rankNum][4];
        this->scope=attributeList[rankNum][5];
        this->attackCount=attributeList[rankNum][6];
        this->attackSpeed=attributeList[rankNum][7];
        this->attackType=int(attributeList[rankNum][8]);
        this->cost=attributeList[rankNum][9];
        this->costRate=attributeList[rankNum][10];
        this->rasistCount=attributeList[rankNum][11];
        if(scope!=lastScope)setScope();

        return true;
    };

    virtual void applyEffect(IStaticActor* staticActorPtr)override{
        return;
    };

    virtual void applyEffect(IMobileActor* mobileActorPtr)override{
        return;
    };

    virtual void checkEffect()override{
        std::vector<int> eraseList;
        for(int i=0;i<effectedList.size();i++){
            auto& effect=effectedList[i];
            if(effect==nullptr){
                eraseList.push_back(i);
                continue;
            }
            if(effect->endTime<=gamePtr->nowTime){
                switch(effect->Id){
                    case 0:
                        this->attackSpeed=effect->normalData;
                        break;
                    case 1:
                        throw std::runtime_error(std::format("[ERROR][StaticActor] the effect Id is {}",effect->Id));
                        break;
                    case 2:
                        break;
                    default:
                        throw std::runtime_error(std::format("[ERROR][StaticActor] the effect Id is {}",effect->Id));
                }
                
                
                delete effect;
                effect=nullptr;
                eraseList.push_back(i);
            }
            else{
                switch(effect->Id){
                    case 0:
                        this->attackSpeed=attributeList[rankNum][7];
                        effect->normalData=this->attackSpeed;
                        this->attackSpeed*=(1-effect->value);
                        break;
                    case 1:
                        throw std::runtime_error(std::format("[ERROR][StaticActor] the effect Id is {}",effect->Id));
                        break;
                    case 2:
                        break;
                    default:
                        throw std::runtime_error(std::format("[ERROR][StaticActor] the effect Id is {}",effect->Id));
                }
            }
        }
        for(int i=eraseList.size()-1;i>=0;i--){
            erase_basedSwap(effectedList,eraseList[i]);
        }
    }

    virtual void setScope()override{//静态,有超出边界不加入
        scopeList.clear();
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

    virtual void beHurted(int attackType,float attackNum)override{
        switch(attackType){
            case 0:
                attackNum=attackNum-attributeList[rankNum][0];
                attackNum=std::max(attackNum,0.0f);
                break;
            case 1:
                attackNum=attackNum*(1-attributeList[rankNum][1]);
                attackNum=std::max(attackNum,0.0f);
                break;
            case 2:
                break;
            default:
                throw std::runtime_error(std::format("[ERROR][StaticActor] the attackType is {}",attackType));
        }
        this->hp-=attackNum;
    }

    virtual void attack()override{
        int attackCount=int(this->attackCount);//取整
        
        auto& nowTime=gamePtr->nowTime;
        
        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }

        auto& mobileActorMap=gamePtr->mobileActorMap;
        auto& eraseMobileActorSet=gamePtr->eraseMobileActorSet;

        std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>attackQueue;//距离,索引
        
        for(auto [x,y]:scopeList){
            for(auto index:mobileActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->mobileActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    attackQueue.emplace(d2,index);
                    while(attackQueue.size()>attackCount){
                        attackQueue.pop();
                    }
                }
            }
        }
        
        int residualAttackCount=attackCount-int64_t(attackQueue.size());
        while(attackQueue.size()>0){//优先级为 先攻击移动单位，再攻击固定单位 其次，优先攻击最近
            auto [d,index]=attackQueue.top();
            attackQueue.pop();
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
        if(residualAttackCount<=0){//攻击数量足够
            return;
        }

;
        auto& staticActorMap=gamePtr->staticActorMap;
        auto& eraseStaticActorSet=gamePtr->eraseStaticActorSet;
        
        for(auto [x,y]:scopeList){
            for(auto index:staticActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    attackQueue.emplace(d2,index);
                    while(attackQueue.size()>residualAttackCount){
                        attackQueue.pop();
                    }
                }
            }
        }
        while(attackQueue.size()>0){
            auto [d,index]=attackQueue.top();
            attackQueue.pop();
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
        lastAttackTime=nowTime;

    }


    virtual void move(std::pair<int,int> goalPos)override{
        gamePtr->staticActorPool[gamePtr->staticActorMap[int(this->y)][int(this->x)][gamePtr->staticActorMap[int(this->y)][int(this->x)].size()-1]]->mapListIndex=this->mapListIndex;
        erase_basedSwap(gamePtr->staticActorMap[int(this->y)][int(this->x)],this->mapListIndex);
        this->x=goalPos.first;
        this->y=goalPos.second;
        this->setScope();
        this->mapListIndex=gamePtr->staticActorMap[int(this->y)][int(this->x)].size();
        gamePtr->staticActorMap[int(this->y)][int(this->x)].push_back(poolIndex);
    };
    virtual float getTotalCost()override{
        return Attribute<SubClass>::getCost(0,this->rankNum);//rankNum在IStaticActor,不用管
    };
    
    virtual void skill1()override{};
    virtual void skill2()override{};

};
template<typename SubClass>
class MobileActor:public IMobileActor{
    public:
    

    MobileActor(Game* gamePtr,int owner,float x,float y,SubClass* subclassPtr):gamePtr(gamePtr),owner(owner),x(x),y(y){

        resistedList=nullptr;

        rankNum=0;

        poolIndex=ActorPool<SubClass>::Pool.size();
        mapListIndex=gamePtr->mobileActorMap[int(y)][int(x)].size();
        aliveListIndex=gamePtr->aliveMobileList[owner].size();

        gamePtr->mobileActorPool.push_back(this);
        gamePtr->mobileActorMap[int(y)][int(x)].push_back(poolIndex);
        gamePtr->aliveMobileList[owner].push_back(poolIndex);
    }
    
    virtual ~MobileActor(){

        for(int i=0;i<effectList.size();i++){
            effectList[i]->endTime=0;
        }
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
                    costOffest+=attributeList[rankNum-i][9]*gamePtr->returnCostMul;
                }
            }
            if(gamePtr->nowCost[this->owner]+costOffest<gamePtr->costMin[this->owner]){//内存低于临界
                throw std::runtime_error(std::format("[StaticActor][setRank]the nowCost={} costOffest={} and rankMax={} the rankMin={}",gamePtr->nowCost[this->owner],costOffest,gamePtr->costMax[this->owner],gamePtr->costMin[this->owner]));
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
        float lastScope=scope;
        this->hp=attributeList[rankNum][3];
        this->attackNum=attributeList[rankNum][4];
        this->scope=attributeList[rankNum][5];
        this->attackCount=attributeList[rankNum][6];
        this->attackSpeed=attributeList[rankNum][7];
        this->attackType=int(attributeList[rankNum][8]);
        this->cost=int(attributeList[rankNum][9]);
        this->costRate=attributeList[rankNum][10];
        this->moveSpeed=attributeList[rankNum][11];
        if(scope!=lastScope)setScope();

        return true;
    };

    virtual void applyEffect(IStaticActor* staticActorPtr)override{
        return;
    };

    virtual void applyEffect(IMobileActor* mobileActorPtr)override{
        return;
    };

    virtual void checkEffect()override{
        std::vector<int> eraseList;
        for(int i=0;i<effectedList.size();i++){
            auto& effect=effectedList[i];
            if(effect==nullptr){
                eraseList.push_back(i);
                continue;
            }
            if(effect->endTime<=gamePtr->nowTime){//已经结束
                switch(effect->Id){
                    case 0:
                        this->attackSpeed=effect->normalData;
                        break;
                    case 1:
                        this->moveSpeed=effect->normalData;
                        break;
                    case 2:
                        break;
                    default:
                        throw std::runtime_error(std::format("[ERROR][StaticActor] the effect Id is {}",effect->Id));
                }
                delete effect;
                effect=nullptr;
                eraseList.push_back(i);
            }
            else{
                switch(effect->Id){
                    case 0:
                        this->attackSpeed=attributeList[rankNum][7];
                        effect->normalData=this->attackSpeed;
                        this->attackSpeed*=(1-effect->value);
                        break;
                    case 1:
                        this->moveSpeed=attributeList[rankNum][11];
                        effect->normalData=this->moveSpeed;
                        this->moveSpeed*=(1-effect->value);
                        break;
                    case 2:
                        break;
                    default:
                        throw std::runtime_error(std::format("[ERROR][StaticActor] the effect Id is {}",effect->Id));
                }
            }
        }
        for(int i=eraseList.size()-1;i>=0;i--){
            erase_basedSwap(effectedList,eraseList[i]);
        }
        
    }

    virtual void setScope()override{//动态,在调用时检测边界
        scopeList.clear();
        int r=int(scope);
        for(int dx=-r;dx<=r;dx++){
            int dy_max=int(sqrt(scope*scope-dx*dx));
            for(int dy=-dy_max;dy<=dy_max;dy++){
                scopeList.push_back({dx,dy});
            }
        }
    }

    virtual void beHurted(int attackType,float attackNum)override{
        switch(attackType){
            case 0:
                attackNum=attackNum-attributeList[rankNum][0];
                attackNum=std::max(attackNum,0.0f);
                break;
            case 1:
                attackNum=attackNum*(1-attributeList[rankNum][1]);
                attackNum=std::max(attackNum,0.0f);
                break;
            case 2:
                break;
            default:
                throw std::runtime_error(std::format("[ERROR][MobileActor] the attackType is {}",attackType));
        }
        this->hp-=attackNum;
    }

    virtual void attack()override{
        int attackCount=int(this->attackCount);//取整
        
        auto& nowTime=gamePtr->nowTime;
        
        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }

        auto& mobileActorMap=gamePtr->mobileActorMap;
        auto& eraseMobileActorSet=gamePtr->eraseMobileActorSet;

        std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>attackQueue;//距离,索引

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
                if(d2<scope*scope){
                    attackQueue.emplace(d2,index);
                    while(attackQueue.size()>attackCount){
                        attackQueue.pop();
                    }
                }
            }
        }
        
        int residualAttackCount=attackCount-int64_t(attackQueue.size());
        while(attackQueue.size()>0){//优先级为 先攻击移动单位，再攻击固定单位 其次，优先攻击最近
            auto [d,index]=attackQueue.top();
            attackQueue.pop();
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
        if(residualAttackCount<=0){//攻击数量足够
            return;
        }

;
        auto& staticActorMap=gamePtr->staticActorMap;
        auto& eraseStaticActorSet=gamePtr->eraseStaticActorSet;
        
        for(auto [x,y]:scopeList){
            for(auto index:staticActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    attackQueue.emplace(d2,index);
                    while(attackQueue.size()>residualAttackCount){
                        attackQueue.pop();
                    }
                }
            }
        }
        while(attackQueue.size()>0){
            auto [d,index]=attackQueue.top();
            attackQueue.pop();
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
        lastAttackTime=nowTime;

    }

    virtual void move()override{//移动以0.1s为单位
        if(path.size()!=0&&pathNum<path.size()-1&&(resistedList==nullptr||(resistedList!=nullptr&&(resistedList->size()<=resistListIndex||(*resistedList)[resistListIndex]==false)))){//有路&&未走完&&未阻挡
            float dx=path[pathNum][0]-this->x;
            float dy=path[pathNum][1]-this->y;
            float d=sqrt(dx*dx+dy*dy);
            float newX=this->x+dx/d*this->moveSpeed*0.1;
            float newY=this->y+dy/d*this->moveSpeed*0.1;
            if(int(newX)!=int(this->x)||int(newY)!=int(this->y)){
                gamePtr->mobileActorPool[gamePtr->mobileActorMap[int(this->y)][int(this->x)][gamePtr->mobileActorMap[int(this->y)][int(this->x)].size()-1]]->mapListIndex=this->mapListIndex;
                erase_basedSwap(gamePtr->mobileActorMap[int(this->y)][int(this->x)],this->mapListIndex);

                this->mapListIndex=gamePtr->mobileActorMap[int(newY)][int(newX)].size();
                gamePtr->mobileActorMap[int(newY)][int(newX)].push_back(poolIndex);
            }
            this->x=newX;
            this->y=newY;
            for(auto index:gamePtr->staticActorMap[int(this->y)][int(this->x)]){
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner!=this->owner){

                    this->resistedList=actor->resistList;
                    this->resistListIndex=actor->resistList->size();

                    (*(actor->resistList)).push_back(true);
                    break;
                }
            }
            if(d<=this->moveSpeed*0.1){
                pathNum++;
            }
        }
    }

    virtual std::vector<std::array<int,2>> getPath(float goalX,float goalY)override{
        erase_basedSwap(*this->resistedList,this->resistListIndex);//主动清除被阻挡状态
        this->resistedList=nullptr;
        this->resistListIndex=-1;
        std::vector<std::array<int,2>> resPath;
        // this->getPath_dfs(goalX,goalY,resPath);
        this->aStar(goalX,goalY,resPath);
        this->pathNum=0;
        return resPath;
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
    virtual float getTotalCost()override{
        return Attribute<SubClass>::getCost(0,this->rankNum);
    };

    virtual void skill1()override{};
    virtual void skill2()override{};
    
};

class SingleTower:public StaticActor<SingleTower>{//单体攻击
    public:
    SingleTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y,this){
       auto flag=setRank(0);
    }

};
class GroupAttackTower:public StaticActor<GroupAttackTower>{//群攻
    public:
    std::vector<std::array<int,2>> groupAttackArea;//溅射攻击范围
    float groupAttackScope;

    GroupAttackTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y,this){
        auto flag=setRank(0);
    }

    bool setRank(int rankOffest=0) override{
        auto flag= _setRank(this,rankOffest);
        if(!flag){
            return false;
        }
        this->groupAttackScope=Attribute<GroupAttackTower>::attributeList[rankNum][12];
        return true;
    }

    void set_groupAttackArea(){
        groupAttackArea.clear();
        int r=int(groupAttackScope);
        for(int dx=-r;dx<=r;dx++){
            int dy_max=int(sqrt(groupAttackScope*groupAttackScope-dx*dx));
            for(int dy=-dy_max;dy<=dy_max;dy++){
                groupAttackArea.push_back({dx,dy});
            }
        }
    }

    void attack() override{
        int attackCount=int(this->attackCount);//取整
        
        auto& nowTime=gamePtr->nowTime;
        
        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }

        auto& mobileActorMap=gamePtr->mobileActorMap;
        auto& eraseMobileActorSet=gamePtr->eraseMobileActorSet;

        std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>attackQueue;//距离,索引
        
        for(auto [x,y]:this->scopeList){
            for(auto index:mobileActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->mobileActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    attackQueue.emplace(d2,index);
                    while(attackQueue.size()>attackCount){
                        attackQueue.pop();
                    }
                }
            }
        }
        
        int residualAttackCount=attackCount-int64_t(attackQueue.size());
        std::vector<int> tempGropAttackList;
        while(attackQueue.size()>0){//优先级为 先攻击移动单位，再攻击固定单位 其次，优先攻击最近
            auto [d,index]=attackQueue.top();
            tempGropAttackList.push_back(index);
            attackQueue.pop();
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
        
        std::vector<int> tempAttackList;
        for(auto& index:tempGropAttackList){
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
                        tempAttackList.push_back(index_map);
                    }
                }
            }
        }
        for(auto& index:tempAttackList){
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

        if(residualAttackCount<=0){//攻击数量足够
            return;
        }
        tempGropAttackList.clear();
        tempAttackList.clear();

        auto& staticActorMap=gamePtr->staticActorMap;
        auto& eraseStaticActorSet=gamePtr->eraseStaticActorSet;
        
        for(auto [x,y]:this->scopeList){
            for(auto index:staticActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    attackQueue.emplace(d2,index);
                    while(attackQueue.size()-residualAttackCount>0){
                        attackQueue.pop();
                    }
                }
            }
        }
        while(attackQueue.size()>0){
            auto [d,index]=attackQueue.top();
            attackQueue.pop();
            tempGropAttackList.push_back(index);
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
        
        for(auto& index:tempGropAttackList){
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
                    float d2=abs(mainActor->x-actor->x)*abs(mainActor->x-actor->x)+abs(mainActor->y-actor->y)*abs(mainActor->y-actor->y);
                    if(d2<groupAttackScope*groupAttackScope){
                        tempAttackList.push_back(index_map);
                    }
                }
            }
        }
        for(auto& index:tempAttackList){
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
        
        lastAttackTime=nowTime;
    }
};
class SlowTower:public StaticActor<SlowTower>{//减速
    public:
    float attackSlowMul;
    float attackSlowTime;
    float moveSlowMul;
    float moveSlowTime;

    SlowTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y,this){
        setRank(0);
    }
    
    bool setRank(int rankOffest){
        auto flag= _setRank(this,rankOffest);
        if(!flag){
            return false;
        }
        this->attackSlowMul=Attribute<SlowTower>::attributeList[rankNum][12];
        this->moveSlowMul=Attribute<SlowTower>::attributeList[rankNum][13];
        this->attackSlowTime=Attribute<SlowTower>::attributeList[rankNum][14];
        this->moveSlowTime=Attribute<SlowTower>::attributeList[rankNum][15];
        return true;
    }

    virtual void applyEffect(IStaticActor* staticActorPtr) override{
        staticActorPtr->effectedList.push_back(new specialEffect{0,attackSlowMul,staticActorPtr->attackSpeed,gamePtr->nowTime+attackSlowTime});
        return;
    };

    virtual void applyEffect(IMobileActor* mobileActorPtr)override{
        mobileActorPtr->effectedList.push_back(new specialEffect{0,attackSlowMul,mobileActorPtr->attackSpeed,gamePtr->nowTime+attackSlowTime});
        mobileActorPtr->effectedList.push_back(new specialEffect{1,moveSlowMul,mobileActorPtr->moveSpeed,gamePtr->nowTime+moveSlowTime});
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

    bool setRank(int rankOffest=0) override{
        auto flag= _setRank(this,rankOffest);
        if(!flag){
            return false;
        }
        this->explosionNum=Attribute<ExplosionMobile>::attributeList[rankNum][13];
        return true;
    }
    
    void beHurted(int attackType,float attackNum) override{
        MobileActor::beHurted(attackType,attackNum);
        if(this->hp<=0){
            this->attackNum=this->explosionNum;
            this->lastAttackTime=-1000.0f;
            this->scope=3.0f;
            this->attackCount=1000.0f;//懒得重载范围攻击了
            setScope();
            this->attack();
        }
    }

};


class Game{
    public:
    std::vector<std::vector<bool>> basicMap;//0为障碍物 1为空地 2为A方固定单位 3为A方移动单位 4为B方固定单位 5为B方移动单位 以A方为例，仅0和2无法通过 //废弃,单格可能有多个不同阵营单位,仅保留0/1来标明障碍物
    std::unordered_map<std::string,std::vector<std::vector<bool>>> mapTable;//存地图列表
    std::vector<std::vector<std::vector<int>>> staticActorMap;//存每个格子的固定单位索引列表
    std::vector<std::vector<std::vector<int>>> mobileActorMap;//存每个格子的移动单位索引列表

    //以下均用,且仅用于转为张量传入CNN
    std::deque<std::pair<std::vector<float>,std::vector<int64_t>>>tensor;//存近frameNum次操作帧 [(张量,形状)]
    std::deque<std::vector<int>>resultOut;//存进行的操作

    std::vector<IStaticActor*> staticActorPool;
    std::vector<IMobileActor*> mobileActorPool;

    std::vector<std::vector<int>> aliveStaticList;
    std::vector<std::vector<int>> aliveMobileList;
    std::vector<int> eraseStaticActorSet;//后续进行sort与unique来处理 从大到小来进行 删除时需要swap待删除与队尾,且将所有对于基类指针池的索引一并swap
    std::vector<int> eraseMobileActorSet;//

    std::vector<float> nowCost;//根据owner作为索引来划分
    std::vector<float> costMax;
    std::vector<float> costMin;
    std::vector<float> costSpeed;

    int frameNum=5;
    int mapIndex;
    float nowTime;

    float returnCostMul=0.5f;//返回时消耗的倍率 

    bool cnnSwitch;


    Game(std::string mapJsonPath="assets/MapData/Map.json",int mapIndex=0,std::string staticActorJsonPath="assets/ActorAttribute/StaticActor.json",std::string mobileActorJsonPath="assets/ActorAttribute/MobileActor.json"){//初始地图,仅01
        using json = nlohmann::json;
        std::ifstream mapFile(mapJsonPath);
        this->mapTable=json::parse(mapFile);
        this->basicMap=this->mapTable[std::to_string(mapIndex)];

        setBaseAttribute(staticActorJsonPath,mobileActorJsonPath);

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

    bool creatStaticActor(int x,int y,int owner,int StaticActorType){
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

    bool creatMobileActor(int x,int y,int owner,int MobileActorType){
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


    void eraseStaticActor(int index){
        eraseStaticActorSet.push_back(index);
        auto actorPtr=staticActorPool[index];
        nowCost[actorPtr->owner]+=actorPtr->getTotalCost()*returnCostMul;
        nowCost[actorPtr->owner]=std::min(nowCost[actorPtr->owner],costMax[actorPtr->owner]);
        solveDeadActor();
    }

    void eraseMobileActor(int index){
        eraseMobileActorSet.push_back(index);
        auto actorPtr=mobileActorPool[index];
        nowCost[actorPtr->owner]+=actorPtr->getTotalCost()*returnCostMul;
        nowCost[actorPtr->owner]=std::min(nowCost[actorPtr->owner],costMax[actorPtr->owner]);
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

    

    void tick(){
            //之后绑定操作按钮 暂时先不处理创建逻辑

            //移动->攻击->删除
            for(auto& actor:staticActorPool){
                actor->checkEffect();
            }
            for(auto& actor:mobileActorPool){
                actor->checkEffect();
            }

            for(int i=0;i<mobileActorPool.size();i++){
                mobileActorPool[i]->move();
            }

            for(int i=0;i<staticActorPool.size();i++){
                staticActorPool[i]->attack();
            }
            for(int i=0;i<mobileActorPool.size();i++){
                mobileActorPool[i]->attack();
                
            }

            this->solveDeadActor();
            
            costSpeed=std::vector<float>(costSpeed.size(),1.0f);//每0.1s产生的费用
            for(auto actor:ActorPool<CenterTower>::Pool){
                costSpeed[actor.owner]+=actor.costRate;  
            }

            for(int i=0;i<nowCost.size();i++){
                nowCost[i]+=costSpeed[i];
            }
            
            nowTime+=0.1;
        }

};

BasicVirtualDebugState GetBasicVirtualDebugState(Game* game)
{
    BasicVirtualDebugState state;
    if (!game)
        return state;

    state.nowTime = game->nowTime;
    state.cnnSwitch = game->cnnSwitch;
    state.mapHeight = (int)game->basicMap.size();
    state.mapWidth = state.mapHeight ? (int)game->basicMap[0].size() : 0;
    state.staticActorCount = (int)game->staticActorPool.size();
    state.mobileActorCount = (int)game->mobileActorPool.size();
    state.nowCost = game->nowCost;
    state.costSpeed = game->costSpeed;
    return state;
}

void SetGameCnnSwitch(Game* game, bool enabled)
{
    if (game)
        game->cnnSwitch = enabled;
}

