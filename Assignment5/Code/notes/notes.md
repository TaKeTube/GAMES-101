#### 智能指针

```C++
void Add(std::unique_ptr<Object> object) { objects.push_back(std::move(object)); }
void Add(std::unique_ptr<Light> light) { lights.push_back(std::move(light)); }
```

uniqut_ptr是一种对资源具有排他性拥有权的智能指针，即一个对象资源只能同时被一个unique_ptr指向

unique_ptr不能被复制或者拷贝

```C++
auto sph1 = std::make_unique<Sphere>(Vector3f(-1, 0, -12), 2);
```

std::make_unique<Sphere> 创建一个指向Sphere类的unique pointer

不用考虑delete 好像也不会内存溢出

https://www.cnblogs.com/leijiangtao/p/12046648.html



#### C++的属性指示符

```C++
[[nodiscard]] const std::vector<std::unique_ptr<Object> >& get_objects() const { return objects; }
```

If [[nodiscard]] is marked on a function, it stresses that the return value is not intended to be discarded. If the return value is discarded, a warning is issued:

```C++
[[nodiscard]] int f();
void g() {
	f(); // WARNING: return value of nodiscard function discarded.
}
```

If [[nodiscard]] is marked on a type, it makes it so that all functions that return that type are implicitly [[nodiscard]].

```C++
[[nodiscard]] struct S { ... };
S f();
void g() {
	f(); // WARNING: return value of nodiscard type discarded.
}
```

https://blog.csdn.net/duandianr/article/details/72784828

#### = default

就是指示编译器生成一个合成版本的构造/析构函数（包括拷贝构造，赋值构造，移动构造，移动赋值构造）

 这里用构造函数来讨论：
 如果用户没有定义，在需要的时候编译器会生成一个默认的构造函数，这个规则你应当是知道的。
 但是，假如用户定义了其他构造函数（比如有参数的，或者参数不同的），那么编译器无论如何就不会再合成默认的构造函数了
 那么如果要使用无参数版本，用户必须显示的定义一个无参版本的构造函数。

 如果使用default指示的办法，那么可以产出比用户定义的无参构造函数有更优的代码（毕竟是编译器干活）
 还有一个作用可以让使用者一眼就看出这是一个合成版本的构造函数（相当于知道类的作者没干其他事情）
 其实一看，也没太多作用，delete这个关键字倒是还行。

https://bbs.csdn.net/topics/392362190?page=1

#### 虚析构函数

直接的讲，C++中基类采用virtual虚析构函数是为了防止内存泄漏。具体地说，如果派生类中申请了内存空间，并在其析构函数中对这些内存空间进行释放。假设基类中采用的是非虚析构函数，当删除基类指针指向的派生类对象时就不会触发动态绑定，因而只会调用基类的析构函数，而不会调用派生类的析构函数。那么在这种情况下，派生类中申请的空间就得不到释放从而产生内存泄漏。所以，为了防止这种情况的发生，C++中基类的析构函数应采用virtual虚析构函数。

https://www.cnblogs.com/liushui-sky/p/5824919.html

用派生类指针删除派生类实例时才会调用派生类的析构函数，进而调用基类的析构函数

用基类指针删除派生类实例时，若基类的析构函数不是虚函数，则只会调用基类的析构函数，派生类申请的空间无法得到释放

#### Override

override相当于提醒编译器 如果忘了重写函数 编译器一定报错 容易纠正错误

如果不加override 那么程序很可能通过编译 但是运行的时候调用的是基类的函数 这个错误很难被发现 等待的可能是漫长的debug

https://www.cnblogs.com/xinxue/p/5471708.html

#### inline 内联函数

https://www.cnblogs.com/fnlingnzb-learner/p/6423917.html

#### std::optional

编程中，我们经常会需要表示或处理一个“可能为空”的变量，可能是一个为包含任何元素的容器，可能是一个类型的指针没有指向任何有效的对象实例，再或者是一个对象没有被赋予有效的值。通常处理这类问题意味着写更多的代码来处理这些“特殊”情况，很容易导致代码变得冗余，可读性变差或者容易出错。比如，我们很容易想到的如下三种方法:

1. 使用特殊值标记，如-1， infinity或者nullptr。这种方法几乎是最常用的方法，在调用一个对象之前，需要先将其与特殊值进行比较保证其有效性。但是这种方法可能比较脆弱，因为在有些corner case下，这些“特殊值”可能也有意义。
2. 如果函数可能出错导致返回结果是无效值，我们会引入boolean或者error code作为函数返回值来表示结果是否有意义。但是这种方法会使函数接口变得隐晦，因为接口的使用者可能并不会检查函数返回值而直接使用结果。
3. 抛出异常。这样我们就必须引入try-catch代码块来处理这些异常，使得代码变得冗余，可读性变差。

C++17中的std::optional<T>为解决这类问题提供了简单的解决方案。optional<T>可以看作是T类型变脸与一个布尔值的打包（wrapper class）。 其中的布尔值用来表示T是否为“空”。

**optional可以显式转换为bool 也就是说if的括号里可以直接放optional 会转换为bool值**

```C++
std::optional<hit_payload> payload;
...;
if(payload){
    ...;
}
```

https://blog.csdn.net/hhdshg/article/details/103433781

https://www.jianshu.com/p/24b4361017f9

#### if和switch语句小括号中初始化变量

**init-statement in selection statements**

```C++
if (auto payload = trace(orig, dir, scene.get_objects()); payload){
    ...;
}
```

https://blog.csdn.net/janeqi1987/article/details/100062524

#### Reflection & Refraction

**reflect:**
$$
R = I-2(I\cdot N)N
$$


<img src="D:\GZM\study\ELSE\Graphics\GAMES 101\HW\Assignment5\Code\notes\reflection.png" alt="reflect" style="zoom:67%;" />

**refract:**

b here is I in the code

a here is N in the code

```C++
Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior)
{    
	float cosi = clamp(-1, 1, dotProduct(I, N));
    float etai = 1, etat = ior;
    Vector3f n = N;
    if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    // 0 is total reflection case
    return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
}
```



<img src="D:\GZM\study\ELSE\Graphics\GAMES 101\HW\Assignment5\Code\notes\refraction.png" alt="refraction" style="zoom:67%;" />

#### 菲涅尔反射 Fresnel Equation



https://en.wikipedia.org/wiki/Fresnel_equations

**Schlick's approximation**
$$
R(\theta) = R_0+(1-R_0)(1-cos\theta)^5
$$
where
$$
R_0 = \big(\frac{n_1-n_2}{n_1+n_2}\big)^2
$$
https://en.wikipedia.org/wiki/Schlick%27s_approximation