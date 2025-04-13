using System.Text;
using IpcPcQueue.Queue;

var q = new SpscQueue("myQueue");
while (true)
{
    var msgStr = GenerateRandomString();;
    if (q.Enqueue(Encoding.UTF8.GetBytes(msgStr)))
    {
        Console.WriteLine($"msg: {msgStr}");
    }
    else
    {
        Console.WriteLine("Queue full");
        Thread.Sleep(1000);
    }
} 
        
static string GenerateRandomString()
{
    Random random = new Random();
    int length = random.Next(10, 10); // Random length between 10 and 50
    const string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    StringBuilder sb = new StringBuilder();

    for (int i = 0; i < length; i++)
    {
        sb.Append(chars[random.Next(chars.Length)]);
    }

    return DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + sb;
}
