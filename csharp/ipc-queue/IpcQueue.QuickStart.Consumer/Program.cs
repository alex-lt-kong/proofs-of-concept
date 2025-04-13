using IpcPcQueue.Queue;

var q = new SpscQueue("myQueue", true);
byte[] msg = new byte[512];
Console.WriteLine("Waiting for messages...");
while (true)
    if (q.Dequeue(ref msg) > 0)
        Console.WriteLine($"msg: {System.Text.Encoding.UTF8.GetString(msg)}");
