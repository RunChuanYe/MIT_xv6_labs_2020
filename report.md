#   实验内容

Xv6中的fork()系统调用将父进程的所有用户空间内存复制到子进程中。如果父进程虚拟内存很大，复制可能需要很长时间。更糟糕的是，这些工作通常被大量浪费; 例如，子进程中的fork()后跟exec()将导致子代丢弃复制的内存，此外也还有可能从未使用过大部分内存。但是，如果父子进程都使用一个页面，并且其中他们其中一个或者都写入该页面，则确实需要一个副本。
本次实验就是对fork进行修改，使得在真正需要copy的时候（写入该页面）才进行复制。

#   实验要点

1. 在fork中将父子进程的pte都标志为not writable
2. 当进程试图写入这些地址的时候，将会除法page fault
3. 在usertrap中通过调用r_scause()判断是否是13或者15判断是否是page fault
4. 通过调用r_stval()得到触发page fault的va
5. 申请新的pa，copy旧的内容，并将它和va建立映射，去除当前进程的not writable
6. 在释放物理内存时候，需要引入**引用计数数组**

#   实验方案
1. 修改uvmcopy()，只是在将父进程的物理页映射到子进程中，而不是alloc一个新的物理页，同时除去writable标识
2. 在usertrap中识别page fault，当一个page fault发生时，alloc一个新的物理页，并将old page拷贝到该物理页，同时在子进程中建立映射，设置writable标志
3. 建立一个引用计数的数组，kalloc时设置计数为1，fork时候增加计数，当在usertrap新申请内存时减少计数，当且仅当引用计数为零时，kfree将它释放
4. 数组的大小：从kinit可以知道，数组的索引直接使用物理地址/PGSIZE
5. copyout中也需要修改成和usertrap同样的方案
6. 可以使用pte中的保留部分，标志pte为cow mapping
7. 如果cow page fault出现，并且没有空闲的内存了，需要将当前的进程kill掉

#   实验测试
1. cowtest, usertests

#   实验结果


