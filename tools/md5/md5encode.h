/**********************************************************************************
函数名称：GetFileMd5Code
函数功能: 获取指定文件的md5值
入口参数：	pSrcFile: 需要计算md5值文件路径
			pMd5File: md5结果存储文件路径,如果不需要将md5值存入文件，该参数设置为NULL
			iCodeSize: md5值返回缓存大小，不能小于32字节
出口参数:			
			pMd5Code: md5值返回缓存
返回值:	0 - 成功，其它: 失败
***********************************************************************************/
int GetFileMd5Code(const char *pSrcFile, const char *pMd5File, char *pMd5Code, int iCodeSize);
/**********************************************************************************
函数名称：GetDataMd5Code
函数功能: 获取指定数据的md5值
入口参数：	pSrcData: 需要计算md5值的数据缓存buffer
			iDataSize: 需计算md5值数据大小
			iCodeSize: md5值返回缓存大小，不能小于32字节
出口参数:			
			pMd5Code: md5值返回缓存
返回值:	0 - 成功，其它: 失败
***********************************************************************************/
int GetDataMd5Code(const char *pSrcData, int iDataSize, char *pMd5code, int iCodeSize);