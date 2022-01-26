#include "stdafx.h"

#pragma once

class CQueue {
        
        #define  MAX_SIZE  (64 * 1024)
        
private:
        CRITICAL_SECTION  AccessLock;
        BYTE              *Buffer;
        int               Header;
        int               Tail; 
        BOOL              Empty;
        BOOL              Full;

public:
        CQueue()  {
                InitializeCriticalSection (&AccessLock);
                Buffer = (BYTE *) new BYTE [MAX_SIZE];
                Header = 0;
                Tail   = 0;
                Full   = FALSE;
                Empty  = TRUE;
        };
        
        ~CQueue() {
                DeleteCriticalSection(&AccessLock);
                delete   Buffer;
                Buffer   = NULL;                                
        };
        
        UINT Put (BYTE *DataPtr, UINT Length = 1)   // Add element into queue
        {                     
                UINT   ProcessedLen = 0;  
				if (Length) {
					EnterCriticalSection(&AccessLock);                
					if (!Full) {  // Not full                   
						while (ProcessedLen < Length) {
							Buffer[Tail++] = *DataPtr++;
							ProcessedLen++;						
							if (Tail == MAX_SIZE) 
								Tail  = 0;
							if (Tail == Header) {
								Full  = TRUE;  
								break;
							}
						}					
						if (Empty)
							Empty = FALSE;
					}
					LeaveCriticalSection(&AccessLock);
				}
                return ProcessedLen;
        };        		

        UINT Get (BYTE *DataPtr, UINT Length = 1)   // Retrieve element from the queue
        {       
                UINT   ProcessedLen = 0; 
				if (Length) {
					EnterCriticalSection(&AccessLock);
					if (!Empty) {  // Not empty
						while (ProcessedLen < Length) {
							*DataPtr++ = Buffer[Header++];
							ProcessedLen++;
							if (Header == MAX_SIZE) 
								Header = 0;                    
							if (Tail == Header) {
								Empty = TRUE;
								break;
							}
						}					                    
						if (Full)
							Full = FALSE;                        
					}
					LeaveCriticalSection(&AccessLock);
				}
                return ProcessedLen;
        };

        VOID Reset () {  // reset queue
                EnterCriticalSection(&AccessLock);
                Header = 0;
                Tail   = 0;
                Full   = FALSE;
                Empty  = TRUE;
                LeaveCriticalSection(&AccessLock);
        }

        BOOL IsEmpty () {    // check if queue is empty
                BOOL Result;
                EnterCriticalSection(&AccessLock);
                Result = Empty;
                LeaveCriticalSection(&AccessLock);
                return Result;
        };
        
        BOOL IsFull () {    // check if queuw is full
                BOOL Result;
                EnterCriticalSection(&AccessLock);
                Result = Full;                
                LeaveCriticalSection(&AccessLock);
                return Result;
        };        
};
