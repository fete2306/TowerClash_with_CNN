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
    std::unordered_set<MobileActor*> resistSet;//被阻挡敌人的集合 在析构时清空对方被阻挡的状态
    std::vector<std::array<int,2>> spoceList;//攻击范围覆盖的格子 非直接坐标而是偏移量[dx,dy] 获取真实坐标是x+dx,y+dy

    std::vector<specialEffect&> effecedtList;//存引用.对方死亡时会将其结束时间置0
    std::vector<specialEffect> effectList;//在析构时将造成的所有异常状态全部结束
    
    Game* gamePtr;
    int poolIndex;
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
        rankNum=0;
        get_attributeList();
        setRank();
        poolIndex=gamePtr->staticActorPool.size();
        gamePtr->staticActorMap[int(y)][int(x)].insert(poolIndex);
        gamePtr->aliveStaticList[owner].push_back(poolIndex);
        gamePtr->staticActorPool.push_back(this);
    };

    virtual void dead(){//之后可能大改 长时间运行需要清除缓存,后继节点索引会需要大幅度更新
        gamePtr->staticActorMap[int(y)][int(x)].erase(poolIndex);
        auto index=std::find(gamePtr->aliveStaticList[owner].begin(),gamePtr->aliveStaticList[owner].end(),poolIndex);
        gamePtr->aliveStaticList[owner].erase(index);


    };

    virtual void get_attributeList(){};
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
        this->rasistCount=attributeList[rankNum][11];
        if(scope!=lastScope)setScope();
    };
    
    virtual void rankUp(){
        rankNum++;
        setRank();
    };
    virtual void rankDown(){
        rankNum--;
        setRank();
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
                attackNum=attackNum*(1-attributeList[rankNum][0]);
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
        auto& mobilePool=gamePtr->mobileActorPool;
        auto& aliveMobileList=gamePtr->aliveMobileList;
        auto& mobileActorMap=gamePtr->mobileActorMap;
        auto& eraseMobileQueue=gamePtr->eraseMobileQueue;
        std::unordered_set<int> eraseMobileSet;//用于去重,随后放入堆
        for(auto [x,y]:spoceList){
            for(auto index:mobileActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->mobileActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    actor->beHurted(attackType,attackNum);
                    this->applyEffect(actor);
                    if(actor->hp<=0){
                        if(eraseMobileSet.insert(index).second){
                            eraseMobileQueue.push(index);//去重并自动排序
                            while(eraseMobileQueue.size()-attackCount>0){
                                eraseMobileQueue.pop();
                            }
                        }
                    }
                }
            }
        }
        if(eraseMobileQueue.size()>=attackCount){//攻击数量足够
            return;
        }

        int residualAttackCount=attackCount-eraseMobileQueue.size();

        auto& staticPool=gamePtr->staticActorPool;
        auto& aliveStaticList=gamePtr->aliveStaticList;
        auto& staticActorMap=gamePtr->staticActorMap;
        auto& eraseStaticQueue=gamePtr->eraseStaticQueue;
        std::unordered_set<int> eraseStaticSet;
        for(auto [x,y]:spoceList){
            for(auto index:staticActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    actor->beHurted(attackType,attackNum);
                    this->applyEffect(actor);
                    if(actor->hp<=0){
                        if(eraseStaticSet.insert(index).second){
                            eraseStaticQueue.push(index);//去重并自动排序
                            while(eraseStaticQueue.size()-residualAttackCount>0){
                                eraseStaticQueue.pop();
                            }
                        }
                    }
                }
            }
        }
   
    }


    virtual void move(std::pair<int,int> goalPos){
        gamePtr->mobileActorMap[int(this->y)][int(this->x)].erase(poolIndex);
        this->x=goalPos.first;
        this->y=goalPos.second;
        this->setScope();
        gamePtr->mobileActorMap[int(this->y)][int(this->x)].insert(poolIndex);
    };

    
    virtual void skill1(){};
    virtual void skill2(){};

};

class MobileActor{
    public:
    std::vector<std::vector<float>> attributeList;
    std::vector<std::array<int,2>> path;
    std::vector<std::array<int,2>> spoceList;//攻击范围覆盖的格子 非直接坐标而是偏移量[dx,dy] 获取真实坐标是x+dx,y+dy

    std::vector<specialEffect&> effecedtList;//存引用.对方死亡时会将其结束时间置0
    std::vector<specialEffect> effectList;//在析构时将造成的所有异常状态全部结束

    Game* gamePtr;
    StaticActor* resistSourceActor;
    int poolIndex;
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
    bool resisted;//是否被阻挡

    MobileActor(Game* gamePtr,int owner,float x,float y):gamePtr(gamePtr),owner(owner),x(x),y(y){
        rankNum=0;
        get_attributeList();
        setRank();
        poolIndex=gamePtr->mobileActorPool.size();
        gamePtr->mobileActorMap[int(y)][int(x)].insert(poolIndex);
        gamePtr->aliveMobileList[owner].push_back(poolIndex);
        gamePtr->mobileActorPool.push_back(this);
    };
    virtual void dead(){
        gamePtr->mobileActorMap[int(y)][int(x)].erase(poolIndex);
        auto index=std::find(gamePtr->aliveMobileList[owner].begin(),gamePtr->aliveMobileList[owner].end(),poolIndex);
        gamePtr->aliveMobileList[owner].erase(index);
    }

    virtual void get_attributeList();
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
    };
    virtual void rankDown(){
        rankNum--;
        setRank();
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
                attackNum=attackNum*(1-attributeList[rankNum][0]);
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
        auto& basicMap=gamePtr->basicMap;
        auto& mobilePool=gamePtr->mobileActorPool;
        auto& aliveMobileList=gamePtr->aliveMobileList;
        auto& mobileActorMap=gamePtr->mobileActorMap;
        auto& eraseMobileQueue=gamePtr->eraseMobileQueue;
        std::unordered_set<int> eraseMobileSet;//用于去重,随后放入堆

        std::vector<std::array<int,2>> spoceList;

        for(auto [x,y]:this->spoceList){//动态获取实时攻击范围
            if(this->x+x<0||this->x+x>=basicMap[0].size()||this->y+y<0||this->y+y>=basicMap.size()){
                continue;
            }
            spoceList.push_back({x,y});
        }

        for(auto [x,y]:spoceList){
            for(auto index:mobileActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->mobileActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    actor->beHurted(attackType,attackNum);
                    this->applyEffect(actor);
                    if(actor->hp<=0){
                        if(eraseMobileSet.insert(index).second){
                            eraseMobileQueue.push(index);//去重并自动排序
                            while(eraseMobileQueue.size()-attackCount>0){
                                eraseMobileQueue.pop();
                            }
                        }
                    }
                }
            }
        }
        if(eraseMobileQueue.size()>=attackCount){//攻击数量足够
            return;
        }

        int residualAttackCount=attackCount-eraseMobileQueue.size();

        auto& staticPool=gamePtr->staticActorPool;
        auto& aliveStaticList=gamePtr->aliveStaticList;
        auto& staticActorMap=gamePtr->staticActorMap;
        auto& eraseStaticQueue=gamePtr->eraseStaticQueue;
        std::unordered_set<int> eraseStaticSet;
        for(auto [x,y]:spoceList){
            
            for(auto index:staticActorMap[int(this->y+y)][int(this->x+x)]){
                auto& actor=gamePtr->staticActorPool[index];
                if(actor->owner==owner){
                    continue;
                }
                float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                if(d2<scope*scope){
                    actor->beHurted(attackType,attackNum);
                    this->applyEffect(actor);
                    if(actor->hp<=0){
                        if(eraseStaticSet.insert(index).second){
                            eraseStaticQueue.push(index);//去重并自动排序
                            while(eraseStaticQueue.size()-residualAttackCount>0){
                                eraseStaticQueue.pop();
                            }
                        }
                    }
                }
            }
        }
   
    }

    virtual void move(){//移动以0.1s为单位
        if(path.size()!=0&&pathNum<path.size()-1&&!resisted){//有路&&未走完&&未阻挡
            float dx=path[pathNum][0]-this->x;
            float dy=path[pathNum][1]-this->y;
            float d=sqrt(dx*dx+dy*dy);
            float newX=this->x+dx/d*this->moveSpeed*0.1;
            float newY=this->y+dy/d*this->moveSpeed*0.1;
            if(int(newX)!=int(this->x)||int(newY)!=int(this->y)){
                gamePtr->mobileActorMap[int(this->y)][int(this->x)].erase(poolIndex);
                gamePtr->mobileActorMap[int(newY)][int(newX)].insert(poolIndex);
            }
            this->x=newX;
            this->y=newY;
            for(auto index:gamePtr->staticActorMap[int(this->y)][int(this->x)]){
                auto& actor=*gamePtr->staticActorPool[index];
                if(actor.owner!=this->owner){
                    this->resisted=true;
                    actor.resistSet.insert(this);
                    break;
                }
            }
            if(d<=this->moveSpeed*0.1){
                pathNum++;
            }
        }
    }

    virtual std::vector<std::array<int,2>> getPath(float goalX,float goalY){
        this->resistSourceActor->resistSet.erase(this);//主动清除被阻挡状态
        this->resisted=false;
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

class SingleTower:public StaticActor{
    public:
    SingleTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y){}
    
};
class GroupAttackTower:public StaticActor{
    public:
    GroupAttackTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y){}
};
class SlowTower:public StaticActor{
    public:
    SlowTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y){}
};
class CenterTower:public StaticActor{
    public:
    CenterTower(Game* gamePtr,int owner,float x,float y):StaticActor(gamePtr,owner,x,y){}
};



class Game{
    public:
    std::vector<std::vector<bool>> basicMap;//0为障碍物 1为空地 2为A方固定单位 3为A方移动单位 4为B方固定单位 5为B方移动单位 以A方为例，仅0和2无法通过 //废弃,单格可能有多个不同阵营单位,仅保留0/1来标明障碍物
    std::vector<std::vector<std::unordered_set<int>>> staticActorMap;//存每个格子的固定单位索引列表
    std::vector<std::vector<std::unordered_set<int>>> mobileActorMap;//存每个格子的移动单位索引列表

    //以下均用,且仅用于转为张量传入CNN
    std::vector<std::vector<float>> meanHpMap;
    std::vector<std::vector<float>> meanAttackNumMap;
    std::vector<std::vector<float>> maxScopeMap;
    std::vector<std::vector<float>> meanAttackSpeedMap;
    std::vector<std::vector<float>> meanAttackCountMap;
    std::vector<std::vector<float>> meanCostMap;
    std::vector<std::vector<float>> meanCostRateMap;
    std::vector<std::vector<float>> meanSpeedMap;

    std::deque<SingleTower> singleTowerPool;
    std::deque<GroupAttackTower> groupAttackTowerPool;
    std::deque<SlowTower> slowTowerPool;
    std::deque<CenterTower> centerTowerPool;


    std::vector<StaticActor*> staticActorPool;//基类指针池恒定不变,与节点池一样,不删除元素
    std::vector<MobileActor*> mobileActorPool;

    std::vector<std::vector<int>> aliveStaticList;
    std::vector<std::vector<int>> aliveMobileList;
    std::priority_queue<int> eraseStaticQueue;//大根堆,自后向前删除
    std::priority_queue<int> eraseMobileQueue;

    std::vector<float> nowCost;//根据owner作为索引来划分
    std::vector<float> costSpeed;

    float nowTime;

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

            //严重问题 直接删除基类指针池中的元素会导致后续元素索引变更而失效 有很多信息都是根据索引来进行 否则需要单独存基类指针,更难管理
            // while(!eraseStaticQueue.empty()){
            //     int index=eraseStaticQueue.top();
            //     eraseStaticQueue.pop();
            //     staticActorPool.erase(staticActorPool.begin()+index);
            // }

            // while(!eraseMobileQueue.empty()){
            //     int index=eraseMobileQueue.top();
            //     eraseMobileQueue.pop();
            //     mobileActorPool.erase(mobileActorPool.begin()+index);
            // }
            //现在处理方案:留空洞,不改节点池和基类指针池，由存活列表与地图单位列表来管理单位存活状态
            while(!eraseStaticQueue.empty()){
                int index=eraseStaticQueue.top();
                eraseStaticQueue.pop();
                staticActorPool[index]->dead();
            }

            while(!eraseMobileQueue.empty()){
                int index=eraseMobileQueue.top();
                eraseMobileQueue.pop();
                mobileActorPool[index]->dead();
            }

                        
            for(int i=0;i<nowCost.size();i++){
                nowCost[i]+=costSpeed[i]*0.1;
            }
            
            nowTime+=0.1;
        }
    }

};
