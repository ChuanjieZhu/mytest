#include <iostream>
#include "tinyxml.h"
#include "myXml.h"

/* base64加密 */
int base64_enc(char *dest, const char *src, int count)
{ 
	const unsigned char Base64_EnCoding[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int len = 0; 
	unsigned char *pt = (unsigned char *)src;

	if( count < 0 )
	{
		while( *pt++ )
		{ 
			count++; 
		}
		
		pt = (unsigned char *)src;
	}
	
	if( !count )
	{
		return 0;
	}
	
	while( count > 0 )
	{
		*dest++ = Base64_EnCoding[ ( pt[0] >> 2 ) & 0x3f];
		if( count > 2 )
		{
			*dest++ = Base64_EnCoding[((pt[0] & 3) << 4) | (pt[1] >> 4)];
			*dest++ = Base64_EnCoding[((pt[1] & 0xF) << 2) | (pt[2] >> 6)];
			*dest++ = Base64_EnCoding[ pt[2] & 0x3F];
		}
		else
		{
			switch( count )
			{
				case 1:
				{
					*dest++ = Base64_EnCoding[(pt[0] & 3) << 4 ];
					*dest++ = '=';
					*dest++ = '=';
					
					break;
				}
				case 2: 
				{
					*dest++ = Base64_EnCoding[((pt[0] & 3) << 4) | (pt[1] >> 4)]; 
					*dest++ = Base64_EnCoding[((pt[1] & 0x0F) << 2) | (pt[2] >> 6)]; 
					*dest++ = '='; 

					break;
				}
			} 
		} 
		
		pt += 3; 
		count -= 3; 
		len += 4; 
	} 
	
	*dest = 0; 

	return len; 
} 

/* base64解密 */
int base64_dec(char* dest,const char* src, int count) 
{
	unsigned char ucs[4];
	unsigned char *pt = (unsigned char *)src;
	int len = 0 , nfag = 0 , i = 0;

	if( count < 0 )
	{
		while( *pt++ )
		{ 
			count++; 
		}
		
		pt = (unsigned char *)src;
	}
	
	if( !count )
	{
		return 0;
	}
	
	while( count > 0 )
	{
		nfag = 0;
		
		for( i=0 ; i<4 ; i++ )
		{
			if (*pt >= 'A' && *pt <= 'Z') 
			{
				ucs[i] = *pt - 'A'; 
			}
			else if (*pt >= 'a' && *pt <= 'z') 
			{
				ucs[i] = *pt - 'a' + 26; 
			}
			else if (*pt >= '0' && *pt <= '9') 
			{
				ucs[i] = *pt - '0' + 52; 
			}
			else
			{
				switch (*pt)
				{ 
					case '+': 
					{
						ucs[i] = 62;
						
						break;
					}
					case '/': 
					{
						ucs[i] = 63;
						
						break;
					}
					case '=': /* base64 padding */ 
					{
						ucs[i] = 0; 
						
						break;
					}
					case '\t':
					case '\r':
					case '\n':
					case ' ':
					{
						nfag++;
						i--;

						break;
					}
					case '\0':
					{
						*dest = 0;
						
						return len;

						break;
					}
					default:
					{
						*dest = 0;
						
						return -1;
					}
				}
			}
			
			pt++;
		}
		
		*dest++ = (ucs[0] << 2) | (ucs[1] >> 4);
		*dest++ = (ucs[1] << 4) | (ucs[2] >> 2);
		*dest++ = (ucs[2] << 6) | (ucs[3]);
		count -= nfag + 4;
		len += 3;
	}
	
	*dest = 0;

	return len;
}

/* 加载Car.xml */
int loadCarXml(char *pPath, car_config_t *pCarConfig)
{
	using namespace std;
	const char * xmlFile = pPath;
	TiXmlDocument doc;                              

	if (doc.LoadFile(xmlFile))
	{
		//		doc.Print();
	}
	else
	{
		cout << "can not parse xml conf/school.xml" << endl;
		
		return -1;
	}

	TiXmlElement* rootElement = doc.RootElement();
	TiXmlElement* SystemInfoDataElement = rootElement->FirstChildElement();

	for(; SystemInfoDataElement != NULL; SystemInfoDataElement = SystemInfoDataElement->NextSiblingElement())
	{
		TiXmlElement* contactElement = SystemInfoDataElement->FirstChildElement();
		
		for(; contactElement != NULL; contactElement = contactElement->NextSiblingElement())
		{
			const char * contactType = contactElement->Value();
			if(contactType != NULL)
			{
				const char * contactValue = contactElement->GetText();
				if(contactValue != NULL)
				{
					if(strncmp(contactType, "Format", strlen("Format")) == 0)
					{
						if(strncmp(contactValue, "no", strlen("no")) == 0)
						{
							pCarConfig->format = 1;
						}
					}
					else if(strncmp(contactType, "EmptyStudents", strlen("EmptyStudents")) == 0)
					{
						if(strncmp(contactValue, "no", strlen("no")) == 0)
						{
							pCarConfig->emptyStudents = 1;
						}
					}
					else if(strncmp(contactType, "EmptyDutyDatas", strlen("EmptyDutyDatas")) == 0)
					{
						if(strncmp(contactValue, "no", strlen("no")) == 0)
						{
							pCarConfig->emptyDutyDatas = 1;
						}
					}
					else if(strncmp(contactType, "SetVolume", strlen("SetVolume")) == 0)
					{
						pCarConfig->volume = atoi(contactValue);
					}
					else if(strncmp(contactType, "DrivingSchoolName", strlen("DrivingSchoolName")) == 0)
					{
						strncpy(pCarConfig->drivingSchoolName, contactValue, sizeof(pCarConfig->drivingSchoolName));
					}
					else if(strncmp(contactType, "SetLicensePlateNumber", strlen("SetLicensePlateNumber")) == 0)
					{
						strncpy(pCarConfig->licensePlateNumber, contactValue, sizeof(pCarConfig->licensePlateNumber));
					}
					else if(strncmp(contactType, "SetLicenseFrameNumber", strlen("SetLicenseFrameNumber")) == 0)
					{
						strncpy(pCarConfig->licenseFrameNumber, contactValue, sizeof(pCarConfig->licenseFrameNumber));
					}
					else if(strncmp(contactType, "TeacherName", strlen("TeacherName")) == 0)
					{
						strncpy(pCarConfig->teacherName, contactValue, sizeof(pCarConfig->teacherName));
					}
				}
			}
		}
	} 

	return 0;
}

/* 加载Student.xml */
int loadStudentXml(char *pPath, student_data_t *pStudentData, int studentNum)
{
	int i = 0;
	using namespace std;
	const char * xmlFile = pPath;
	TiXmlDocument doc;                              

	if (doc.LoadFile(xmlFile))
	{
		//		doc.Print();
	}
	else
	{
		cout << "can not parse xml conf/school.xml" << endl;
		
		return -1;
	}

	TiXmlElement* rootElement = doc.RootElement();
	TiXmlElement* studentDataElement = rootElement->FirstChildElement();
	TiXmlElement* studentElement = studentDataElement->FirstChildElement();

	for(; studentElement != NULL; studentElement = studentElement->NextSiblingElement())
	{
		if(i >= studentNum)
		{
			break;
		}

		TiXmlElement* contactElement = studentElement->FirstChildElement();

		for(; contactElement != NULL; contactElement = contactElement->NextSiblingElement())
		{
			const char * contactType = contactElement->Value();
			if(contactType != NULL)
			{
				const char * contactValue = contactElement->GetText();
				if(contactValue != NULL)
				{
					if(strncmp(contactType, "IdCard", strlen("IdCard")) == 0)
					{
						strncpy(pStudentData[i].idCard, contactValue, sizeof(pStudentData[i].idCard));
					}
					else if(strncmp(contactType, "SQCX", strlen("SQCX")) == 0)
					{
						strncpy(pStudentData[i].sqcx, contactValue, sizeof(pStudentData[i].sqcx));
					}
					else if(strncmp(contactType, "Name", strlen("Name")) == 0)
					{
						strncpy(pStudentData[i].name, contactValue, sizeof(pStudentData[i].name));
					}
					else if(strncmp(contactType, "SubjectName", strlen("SubjectName")) == 0)
					{
						strncpy(pStudentData[i].subjectName, contactValue, sizeof(pStudentData[i].subjectName));
					}
					else if(strncmp(contactType, "FaceBase64", strlen("FaceBase64")) == 0)
					{
						pStudentData[i].size = base64_dec(pStudentData[i].faceBuf, contactValue, strlen(contactValue));
					}
				}
			}
		}
		
		i++;
	} 

	return i;
}

/* 加载Ad.xml */
int loadAdXml(char *pPath, ad_config_t *pAdConfig, int adNum)
{
	int i = 0;
	using namespace std;
	const char * xmlFile = pPath;
	TiXmlDocument doc;                              

	if (doc.LoadFile(xmlFile))
	{
		//		doc.Print();
	}
	else
	{
		cout << "can not parse xml conf/school.xml" << endl;
		
		return -1;
	}

	TiXmlElement* rootElement = doc.RootElement();
	TiXmlElement* AdvertisementDataElement = rootElement->FirstChildElement();
	TiXmlElement* AdElement = AdvertisementDataElement->FirstChildElement();

	for(; AdElement != NULL; AdElement = AdElement->NextSiblingElement())
	{
		if(i >= adNum)
		{
			break;
		}

		TiXmlElement* contactElement = AdElement->FirstChildElement();
		
		for(; contactElement != NULL; contactElement = contactElement->NextSiblingElement())
		{
			const char * contactType = contactElement->Value();
			if(contactType != NULL)
			{
				const char * contactValue = contactElement->GetText();
				if(contactValue != NULL)
				{
					if(strncmp(contactType, "PicBase64", strlen("PicBase64")) == 0)
					{
						pAdConfig[i].size = base64_dec(pAdConfig[i].adPicBuf, contactValue, strlen(contactValue));
					}
					else if(strncmp(contactType, "PerTime", strlen("PerTime")) == 0)
					{
						pAdConfig[i].perTime = atoi(contactValue);
					}
				}
			}
		}
		
		i++;
	} 

	return i;
}

/* 加载DelTime.xml */
int loadDeleteTimeXml(char *pPath, delete_time_t *pDeleteTime, int deleteNum)
{
	int i = 0;
	using namespace std;
	const char * xmlFile = pPath;
	TiXmlDocument doc;                              

	if (doc.LoadFile(xmlFile))
	{
		//		doc.Print();
	}
	else
	{
		cout << "can not parse xml conf/school.xml" << endl;
		
		return -1;
	}

	TiXmlElement* rootElement = doc.RootElement();
	TiXmlElement* deleteRecordElement = rootElement->FirstChildElement();
	TiXmlElement* recordElement = deleteRecordElement->FirstChildElement();

	for(; recordElement != NULL; recordElement = recordElement->NextSiblingElement())
	{
		if(i >= deleteNum)
		{
			break;
		}

		TiXmlElement* contactElement = recordElement->FirstChildElement();
		
		for(; contactElement != NULL; contactElement = contactElement->NextSiblingElement())
		{
			const char * contactType = contactElement->Value();
			if(contactType != NULL)
			{
				const char * contactValue = contactElement->GetText();
				if(contactValue != NULL)
				{
					if(strncmp(contactType, "DeviceNum", strlen("DeviceNum")) == 0)
					{
						strncpy(pDeleteTime[i].deviceNum, contactValue, sizeof(pDeleteTime[i].deviceNum) - 1);
					}
					else if(strncmp(contactType, "SerialNum", strlen("SerialNum")) == 0)
					{
						pDeleteTime[i].serialNum = atoi(contactValue);
					}
				}
			}
		}
		
		i++;
	} 

	return i;
}

/* 创建Time.xml */
int createTimeXml(char *pPath, student_record_t * pStudentRecord, int recordNum)
{
	int i = 0;
	char picBase64Buf[20 * 1024] = {0};
	char tmpBuf[1024] = {0};

	TiXmlDocument *recordXml = new TiXmlDocument();

	TiXmlDeclaration Declaration( "1.0", "gb18030", "no" );
	recordXml->InsertEndChild( Declaration );

	TiXmlElement *rootElement = new TiXmlElement("HHProtocol");
	recordXml->LinkEndChild(rootElement);

	TiXmlElement *studentRecordElement = new TiXmlElement("StudentRecords");
	rootElement->LinkEndChild(studentRecordElement);

	for(i = 0; i < recordNum; i++)
	{
		TiXmlElement *recordElement = new TiXmlElement("Record");
		studentRecordElement->LinkEndChild(recordElement);

		/* 填写设备号 */
		TiXmlElement *deviceNumElement = new TiXmlElement("DeviceNum");
		recordElement->LinkEndChild(deviceNumElement);
		
		TiXmlText *deviceNumContent = new TiXmlText(pStudentRecord[i].deviceNum);
		deviceNumElement->LinkEndChild(deviceNumContent);

		/* 填写序号 */
		TiXmlElement *serialNumElement = new TiXmlElement("SerialNum");
		recordElement->LinkEndChild(serialNumElement);
		
		memset(tmpBuf, 0, sizeof(tmpBuf));
		sprintf(tmpBuf, "%d", pStudentRecord[i].serialNum);
		TiXmlText *serialNumContent = new TiXmlText(tmpBuf);
		serialNumElement->LinkEndChild(serialNumContent);

		/* 填写驾校名 */
		TiXmlElement *drivingSchoolNameElement = new TiXmlElement("DrivingSchoolName");
		recordElement->LinkEndChild(drivingSchoolNameElement);
		
		TiXmlText *drivingSchoolNameContent = new TiXmlText(pStudentRecord[i].drivingSchoolName);
		drivingSchoolNameElement->LinkEndChild(drivingSchoolNameContent);

		/* 填写申请车型 */
		TiXmlElement *sqcxElement = new TiXmlElement("SQCX");
		recordElement->LinkEndChild(sqcxElement);
		
		TiXmlText *sqcxContent = new TiXmlText(pStudentRecord[i].sqcx);
		sqcxElement->LinkEndChild(sqcxContent);

		/* 填写车牌号 */
		TiXmlElement *licensePlateNumElement = new TiXmlElement("LicensePlateNum");
		recordElement->LinkEndChild(licensePlateNumElement);
		
		TiXmlText *licensePlateNumContent = new TiXmlText(pStudentRecord[i].licensePlateNum);
		licensePlateNumElement->LinkEndChild(licensePlateNumContent);

		/* 填写车架号 */
		TiXmlElement *licenseFrameNumElement = new TiXmlElement("LicenseFrameNum");
		recordElement->LinkEndChild(licenseFrameNumElement);
		
		TiXmlText *licenseFrameNumContent = new TiXmlText(pStudentRecord[i].licenseFrameNum);
		licenseFrameNumElement->LinkEndChild(licenseFrameNumContent);

		/* 填写教练员姓名 */
		TiXmlElement *teacherNameElement = new TiXmlElement("TeacherName");
		recordElement->LinkEndChild(teacherNameElement);
		
		TiXmlText *teacherNameContent = new TiXmlText(pStudentRecord[i].teacherName);
		teacherNameElement->LinkEndChild(teacherNameContent);

		/* 填写学员姓名 */
		TiXmlElement *studentNameElement = new TiXmlElement("StudentName");
		recordElement->LinkEndChild(studentNameElement);
		
		TiXmlText *studentNameContent = new TiXmlText(pStudentRecord[i].studentName);
		studentNameElement->LinkEndChild(studentNameContent);

		/* 填写学员身份证号码 */
		TiXmlElement *idCardElement = new TiXmlElement("IdCard");
		recordElement->LinkEndChild(idCardElement);
		
		TiXmlText *idCardContent = new TiXmlText(pStudentRecord[i].idCard);
		idCardElement->LinkEndChild(idCardContent);

		/* 填写学员所学科目名 */
		TiXmlElement *subjectNameElement = new TiXmlElement("SubjectName");
		recordElement->LinkEndChild(subjectNameElement);
		
		TiXmlText *subjectNameContent = new TiXmlText(pStudentRecord[i].subjectName);
		subjectNameElement->LinkEndChild(subjectNameContent);

		/* 填写学员识别时的照片 */
		TiXmlElement *picBase64Element = new TiXmlElement("PicBase64");
		recordElement->LinkEndChild(picBase64Element);

		memset(picBase64Buf, 0, sizeof(picBase64Buf));

		/* 如果pStudentRecord[i].pPicBuf非空，则将pStudentRecord[i].pPicBuf进行base64加密 */
		if(pStudentRecord[i].pPicBuf != NULL)
		{
			base64_enc(picBase64Buf, pStudentRecord[i].pPicBuf, pStudentRecord[i].picBufSize);
		}
		
		TiXmlText *picBase64Content = new TiXmlText(picBase64Buf);
		picBase64Element->LinkEndChild(picBase64Content);

		/* 填写学员上机时刻 */
		TiXmlElement *startTimeElement = new TiXmlElement("StartTime");
		recordElement->LinkEndChild(startTimeElement);
		
		TiXmlText *startTimeContent = new TiXmlText(pStudentRecord[i].startTime);
		startTimeElement->LinkEndChild(startTimeContent);

		/* 填写学员下机时刻 */
		TiXmlElement *endTimeElement = new TiXmlElement("EndTime");
		recordElement->LinkEndChild(endTimeElement);
		
		TiXmlText *endTimeContent = new TiXmlText(pStudentRecord[i].endTime);
		endTimeElement->LinkEndChild(endTimeContent);
	}

	//recordXml->Print();

	recordXml->SaveFile(pPath);

	return 0;
}


