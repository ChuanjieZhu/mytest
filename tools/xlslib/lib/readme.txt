1. xlslib����
x86ƽ̨��
./configure 
make
armƽ̨
./configure --host=arm-linux
make


2. ������ɺ���srcĿ¼�µ�.libsĿ¼�а���Ŀ���.so�ļ�
��libxls.so.2.0.3�ļ�������ϵͳ/usr/libĿ¼
�½���������
ln -s libxls.so.2.0.3 libxls.so.2
ln -s libxls.so.2.0.3 libxls.so
