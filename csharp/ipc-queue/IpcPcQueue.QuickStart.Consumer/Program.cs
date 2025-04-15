using IpcPcQueue.Queue;

var q = new SpscQueue("myQueue", true, 10);
q.Init();
var msg = new byte[512];
Console.WriteLine("Waiting for messages...");
while (true)
{
    var recvSize = q.Dequeue(ref msg);
    //Thread.Sleep(2000);
    if (recvSize > 0)
        Console.WriteLine($"msg: {System.Text.Encoding.UTF8.GetString(msg[..recvSize])}, head: {q.GetHeadOffset()}");
    else
    {
     //   Console.WriteLine($"recvSize: {recvSize}");
    }
}

