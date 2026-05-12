#include<iostream>
#include<vector>
#include<array>
#include<unordered_set>
#include<format>
#include<algorithm>
#include<optional>
#include<deque>

class StaticActor{
    public:
    std::vector<std::vector<float>> attributeList;/*
    抗性 (maxRank,attackTypeCount,1)
        
        生命
        生命上限
        
        攻击力
        攻击范围
        攻击数量
        攻击速度

        造成攻击减速倍率
        造成移动减速倍率
        脆弱(加伤)效果

        到达该等级的费用消耗
        费用产出(/s)*/
    std::vector<float> nowAttribute;
    std::vector<float> stateEndTime;//[0]攻击 [1]移动 [2]减速 [3]脆弱 
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

    virtual void get_attributeList();
    virtual void setRank();
    virtual void rankUp(){
        rankNum++;
        setRank();
    };
    virtual void rankDown(){
        rankNum--;
        setRank();
    };

    virtual void applyEffect(StaticActor* staticActorPtr,MobileActor* mobileActorPtr,bool flag){
        return;
    };

    virtual void beHurted(int attackType,float attackNum){
        this->hp-=attackNum;
    }

    virtual void attack(std::vector<StaticActor*>& StaticPool,std::vector<MobileActor*>& MobilePool,std::vector<std::vector<int>>& aliveStaticList,std::vector<std::vector<int>>& aliveMobileList,std::unordered_set<int>& eraseStaticSet,std::unordered_set<int>& eraseMobileSet,float nowTime){
        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }
        for(int i=0;i<attackCount;i++){
            bool attacked=false;
            for(int j=0;j<aliveStaticList.size()&&!attacked;j++){
                if(j==owner){
                    continue;
                }
                for(int k=0;k<aliveStaticList[j].size();k++){
                    auto actor=StaticPool[aliveStaticList[j][k]];
                    float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                    if(d2<scope*scope){
                        this->applyEffect(actor,nullptr,false);
                        actor->beHurted(attackType,attackNum);
                        if(actor->hp<=0){
                            eraseStaticSet.insert(aliveStaticList[j][k]);
                            aliveStaticList[j].erase(aliveStaticList[j].begin()+k);
                        }
                        attacked=true;
                        break;
                    }
                }
            }
            if(attacked){
                continue;
            }
            for(int j=0;j<aliveMobileList.size()&&!attacked;j++){
                if(j==owner){
                    continue;
                }
                for(int k=0;k<aliveMobileList[j].size();k++){
                    auto actor=MobilePool[aliveMobileList[j][k]];
                    float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                    if(d2<scope*scope){
                        this->applyEffect(nullptr,actor,false);
                        actor->beHurted(attackType,attackNum);
                        if(attackNum>=actor->hp){
                            eraseMobileSet.insert(aliveMobileList[j][k]);
                            aliveMobileList[j].erase(aliveMobileList[j].begin()+k);
                        }
                        attacked=true;
                        break;
                    }
                }
            }
            if(!attacked){//无单位可攻击
                break;
            }
        }
    };
    virtual void move(std::pair<int,int> goalPos){
        this->x=goalPos.first;
        this->y=goalPos.second;
    };
    virtual void del(){};
    virtual void skill1(){};
    virtual void skill2(){};

};

class MobileActor{
    public:
    std::vector<std::vector<float>> attributeList;
    std::vector<float> nowAttribute;
    std::vector<float> stateEndTime;//[0]攻击 [1]移动 [2]减速 [3]脆弱 
    std::vector<std::array<int,2>> path;
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

    virtual void get_attributeList();
    virtual void setRank();
    virtual void rankUp(){
        rankNum++;
        setRank();
    };
    virtual void rankDown(){
        rankNum--;
        setRank();
    };

    virtual void applyEffect(StaticActor* staticActorPtr,MobileActor* mobileActorPtr,bool flag){
        return;
    };

    virtual void beHurted(int attackType,float attackNum){
        this->hp-=attackNum;
    }

    virtual void attack(std::vector<StaticActor*>& StaticPool,std::vector<MobileActor*>& MobilePool,std::vector<std::vector<int>>& aliveStaticList,std::vector<std::vector<int>>& aliveMobileList,std::unordered_set<int>& eraseStaticSet,std::unordered_set<int>& eraseMobileSet,float nowTime){
        if(nowTime-lastAttackTime<1/attackSpeed){
            return;
        }
        for(int i=0;i<attackCount;i++){
            bool attacked=false;
            for(int j=0;j<aliveStaticList.size()&&!attacked;j++){
                if(j==owner){
                    continue;
                }
                for(int k=0;k<aliveStaticList[j].size();k++){
                    auto actor=StaticPool[aliveStaticList[j][k]];
                    float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                    if(d2<scope*scope){
                        this->applyEffect(actor,nullptr,false);
                        actor->beHurted(attackType,attackNum);
                        if(actor->hp<=0){
                            eraseStaticSet.insert(aliveStaticList[j][k]);
                            aliveStaticList[j].erase(aliveStaticList[j].begin()+k);
                        }
                        attacked=true;
                        break;
                    }
                }
            }
            if(attacked){
                continue;
            }
            for(int j=0;j<aliveMobileList.size()&&!attacked;j++){
                if(j==owner){
                    continue;
                }
                for(int k=0;k<aliveMobileList[j].size();k++){
                    auto actor=MobilePool[aliveMobileList[j][k]];
                    float d2=abs(this->x-actor->x)*abs(this->x-actor->x)+abs(this->y-actor->y)*abs(this->y-actor->y);
                    if(d2<scope*scope){
                        this->applyEffect(nullptr,actor,false);
                        actor->beHurted(attackType,attackNum);
                        if(attackNum>=actor->hp){
                            eraseMobileSet.insert(aliveMobileList[j][k]);
                            aliveMobileList[j].erase(aliveMobileList[j].begin()+k);
                        }
                        attacked=true;
                        break;
                    }
                }
            }
            if(!attacked){//无单位可攻击
                break;
            }
        }
    };
    virtual void move(){//移动以0.1s为单位
        if(path.size()!=0&&pathNum<path.size()-1){
            float dx=path[pathNum][0]-this->x;
            float dy=path[pathNum][1]-this->y;
            float d=sqrt(dx*dx+dy*dy);
            this->x+=dx/d*this->moveSpeed*0.1;
            this->y+=dy/d*this->moveSpeed*0.1;
            if(d<=this->moveSpeed*0.1){
                pathNum++;
            }
        }
    }
    virtual void del(){};

    virtual std::vector<std::array<int,2>> getPath(float goalX,float goalY,std::vector<std::vector<int>>&ownerShip){
        std::vector<std::array<int,2>> resPath;
        std::vector<std::vector<bool>>visited(ownerShip.size(),std::vector<bool>(ownerShip[0].size(),false));
        auto minStep=1000000;
        int& minStep=minStep;
        dfs(x,y,goalX,goalY,minStep,0,ownerShip,visited,resPath,resPath);//不包含起点
        this->pathNum=0;
        return resPath;
    }
    
    void dfs(int nowX,int nowY,int goalX,int goalY,int& minStep,int moveStep,std::vector<std::vector<int>>&ownerShip,std::vector<std::vector<bool>>&visited,std::vector<std::array<int,2>>tempPath,std::vector<std::array<int,2>>&resPath){
        if(nowX==goalX&&nowY==goalY){
            if(moveStep<minStep){
                resPath=tempPath;
                minStep=moveStep;
            }
            return;
        }
        if(nowX<0||nowX>=ownerShip[0].size()||nowY<0||nowY>=ownerShip.size()||ownerShip[nowY][nowX]==0||ownerShip[nowY][nowX]==(owner+1)*2||visited[nowY][nowX]){
            return;
        }
        int dx[4]={0,1,0,-1};
        int dy[4]={1,0,-1,0};
        visited[nowY][nowX]=true;

        for(int i=0;i<4;i++){
            int nextX=nowX+dx[i];
            int nextY=nowY+dy[i];
            if(nextX<0||nextX>=ownerShip[0].size()||nextY<0||nextY>=ownerShip.size()||ownerShip[nextY][nextX]==0||ownerShip[nowY][nowX]==(owner+1)*2||visited[nextY][nextX]){
                continue;
            }
            std::array<int,2> pos{nextX,nextY};
            tempPath.push_back(pos);
            dfs(nextX,nextY,goalX,goalY,minStep,moveStep+1,ownerShip,visited,tempPath,resPath);
         }
        visited[nowY][nowX]=false;
    }

    virtual void skill1(){};
    virtual void skill2(){};
    
};

class basicTower:public StaticActor{
    public:
    
};



class Game{
    public:
    std::vector<std::vector<int>> ownerShip;//0为障碍物 1为空地 2为A方固定单位 3为A方移动单位 4为B方固定单位 5为B方移动单位 以A方为例，仅0和2无法通过

    //以下均用,且仅用于转为张量传入CNN
    std::vector<std::vector<float>> meanHpMap;
    std::vector<std::vector<float>> meanAttackNumMap;
    std::vector<std::vector<float>> maxScopeMap;
    std::vector<std::vector<float>> meanAttackSpeedMap;
    std::vector<std::vector<float>> meanAttackCountMap;
    std::vector<std::vector<float>> meanCostMap;
    std::vector<std::vector<float>> meanCostRateMap;
    std::vector<std::vector<float>> meanSpeedMap; 

    std::deque<basicTower> basicTowerPool;



    std::vector<StaticActor*> staticActorPool;
    std::vector<MobileActor*> mobileActorPool;

    std::vector<std::vector<int>> aliveStaticList;
    std::vector<std::vector<int>> aliveMobileList;
    std::vector<float> nowCost;//根据owner作为索引来划分
    std::vector<float> costSpeed;
    int nowtime;

    bool cnnSwitch;

    


    Game(std::vector<std::vector<int>> ownerShip){//初始地图,仅01
        this->ownerShip=ownerShip;
    }

    void creatStaticActor(int x,int y,int owner){
        aliveStaticList[owner].push_back(staticActorPool.size());
        staticActorPool.emplace_back(x,y);
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

            std::unordered_set<int> eraseStaticList;
            std::unordered_set<int> eraseMobileList;
            for(int i=0;i<staticActorPool.size();i++){
                if(eraseStaticList.find(i)==eraseStaticList.end()){
                    staticActorPool[i]->attack(staticActorPool,mobileActorPool,aliveStaticList,aliveMobileList,eraseStaticList,eraseMobileList,nowtime);
                }
            }
            for(int i=0;i<mobileActorPool.size();i++){
                if(eraseMobileList.find(i)==eraseMobileList.end()){
                    mobileActorPool[i]->attack(staticActorPool,mobileActorPool,aliveStaticList,aliveMobileList,eraseStaticList,eraseMobileList,nowtime);
                }
            }

            // for(int i=0;i<eraseStaticList.size();i++){
            //     staticActorPool.erase(staticActorPool.begin()+eraseStaticList[i]);
            // }
            // for(int i=0;i<eraseMobileList.size();i++){
            //     mobileActorPool.erase(mobileActorPool.begin()+eraseMobileList[i]);
            // }
            // for(auto d:eraseStaticList){
            //     staticActorPool.erase(static)
            // }
            //删除逻辑有问题
            //
            std::vector<int> tempEraseStaticList(eraseStaticList.begin(),eraseStaticList.end());//需要从后向前删
            std::vector<int> tempEraseMobileList(eraseMobileList.begin(),eraseMobileList.end());
            std::sort(tempEraseStaticList.begin(),tempEraseStaticList.end(),[](int a,int b){return a>b;});
            std::sort(tempEraseMobileList.begin(),tempEraseMobileList.end(),[](int a,int b){return a>b;});

            for(auto d:tempEraseStaticList){
                staticActorPool.erase(staticActorPool.begin()+d);
            }
            for(auto d:tempEraseMobileList){
                mobileActorPool.erase(mobileActorPool.begin()+d);
            }
            
            for(int i=0;i<nowCost.size();i++){
                nowCost[i]+=costSpeed[i]*0.1;
            }
            
            nowtime+=0.1;
        }
    }

};
