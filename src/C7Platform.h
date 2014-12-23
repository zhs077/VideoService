#ifndef C7_PLATFROM_
#define C7_PLATFROM_
#include "platform_config.h"
#define  GET_CNT 100

typedef struct ResourceInfo
{
	C7_HRESOURCE hresource;
	string szPUID;
	string szName;
	string szType;
	int usable;	
	int index;
}ResourceInfo;
typedef map<string,vector<ResourceInfo> > ResourceMap;
class C7Platform
{
public:
	C7Platform(void);
	~C7Platform(void);
	//初始化C7环境
	static bool Init(const string& current_path);
	static void UnInit();
private:
	static void CreateResource(void* *pRsc,int count);

public:
	static C7_HSESSION session;
public:
	static ResourceMap resourceMap;

};


#endif
