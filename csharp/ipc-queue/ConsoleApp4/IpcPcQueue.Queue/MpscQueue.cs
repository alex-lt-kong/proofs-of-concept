using System.Diagnostics;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;

namespace IpcPcQueue.Queue;

public unsafe class MpscQueue : SpscQueue
{
    public MpscQueue(string queueName, int queueSizeBytes = 1357, bool initialize = false) : base(queueName, queueSizeBytes, initialize)
    {
    }

    public new bool Enqueue(byte[] msgBytes)
    {
        int msgLength = msgBytes.Length;
        int recordLength = 4 + msgLength; // 4 bytes for the length field plus the payload.

        int* headPtr = (int*)BasePtr;
        int* tailPtr = (int*)(BasePtr + sizeof(int));
        byte* dataOffset = BasePtr + HeaderSize;

        int head = Volatile.Read(ref *headPtr);

        int tail;
        int newTail;

        do
        {
            tail = Volatile.Read(ref *tailPtr);
            int used = GetUsedSpace(head, tail);
            int free = DataSize - used;

            if (free < recordLength * 20)
                return false;

            newTail = tail + recordLength;
            if (newTail >= DataSize)
            {
                if (DataSize - tail >= 4)
                {
                    // Write a wrap marker (-1) to signal a jump to the start.
                    *((int*)(dataOffset + tail)) = -1;
                }
                newTail = recordLength; // Funny, newTail will be the size of the record.
            }
            /*
                reserves space by doing a CAS:  compares a value in a memory location with an expected value and,
                if they're equal, replaces it with a new value.
                 
                function CompareExchange(location, newValue, expectedValue):
                    originalValue = location
                    if (location == expectedValue):
                        location = newValue
                    return originalValue
             */
        } while (Interlocked.CompareExchange(ref *tailPtr, newTail, tail) != tail);

        int msgOffset = tail;
        
        var t = Volatile.Read(ref *((int*)(dataOffset + msgOffset)));
        if (t != -2 && t != -1 && t != 0)
        {
            throw new ApplicationException($"current msgLength not -1 and -2 ({t}), head: {head}, tail: {tail}");
        }

        if (msgOffset + recordLength > DataSize)
        {
            if (DataSize - msgOffset >= sizeof(int))
            {
                // Write a wrap marker (-1) to signal a jump to the start.
                *((int*)(dataOffset + msgOffset)) = -1;
            }
            msgOffset = 0; // Wrapped, new position starts at the beginning
        }
        

        Marshal.Copy(msgBytes, 0, new IntPtr(dataOffset + msgOffset + sizeof(int)), msgLength);
        Volatile.Write(ref *((int*)(dataOffset + msgOffset)), msgLength);

        return true;
    }
}
