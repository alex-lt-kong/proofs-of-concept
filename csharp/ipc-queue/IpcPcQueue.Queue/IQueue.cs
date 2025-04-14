namespace IpcPcQueue.Queue;

public interface IQueue
{
    public void Init();
    public bool Enqueue(byte[] msgBytes);
    public int Dequeue(ref byte[] buffer);
    public int GetHeadOffset();
    public int GetTailOffset();
    public int GetUsedBytes(int head = -1, int tail = -1);
}