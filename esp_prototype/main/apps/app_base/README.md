# AppBase 开发指南

`app_base` 是一个基于 LVGL 的轻量级应用管理框架，提供应用生命周期管理、视图切换动画、应用栈管理等功能。

## 1. 核心概念

- **AppBase**: 所有应用的基类，定义了应用的生命周期回调。
- **AppManager**: 应用管理器，负责应用的安装、卸载、启动、退出以及页面切换动画。
- **AppFactory**: 应用工厂接口，用于创建具体的应用实例。

## 2. 开发步骤

### 2.1 定义应用类

新建一个类继承自 `AppBase`，并根据需要重写生命周期函数。

```cpp
#include "AppBase.h"

class MyApp : public AppBase {
public:
    MyApp() {
        // 构造函数，初始化非UI资源
    }

    ~MyApp() {
        // 析构函数，释放资源
    }

    // [必选] 视图加载回调：在此处创建UI
    void onViewLoad() override {
        // _root 是框架自动创建的根对象 (lv_obj_t*)
        // 在此基础上添加控件
        lv_obj_t* label = lv_label_create(_root);
        lv_label_set_text(label, "Hello AppBase");
        lv_obj_center(label);
    }

    // [可选] 视图加载完成
    void onViewDidLoad() override {
        // UI创建完成后的逻辑，如数据加载
    }

    // [可选] 视图即将显示 (动画开始前)
    void onViewWillAppear() override {
        // 恢复定时器、动画等
    }

    // [可选] 视图完全显示 (动画结束后)
    void onViewDidAppear() override {
        // 页面完全进入焦点
    }

    // [可选] 视图即将消失 (动画开始前)
    void onViewWillDisappear() override {
        // 暂停定时器、动画等
    }

    // [可选] 视图完全消失 (动画结束后)
    void onViewDidDisappear() override {
        // 页面离开焦点
    }

    // [可选] 视图卸载
    void onViewUnLoad() override {
        // 清理UI相关的特定资源（LVGL对象会自动清理，无需手动删除）
    }
};
```

### 2.2 实现 AppFactory

你需要实现 `AppFactory` 接口，以便 `AppManager` 能够根据名称创建应用实例。

```cpp
#include "AppFactory.h"
#include "MyApp.h"

class MyAppFactory : public AppFactory {
public:
    AppBase* createApp(const char* name) override {
        if (strcmp(name, "MyApp") == 0) {
            return new MyApp();
        }
        return nullptr;
    }
};
```

### 2.3 初始化与运行

在系统初始化阶段（如 `main` 函数中），实例化 `AppManager` 并安装应用。

```cpp
// 1. 创建工厂
MyAppFactory factory;

// 2. 创建管理器
AppManager manager(&factory);

// 3. 安装应用 (仅注册，不立即显示)
manager.installApp("MyApp", "MyApp"); 
// 参数1: 类名 (对应工厂createApp中的name)
// 参数2: 实例名 (用于后续查找和跳转，通常与类名相同)

// 4. 启动应用
manager.pushApp("MyApp");
```

## 3. 应用管理与导航

在 `AppBase` 的派生类中，可以通过 `getManager()` 获取管理器实例进行跳转。

- **打开新应用**:
  ```cpp
  getManager()->pushApp("NextApp");
  ```

- **返回上一级**:
  ```cpp
  getManager()->popApp();
  ```

- **返回主页 (栈底应用)**:
  ```cpp
  getManager()->backToHome();
  ```

- **替换当前应用 (无返回栈)**:
  ```cpp
  getManager()->replaceApp("AnotherApp");
  ```

## 4. 生命周期图示

```text
installApp() -> onCustomPreConfig()

pushApp() 
   |
   v
[Load State]
   |--> onViewLoad()      (创建 _root)
   |--> onViewDidLoad()
   |
   v
[Will Appear State]
   |--> onViewWillAppear()
   |--> [Animation Start]
   |
   v
[Did Appear State]
   |--> [Animation End]
   |--> onViewDidAppear()
   |
   v
[Activity State] (用户交互)
   |
   v
popApp() / pushApp(new)
   |
   v
[Will Disappear State]
   |--> onViewWillDisappear()
   |--> [Animation Start]
   |
   v
[Did Disappear State]
   |--> [Animation End]
   |--> onViewDidDisappear()
   |
   v
[Unload State] (如果未开启缓存)
   |--> onViewUnLoad()
   |--> onViewDidUnLoad()
   |--> [Destroy _root]
```

## 5. 其它功能

- **数据传递 (Stash)**:
  `pushApp` 和 `replaceApp` 支持传递 `AppStash` 数据包。

  ```cpp
  // 准备数据
  struct MyData {
      int id;
      char msg[32];
  };
  MyData data = {1, "Hello"};

  AppBase::AppStash stash;
  stash.ptr = &data;
  stash.size = sizeof(MyData);

  // 发送数据
  getManager()->pushApp("NextApp", &stash);

  // 在 NextApp 中接收数据
  // bool stashExtract(void* ptr, uint32_t size);
  MyData recvData;
  if (stashExtract(&recvData, sizeof(MyData))) {
      // 成功接收
  }
  ```

- **自定义转场动画**:
  使用 `setGlobalLoadAnimType` 设置全局动画，或在 `AppBase` 中设置 `setCustomLoadAnimType`。

- **页面缓存**:
  调用 `setCustomCacheEnable(true)` 可让页面在退出后不销毁，下次进入时直接显示（不走 onViewLoad）。

## 6. API 参考

### AppBase (应用基类)

| 方法 | 说明 |
| --- | --- |
| `lv_obj_t* getRoot()` | 获取当前 App 的根对象指针 (通常用于添加子控件) |
| `AppManager* getManager()` | 获取 AppManager 实例指针 |
| `void setCustomCacheEnable(bool en)` | 设置是否启用手动缓存 (退出时不销毁) |
| `void setCustomAutoCacheEnable(bool en)` | 设置是否启用自动缓存 (默认启用，除非手动禁用) |
| `bool stashExtract(void* ptr, uint32_t size)` | 从暂存区提取传递的数据 |
| `void setCustomLoadAnimType(uint8_t animType, uint16_t time, lv_anim_path_cb_t path)` | 设置当前 App 的自定义加载动画 |

### AppManager (应用管理器)

| 方法 | 说明 |
| --- | --- |
| `bool installApp(const char* className, const char* appName)` | 安装 App (注册并初始化) |
| `bool unInstallApp(const char* appName)` | 卸载 App (销毁并从对象池移除) |
| `bool pushApp(const char* name, const AppBase::AppStash* stash)` | 压栈进入新 App |
| `bool popApp()` | 弹栈返回上一个 App |
| `bool replaceApp(const char* name, const AppBase::AppStash* stash)` | 替换当前栈顶 App |
| `bool backToHome()` | 清空栈并返回栈底 App (首页) |
| `const char* getAppPrevName()` | 获取上一个 App 的名称 |
| `void setGlobalLoadAnimType(LoadAnim anim, uint16_t time, lv_anim_path_cb_t path)` | 设置全局默认加载动画 |
| `void setRootDefaultStyle(lv_style_t* style)` | 设置 App 根对象的默认样式 |

