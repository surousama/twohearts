# 基础闪避第二轮跑测与白盒反馈

## 1. 第二轮跑测结果

### 1.1 冷却问题已明显改善

1. 结果：
   当前闪避冷却看起来已经生效。
   角色可以重复使用闪避。
2. 结论：
   上一轮“第一次闪避后永久被 cooldown 阻挡”的问题，本轮暂未复现。

### 1.2 上一轮其余问题本轮暂未复现

1. 结果：
   上一轮“闪避后闪回原位”和“闪避后移动模式被改掉”的问题，本轮暂时没有稳定跑出来。
2. 结论：
   这两项先记为“本轮暂未复现”，不直接判定彻底解决，后续还要继续盯。

## 2. 第二轮跑测问题

### 2.1 靶向移动下，向右移动时按闪避会先右转 90 度再闪避，动作形成之字形

1. 优先级：
   P0
2. 现象：
   当前角色处于靶向移动。
   当角色向右移动时按下闪避，角色会先继续向右旋转约 `90` 度，随后再执行闪避。
   最终视觉结果像先拐一下再闪，形成之字形。
3. 预期：
   靶向移动下，向右移动时按闪避，应直接按“右闪”的口径执行。
   不应先把角色整体转到右侧再闪。
4. 实际：
   当前方向选择逻辑和角色朝向处理逻辑存在冲突。
5. 影响：
   直接破坏靶向移动下的闪避手感。
   当前“8 方向闪避”在靶向移动场景下不能认为已经正确落地。

## 3. 白盒问题

### 3.1 闪避开始时强制把角色朝向改成闪避方向，直接导致靶向移动下先转身再闪

1. 优先级：
   P0
2. 问题定位：
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:181>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:184>)
3. 问题说明：
   当前闪避开始时会：
   缓存旋转参数；
   把 `bUseControllerRotationYaw` 改成 `false`；
   然后直接 `SetActorRotation(DodgeDirection.Rotation())`。
   这在靶向移动里是有问题的。
   因为靶向移动下，角色朝向通常应该继续保持锁定目标或当前战斗朝向，而不是为了闪避方向先整体转身。
   现在代码相当于把“移动输入方向”和“角色朝向”绑死成了一件事，所以一按右闪就会先右转。
4. 和跑测现象的对应：
   直接对应问题 `2.1`。
5. 建议的修改方式：
   靶向移动场景下，不要在闪避开始时直接 `SetActorRotation(DodgeDirection.Rotation())`。
   闪避方向应驱动闪避表现和位移，不应强制改最终角色朝向。
   如果资源确实需要临时朝向修正，也应该区分：
   靶向移动下是否允许转身；
   非靶向移动下是否允许转身。
   不要共用一套“先转到 DodgeDirection 再播”的逻辑。

### 3.2 闪避世界方向用控制器朝向算，方向命名却用角色朝向算，坐标系混用

1. 优先级：
   P0
2. 问题定位：
   [twoheartsCharacter.cpp](</g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:444>)
   [twoheartsCharacter.cpp](</g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:463>)
3. 问题说明：
   `GetDesiredDodgeDirectionWorld` 里，闪避世界方向来自：
   `Controller ControlRotation` + `CachedMoveInput`。
   但 `GetDesiredDodgeDirectionName` 里，方向命名又是拿这个世界方向去和：
   `ActorForward` / `ActorRight`
   做夹角判断。
   这等于“方向来源”按控制器坐标系算，“方向分类”按角色坐标系算。
   在普通自由移动里这两套坐标系可能接近。
   但在靶向移动里，角色朝向和控制器朝向本来就不一定一致，所以很容易出现：
   世界位移方向是对的，
   但方向命名、Montage 选择、角色朝向处理不一致。
4. 和跑测现象的对应：
   这是这轮“右移右闪却出现转身之字形”的根本原因之一。
5. 建议的修改方式：
   先统一“闪避方向到底参考谁”。
   如果当前项目已经明确是靶向移动口径，就应优先用角色战斗朝向/锁定朝向作为方向分类基准。
   不要一半用控制器朝向，一半用角色朝向。
   最少也要保证：
   世界方向计算、
   8 方向命名、
   Montage 选择、
   是否转身
   这四件事全部使用同一套坐标参考系。

### 3.3 当前实现没有区分“靶向移动闪避”和“非靶向移动闪避”的处理分支

1. 优先级：
   P1
2. 问题定位：
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:173>)
   [twoheartsCharacter.cpp](</g:/twohearts/twohearts/Source/twohearts/twoheartsCharacter.cpp:444>)
3. 问题说明：
   当前代码默认所有场景都走同一套闪避方向和朝向处理：
   同一套输入方向解析；
   同一套角色旋转处理；
   同一套方向 Montage 选择。
   但你当前已经明确：
   现在角色都是靶向移动。
   这意味着闪避其实不该再沿用“自由移动角色常规翻滚”的默认思路。
   当前代码没有看到任何“如果是靶向移动，则按靶向规则处理”的分支或状态入口。
4. 当前影响：
   就算把 `SetActorRotation` 这一个点去掉，后面仍有可能继续在方向命名或资源选择上冒偏差。
5. 建议的修改方式：
   明确给闪避方向逻辑补一个“当前移动模式/战斗模式”入口。
   至少区分：
   靶向移动：保持战斗朝向，只处理相对方向闪避；
   非靶向移动：可按自由移动逻辑处理。
   不建议继续把两种模式混成一套。

### 3.4 Notify 回调没有校验当前通知是否来自本次闪避 Montage，后续容易误吃别的 Montage 事件

1. 优先级：
   P2
2. 问题定位：
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:385>)
   [TwoHeartsGA_Dodge.cpp](</g:/twohearts/twohearts/Source/twohearts/TwoHearts/Combat/Gameplay/Abilities/TwoHeartsGA_Dodge.cpp:490>)
3. 问题说明：
   当前 `OnPlayMontageNotifyBegin` 绑定后，只按 `NotifyName` 判断。
   没有看到根据 `BranchingPointNotifyPayload` 再校验“这个 Notify 是否来自当前 ActiveDodgeMontage”。
   这意味着如果别的 Montage 也刚好有同名 Notify，闪避 Ability 也可能误响应。
4. 当前影响：
   这轮跑测未必立刻复现。
   但后面动作变多后，可能出现：
   无敌帧误开；
   无敌帧误关；
   闪避被错误 Finish。
5. 建议的修改方式：
   在 Notify 回调里补“当前 Montage 是否就是 ActiveDodgeMontage”的过滤。
   不要只按 Notify 名称响应。

## 4. 当前结论

1. 第二轮修改至少已经把冷却永久卡死的问题压下去了，这是有效进展。
2. 当前新的主问题已经收敛到“靶向移动下的闪避方向与朝向处理错误”。
3. 这轮最该优先修的是：
   `3.1`
   `3.2`
   这两个问题本质上是同一组方向坐标系和朝向策略问题。

## 5. 建议程序先修顺序

1. 先修 `3.1`，把“闪避时强制转到 DodgeDirection”这条逻辑拿掉或改成按模式分支。
2. 再修 `3.2`，统一闪避方向计算和方向命名的参考坐标系。
3. 再补 `3.3`，把靶向移动和非靶向移动的闪避逻辑正式拆开。
4. 最后补 `3.4`，避免后面动作一多又出现 Notify 误触发。

## 6. 第三轮复测重点

1. 靶向移动下，向右移动时按闪避，是否能直接右闪，不再先右转。
2. 靶向移动下，左、后、斜方向闪避是否都保持同一套相对方向口径。
3. 冷却是否仍然稳定正常。
4. 上一轮暂未复现的问题：
   闪回原位；
   闪避后移动模式错误；
   这两项也要继续顺手回归。
