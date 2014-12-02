#include "VideoCalculator.h"

CVideoCalculator::CVideoCalculator(void)
: CFpsCounter(10)
{
}

CVideoCalculator::~CVideoCalculator(void)
{
}

void CVideoCalculator::onReceivedImage() {
	inc();
	//m_fpsCounter.inc();
}

int CVideoCalculator::getFps() {
	return CFpsCounter::getFps();
}

int CVideoCalculator::getAverageFps() {
	return CFpsCounter::getAverageFps();
}