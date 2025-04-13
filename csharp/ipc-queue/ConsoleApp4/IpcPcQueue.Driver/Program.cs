using System.Runtime.InteropServices;
using IpcPcQueue.Queue;

namespace PcQueueExample
{
    [StructLayout(LayoutKind.Sequential)]
    public struct MyPayload
    {
        public long UnixTimeTs;
        public long SeqNum;
        public short ProducerId;
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
                RunProducer(args[1], short.Parse(args[2]));
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
            long[] seqNumByProducers = new long[short.MaxValue];
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
            var queue = new MpscQueue(qName);
            Console.WriteLine("Waiting for messages...");
            while (true)
            {
                //Thread.Sleep(1500);
                //Console.WriteLine($"Before Dequeue(), head: {queue.GetHeadIndex()}, tail: {queue.GetTailIndex()}, UsedSpace: {queue.GetUsedSpace()}");
                if (queue.Dequeue(ref msgBytes) <= 0)
                {
                    continue;
                }
                //Console.WriteLine($"After  Dequeue(), head: {queue.GetHeadIndex()}, tail: {queue.GetTailIndex()}, UsedSpace: {queue.GetUsedSpace()}");
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
                
                if (payload.SeqNum != seqNumByProducers[payload.ProducerId] + 1) {
                    Console.WriteLine($"producerId: {payload.ProducerId}, prevSeqNum: {seqNumByProducers[payload.ProducerId]}, payload.SeqNum: {payload.SeqNum}, diff:{payload.SeqNum - seqNumByProducers[payload.ProducerId]}, head: {queue.GetHeadIndex()}, tail: {queue.GetTailIndex()}, UsedSpace: {queue.GetUsedSpace()}");
                }
                seqNumByProducers[payload.ProducerId] = payload.SeqNum;
                if (!(t1 - prevT1 > stdoutIntervalSec * 1_000_000)  /*&& queue.GetUsedSpace() > 0*/) continue;
                
                ++sampleCount;
                var latencyUs = t1 - payload.UnixTimeTs;
                if (maxLatencyUs < latencyUs && sampleCount > 1)
                    maxLatencyUs = latencyUs;
                Console.WriteLine($"SeqNum: {payload.SeqNum}, ProducerId: {payload.ProducerId}, t0: {payload.UnixTimeTs}, latencyUs: {latencyUs}, maxLatencyUs (percentile:{(1-1.0/sampleCount)*100.0:n1}): {maxLatencyUs}, msgCount/sec: {msgCount / stdoutIntervalSec:n0}");
                prevT1 = t1;
                msgCount = 0;
            }
        }

        static void RunProducer(string qName, short producerId)
        {
            var byteArrayPool = new byte[128][];
            for (int i = 0; i < byteArrayPool.Length; i++)
            {
                unsafe {
                    byteArrayPool[i] = new byte[sizeof(MyPayload)];
                }
            }

            var queue = new MpscQueue(qName);
            Console.WriteLine("Producer started.");
            long msgCount = 0;
            long idx = 0;
            MyPayload payload;
            payload.ProducerId = producerId;
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
              //  Thread.Sleep(1000);
                if (!success)
                {
                    Console.WriteLine($"Enqueue() failed (msgCount: {msgCount}, UsedSpace: {queue.GetUsedSpace()}, head: {queue.GetHeadIndex()}, tail: {queue.GetTailIndex()})");
                    //Thread.Sleep(1);
                }
                else
                {
                    msgCount++;
                //    Console.WriteLine($"Enqueue()ed (head: {queue.GetHeadIndex()}, tail: {queue.GetTailIndex()}, msgCount: {msgCount}, UsedSpace: {queue.GetUsedSpace()})");
                    idx = msgCount % 128;
                }
            }
        }
    }
}


