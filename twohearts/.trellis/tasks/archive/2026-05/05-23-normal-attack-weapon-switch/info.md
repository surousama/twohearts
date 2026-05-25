# 普通攻击武器配置与持武器切换实现记录

## 本轮实际完成

1. 在 `AtwoheartsCharacter` 中新增最小武器显示承载：
   `WeaponVisualComponent`
   `WeaponVisualConfig`
2. 武器显示状态按当前阶段口径切换：
   普通攻击中强制显示为持武器；
   非普通攻击时，静止显示持武器；
   检测到移动输入或水平速度超过阈值时，切到非持武器状态。
3. 持武器与非持武器都走配置化 Socket：
   `EquippedSocketName`
   `UnequippedSocketName`
   对应各自的相对位移 / 旋转 / 缩放
4. 若未配置非持武器 Socket，可选择直接隐藏武器，作为当前阶段可接受的过渡实现。

## 代码落点

1. `Source/twohearts/twoheartsCharacter.h`
2. `Source/twohearts/twoheartsCharacter.cpp`

## 当前实现边界

1. 这是角色资源配置入口层的最小承载，不是完整武器系统。
2. 没有引入新的 Character 战斗状态机；动作优先级仍以现有 `Ability + CombatActionContext` 为准。
3. “普通状态”当前落地为：
   非移动时持武器；
   普攻激活时持武器；
   移动时非持武器。
4. 当前只处理武器显示 / 挂点切换，不处理伤害盒、命中判定、装备栏或多武器扩展。

## Unreal Editor 配置步骤

1. 打开 `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`。
2. 选中角色蓝图根类，确认父类为当前 `AtwoheartsCharacter`。
3. 在 Components 面板里确认出现 `WeaponVisualComponent`。
4. 在 Details 里找到 `Combat | Weapon`：
   把 `Weapon Mesh` 指向你要用的武器资源；
   设置 `Equipped Socket Name` 为持武器挂点；
   设置 `Unequipped Socket Name` 为背挂 / 腰挂挂点；
   按需要微调两组 Relative Transform；
   若当前没有非持武器挂点，可以先保留空，并开启 `Hide When Unequipped Socket Missing`。
5. 在角色骨骼或角色蓝图预览里补好对应 Socket。
   推荐至少准备：
   一个手部持武器 Socket；
   一个背部或腰部收刀 Socket。
6. 进入 PIE 后验证：
   静止待机时武器应在持武器挂点；
   开始移动后武器应切到非持武器挂点，或按配置隐藏；
   点击普通攻击后武器应回到持武器挂点。

## 建议首轮资源

1. `Content/Chinese_Warrior/Mesh/SM_Chinese_Hanfu_Dress_Warrior_Sword.uasset`
2. `Content/pack/StaticMesh/SM_LongSword.uasset`
3. `Content/pack/StaticMesh/SM_ShortSword.uasset`

## 已知限制

1. 当前切换依据是“普攻动作上下文 + 移动输入 / 速度阈值”，不是长期正式的公共战斗姿态系统。
2. 若角色未来引入冲刺、受击、格挡、锁定姿态等更复杂规则，应在后续公共语义层稳定后再统一收束。
