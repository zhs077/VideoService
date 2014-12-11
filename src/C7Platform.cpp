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
		exit(-1);
	}
	VADR_GlobalInitEx((char*)oemPath.c_str());
	rv = C7_Open(remote_address,user_name,password,edpit,&session);
	if (rv != NRCAP_SUCCESS)
	{
		//LOG4CPLUS_ERROR(logger," Error: C7_Open ERROR_CODE:"<<rv);
		exit(-1);
	}
	int nPlatformType;
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
			//LOG4CPLUS_ERROR(logger," Error: C7_ForkPUList ERROR_CODE:"<<rv);
			exit(-1);

		}

		for (int i=0 ; i!= count;i++)
		{
			C7_HRESOURCE hPU = PUArr[i];
			C7_STR szPUID;
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
		//直连设备
		// 		C7_HRESOURCE hPU;
		// 		rv = C7_ForkOnePU(session, "", &hPU);
		// 		if (rv == NRCAP_SUCCESS)
		// 		{
		// 			C7_GetResourceType(hPU,szType);
		// 			C7_GetResourceUsable(hPU,&nUsable);
		// 			if (nUsable == 0 && strcmp(szType, RT_SELF) == 0)
		// 			{
		// 				//LOG4CPLUS_ERROR(logger,"No Find " <<id << " Camera!!!");
		// 				//不在线设备
		// 				return false;
		// 			}
		// 			C7_GetResourcePUID(hPU,szPUID);
		// 			C7_GetResourceName(hPU,szName);
		// 			ResourceInfo info;
		// 			info.hresource = hPU;
		// 			info.szPUID = szPUID;
		// 			info.szType = szType;
		// 			info.szName = szName;
		// 			info.usable = nUsable;
		// 			resourceMap.insert(make_pair(id,info));

		//		}
	}

	return true;
}