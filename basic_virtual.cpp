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

class resist{
    public:
    bool data;//1为被阻挡
    resist(bool data){
        this->data=data;
    }
};

class specialEffect{
    public:
    int Id;
    float value;
    float endTime;
};

class StaticActor{
    public:
    std::vector<std::vector<float>> attributeList;/*
    抗性 (maxRank,attackTypeCount,1) //暂定attackTypeCount=3 物理/魔法/真实伤害
    生命上限
    攻击力
    攻击范围
    攻击数量
    攻击速度
    攻击类型
    阻挡数目
    到达该等级的费用消耗
    费用产出(/s)*/
    //特殊属性由子类单独实现
    // std::vector<float> stateEndTime;//[0]攻击 [1]移动 [2]减速 [3]脆弱 //由子类各自实现吧
    std::vector<bool>* resistList;//阻挡状态的列表 在析构时清空对方被阻挡的状态
    std::vector<std::array<int,2>> spoceList;//攻击范围覆盖的格子 非直接坐标而是偏移量[dx,dy] 获取真实坐标是x+dx,y+dy

    std::vector<specialEffect*> effecedtList;//所拥有的特殊状态         对方析构时会将其结束时间置0
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

    StaticActor(Game* gamePtr,int owner,float x,float y):gamePtr(gamePtr),owner(owner),x(x),y(y){

        resistList=new std::vector<bool>;

        rankNum=0;
        get_attributeList();
        setRank();
        subclassPoolIndex=gamePtr->basicTowerPool.size();//构造函数结束后size++,此刻可正常表示当前对象插入位置
        poolIndex=gamePtr->staticActorPool.size();
        mapListIndex=gamePtr->staticActorMap[int(y)][int(x)].size();
        aliveListIndex=gamePtr->aliveStaticList[owner].size();

        gamePtr->staticActorMap[int(y)][int(x)].push_back(poolIndex);
        gamePtr->aliveStaticList[owner].push_back(poolIndex);
        gamePtr->staticActorPool.push_back(this);
    };

    virtual void dead(){//仅删除子类对象池中的对象的数据 基类指针池、存活列表、对象图等由外部进行删除(对整个索引序列重排)
        //只有基类指针池不能由子类进行 因为删除列表使用的是其索引
        gamePtr->staticActorPool[gamePtr->staticActorMap[int(this->y)][int(this->x)][gamePtr->staticActorMap[int(this->y)][int(this->x)].size()-1]]->mapListIndex=this->mapListIndex;
        erase_basedSwap(gamePtr->staticActorMap[int(this->y)][int(this->x)],this->mapListIndex);

        gamePtr->staticActorPool[gamePtr->aliveStaticList[owner][gamePtr->aliveStaticList[owner].size()-1]]->aliveListIndex=this->aliveListIndex;
        erase_basedSwap(gamePtr->aliveStaticList[owner],this->aliveListIndex);

        erase_basedSwap(gamePtr->basicTowerPool,poolIndex);
    };

    virtual void get_attributeList(){
        /*物理抗性 魔法抗性 真实伤害抗性(?) 生命上限 攻击力 攻击范围 攻击数量 攻击速度 攻击类型 到达该等级的费用消耗 费用产出(/s) 阻挡数目*/
        this->attributeList={
            {50.0f  ,0.1f   ,0.0f   ,1000.0f    ,100.0f ,2.0f   ,1.0f   ,1.0f   ,1.0f   ,100.0f      ,0.f    ,1.0f},
            {100.0f ,0.15f  ,0.0f   ,1500.0f    ,150.0f ,2.5f   ,1.0f   ,1.5f   ,1.0f   ,100.0f      ,0.f    ,1.0f},
            {150.0f ,0.2f   ,0.0f   ,2000.0f    ,200.0f ,3.0f   ,2.0f   ,2.0f   ,1.0f   ,100.0f      ,0.f    ,1.0f},
        };

    };
    virtual void setRank(){
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

        
    };
    
    virtual void rankUp(){
        rankNum++;
        setRank();
        gamePtr->nowCost[this->owner]-=this->cost;
    };
    virtual void rankDown(){
        rankNum--;
        setRank();
        gamePtr->nowCost[this->owner]+=this->cost * gamePtr->returnCostMul;
    };

    virtual void applyEffect(StaticActor* staticActorPtr){
        return;
    };

    virtual void applyEffect(MobileActor* mobileActorPtr){
        return;
    };

    virtual void setScope(){//静态,有超出边界不加入
        spoceList.clear();
        int r=int(scope);
        for(int dx=-r;dx<=r;dx++){
            int dy_max=int(sqrt(scope*scope-dx*dx));
            for(int dy=-dy_max;dy<=dy_max;dy++){
                if(x+dx<0||x+dx>=gamePtr->basicMap[0].size()||y+dy<0||y+dy>=gamePtr->basicMap.size()){
                    continue;
                }
                spoceList.push_back({dx,dy});
            }
        }
    }

    virtual void beHurted(int attackType,float attackNum){
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

    virtual void attack(){
        int attackCount=int(this->attackCount);//取整
        
        auto& nowTime=gamePtr->nowTime;
        
        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }

        auto& mobileActorMap=gamePtr->mobileActorMap;
        auto& eraseMobileActorSet=gamePtr->eraseMobileActorSet;

        std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>attackQueue;//距离,索引
        
        for(auto [x,y]:spoceList){
            for(auto index:mobileActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->mobileActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    attackQueue.emplace(d2,index);
                    while(attackQueue.size()-attackCount>0){
                        attackQueue.pop();
                    }
                }
            }
        }
        
        int residualAttackCount=attackCount-attackQueue.size();
        while(attackQueue.size()>0){//优先级为 先攻击移动单位，再攻击固定单位 其次，优先攻击最近
            auto&[d,index]=attackQueue.top();
            attackQueue.pop();
            auto& actor=gamePtr->staticActorPool[index];
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
        
        for(auto [x,y]:spoceList){
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
            auto&[d,index]=attackQueue.top();
            attackQueue.pop();
            auto& actor=gamePtr->staticActorPool[index];
            actor->beHurted(attackType,attackNum);
            this->applyEffect(actor);
            if(actor->hp<=0){
                eraseStaticActorSet.push_back(index);
            }
        }

    }


    virtual void move(std::pair<int,int> goalPos){
        gamePtr->staticActorPool[gamePtr->staticActorMap[int(this->y)][int(this->x)][gamePtr->staticActorMap[int(this->y)][int(this->x)].size()-1]]->mapListIndex=this->mapListIndex;
        erase_basedSwap(gamePtr->staticActorMap[int(this->y)][int(this->x)],this->mapListIndex);
        this->x=goalPos.first;
        this->y=goalPos.second;
        this->setScope();
        this->mapListIndex=gamePtr->mobileActorMap[int(this->y)][int(this->x)].size();
        gamePtr->mobileActorMap[int(this->y)][int(this->x)].push_back(poolIndex);
    };

    
    virtual void skill1(){};
    virtual void skill2(){};

};

class MobileActor{
    public:
    std::vector<std::vector<float>> attributeList;
    std::vector<std::array<int,2>> path;
    
    std::vector<bool>* resistedList;//阻挡方的列表
    
    std::vector<std::array<int,2>> spoceList;//攻击范围覆盖的格子 非直接坐标而是偏移量[dx,dy] 获取真实坐标是x+dx,y+dy
    
    std::vector<specialEffect*> effecedtList;//所拥有的特殊状态         对方析构时会将其结束时间置0
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

    MobileActor(Game* gamePtr,int owner,float x,float y):gamePtr(gamePtr),owner(owner),x(x),y(y){

        resistedList=nullptr;

        rankNum=0;
        get_attributeList();
        setRank();
        subclassPoolIndex=gamePtr->basicMobilePool.size();//构造函数结束后size++,此刻可正常表示当前对象插入位置
        poolIndex=gamePtr->mobileActorPool.size();
        mapListIndex=gamePtr->mobileActorMap[int(y)][int(x)].size();
        aliveListIndex=gamePtr->aliveMobileList[owner].size();

        gamePtr->mobileActorMap[int(y)][int(x)].push_back(poolIndex);
        gamePtr->aliveMobileList[owner].push_back(poolIndex);
        gamePtr->mobileActorPool.push_back(this);
    }

    virtual void dead(){//仅删除子类对象池中的对象的数据 基类指针池、存活列表、对象图等由外部进行删除(对整个索引序列重排)
        //只有基类指针池不能由子类进行 因为删除列表使用的是其索引
        gamePtr->mobileActorPool[gamePtr->mobileActorMap[int(this->y)][int(this->x)][gamePtr->mobileActorMap[int(this->y)][int(this->x)].size()]]->mapListIndex=this->mapListIndex;
        erase_basedSwap(gamePtr->mobileActorMap[int(this->y)][int(this->x)],this->mapListIndex);

        gamePtr->mobileActorPool[gamePtr->aliveMobileList[owner][gamePtr->aliveMobileList[owner].size()-1]]->aliveListIndex=this->aliveListIndex;
        erase_basedSwap(gamePtr->aliveMobileList[owner],this->aliveListIndex);

        erase_basedSwap(gamePtr->basicMobilePool,poolIndex);
    };

    virtual void get_attributeList(){
        /*物理抗性 魔法抗性 真实伤害抗性(?) 生命上限 攻击力 攻击范围 攻击数量 攻击速度 攻击类型 到达该等级的费用消耗 费用产出(/s) 移动速度*/
        this->attributeList={
            {10.0f  ,0.05f  ,0.0f   ,300.0f     ,30.0f  ,1.0f   ,1.0f   ,1.0f   ,1.0f   ,30.0f  ,0.f    ,1.0f},
            {20.0f  ,0.08f  ,0.0f   ,500.0f     ,50.0f  ,1.0f   ,1.0f   ,1.5f   ,1.0f   ,50.0f  ,0.f    ,1.5f},
            {30.0f  ,0.1f   ,0.0f   ,800.0f     ,80.0f  ,1.0f   ,1.0f   ,2.0f   ,1.0f   ,80.0f  ,0.f    ,2.0f},
        };
    };
    virtual void setRank(){
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

        
    };
    virtual void rankUp(){
        rankNum++;
        setRank();
        gamePtr->nowCost[this->owner]-=this->cost;
    };
    virtual void rankDown(){
        rankNum--;
        setRank();
        gamePtr->nowCost[this->owner]+=this->cost * gamePtr->returnCostMul;
    };

    virtual void applyEffect(StaticActor* staticActorPtr){
        return;
    };

    virtual void applyEffect(MobileActor* mobileActorPtr){
        return;
    };

    virtual void setScope(){//动态,在调用时检测边界
        spoceList.clear();
        int r=int(scope);
        for(int dx=-r;dx<=r;dx++){
            int dy_max=int(sqrt(scope*scope-dx*dx));
            for(int dy=-dy_max;dy<=dy_max;dy++){
                spoceList.push_back({dx,dy});
            }
        }
    }

    virtual void beHurted(int attackType,float attackNum){
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

    virtual void attack(){
        int attackCount=int(this->attackCount);//取整
        
        auto& nowTime=gamePtr->nowTime;
        
        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }

        auto& mobileActorMap=gamePtr->mobileActorMap;
        auto& eraseMobileActorSet=gamePtr->eraseMobileActorSet;

        std::priority_queue<std::tuple<float,int>,std::vector<std::tuple<float,int>>,std::greater<std::tuple<float,int>>>attackQueue;//距离,索引
        
        for(auto [x,y]:spoceList){
            for(auto index:mobileActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->mobileActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    attackQueue.emplace(d2,index);
                    while(attackQueue.size()-attackCount>0){
                        attackQueue.pop();
                    }
                }
            }
        }
        
        int residualAttackCount=attackCount-attackQueue.size();
        while(attackQueue.size()>0){//优先级为 先攻击移动单位，再攻击固定单位 其次，优先攻击最近
            auto&[d,index]=attackQueue.top();
            attackQueue.pop();
            auto& actor=gamePtr->staticActorPool[index];
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
        
        for(auto [x,y]:spoceList){
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
            auto&[d,index]=attackQueue.top();
            attackQueue.pop();
            auto& actor=gamePtr->staticActorPool[index];
            actor->beHurted(attackType,attackNum);
            this->applyEffect(actor);
            if(actor->hp<=0){
                eraseStaticActorSet.push_back(index);
            }
        }

    }

    virtual void move(){//移动以0.1s为单位
        if(path.size()!=0&&pathNum<path.size()-1&&(resistedList==nullptr||(resistedList!=nullptr&&(resistedList->size()<=resistListIndex||(*resistedList)[resistListIndex]==false)))){//有路&&未走完&&未阻挡
            float dx=path[pathNum][0]-this->x;
            float dy=path[pathNum][1]-this->y;
            float d=sqrt(dx*dx+dy*dy);
            float newX=this->x+dx/d*this->moveSpeed*0.1;
            float newY=this->y+dy/d*this->moveSpeed*0.1;
            if(int(newX)!=int(this->x)||int(newY)!=int(this->y)){
                gamePtr->staticActorPool[gamePtr->staticActorMap[int(this->y)][int(this->x)][gamePtr->staticActorMap[int(this->y)][int(this->x)].size()]]->mapListIndex=this->mapListIndex;
                erase_basedSwap(gamePtr->staticActorMap[int(this->y)][int(this->x)],this->mapListIndex);

                this->mapListIndex=gamePtr->mobileActorMap[newY][newX].size();
                gamePtr->mobileActorMap[newY][newX].push_back(poolIndex);
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

    virtual std::vector<std::array<int,2>> getPath(float goalX,float goalY){
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
        std::priority_queue<std::tuple<float,int,int>,std::vector<std::tuple<float,int,int>>,std::greater<std::tuple<float,int,int>>> openSet;//f,g,x,y
        std::unordered_set<std::pair<int,int>> closedSet;
        std::vector<std::vector<std::pair<int,int>>> parent(gamePtr->basicMap.size(),std::vector<std::pair<int,int>>(gamePtr->basicMap[0].size(),{-1,-1}));
        std::vector<std::vector<int>> g(gamePtr->basicMap.size(),std::vector<int>(gamePtr->basicMap[0].size(),-1));
        int dx[4]={0,1,0,-1};
        int dy[4]={1,0,-1,0};
        bool findFlag=false;
        openSet.push({abs(goalX-startX)+abs(goalY-startY),startX,startY});
        
        while(openSet.size()!=0&&!findFlag){
            auto& [f,tempX,tempY]=openSet.top();
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
        auto minStep=1000000;
        int& minStep=minStep;
        std::vector<std::vector<bool>>visited(gamePtr->basicMap.size(),std::vector<bool>(gamePtr->basicMap[0].size(),false));
        dfs(x,y,goalX,goalY,minStep,0,visited,resPath,resPath);
    }

    void dfs(int nowX,int nowY,int goalX,int goalY,int& minStep,int moveStep,std::vector<std::vector<bool>>&visited,std::vector<std::array<int,2>>tempPath,std::vector<std::array<int,2>>&resPath){
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

    virtual void skill1(){};
    virtual void skill2(){};
    
};

class SingleTower:public StaticActor{//单体攻击
    public:
    SingleTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y){}
    
};
class GroupAttackTower:public StaticActor{//群攻
    public:
    GroupAttackTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y){}
};
class SlowTower:public StaticActor{//减速
    public:
    SlowTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y){}
};
class CenterTower:public StaticActor{//枢纽
    public:
    CenterTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y){}
};

class meleeMobile:public MobileActor{//近战
    public:

};
class rangedMobile:public MobileActor{//远程
    public:
};
class defenseMobile:public MobileActor{//防御
    public:
};
class explosionMobile:public MobileActor{//自爆
    public:
};


class Game{
    public:
    std::vector<std::vector<bool>> basicMap;//0为障碍物 1为空地 2为A方固定单位 3为A方移动单位 4为B方固定单位 5为B方移动单位 以A方为例，仅0和2无法通过 //废弃,单格可能有多个不同阵营单位,仅保留0/1来标明障碍物
    std::vector<std::vector<std::vector<int>>> staticActorMap;//存每个格子的固定单位索引列表
    std::vector<std::vector<std::vector<int>>> mobileActorMap;//存每个格子的移动单位索引列表

    //以下均用,且仅用于转为张量传入CNN
    std::vector<std::vector<float>> meanHpMap;
    std::vector<std::vector<float>> meanAttackNumMap;
    std::vector<std::vector<float>> maxScopeMap;
    std::vector<std::vector<float>> meanAttackSpeedMap;
    std::vector<std::vector<float>> meanAttackCountMap;
    std::vector<std::vector<float>> meanCostMap;
    std::vector<std::vector<float>> meanCostRateMap;
    std::vector<std::vector<float>> meanSpeedMap;

    
    //固定单位子类对象池
    std::deque<StaticActor> basicTowerPool;
    std::deque<SingleTower> singleTowerPool;
    std::deque<GroupAttackTower> groupAttackTowerPool;
    std::deque<SlowTower> slowTowerPool;
    std::deque<CenterTower> centerTowerPool;

    //可移动单位子类对象池
    std::deque<MobileActor> basicMobilePool;
    std::deque<meleeMobile> meleeMobilePool;
    std::deque<rangedMobile> rangedMobilePool;
    std::deque<defenseMobile> defenseMobilePool;
    std::deque<explosionMobile> explosionMobilePool;

    std::vector<StaticActor*> staticActorPool;
    std::vector<MobileActor*> mobileActorPool;

    std::vector<std::vector<int>> aliveStaticList;
    std::vector<std::vector<int>> aliveMobileList;
    std::vector<int> eraseStaticActorSet;//后续进行sort与unique来处理 从大到小来进行 删除时需要swap待删除与队尾,且将所有对于基类指针池的索引一并swap
    std::vector<int> eraseMobileActorSet;//

    std::vector<float> nowCost;//根据owner作为索引来划分
    std::vector<float> costSpeed;

    float nowTime;

    float returnCostMul=0.5f;//返回时消耗的倍率 

    bool cnnSwitch;

    


    Game(std::vector<std::vector<bool>> basicMap){//初始地图,仅01
        this->basicMap=basicMap;

    }

    void creatStaticActor(int x,int y,int owner,int StaticActorType){
        switch(StaticActorType){
            case 0:
                singleTowerPool.emplace_back(SingleTower(this,owner,x,y));
                break;
            case 1:
                groupAttackTowerPool.emplace_back(GroupAttackTower(this,owner,x,y));
                break;
            case 2:
                slowTowerPool.emplace_back(SlowTower(this,owner,x,y));
                break;
            case 3:
                centerTowerPool.emplace_back(CenterTower(this,owner,x,y));
                break;
            default:
                throw std::runtime_error(std::format("[ERROR][Game] the StaticActorType is {}",StaticActorType));
        }
    }

    void creatMobileActor(int x,int y,int owner){
        aliveMobileList[owner].push_back(mobileActorPool.size());
        mobileActorPool.emplace_back(x,y);
    }

    void solveDeadActor(){
        std::vector<std::tuple<int,StaticActor*>> subclassPoolIndexs;
        for(auto index:eraseStaticActorSet){
            auto& actor=staticActorPool[index];
            subclassPoolIndexs.emplace_back(actor->subclassPoolIndex,actor);
        }
        std::sort(subclassPoolIndexs.begin(),subclassPoolIndexs.end(),std::greater<std::tuple<int,StaticActor*>>());
        std::sort(eraseStaticActorSet.begin(),eraseStaticActorSet.end(),std::greater<int>());
        for(auto eraseIndex:eraseStaticActorSet){//删除节点池元素并更新其索引
            auto& eraseActor=this->staticActorPool[eraseIndex];
            auto& endActor=this->staticActorPool[this->staticActorPool.size()-1];
            
            aliveStaticList[endActor->owner][endActor->aliveListIndex]=eraseIndex;
            endActor->aliveListIndex=eraseIndex;

            staticActorMap[int(eraseActor->y)][int(eraseActor->x)][eraseActor->mapListIndex]=eraseIndex;
            endActor->mapListIndex=eraseActor->mapListIndex;

            erase_basedSwap(staticActorPool,eraseIndex);
        }
        for(auto [subclassPoolIndex,eraseActor]:subclassPoolIndexs){//删除子类对象池元素并更新其索引
            eraseActor->dead();//把自身索引所在数组的最后一个元素与自己的索引更换位置,最后删除自身对象
        }

        eraseStaticActorSet.clear();

        std::vector<std::tuple<int,MobileActor*>> subclassPoolIndexs;
        for(auto index:eraseMobileActorSet){
            auto& actor=mobileActorPool[index];
            subclassPoolIndexs.emplace_back(actor->subclassPoolIndex,actor);
        }
        std::sort(subclassPoolIndexs.begin(),subclassPoolIndexs.end(),std::greater<std::tuple<int,MobileActor*>>());
        std::sort(eraseMobileActorSet.begin(),eraseMobileActorSet.end(),std::greater<int>());
        for(auto eraseIndex:eraseMobileActorSet){//删除节点池元素并更新其索引
            auto& eraseActor=this->mobileActorPool[eraseIndex];
            auto& endActor=this->mobileActorPool[this->mobileActorPool.size()-1];

            aliveMobileList[endActor->owner][endActor->aliveListIndex]=eraseIndex;
            endActor->aliveListIndex=eraseIndex;

            mobileActorMap[int(eraseActor->y)][int(eraseActor->x)][eraseActor->mapListIndex]=eraseIndex;
            endActor->mapListIndex=eraseActor->mapListIndex;

            erase_basedSwap(mobileActorPool,eraseIndex);
        }
        for(auto [subclassPoolIndex,eraseActor]:subclassPoolIndexs){//删除子类对象池元素并更新其索引
            eraseActor->dead();//把自身索引所在数组的最后一个元素与自己的索引更换位置,最后删除自身对象
        }
        eraseMobileActorSet.clear();

    }

    void run(){
        while(true){
            //之后绑定操作按钮 暂时先不处理创建逻辑

            //移动->攻击->删除

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

                        
            for(int i=0;i<nowCost.size();i++){
                nowCost[i]+=costSpeed[i]*0.1;
            }
            
            nowTime+=0.1;
        }
    }

};
