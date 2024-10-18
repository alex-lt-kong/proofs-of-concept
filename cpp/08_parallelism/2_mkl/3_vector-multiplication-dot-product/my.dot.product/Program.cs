using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

class Program
{
    static void DotProductJob(double[] vecA, double[] vecB, int blockSize, long offset, ref double sum)
    {
        for (int i = 0; i < blockSize; ++i)
        {
            sum += vecA[i + offset] * vecB[i + offset];
        }
    }

    static double JobsDispatcher(double[] vecA, double[] vecB, long arrSize)
    {
        double sum = 0;
        int threadCount = (int)(arrSize / (1000 * 1000));
        int cpuCount = Environment.ProcessorCount;
        if (cpuCount <= 0)
        {
            cpuCount = 8;
            Console.Error.WriteLine($"Environment.ProcessorCount failed, default to {cpuCount}");
        }
        threadCount = Math.Min(threadCount, cpuCount);
        threadCount = Math.Max(threadCount, 1);

        double[] sums = new double[threadCount + 1];
        Task[] tasks = new Task[threadCount + 1];

        for (int i = 0; i < threadCount; ++i)
        {
            int index = i;
            tasks[i] = Task.Run(() => DotProductJob(vecA, vecB, (int)(arrSize / threadCount),
                                                    arrSize / threadCount * index, ref sums[index]));
        }

        if (arrSize % threadCount != 0)
        {
            tasks[threadCount] = Task.Run(() => DotProductJob(vecA, vecB, (int)(arrSize % threadCount),
                                                              arrSize / threadCount * threadCount, ref sums[threadCount]));
        }

        foreach (var task in tasks) {
            if (task != null) Task.WaitAll(task);
        }
        //Task.WaitAll(tasks);

        sum = sums.Sum();
        return sum;
    }

    static double GetRandom0To1()
    {
        return new Random().NextDouble();
    }

    static void Main(string[] args)
    {
        double baseNum = 2;
        int exp = 0;
        if (args.Length == 1)
            exp = int.Parse(args[0]);
        if (exp <= 0)
            exp = 5;

        Console.WriteLine("exp,vector_size,result,takes(ms)");

        for (int e = 0; e < exp; ++e)
        {
            long arrSize = (long)Math.Pow(baseNum, e);
            Console.Write($"{e,3}, {arrSize,15}, ");

            double[] vecA = Enumerable.Range(0, (int)arrSize).Select(_ => GetRandom0To1()).ToArray();
            double[] vecB = Enumerable.Range(0, (int)arrSize).Select(_ => GetRandom0To1()).ToArray();

            Stopwatch stopwatch = Stopwatch.StartNew();
            double sum = JobsDispatcher(vecA, vecB, arrSize);
            stopwatch.Stop();

            Console.WriteLine($"{sum,15:F5}, {stopwatch.Elapsed.TotalMilliseconds,10:F2}");
        }
    }
}