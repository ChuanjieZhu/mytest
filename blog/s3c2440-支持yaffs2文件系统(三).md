title: s3c2440-支持yaffs2文件系统(三)
date: 2015-12-10 15:22:10
category: linux驱动
---
内核版本: linux-2.6.30.4
busybox版本： busybox-1.23.2
目标：内核支持yaffs2文件系统，使用busybox制作最小文件系统

***

首先让内核支持yaffs2文件系统，然后使用busybox制作最小文件系统.

### 让内核支持yaffs2文件系统

* 下载yaffs2源码并解压，进入源码目录：

	    tar zxvf yaffs2.tar.gz
    	cd yaffs2

* 给内核打上yaffs2文件系统补丁，执行：

		./patch-ker.sh c /../../linux-2.6.30.4 <-------------内核解压后的源码目录

  打完补丁后，内核源码fs目录下会生产一个yaffs2目录，同时Makefile文件和Kconfig文件增加了yaffs2的配置和编译条件。

* 配置内核支持yaffs2
  这里配置选项较多，可根据自己的需要进行配置，把不需要的文件系统去掉，如下为几个主要的配置：

	    File systems --->
    	DOS/FAT/NT Filesystems --->
    		<*> MSDOS fs support
    		<*> VFAT (Windows95) fs support
    	Miscellaneous filesystems --->
    		<*> YAFFS2 file system support
    		[*] Autoselect yaffs2 format

  配置语言选项：

		Native Language support --->
		(iso8859-1) Default NLS Option
			<*> Codepage 437(United States, Canada)
			<*> Simplified Chinese charset(CP936, GB2312)
			<*> NLS ISO8859-1 (Latin 1; Western European Language)
			<*> NLS UTF-8

  经过上面的配置后，使用make zImage重新编译内核，编译出的内核就已经支持yaffs2文件系统了.

### 制作最小文件系统

移植内核，制作文件系统，我们的最终目的是在对应硬件平台上运行我们的应用程序，下面我们制作一个满足我们要求的最小基本文件系统.

* 解压busybox源码，进入源码目录，修改Makefile中CROSS_COMPILE添加交叉编译工具：
 
		CROSS\_COMPILE = arm-linux-

* 配置编译选项，运行make menuconfig对busybox进行配置，可以直接只用默认配置，这里我使用默认配置。
		
		make menuconfig

* 编译，运行make命令：

		make

* 运行make install，在busybox根目录下生产_install目录  

		make install
  
busybox配置编译工作完成，下面制作最小基础文件系统；

* 创建console和null设备文件，在_install目录下新建dev目录，再执行：  

	    mkdir dev
    	sudo mknod console c 5 1  
    	sudo mknod null c 1 3

* 创建/etc/inittab文件，在_install目录下新建etc目录，在etc目录下新建inittab文件，在inittab文件中添加如下内容：  

		mkdir etc
		cd etc
		touch inittab  
		console::askfirst:-/bin/sh  

* 添加C动态库文件，busybox使用动态库方式编译后，需要添加库文件，如果busybox采用静态库方式编译，则不需要添加库文件：  
		
		mkdir lib
		cp *.so xxx/_install/lib/ -d  

* 制作yaffs2文件系统：

		mkyaffs2image _install yaffs2.bin

经过上述步骤后，将生产的yaffs2.bin文件系统烧入硬件即可。

### 碰到问题及解决方法

* 将制作好的文件系统烧写到开发板后，启动报如下错误：

		NET: Registered protocol family 17
		RPC: Registered udp transport module.
		RPC: Registered tcp transport module.
		drivers/rtc/hctosys.c: unable to open rtc device (rtc0)
		end_request: I/O error, dev mtdblock2, sector 0
		FAT: unable to read boot sector
		VFS: Cannot open root device "mtdblock2" or unknown-block(31,2)
		Please append a correct "root=" boot option; here are the available partitions:
		1f00          261120 mtdblock0 (driver?)
		1f01            4096 mtdblock1 (driver?)
		1f02          256896 mtdblock2 (driver?)
		Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(31,2)
		[<c002c700>] (unwind_backtrace+0x0/0xdc) from [<c027f3f4>] (panic+0x40/0x110)
		[<c027f3f4>] (panic+0x40/0x110) from [<c0008fcc>] (mount_block_root+0x1d0/0x210)
		[<c0008fcc>] (mount_block_root+0x1d0/0x210) from [<c0009264>] (prepare_namespace+0x164/0x1bc) 
		[<c0009264>] (prepare_namespace+0x164/0x1bc) from [<c0008598>] (kernel_init+0xb4/0xe0)
		[<c0008598>] (kernel_init+0xb4/0xe0) from [<c004814c>] (do_exit+0x0/0x578)
		[<c004814c>] (do_exit+0x0/0x578) from [<00000001>] (0x1)

	解决方法：

		使用错误信息“end_request: I/O error, dev mtdblock2, sector 0”搜索，解决方法如下：
		Kernel command line出错，进入uboot命令行，用pri命令查看启动参数配置如下：
		bootargs=noinitrd root=/dev/mtdblock2 init=/linuxrc console=ttySAC0
		启动参数缺少rootfstype=yaffs2
		在uboot命令行下输入下面的命令：
		setenv bootargs noinitrd root=/dev/mtdblock2 rootfstype=yaffs2 init=/linuxrc console=ttySAC0
		saveenv
		重启开发板，启动成功

* 使用动态库选项编译busybox后烧写到开发板，启动报错：

		Failed to execute /linuxrc.  Attempting defaults...
		Kernel panic - not syncing: No init found.  Try passing init= option to kernel.
		[<c002c700>] (unwind_backtrace+0x0/0xdc) from [<c027f2e8>] (panic+0x40/0x110)
		[<c027f2e8>] (panic+0x40/0x110) from [<00264c0>] (init_post+0xcc/0xf4)
		[<c00264c0>] (init_post+0xcc/0xf4) from [<c000859c>] (kernel_init+0xb8/0xe0)
		[<c000859c>] (kernel_init+0xb8/0xe0) from [<c004814c>] (do_exit+0x0/0x578)
		[<c004814c>] (do_exit+0x0/0x578) from [<00000001>] (0x1)
	
	解决方法：

		lib目录下库文件拷贝不对：
		编译器库文件位置：arm-none-linux-gnueabi/libc/armv4t/lib/
		将该目录下*.so.*全部拷贝到制作的文件系统_install/lib目录下即可
		cp ~/work/tool/opt/EmbedSky/4.3.3/arm-none-linux-gnueabi/libc/armv4t/lib/*.so* . -d
		应用程序需要使用c++的还需要添加c++库：
		cp ~/work/tool/opt/EmbedSky/4.3.3/arm-none-linux-gnueabi/libc/thumb2/usr/lib/libstdc++.so* . -d

至此，最小基本文件系统制作完成.

### 最小文件系统改进

*	创建proc文件系统，在_install目录下新建proc目录即可:

		mkdir _install/proc

*	创建rcS脚本，在_install/etc/下新建init.d目录，在init.d目录中新建rcS文件，

    	mkdir -p _install/etc/init.d
    	touch _install/etc/init.d/rcS

*	修改rcS文件，在该文件中新增如下内容：

		mount -a

*	修改inittab文件，增加如下内容：

		::sysinit:/etc/init.d/rcS

*	创建/etc/fstab文件，并增加如下内容：

		proc            /proc           proc    defaults        0       0

*	给/etc/init.d/rcS文件添加可执行权限

		chmod +x /etc/init.d/rcS

*	udev自动创建/dev目录下设备节点

		mkdir /sys
		mkdir /tmp

*	修改/etc/fstab增加如下内容：

		tmpfs           /tmp            tmpfs   defaults        0       0
		sysfs           /sys            sysfs   defaults        0       0
		tmpfs           /dev            tmpfs   defaults        0       0

*   修改/etc/init.d/rcS文件，新增如下内容：

		mkdir /dev/pts
		mount -t devpts devpts /dev/pts
		echo /sbin/mdev > /proc/sys/kernel/hotplug
		mdev -s

### 最终相关配置文件内容如下

*	/etc/inittab

		console::askfirst:-/bin/sh
		::sysinit:/etc/init.d/rcS

*	/etc/fstab

		proc            /proc           proc    defaults        0       0
		tmpfs           /tmp            tmpfs   defaults        0       0
		sysfs           /sys            sysfs   defaults        0       0
		tmpfs           /dev            tmpfs   defaults        0       0

*	/etc/init.d/rcS
	
		mount -a
		mkdir /dev/pts
		mount -t devpts devpts /dev/pts
		echo /sbin/mdev > /proc/sys/kernel/hotplug
		mdev -s

### 最后制作新的文件系统

* 制作yaffs2文件系统：
	
		mkyaffs2image _install yaffs2.bin

***

通过上面步骤，一个最小的基础文件便完成后，后续随着驱动和应用功能的增加，还需要再做进一步的修改，在整个制作过程中只是给出了操作的命令，并没有说明为什么要这样做，制作最小文件系统是跟踪内核启动的过程，分析内核启动过程代码一步一步得出需要做的操作，所以深入的学习内核代码，弄懂内核启动的过程非常有必要。