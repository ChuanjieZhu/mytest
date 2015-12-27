title: s3c2440-NANDFLASH支持（二）
date: 2015-12-10 12:08:10
category: linux驱动
---
内核版本: linux-2.6.30.4
目标：配置内核支持NANDFLASH

***

在前面熟悉了内核编译的基本方法后，现在来让内核支持NANDFLASH

### 修改smdk\_default\_nand\_part变量

这里我们要使NandFlash驱动同时支持64M，256M或更高容量。
修改arch/arm/plat-s3c24xx/common-smdk.c文件，找到smdk\_default\_nand\_part变量，该变量定义了系统的NAND flash分区信息，修改为如下值:

    static struct mtd_partition smdk_default_nand_part[] = {  
    	#if defined(CONFIG_64M_NAND)  
    	[0] = {  
    		.name = "boot",  
    		.offset = 0,  
    		.size = SZ_1M,
    	},  
    	[1] = {  
    		.name = "kernel",  
    		.offset = SZ_1M + SZ_128K,  
    		.size = SZ_4M,  
    	},  
    	[2] = {  
    		.name = "yaffs2",  
    		.offset = SZ_1M + SZ_128K + SZ_4M,  
    		.size = SZ_64M - SZ_4M - SZ_1M - SZ_128K,  
    	}  
    	#elif defined(CONFIG_256M_NAND)   
    	[0] = {  
    		.name	= "boot",  
    		.size	= 0,  
    		.offset	= SZ_1M,  
    	},  
    	[1] = {  
    		.name	= "kernel",  
    		.offset = SZ_1M + SZ_128K,  
    		.size	= SZ_4M,  
    	},  
    	[2] = {  
    		.name	= "yaffs2",  
    		.offset = SZ_1M + SZ_128K + SZ_4M,  
    		.size	= SZ_256M - SZ_4M - SZ_1M - SZ_128K,  
    	}  
    	#endif  
    	}; 
	
分区的名字可以随便取。

### 修改Nand读写匹配时间

修改Nand读写匹配时间，这个改不改应该问题都不大，与Nand的读写特性相关的，根据查芯片资料得到的值，每种Nand的值都不一样；修改arch/arm/plat-s3c24xx/common-smdk.c文件，找到smdk\_nand\_info变量，修改为如下：

	static struct s3c2410_platform_nand smdk_nand_info = {  
		.tacls = 10,  
		.twrph0 = 25,  
		.twrph1 = 10,  
		.nr_sets = ARRAY_SIZE(smdk_nand_sets),  
		.sets = smdk_nand_sets,  
	};
	
###  修改Kconfig文件，增加menuconfig配置选项  

修改 Kconfig 文件，在配置时选择NAND类型，修改driver/mtd/nand/Kconfig在适当位置添加如下内如：	

	Choice
	    prompt "Nand Flash Capacity Select"
		depends on MTD
	config 64M_NAND
		boolean "64M NAND For S3C2440"
		depends on MTD
	config 256M_NAND
		boolean "256M NAND For S3C2440"
		depends on MTD
	endchoice

### 配置内核，支持NANDFLASH

运行make menuconfig，找到Device Drivers配置项；根据自己硬件的时间情况在“Nand Flash Capacity Select”配置项中选择NANDFLASH大小；

	Device Drivers --->
	<*> Memory Technology Device (MTD) support --->
		[*] MTD partitioning support
	<*> NAND Device Support --->
		<*> NAND Flash support for S3C2410/S3C2440 SoC
		[*] S3C2410 NAND Hardware ECC <—----------- 这个一定要选上
		Nand Flash Capacity Select(256M Nand For S3C2440)--->

###  编译内核

	make zImage

将编译的内核烧写到硬件，观察内核启动信息中是否有NANDFLASH相关硬件信息。


	