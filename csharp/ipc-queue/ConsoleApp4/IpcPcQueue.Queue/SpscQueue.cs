using System.Diagnostics;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;

namespace IpcPcQueue.Queue;

public unsafe class SpscQueue : IDisposable
{
    protected readonly string MappedFileName = "Local\\";
    protected const int HeaderSize = sizeof(int) * 2;
    protected readonly int DataSize;
    protected readonly byte* BasePtr = null;
    protected readonly MemoryMappedFile _mmf;
    protected readonly MemoryMappedViewAccessor _accessor;

    public SpscQueue(string queueName, int queueSizeBytes = 4096, bool initialize = false)
    {
        MappedFileName += queueName;
        DataSize = queueSizeBytes;
        var totalSize = DataSize + HeaderSize;
        _mmf = MemoryMappedFile.CreateOrOpen(MappedFileName, totalSize, MemoryMappedFileAccess.ReadWrite);
        _accessor = _mmf.CreateViewAccessor(0, totalSize, MemoryMappedFileAccess.ReadWrite);

        _accessor.SafeMemoryMappedViewHandle.AcquirePointer(ref BasePtr);

        if (!initialize) return;
        _accessor.Write(0, 0); // head
        _accessor.Write(4, 0); // tail
    }

    public bool Enqueue(byte[] msgBytes)
    {
        int msgLength = msgBytes.Length;
        int recordLength = sizeof(int) + msgLength; // 4 bytes for the length field plus the payload.
        
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

    public int Dequeue(ref byte[] buffer)
    {
       
        int* headPtr = (int*)BasePtr;
        int* tailPtr = (int*)(BasePtr + sizeof(int));
        byte* dataOffset = BasePtr + HeaderSize;

        int head = Volatile.Read(ref *headPtr);
        int tail = Volatile.Read(ref *tailPtr);
        if (head == tail)
            return -1; // No message available.
        // Console.WriteLine($"In Dequeue(): head: {head}, tail: {tail}");
        int msgLength = *((int*)(dataOffset + head));
        if (msgLength == 0) // For MPSC issue, record == 0 means space allocated, but not written yet
            return -1; 

        // If a wrap marker (-1) is encountered, reset head to the beginning.
        if (msgLength == -1)
        {
            head = 0;
            Volatile.Write(ref *headPtr, head);
            msgLength = *((int*)(dataOffset + head));
        }
        
        if (msgLength == 0) throw new UnreachableException($"msgLength == 0 in Dequeue(), head: {head}, tail: {tail}");

        Marshal.Copy(new IntPtr(dataOffset + head + 4), buffer, 0, msgLength);

        // Advance the head pointer past the record.
        int newHead = head + 4 + msgLength;
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
        _accessor?.Dispose();
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