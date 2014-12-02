#pragma once
#include <windows.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include "myfunction.h"
using namespace std;


class CFpsCounter {
public:
	CFpsCounter(int nMaxSecond)
		: m_nMaxSecond(nMaxSecond)
		, m_nThisSecond(0)
		, m_nThisSecondImg(0)
		, m_nLastSecond(0)
		, m_nLastSecondImg(0) {
			
	}
	

public:
	void inc() {
		
		int nTime = time(NULL);
		if(nTime != m_nThisSecond) {
			m_nLastSecond = m_nThisSecond;
			m_nLastSecondImg = m_nThisSecondImg;
			m_nThisSecondImg = 1;
			m_nThisSecond = nTime;
			findAndInsertIfNotExists(m_mapTimeToFps, m_nLastSecond)
				= m_nLastSecondImg;
		} else {
			++ m_nThisSecondImg;
		}
		
	}

public:
	int getFps() {
		return m_nLastSecondImg;
	}

	int getAverageFps() {
		int t = time(NULL) - m_nMaxSecond;
		int total = 0;
		while(m_mapTimeToFps.begin() != m_mapTimeToFps.end()) {
			if(m_mapTimeToFps.begin()->first < t)
				m_mapTimeToFps.erase(m_mapTimeToFps.begin());
			else break;
		}
		for(map< int, int>::iterator itor = m_mapTimeToFps.begin();
			m_mapTimeToFps.end() != itor; ++ itor)
		{
			total += itor->second;
		}
		return total / m_nMaxSecond;
	}

private:
	map< int, int> m_mapTimeToFps;
	 int m_nMaxSecond;
	 int m_nThisSecond;
	 int m_nThisSecondImg;
	 int m_nLastSecond;
	 int m_nLastSecondImg;
	 	CRITICAL_SECTION m_cs;
	
};

class CVideoCalculator
	: public CFpsCounter
{
public:
	CVideoCalculator(void);
	~CVideoCalculator(void);

public:
	void onReceivedImage();

public:
	int getFps();

public:
	int getAverageFps();

private:
	//CFpsCounter m_fpsCounter;
};
