### std::transform()

```C++
template <class InputIterator, class OutputIterator, class UnaryOperation>  OutputIterator transform (InputIterator first1, InputIterator last1,                            OutputIterator result, UnaryOperation op);
```

对于一元操作，将op应用于[first1, last1)范围内的每个元素，并将每个操作返回的值存储在以result开头的范围内。给定的op将被连续调用last1-first1次。op可以是函数指针或函数对象或lambda表达式。

如op的一个实现 即将[first1, last1)范围内的每个元素加5，然后依次存储到result中。

```C++
int op_increase(int i) {return (i + 5)};
```

https://blog.csdn.net/fengbingchun/article/details/63252470



WHY inverse transpose???

### 法向量变换矩阵

因为法向量和点通过矩阵拉伸以后不再垂直 所以法向量不能单纯用原有的矩阵变换

<img src="D:\GZM\study\ELSE\Graphics\GAMES 101\HW\Assignment3\Code\notes\1.png" alt="1" style="zoom: 67%;" />

<img src="D:\GZM\study\ELSE\Graphics\GAMES 101\HW\Assignment3\Code\notes\2.png" alt="2" style="zoom:67%;" />

<img src="D:\GZM\study\ELSE\Graphics\GAMES 101\HW\Assignment3\Code\notes\3.png" alt="3" style="zoom:67%;" />

首先假设该法向量变换矩阵为$B$， 原变换矩阵为$A$，原法向量为$n$，原向量为$v$

可知
$$
(Bn)^T(Av) = 0
$$

$$
n^TB^TAv = n^T(B^TA)v = 0
$$

又因为$n^Tv = 0$，所以$B^TA = I$

所以法向量变换矩阵$B = (A^{-1})^T$



### TNB系的推导

![TNB](D:\GZM\study\ELSE\Graphics\GAMES 101\HW\Assignment3\Code\notes\TNB.png)

问题：TBN系的朝向不同 渲染结果好像也有所不同 这里有没有一个固定的朝向？

### 深度矫正插值

https://zhuanlan.zhihu.com/p/144331875

