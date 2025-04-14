using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;

namespace IpcPcQueue.Queue;


public unsafe class SpscQueue : IQueue, IDisposable
{
    protected const int FLAG_HEAD_WRAPPED = -1;
    private const int FLAG_MSG_UNCOMMITTED = -2;
    private readonly string _mappedFileName = "Local\\";
    protected const int HeaderSize = sizeof(int) * 2;
    protected int QueueSize;
    protected readonly int MaxMsgSizeExclusive;
    protected readonly int _maxElementSize;
    protected  byte* BasePtr = null;
    private readonly bool _ownership;
    private  MemoryMappedFile _mmf;
    private  MemoryMappedViewAccessor _accessor;
    private int TotalSize => QueueSize + HeaderSize;

    public void Init()
    {
        _mmf = MemoryMappedFile.CreateOrOpen(_mappedFileName, TotalSize, MemoryMappedFileAccess.ReadWrite);
        _accessor = _mmf.CreateViewAccessor(0, TotalSize, MemoryMappedFileAccess.ReadWrite);
        _accessor.SafeMemoryMappedViewHandle.AcquirePointer(ref BasePtr);

        if (!_ownership) return;
        _accessor.Write(0, 0); // head
        _accessor.Write(4, 0); // tail
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="queueName"></param>
    /// <param name="ownership">Whether this process owns the queue, i.e., it is responsible for the allocation and release the resources used by the queue</param>
    /// <param name="capacity">The lower bound of the max number of messages this queue can hold</param>
    /// <param name="maxMsgSize">The max size in bytes a message can have</param>
    public SpscQueue(string queueName, bool ownership = false, int capacity = 1000, int maxMsgSize = 128)
    {
        _ownership = ownership;
        _mappedFileName += queueName;
        MaxMsgSizeExclusive = maxMsgSize + 10; // This should be opening range
        _maxElementSize = MaxMsgSizeExclusive + sizeof(int); // 4 bytes for the length field plus the payload.
        QueueSize = capacity * (_maxElementSize + 1);
    }
    
    /// <summary>
    /// 
    /// </summary>
    /// <param name="msgBytes">Enqueue() does not take ownership of msgBytes, feel free to reused it</param>
    /// <returns></returns>
    public bool Enqueue(byte[] msgBytes)
    {
        int msgLength = msgBytes.Length;
        if (msgLength > MaxMsgSizeExclusive - 1)
            throw new ArgumentException($"Message length {msgLength} exceeds max element size {MaxMsgSizeExclusive - 1}");
        int elementLength = sizeof(int) + msgLength; // one int for the length field plus the payload.

        int* headPtr = (int*)BasePtr;
        int* tailPtr = (int*)(BasePtr + sizeof(int));
        byte* dataOffset = BasePtr + HeaderSize;

        int head = Volatile.Read(ref *headPtr);
        int tail = Volatile.Read(ref *tailPtr);

        int used = GetUsedBytes(head, tail);
        int free = QueueSize - used;

        if (free < _maxElementSize * 2)
            return false;

        int msgOffset = tail;
        // If the record would not fit contiguously, write a wrap marker and wrap.
        if (msgOffset + _maxElementSize > QueueSize)
        {
            if (QueueSize - msgOffset >= sizeof(int))
            {
                // Write a wrap marker (-1) to signal a jump to the start.
                *((int*)(dataOffset + msgOffset)) = FLAG_HEAD_WRAPPED;
            }

            msgOffset = 0;
        }

        Marshal.Copy(msgBytes, 0, new IntPtr(dataOffset + msgOffset + 4), msgLength);
        Volatile.Write(ref *((int*)(dataOffset + msgOffset)), msgLength);

        int newTail = msgOffset + elementLength;
        if (newTail >= QueueSize)
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
        byte* queueBase = BasePtr + HeaderSize;

        int head = Volatile.Read(ref *headPtr);
        int tail = Volatile.Read(ref *tailPtr);
        if (head == tail)
            return -1; // No message available.

        var msgLength = Volatile.Read(ref *((int*)(queueBase + head)));
        if (msgLength == FLAG_HEAD_WRAPPED)
        {
            head = 0;
            Volatile.Write(ref *headPtr, head);
            msgLength = *((int*)(queueBase + head));
            if (msgLength == 0) return -1;
        }
        if (msgLength == FLAG_MSG_UNCOMMITTED) // MPSC-specific, slot allocated, but not committed yet
            return -1; 
        if (msgLength == 0)
            return -1;

        Volatile.Write(ref *((int*)(queueBase + head)), FLAG_MSG_UNCOMMITTED); // MPSC-specific, mark slot as uncommitted
        Marshal.Copy(new IntPtr(queueBase + head + 4), buffer, 0, msgLength);

        // Advance the head pointer past the record.
        int newHead = head + 4 + msgLength;
        //Console.WriteLine($"msgLength: {msgLength}");
        // Volatile.Write(ref *((int*)(dataOffset + head)), FLAG_MSG_UNCOMMITTED);
        if (newHead >= QueueSize)
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
        if (_ownership)
        {
            _mmf?.Dispose();
            Console.WriteLine($"This process owns the queue at [{_mappedFileName}], so it will release the queue now.");
        }
        else
        {
            Console.WriteLine($"This process does NOT own the queue at [{_mappedFileName}], so it will NOT release the queue.");
        }
    }

    public int GetUsedBytes(int head = -1, int tail = -1)
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
        return QueueSize - (head - tail);
    }

    public int GetHeadOffset()
    {
        var headPtr = (int*)BasePtr;
        return Volatile.Read(ref *headPtr);
    }

    public int GetTailOffset()
    {
        var tailPtr = (int*)(BasePtr + sizeof(int));
        return Volatile.Read(ref *tailPtr);
    }
}