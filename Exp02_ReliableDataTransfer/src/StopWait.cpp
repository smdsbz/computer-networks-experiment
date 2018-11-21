// StopWait.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"

#define PROTOCAL_SR

#if defined(PROTOCAL_STOPWAIT)
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
using Sender = StopWaitRdtSender;
using Sender = StopWaitRdtReceiver;
#elif defined(PROTOCAL_GBN)
#include "GBNRdtProtocal.h"
using Sender = GBNRdtProtocal::Sender;
using Receiver = GBNRdtProtocal::Receiver;
#elif defined(PROTOCAL_SR)
#include "SRRdtProtocal.h"
using Sender = SRRdtProtocal::Sender;
using Receiver = SRRdtProtocal::Receiver;
#else
#error Unknown potocal!
#endif


int main(int argc, char** argv[])
{
	RdtSender *ps = new Sender();
	RdtReceiver * pr = new Receiver();
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("..\\input.txt");
	pns->setOutputFile("..\\output.txt");
	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	
	return 0;
}

