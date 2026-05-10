from typing import List

class StaticActor:
    """
    固定单位,例如箭塔 固定位置,
    """
    def __init__(self,attackNum:List,hp:List,rank:int,cost:List,boxes:List[int],scope:int):
        self.attackNum=attackNum
        self.hp=hp
        self.rank=rank
        self.cost=cost
        self.x,self.y=boxes
        self.scope=scope
    
    def attack(self,enemyMap:List[List[bool]],basicMap:List[List[int]]):
        #fightMap,仅包含敌方(True)
        #返回攻击目标索引列表
        pass

class MobileActor:
    pass
