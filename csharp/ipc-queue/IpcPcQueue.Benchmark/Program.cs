using System.Net.Mime;
using System.Runtime.InteropServices;
using Fclp;
using IpcPcQueue.Queue;

namespace IpcPcQueue.Benchmark
{
    [StructLayout(LayoutKind.Sequential)]
    public struct MyPayload
    {
        public Int64 UnixTimeTs;
        public Int64 SeqNum;
        public Int16 ProducerId;
    }
    
    internal class AppArgs
    {
        public string Role { get; set; }
        public string QueueName { get; set; }
        public string ProducerIdStr { get; set; }
        public short ProducerId => short.Parse(ProducerIdStr);
    }
    
    class Program
    {
        private static AppArgs? ReadCommandLineArgs(string[] args)
        {
            var parser = new FluentCommandLineParser<AppArgs?>();
            parser.Setup(arg => arg.Role).As('r', "role").WithDescription("Valid values are: 'SpscConsumer', 'SpscProducer', 'MpscConsumer', 'MpscProducer'").Required();
            parser.Setup(arg => arg.QueueName).As('q', "queue-name").Required();
            parser.Setup(arg => arg.ProducerIdStr).As('i', "producer-id").SetDefault("1");
            parser.SetupHelp("h", "help").Callback(text =>
            {
                Console.WriteLine(text);
                Environment.Exit(0);
            });
            
            var result = parser.Parse(args);
            if (!result.HasErrors) return parser.Object;
            Console.Error.WriteLine(result.ErrorText);
            return null;
        }
        
        static void Main(string[] args)
        {
            var commandLine = ReadCommandLineArgs(args);
            if (commandLine == null) return;

            switch (commandLine.Role)
            {
                case "SpscProducer":
                {
                    var queue = new SpscQueue(commandLine.QueueName);
                    RunProducer(queue, commandLine.ProducerId);
                    break;
                }
                case "MpscProducer":
                {
                    var queue = new MpscQueue(commandLine.QueueName);
                    RunProducer(queue, commandLine.ProducerId);
                    break;
                }
                case "SpscConsumer":
                {
                    var queue = new SpscQueue(commandLine.QueueName, true);
                    RunConsumer(queue);
                    break;
                }
                case "MpscConsumer":
                {
                    var queue = new MpscQueue(commandLine.QueueName, true);
                    RunConsumer(queue);
                    break;
                }
                default:
                    Console.WriteLine("Invalid role specified.");
                    break;
            }
        }

        private static void RunConsumer(IQueue queue)
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
            long sampleCount = 0;
            long maxLatencyUs = -1;
            const long stdoutIntervalSec = 5;
            //var queue = new T(queueName);
            Console.WriteLine("Waiting for messages...");
            while (true)
            {
                //Thread.Sleep(1);
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
                    Console.WriteLine($"producerId: {payload.ProducerId}, prevSeqNum: {seqNumByProducers[payload.ProducerId]}, payload.SeqNum: {payload.SeqNum}, diff:{payload.SeqNum - seqNumByProducers[payload.ProducerId]}");
                }

                seqNumByProducers[payload.ProducerId] = payload.SeqNum;
                if (!(t1 - prevT1 > stdoutIntervalSec * 1_000_000) /*&& queue.GetUsedSpace() > 0*/) continue;
                
                ++sampleCount;
                var latencyUs = t1 - payload.UnixTimeTs;
                if (maxLatencyUs < latencyUs && sampleCount > 1)
                    maxLatencyUs = latencyUs;
                Console.WriteLine($"SeqNum: {payload.SeqNum}, ProducerId: {payload.ProducerId}, t0: {payload.UnixTimeTs}, latencyUs: {latencyUs}, maxLatencyUs (percentile:{(1-1.0/sampleCount)*100.0:n1}): {maxLatencyUs}, msgCount/sec: {msgCount / stdoutIntervalSec:n0}");
                prevT1 = t1;
                msgCount = 0;
            }
        }

        static void RunProducer(IQueue queue, short producerId)
        {
            var byteArrayPool = new byte[128][];
            for (int i = 0; i < byteArrayPool.Length; i++)
            {
                unsafe {
                    byteArrayPool[i] = new byte[sizeof(MyPayload)];
                }
            }

            //var queue = new MpscQueue(queueName);
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
                    Console.WriteLine($"Enqueue() failed (msgCount: {msgCount})");
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


