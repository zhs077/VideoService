#include "C7Platform.h"
#include "configlib.h"


C7_HSESSION C7Platform::session = NULL;
ResourceMap C7Platform::resourceMap;
C7Platform::C7Platform(void)
{
}


C7Platform::~C7Platform(void)
{
	UnInit();
}
void C7Platform::UnInit()
{
	VADR_GlobalClose();
	C7_Terminate();
}
bool C7Platform::Init(const string& current_path)
{
	string 	iniPath = current_path + RUNTIME_CONFIG_FILE;
	string oemPath = current_path +"VS_OEMPlugin\\";

	char remote_address[100]={0};
	int port=0;
	char user_name[100]={0};
	char password[100]={0};
	char edpit[100]={0};
	char id[100]={0};
	//读取配置文件
	config_read_string(iniPath.c_str(),"Nrcapc","RemoteAddress",remote_address,"");
	config_read_string(iniPath.c_str(),"Nrcapc","UserName",user_name,"");
	config_read_string(iniPath.c_str(),"Nrcapc","PassWord",password,"");
	config_read_string(iniPath.c_str(),"Nrcapc","EPID",edpit,"");
	//config_read_string(iniPath.c_str(),"Nrcapc","ID",id,"151038400990972028");
	int rv;
	rv = C7_Initialize();
	if (rv != NRCAP_SUCCESS)
	{
		//LOG4CPLUS_ERROR(logger," Error: C7_Initialize ERROR_CODE:"<<rv);
		fprintf(stdout,"Error: C7_Initialize ERROR_CODE:%d\n",rv);
		exit(-1);
	}
	VADR_GlobalInitEx((char*)oemPath.c_str());
	rv = C7_Open(remote_address,user_name,password,edpit,&session);
	if (rv != NRCAP_SUCCESS)
	{
		fprintf(stdout,"Error: C7_Open ERROR_CODE:%d\n",rv);
		//LOG4CPLUS_ERROR(logger," Error: C7_Open ERROR_CODE:"<<rv);
		exit(-1);
	}
	int nPlatformType;
	C7_STR szPUID;
	C7_STR szName;
	C7_STR szType;
	int nUsable = 0;
	rv = C7_GetPlatformType(session, &nPlatformType);
	if (nPlatformType == 0)
	{
		//连接平台
		int count = GET_CNT;
		C7_HRESOURCE PUArr[GET_CNT] = { NULL };
		int nOffset=0;
		rv = C7_ForkPUList(session,PUArr,&count,nOffset);
		if (rv != NRCAP_SUCCESS)
		{
			fprintf(stdout,"Error: C7_ForkPUList ERROR_CODE:%d\n",rv);
			//LOG4CPLUS_ERROR(logger," Error: C7_ForkPUList ERROR_CODE:"<<rv);
			exit(-1);

		}

		for (int i=0 ; i!= count;i++)
		{
			C7_HRESOURCE hPU = PUArr[i];
			//C7_STR szPUID;
			int nOnline = 0;
			C7_GetResourcePUID(hPU, szPUID);
			C7_GetResourceUsable(hPU, &nUsable);
			C7_GetResourceType(hPU, szType);
			if (nUsable == 0 && strcmp(szType, RT_SELF) == 0)
			{			
				//不在线设备
				continue;
			}
			C7_HRESOURCE ResArr[100];
			int nResCnt = sizeof(ResArr) / sizeof(C7_HRESOURCE);
			rv = C7_ForkPUResource(PUArr[i],ResArr,&nResCnt);



			if (rv != NRCAP_SUCCESS)
			{
				//LOG4CPLUS_ERROR(logger," Error: C7_ForkPUList ERROR_CODE:"<<rv);
				exit(-1);

			}
			vector<ResourceInfo> vtInfo;
			for (int j=0;j < nResCnt;j++)
			{
				C7_HRESOURCE hRes = ResArr[j];
				C7_STR szType;
				int nResIdx = 0;
				C7_GetResourceName(hRes, szName);
				C7_GetResourceType(hRes, szType);
				C7_GetResourceIndex(hRes, &nResIdx);
				C7_GetResourceUsable(hRes,&nUsable);
				if (nUsable == 0 && strcmp(szType, RT_SELF) == 0)
				{			
					//不在线设备
					continue;
				}
				C7_GetResourcePUID(hPU, szPUID);
				ResourceInfo info;
				info.hresource = hRes;
				info.szPUID = szPUID;
				info.szType = szType;
				info.szName = szName;
				info.usable = nUsable;
				info.index = nResIdx;
				vtInfo.push_back(info);
			}

			resourceMap.insert(make_pair(szPUID,vtInfo));
		}

	}
	else //连接设备
	{
				vector<ResourceInfo> vtInfo;
		 		C7_HRESOURCE hPU;
		 		rv = C7_ForkOnePU(session, "", &hPU);
		 		if (rv == NRCAP_SUCCESS)
		 		{
		 			CreateResource(&hPU, 1);

				}
	}

	return true;
}


void C7Platform::CreateResource(void* *pRsc, int nCnt)
{
		for (int i = 0; i != nCnt; i++)
		{
			C7_HRESOURCE hPU = pRsc[i];
			C7_STR szPUID;
			C7_GetResourcePUID(pRsc[i], szPUID);

			//获取资源类型
			C7_STR szType = { NULL };
			int rv = C7_GetResourceType(pRsc[i], szType);
			if (rv != NRCAP_SUCCESS)
			{
				return;

			}
			int nUsable = 0;
			rv = C7_GetResourceUsable(pRsc[i], &nUsable);
			if (nUsable == 0 && strcmp(szType, RT_SELF) == 0)
			{
				//不在线设备
				continue;
			}

			C7_STR szName = { NULL };
			rv = C7_GetResourceName(pRsc[i], szName);
			//如果资源类型是站点本身则进行递归
			if (strcmp(szType, RT_SELF) == 0)
			{
				
				C7_HRESOURCE ResourcesArr[GET_CNT] = { NULL };
				int nResCnt = GET_CNT;
				rv = C7_ForkPUResource(pRsc[i], ResourcesArr, &nResCnt);
				if (rv !=NRCAP_SUCCESS)
				{
					return;
				}
				vector<ResourceInfo> vtInfo;
				for (int j =0; j < nResCnt; j++)
				{
					C7_HRESOURCE hRes = ResourcesArr[j];
					C7_STR szType;
					int nResIdx = 0;
					C7_GetResourceName(hRes, szName);
					C7_GetResourceType(hRes, szType);
					C7_GetResourceIndex(hRes, &nResIdx);
					C7_GetResourceUsable(hRes,&nUsable);
					if (nUsable == 0 && strcmp(szType, RT_SELF) == 0)
					{			
						//不在线设备
						continue;
					}
					C7_GetResourcePUID(hPU, szPUID);
					ResourceInfo info;
					info.hresource = hRes;
					info.szPUID = szPUID;
					info.szType = szType;
					info.szName = szName;
					info.usable = nUsable;
					info.index = nResIdx;
					vtInfo.push_back(info);
				}
				resourceMap.insert(make_pair("151038402365732956",vtInfo));
			

			}
			else
			{
				
			}
		}
		
}