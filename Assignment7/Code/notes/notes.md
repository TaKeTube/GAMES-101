#### Sample

Alias method

https://blog.csdn.net/haolexiao/article/details/65157026 

均匀采样

https://blog.csdn.net/u010281174/article/details/109123974 

https://zhuanlan.zhihu.com/p/26052376

在流形上的均匀采样

首先要根据流形上的均匀分布找到参数化空间里的分布 再根据逆采样变换得到和$[0,1]^n$上均匀分布的随机变量的变换函数

BVH line 138 为什么我们要在这里对随机数开平方？

#### 性能

用了github开源的ThreadPool https://github.com/progschj/ThreadPool

开了8线程池 对rayCast多线程运算 性能大幅提升

用了-O3优化编译 性能大幅提升 快了1倍至少

随机数生成器加了local_thread 使得随机数生成器对于单个线程来说是静态变量 避免重复创建随机数生成器 同时保证了线程安全 快了1倍 

此时100*100 8192spp 216s就能渲染完毕

把BVH树里的fmax fmin改写成了三元运算符分支 100*100 8192spp 快了8s左右