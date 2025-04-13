namespace IpcPcQueue.Queue;

public abstract class IQueue
{
    public abstract bool Enqueue(byte[] msgBytes);
    public abstract int Dequeue(ref byte[] buffer);
}