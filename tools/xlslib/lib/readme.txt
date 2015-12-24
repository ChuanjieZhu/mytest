1. xlslib编译
x86平台：
./configure 
make
arm平台
./configure --host=arm-linux
make


2. 编译完成后在src目录下的.libs目录中包含目标库.so文件
将libxls.so.2.0.3文件拷贝到系统/usr/lib目录
新建两个链接
ln -s libxls.so.2.0.3 libxls.so.2
ln -s libxls.so.2.0.3 libxls.so
