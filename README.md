![1669690523979](image/README/1669690523979.png)

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
        <a href="#about-the-project">About The Project</a>
    </li>
    <li>
        <a href="#how-to-get-started">How to Get Started</a>
    </li>
    <li><a href="#done-and-todo">Done and TODO</a>
    <li><a href="#gains">Gains</a>
    </li>
    <li><a href="#recommendation">Recommendation</a></li>
  </ol>
</details>

# About the Project

此项目为MIT的[6.S081](https://pdos.csail.mit.edu/6.S081/2020/)课程实验。实验中囊括了操作系统的大部分知识，这十个实验都十分值得一做，笔者花费大约一个月，从lab1到lab10都独立思考完成，收获不小。正是应了那句话，**纸上得来终觉浅，绝知此事要躬行。**

推荐一本好书——[《Operating Systems: Three Easy Pieces》](https://pages.cs.wisc.edu/~remzi/OSTEP/)。如果你和我一样对OS一无所知，那么**强烈建议**你边看这本书边写实验。这本书对初学者**十分友好**，而且**不会很枯燥**，可以很好地帮助你理解OS的不少细节，可以让你更快看懂`xv6`的代码。总之，这本书值得一读。

# How to Get Started

相关资源，参见[xv6-tools](https://pdos.csail.mit.edu/6.S081/2020/tools.html)，[Boot-xv6](https://pdos.csail.mit.edu/6.S081/2020/labs/util.html)

# Done and TODO

- [x] Utilities
- [x] System calls
- [x] Page tables
- [x] Traps
- [x] Lazy allocatioin
- [x] Copy on-write
- [x] Multitheading
- [x] Lock
- [x] File system
- [x] mmap
- [ ] network driver

# Gains

1. 什么是系统调用，系统调用的过程，如何编写系统调用
2. 什么是用户态，内核态，如何在两者之间进行切换
3. 为什么使用`pgtbl`，如何管理内存
4. 什么是`trap`，为什么设置`trap`，如何让`trap`正确返回
5. 为什么要`lazy allocation`，有什么好处
6. 为什么要`copy on write`，有什么好处
7. `xv6`内核并没有多线程机制，如何编写一个用户态多线程，线程如何切换，线程栈是什么
8. 为什么要使用锁，如何在保证正确性的前提下提高**并发性**，提高**性能**
9. 什么是文件系统，什么是**直接索引**，**间接索引**，**多级索引**，文件系统布局是什么样的
10. `mmap`是什么，实现`mmap`要注意什么

# Recommendation

相信大家在只做xv6中的文件系统实验后，对于文件系统的细节实际上还是不够了解。这里推荐一个实验[HITSZ-fs-lab](http://hitsz-cslab.gitee.io/os-labs/lab5/part1/)，这个lab是让大家从文件系统的**内存布局**，**磁盘布局**开始设计**自己的**`fs`，并且上限可以很高。你可以仅仅做基本的文件系统命令`mount`，`unmount`，`ls`，`touch`，`mkdir`，也可以做**加强版**，如加入**更多`fs`命令**，**磁盘缓冲**，**日志系统**等。
