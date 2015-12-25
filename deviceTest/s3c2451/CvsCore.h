#ifndef __CVS_CORE_H
#define __CVS_CORE_H

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif 


/////////////////////////////////////////////////////////////////////
//
//  传入/返回参数: ftpserver --- 返回FTP服务器IP地址
//                 ftpport   --- 返回FTP服务器端口号
//                 ftpusr    --- 返回FTP服务器用户名
//                 ftppass   --- 返回FTP服务器密码
//
//                 hostId    --- 返回主机ID
//                 schoolId  --- 返回学校ID
//                 schoolName --- 返回学校名字
//                 resetHour, resetMin --- 返回系统重启时间(小时和分钟)
//                 allowMoreCardsPerSec  --- 返回是否允许1秒钟连续多次刷同一张卡
//                                             1为允许，0为不允许
//                
// 说明：这个接口首先需要调用，而且仅能调用一次，在程序启动初始化时
//       调用 
/////////////////////////////////////////////////////////////////////
void CvsInit(char* ftpserver, int* ftpport, char* ftpusr, char* ftppass \
           , int* hostId, int* schoolId,  char* schoolName, int* resetHour, int* resetMin, int * allowMoreCardsPerSec);

		   
		   
/////////////////////////////////////////////////////////////////////
//
//  传入参数: func --- 是void 返回类型，参数类型为void类型
//            func用于设置回调函数，当网络有错误（断开时候），核心库会调用
//            func函数来通知应用程序；这时候应用程序应该尝试重新拨号3G网络
//            (假设应用程序启动时候，使用的是3G网络！！！否则，应该忽略这个
//            操作)     
//                
// 返回参数：无
//
// 说明：这个接口第二个需要调用，而且仅能调用一次，在调用CvsInit之后调用
//
/////////////////////////////////////////////////////////////////////
void CvsSetNetErrorFunc( void (*func)(void) );


/////////////////////////////////////////////////////////////////////
//
// 传入参数: studentId --- 学生ID
//           cardId    --- 学生卡ID
//           picname   --- 给学生拍照的图片文件名
//
// 返回参数：无
//
// 说明：这个接口在每次扫描学生卡和拍照后，调用这个函数
//
/////////////////////////////////////////////////////////////////////
void CvsSendData(const char* studentId, const char* cardId, const char* picname);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 

#endif
