// StopWait.cpp : �������̨Ӧ�ó������ڵ㡣
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
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	
	return 0;
}

