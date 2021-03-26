#### emplace_back()

emplace_back能够根据输入的参数就地构造一个对象实例插入到容器末尾

- 当调用push_back或insert成员函数时，是把元素类型的对象传递给它们，这些对象被拷贝到容器中。而当我们调用一个emplace系列函数时，则是将相应参数传递给元素类型的构造函数。

- 这样emplace_back能就地通过参数构造对象，不需要拷贝操作，相比push_back能更好的避免内存的拷贝和移动，提升容器插入元素的性能。

- emplace函数需要对应的参数对象有对应的构造函数，不然编译报错
- emplace函数在容器中直接构造元素。传递给emplace函数的参数必须与元素类型的构造函数相匹配

```C++
class Triangle : public Object
{
public:
    Vector3f v0, v1, v2;
    Material* m;

    Triangle(Vector3f _v0, Vector3f _v1, Vector3f _v2, Material* _m = nullptr)
        : v0(_v0), v1(_v1), v2(_v2), m(_m)
    {...;}
	...;
}

triangles.emplace_back(face_vertices[0], face_vertices[1],
                       face_vertices[2], new_mat);
triangles.push_back(Triangle(face_vertices[0], face_vertices[1],
                             face_vertices[2], new_mat));
```

https://www.jianshu.com/p/1fb2daf66582

#### inline 变量

https://blog.csdn.net/janeqi1987/article/details/100108589?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-2.channel_param&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-2.channel_param

#### 右值引用 & std::move

https://www.jianshu.com/p/d19fc8447eaa

#### time_t

C++用来存时间的函数

#### maxExtent

判断bounding box 最长的方向 以便较为平均的分割