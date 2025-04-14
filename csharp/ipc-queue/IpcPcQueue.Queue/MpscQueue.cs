using System.Runtime.InteropServices;

namespace IpcPcQueue.Queue;

public unsafe class MpscQueue : SpscQueue, IQueue
{
    private readonly int _maxProducerCount;
    
    public MpscQueue(string queueName, bool ownership = false, int capacity = 1000, int maxMsgSize = 128, int maxProducerCount = 8): 
        base(queueName, ownership, capacity, maxMsgSize)
    {
        _maxProducerCount = maxProducerCount;
        QueueSize = capacity * (_maxElementSize + maxProducerCount + 1);
        
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="msgBytes">Enqueue() does not take ownership of msgBytes, feel free to reused it</param>
    /// <returns></returns>
    public new bool Enqueue(byte[] msgBytes)
    {
        int msgLength = msgBytes.Length;
        if (msgLength > MaxMsgSizeExclusive - 1)
            throw new ArgumentException($"Message length {msgLength} exceeds max element size {MaxMsgSizeExclusive - 1}");
        int elementLength = sizeof(int) + msgLength; // 4 bytes for the length field plus the payload.

        int* headPtr = (int*)BasePtr;
        int* tailPtr = (int*)(BasePtr + sizeof(int));
        byte* dataOffset = BasePtr + HeaderSize;

        int head = Volatile.Read(ref *headPtr);

        int tail;
        int newTail;

        do
        {
            tail = Volatile.Read(ref *tailPtr);
            int used = GetUsedBytes(head, tail);
            int free = QueueSize - used;

            if (free < _maxElementSize * (_maxProducerCount + 1))
                return false;

            
            if (tail + _maxElementSize >= QueueSize)
            {
                if (QueueSize - tail >= 4)
                {
                    // Write a wrap marker (-1) to signal a jump to the start.
                    *((int*)(dataOffset + tail)) = FLAG_HEAD_WRAPPED;
                }

                newTail = elementLength; // Funny, newTail will be the size of the record.
            }
            else
            {
                newTail = tail + elementLength;
            }
            /*
                reserves space by doing a CAS: compares a value in a memory location with an expected value and,
                if they're equal, replaces it with a new value.

                function CompareExchange(location, newValue, expectedValue):
                    originalValue = location
                    if (location == expectedValue):
                        location = newValue
                    return originalValue
             */
        } while (Interlocked.CompareExchange(ref *tailPtr, newTail, tail) != tail);

        int msgOffset = tail;

        if (msgOffset + elementLength > QueueSize)
        {
            if (QueueSize - msgOffset >= sizeof(int))
            {
                // Write a wrap marker (-1) to signal a jump to the start.
                *((int*)(dataOffset + msgOffset)) = FLAG_HEAD_WRAPPED;
            }

            msgOffset = 0; // Wrapped, new position starts at the beginning
        }


        Marshal.Copy(msgBytes, 0, new IntPtr(dataOffset + msgOffset + sizeof(int)), msgLength);
        Volatile.Write(ref *((int*)(dataOffset + msgOffset)), msgLength);

        return true;
    }
}