# USB hub 与 mux芯片选型

## 参考设计
### 1.能够满足：KVM切换功能、DP输出功能

[KVM DP](https://oshwhub.com/nicicily/hdmi2-qie-1)

- 优点：唯一一个dp kvm参考例子
- 缺点：需要占用两个输入usb，两台电脑分别要插入一个dp视频线和一个usb线
- 实现方法：一个芯片专门用来切换视频新好，一个芯片用来切换USB信号
- 相似参考：[dp切换功能](https://oshwhub.com/aknice/4k-144hz-10bit-si-xuan-yi-de-dp1-4-qie-huan-qi-fen-pei-qi)

### 2.能够满足：PD充电+usb hub功能 USB3.0
[text](https://oshwhub.com/eda_kwazdnpkc/ch634w6g_test)

- 优点：速度快，usb口电流功耗显示、设计简单
- 缺点：没有dp输出的设计参考，没有kvm功能



总结1：如果一定要kvm＋dp功能，第一个设计是唯一一个有参考的。目前没有找到2个typec输入的dp kvm参考设计，但理论上是可行的，只需要把上面的芯片排列组合。

总结2：
1. 如果要KVM（DP）+HUB + 仅TYPEC输入功能，只能开始研发。
2. 如果要KVM（DP）+ HUB功能 输入两个typec+两个dp，有参考例子。
3. 如果只要单设备的usb转dp+3个usb，有参考设计，轻量化。
4. 只要KM（两台电脑不切换视频信号），有参考设计，但是功能残疾。

## 推荐芯片选型：

### CH634
CH634 是一顆以 USB 3.2 Gen1（5Gbps）為主的 4 埠 SuperSpeed HUB 控制器，並在部分型號中整合 Type‑C（正反插自適應的 USB3 PHY）與 USB PD PHY，可做為「USB 擴展塢」或「PDHUB 快充 HUB」的核心。其架構上同時包含 USB2 HUB 與 USB3 HUB（各自的控制邏輯與 PHY），並在系統框圖中可見內建處理器（RISC‑V4C）與 USB PD 模組。

若要在 CH634 擴展塢上提供 DP/HDMI 視訊輸出，常見可行路徑：
DP Alt Mode（USB‑C 的 Alternate Mode）：由主機（筆電/手機）提供原生 DP Main Link，擴展塢內部用 USB‑C/PD 控制器完成 Alt Mode/VDM 協商，並用 高速 MUX/重定時/重驅動把 Type‑C 的高速線對（lanes）在「USB3」與「DP」之間重新分配（例如 2‑lane DP + 1‑lane USB3 的多工方案）。



## 关键参数
1. USB速率：对于一般设备USB2.0足够。 对于U盘、外接硬盘推荐USB3.0
2. DP信号：1.4才能满足2k144hz输出。HDMI无法满足该性能，只能选择DP



## 参考风险与成本控制：
1. PD功能设计可能导致烧电脑主板，我建议不去做这个功能，可以靠PD来产生5v供给设备，这样就不需要单靠电脑USB的5v2A（大部分时候都跑不到2A）了
2. 如果要做DP视频输出功能：
3. TVS是非常重要的
4. 铝合金外壳控制在两面打孔总共10个孔的情况下，根据尺寸大小成本在30-50以内（参考嘉立创铝合金外壳加工外壳）
