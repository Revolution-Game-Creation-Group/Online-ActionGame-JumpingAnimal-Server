#include "Entry.h"
#include "WorkerThread.h"
#include "CompletionHandler.h"
#include "SessionManager.h"

bool		WorkerThread::threadLoop = true;

WorkerThread::WorkerThread()
{
}

WorkerThread::~WorkerThread()
{
	threadLoop = false;
	::WaitForSingleObject( handleThread, 1000 );
}

void WorkerThread::run()
{
	DWORD bytesTransfer, keyValue;
	LPOVERLAPPED overlapped;
	BOOL retVal;
	HANDLE handleIOCP = CompletionHandler::getInstance()->getWorkerIOCPHandle();

	while( threadLoop == true )
	{
		retVal = ::GetQueuedCompletionStatus( handleIOCP, &bytesTransfer, &keyValue, &overlapped, INFINITE );

		if( retVal == TRUE && keyValue != 0 && bytesTransfer != 0 && overlapped != 0 )
		{
			Session* session = ( Session* )keyValue;

			try
			{
				session->dispatch( bytesTransfer, overlapped );
			}
			catch( SessionAbortException* ex )
			{
				SessionManager::getInstance()->removeSession( session );

				delete ex;
			}
		}
		else if( keyValue != 0 && ( retVal == FALSE || bytesTransfer == 0 ) )
			SessionManager::getInstance()->removeSession( ( Session* )keyValue );
	}
}