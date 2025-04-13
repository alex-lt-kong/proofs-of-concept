using System.Runtime.InteropServices;


namespace IpcPcQueue.Queue
{
    public unsafe class MpscLockedQueue : SpscQueue
    {
        private readonly Mutex _producerMutex;

        public MpscLockedQueue(string queueName, int queueSizeBytes = 1357, bool initialize = false)
            : base(queueName, queueSizeBytes, initialize)
        {
            // Using "Global\\" so that the mutex is visible to all processes.
            // (If your IPC is confined to a single session, "Local\\" might suffice.)
            _producerMutex = new Mutex(false, "Global\\" + queueName + "_MpscProducerMutex");
        }


        public new bool Enqueue(byte[] msgBytes)
        {
            // Acquire the producer mutex; this ensures that even if multiple producers call
            // Enqueue() concurrently, only one will reserve and write its record at any given moment.
            _producerMutex.WaitOne();
            try
            {
                int msgLength = msgBytes.Length;
                // Each record takes 4 bytes for the length and the bytes for the payload.
                int recordLength = sizeof(int) + msgLength;

                int* headPtr = (int*)BasePtr;
                int* tailPtr = (int*)(BasePtr + sizeof(int));
                byte* dataOffset = BasePtr + HeaderSize;

                int head = Volatile.Read(ref *headPtr);
                int tail = Volatile.Read(ref *tailPtr);

                int used = GetUsedSpace(head, tail);
                int free = DataSize - used;

                // Check free space. Multiplying recordLength by 2 (as in your SPSC code)
                // provides some headroom; adjust as needed.
                if (free < recordLength * 2)
                    return false;

                int msgOffset = tail;
                // If the record would not fit contiguously to the end, insert a wrap marker.
                if (msgOffset + recordLength > DataSize)
                {
                    if (DataSize - msgOffset >= sizeof(int))
                    {
                        // Write a wrap marker (-1) to signal to the consumer a jump to the beginning.
                        *((int*)(dataOffset + msgOffset)) = -1;
                    }
                    msgOffset = 0;
                }

                // Copy the message's payload.
                Marshal.Copy(msgBytes, 0, new IntPtr(dataOffset + msgOffset + 4), msgLength);
                // Publish the record by writing the message length last.
                // This ensures the consumer will see a complete record when reading.
                Volatile.Write(ref *((int*)(dataOffset + msgOffset)), msgLength);

                int newTail = msgOffset + recordLength;
                if (newTail >= DataSize)
                    newTail = 0;

                // Update the tail pointer.
                Volatile.Write(ref *tailPtr, newTail);
                return true;
            }
            finally
            {
                _producerMutex.ReleaseMutex();
            }
        }

        /// <summary>
        /// Dispose of the producer mutex along with the base resources.
        /// </summary>
        public new void Dispose()
        {
            base.Dispose();
            _producerMutex.Dispose();
        }
    }
}
