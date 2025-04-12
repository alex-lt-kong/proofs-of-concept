using System.Diagnostics;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;

namespace PcQueueExample
{

    public unsafe class LockFreePcQueue : IDisposable
    {
        private string MAPPED_FILE_NAME = "Local\\";
        
        private const int HeaderSize = sizeof(int) * 2;
        private const int DataSize = 20 * 100;
        private const int TotalSize = DataSize + HeaderSize;  // Total file size in bytes (header + data)
        private byte* basePtr = null;
        private MemoryMappedFile _mmf;
        private MemoryMappedViewAccessor _accessor;
        public LockFreePcQueue(string queueName, bool initialize = false)
        {
            MAPPED_FILE_NAME += queueName;
            _mmf = MemoryMappedFile.CreateOrOpen(MAPPED_FILE_NAME, TotalSize, MemoryMappedFileAccess.ReadWrite);
            _accessor = _mmf.CreateViewAccessor(0, TotalSize, MemoryMappedFileAccess.ReadWrite);
            
            _accessor.SafeMemoryMappedViewHandle.AcquirePointer(ref basePtr);
            
            if (!initialize) return;
            _accessor.Write(0, 0); // head
            _accessor.Write(4, 0); // tail
            
        }
        
        public unsafe bool Enqueue(byte[] msgBytes)
        {
            int msgLength = msgBytes.Length;
            int recordLength = 4 + msgLength; // 4 bytes for the length field plus the payload.
        

            int* headPtr = (int*)basePtr;
            int* tailPtr = (int*)(basePtr + sizeof(int));
            byte* dataOffset = basePtr + HeaderSize;

            int head = Volatile.Read(ref *headPtr);
            int tail = Volatile.Read(ref *tailPtr);
            
            int used = GetUsedSpace(head, tail);
            int free = DataSize - used;
            
            //Console.WriteLine($"tail: {tail}, head: {head}, DataSize: {DataSize}, used: {used}");
            if (free < recordLength * 2)
                return false;

            int pos = tail;
            // If the record would not fit contiguously, write a wrap marker and wrap.
            if (pos + recordLength > DataSize)
            {
                if (DataSize - pos >= 4)
                {
                    // Write a wrap marker (-1) to signal a jump to the start.
                    *((int*)(dataOffset + pos)) = -1;
                }
                pos = 0;
            }

            // Copy the message bytes into the buffer at the reserved position.
            Marshal.Copy(msgBytes, 0, new IntPtr(dataOffset + pos + 4), msgLength);
            // Publish the record by writing the message length with a release (volatile) write.
            // (This acts as a commit step, ensuring that the message bytes are visible before the consumer reads them.)
            Volatile.Write(ref *((int*)(dataOffset + pos)), msgLength);

            int newTail = pos + recordLength;
            if (newTail >= DataSize)
            {
                newTail = 0;
            }
            Volatile.Write(ref *tailPtr, newTail);

            return true;
        }
        
        public int Dequeue(ref byte[] buffer)
        {
            if (!MessageAvailable()) 
                return -1;
            
            int msgLength = -1;
            int* headPtr = (int*)basePtr;
            byte* dataBase = basePtr + HeaderSize;

            int head = Volatile.Read(ref *headPtr);
            int record = *((int*)(dataBase + head));

            // If a wrap marker (-1) is encountered, reset head to the beginning.
            if (record == -1)
            {
                head = 0;
                Volatile.Write(ref *headPtr, head);
                record = *((int*)(dataBase + head));
            }
            msgLength = record;
            
            if (buffer.Length < msgLength)
                throw new AccessViolationException();

            // Copy the message bytes from shared memory into the provided buffer.
            Marshal.Copy(new IntPtr(dataBase + head + 4), buffer, 0, msgLength);

            // Advance the head pointer past the record.
            int newHead = head + 4 + msgLength;
            if (newHead >= DataSize)
            {
                newHead = 0;
            }
            Volatile.Write(ref *headPtr, newHead);

            
            return msgLength;
        }
        
        private bool MessageAvailable()
        {
            int* headPtr = (int*)basePtr;
            int* tailPtr = (int*)(basePtr + sizeof(int));
            int head = Volatile.Read(ref *headPtr);
            int tail = Volatile.Read(ref *tailPtr);
            return head != tail;
        }

        public void Dispose()
        {
            _accessor.SafeMemoryMappedViewHandle.ReleasePointer();
            _accessor?.Dispose();
            _mmf?.Dispose();
        }

        private int GetUsedSpace(int head, int tail)
        {
            if (tail >= head)
                return tail - head;
            return DataSize - (head - tail);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct MyPayload
    {
        public long UnixTimeTs;
        public long SeqNum;
        public string text;
    }

    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length > 0 && args[0].ToLower() == "consumer")
            {
                RunConsumer(args[1]);
            }
            else if (args.Length > 0 && args[0].ToLower() == "producer")
            {
                RunProducer(args[1]);
            }
            else
            {
                Console.WriteLine("Usage: PcQueueExample.exe [consumer|producer]");
            }
        }
        
        static void RunConsumer(string qName)
        {
            Console.WriteLine("Consumer started and initializing the queue...");
            long prevT1 = 0;
            byte[] msgBytes;
            long[] seqNumByProducers = new long[1];
            unsafe
            {
                msgBytes = new byte[sizeof(MyPayload)];
            }
            MyPayload payload;
            long msgCount = 0;
            //long prevSeqNum = -1;
            long sampleCount = 0;
            long maxLatencyUs = -1;
            const long stdoutIntervalSec = 5;
            var queue = new LockFreePcQueue(qName);
            Console.WriteLine("Waiting for messages...");
            while (true)
            {
                if (queue.Dequeue(ref msgBytes) < 0)
                {
                    continue;
                }
                unsafe {
                    fixed (byte* bytePtr = msgBytes)
                    {
                        payload = *(MyPayload*)bytePtr;
                    }
                }
                DateTimeOffset now = DateTimeOffset.UtcNow;
                TimeSpan timeSinceEpoch = now - DateTimeOffset.UnixEpoch;
                long t1 = timeSinceEpoch.Ticks / (TimeSpan.TicksPerMillisecond / 1000);
                ++msgCount;
                
                if (payload.SeqNum != seqNumByProducers[0] + 1) {
                    Console.WriteLine($"prevSeqNum: {seqNumByProducers[0]}, payload.SeqNum: {payload.SeqNum}, diff:{payload.SeqNum - seqNumByProducers[0]}");
                }
                seqNumByProducers[0] = payload.SeqNum;
                if (!(t1 - prevT1 > stdoutIntervalSec * 1_000_000)) continue;
                
                ++sampleCount;
                var latencyUs = t1 - payload.UnixTimeTs;
                if (maxLatencyUs < latencyUs && sampleCount > 1)
                    maxLatencyUs = latencyUs;
                Console.WriteLine($"SeqNum: {payload.SeqNum}, t0: {payload.UnixTimeTs}, latencyUs: {latencyUs}, maxLatencyUs (percentile:{(1-1.0/sampleCount)*100.0:n1}): {maxLatencyUs}, msgCount/sec: {msgCount / stdoutIntervalSec:n0}");
                prevT1 = t1;
                msgCount = 0;
            }
        }

        static void RunProducer(string qName)
        {
            var byteArrayPool = new byte[128][];
            for (int i = 0; i < byteArrayPool.Length; i++)
            {
                unsafe {
                    byteArrayPool[i] = new byte[sizeof(MyPayload)];
                }
            }

            var queue = new LockFreePcQueue(qName);
            Console.WriteLine("Producer started.");
            long msgCount = 0;
            long idx = 0;
            MyPayload payload;
            while (true)
            {
                DateTimeOffset now = DateTimeOffset.UtcNow;
                TimeSpan timeSinceEpoch = now - DateTimeOffset.UnixEpoch;
                payload.UnixTimeTs = timeSinceEpoch.Ticks / (TimeSpan.TicksPerMillisecond / 1000);
                payload.SeqNum = msgCount;
                unsafe {
                    fixed (byte* bytePtr = byteArrayPool[idx])
                    {
                        var structPtr = (byte*)&payload;
                        for (int i = 0; i < sizeof(MyPayload); i++)
                        {
                            bytePtr[i] = structPtr[i];
                        }
                    }
                }
                bool success = queue.Enqueue(byteArrayPool[idx]);
                /*
                for (int i = 0; i < 10000; ++i)
                {
                }*/

                //Thread.Sleep(5000);
                if (!success)
                {
                //    Console.WriteLine($"Queue is full. Try again later. (msgCount: {msgCount})");
                //   Thread.Sleep(1);
                }
                else
                {
                  //  Console.WriteLine($"Enqueue()ed {msgCount}");
                    msgCount++;
                    idx = msgCount % 128;
                }
            }
        }
    }
}


