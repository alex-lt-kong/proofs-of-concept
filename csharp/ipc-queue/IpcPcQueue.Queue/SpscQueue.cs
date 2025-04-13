using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;

namespace IpcPcQueue.Queue;

public unsafe class SpscQueue : IQueue, IDisposable
{
    private readonly string _mappedFileName = "Local\\";
    protected const int HeaderSize = sizeof(int) * 2;
    protected readonly int DataSize;
    protected readonly byte* BasePtr = null;
    private readonly bool _initialize;
    private readonly MemoryMappedFile _mmf;
    private readonly MemoryMappedViewAccessor _accessor;

    public SpscQueue(string queueName, bool initialize = false, int capacityBytes = 3210)
    {
        _mappedFileName += queueName;
        _initialize = initialize;
        DataSize = capacityBytes;
        var totalSize = DataSize + HeaderSize;
        _mmf = MemoryMappedFile.CreateOrOpen(_mappedFileName, totalSize, MemoryMappedFileAccess.ReadWrite);
        _accessor = _mmf.CreateViewAccessor(0, totalSize, MemoryMappedFileAccess.ReadWrite);

        _accessor.SafeMemoryMappedViewHandle.AcquirePointer(ref BasePtr);

        if (!initialize) return;
        _accessor.Write(0, 0); // head
        _accessor.Write(4, 0); // tail
    }

    public override bool Enqueue(byte[] msgBytes)
    {
        int msgLength = msgBytes.Length;
        int recordLength = sizeof(int) + msgLength; // one int for the length field plus the payload.

        int* headPtr = (int*)BasePtr;
        int* tailPtr = (int*)(BasePtr + sizeof(int));
        byte* dataOffset = BasePtr + HeaderSize;

        int head = Volatile.Read(ref *headPtr);
        int tail = Volatile.Read(ref *tailPtr);

        int used = GetUsedSpace(head, tail);
        int free = DataSize - used;

        if (free < recordLength * 2)
            return false;

        int msgOffset = tail;
        // If the record would not fit contiguously, write a wrap marker and wrap.
        if (msgOffset + recordLength > DataSize)
        {
            if (DataSize - msgOffset >= sizeof(int))
            {
                // Write a wrap marker (-1) to signal a jump to the start.
                *((int*)(dataOffset + msgOffset)) = -1;
            }

            msgOffset = 0;
        }

        Marshal.Copy(msgBytes, 0, new IntPtr(dataOffset + msgOffset + 4), msgLength);
        Volatile.Write(ref *((int*)(dataOffset + msgOffset)), msgLength);

        int newTail = msgOffset + recordLength;
        if (newTail >= DataSize)
        {
            newTail = 0;
        }

        Volatile.Write(ref *tailPtr, newTail);
        // Volatile.Write() is C#'s equivalent of std::atomic.store(next_tail, std::memory_order_release);

        return true;
    }

    public override int Dequeue(ref byte[] buffer)
    {
        int* headPtr = (int*)BasePtr;
        int* tailPtr = (int*)(BasePtr + sizeof(int));
        byte* dataOffset = BasePtr + HeaderSize;

        int head = Volatile.Read(ref *headPtr);
        int tail = Volatile.Read(ref *tailPtr);
        if (head == tail)
            return -1; // No message available.

        int msgLength = Volatile.Read(ref *((int*)(dataOffset + head)));
        if (msgLength == 0)
        {
            return -1;
        }
        if (msgLength == -2) // For MPSC issue, slot allocated, but not written yet
            return -1; 

        
        if (msgLength == -1)
        {
            head = 0;
            Volatile.Write(ref *headPtr, head);
            msgLength = *((int*)(dataOffset + head));
            if (msgLength == 0) return -1;
        }

        if (msgLength == -2) // For MPSC issue, slot allocated, but not written yet
            return -1; 


        Volatile.Write(ref *((int*)(dataOffset + head)), -2);
        Marshal.Copy(new IntPtr(dataOffset + head + 4), buffer, 0, msgLength);

        // Advance the head pointer past the record.
        int newHead = head + 4 + msgLength;
        Volatile.Write(ref *((int*)(dataOffset + head)), -2);
        if (newHead >= DataSize)
        {
            newHead = 0;
        }

        Volatile.Write(ref *headPtr, newHead);
        return msgLength;
    }

    public void Dispose()
    {
        _accessor.SafeMemoryMappedViewHandle.ReleasePointer();
        //Accessor?.Dispose();
        if (_initialize)
            _mmf?.Dispose();
    }

    public int GetUsedSpace(int head = -1, int tail = -1)
    {
        if (head == -1)
        {
            int* headPtr = (int*)BasePtr;
            head = Volatile.Read(ref *headPtr);
        }

        if (tail == -1)
        {
            int* tailPtr = (int*)(BasePtr + sizeof(int));
            tail = Volatile.Read(ref *tailPtr);
        }

        if (tail >= head)
            return tail - head;
        return DataSize - (head - tail);
    }

    public int GetHeadIndex()
    {
        int* headPtr = (int*)BasePtr;
        return Volatile.Read(ref *headPtr);
    }

    public int GetTailIndex()
    {
        int* tailPtr = (int*)(BasePtr + sizeof(int));
        return Volatile.Read(ref *tailPtr);
    }
}