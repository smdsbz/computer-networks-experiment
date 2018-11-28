// StopWait.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"


#define PROTOCAL_FAKETCP

#if defined(PROTOCAL_STOPWAIT)
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
using Sender = StopWaitRdtSender;
using Sender = StopWaitRdtReceiver;
#elif defined(PROTOCAL_GBN) || defined(PROTOCAL_FAKETCP)
#include "GBNRdtProtocal.h"
using Sender = GBNRdtProtocal::Sender;
using Receiver = GBNRdtProtocal::Receiver;
#elif defined(PROTOCAL_SR)
#include "SRRdtProtocal.h"
using Sender = SRRdtProtocal::Sender;
using Receiver = SRRdtProtocal::Receiver;
#else
#error Unknown protocal!
#endif


int main(int argc, char** argv[])
{
	#if defined(PROTOCAL_FAKETCP)
	RdtSender *ps = new Sender(true);
	#else
	RdtSender *ps = new Sender();
	#endif
	RdtReceiver * pr = new Receiver();
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("C:\\Users\\smdsbz\\Documents\\HUST\\CSIE1601\\Courseware\\Computer Networks\\computer-networks-experiment_local\\Exp02_TCPProtocal\\rdt-win-student\\input.txt");
	pns->setOutputFile("C:\\Users\\smdsbz\\Documents\\HUST\\CSIE1601\\Courseware\\Computer Networks\\computer-networks-experiment_local\\Exp02_TCPProtocal\\rdt-win-student\\output.txt");
	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//ָ��Ψһ�Ĺ�����ʵ����ֻ��main��������ǰdelete
	delete pns;										//ָ��Ψһ��ģ�����绷����ʵ����ֻ��main��������ǰdelete
	
	return 0;
}

