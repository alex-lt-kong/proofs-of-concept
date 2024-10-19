using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

class Program
{
    // Import the functions from the unmanaged DLL
    [DllImport("librand.so")]
    private static extern void init_random();

    [DllImport("librand.so")]
    private static extern double get_random_0_to_1();

    private static int cpuCount = -1;

    delegate void MathFunc(double[] vecA, double[] vecB, int blockSize, long offset, ref double sum);

    static void DotProductJob(double[] vecA, double[] vecB, int blockSize, long offset, ref double sum)
    {
        for (int i = 0; i < blockSize; ++i)
        {
            sum += vecA[i + offset] * vecB[i + offset];
        }
    }

    static void ElementWisePowJob(double[] vecA, double[] vecB, int blockSize, long offset, ref double sum)
    {
        for (int i = 0; i < blockSize; ++i)
        {
            sum += Math.Pow(vecA[i + offset], vecB[i + offset]);
        }
    }

    static void QueryCpuCount()
    {
        if (cpuCount == -1)
            cpuCount = Environment.ProcessorCount;
        if (cpuCount <= 0)
        {
            cpuCount = 8;
            Console.Error.WriteLine($"Environment.ProcessorCount failed, default to {cpuCount}");
        }
    }

    static double JobsDispatcher(MathFunc jobFunc, double[] vecA, double[] vecB, long arrSize)
    {
        double sum = 0;

        int threadCount = (int)Math.Ceiling(arrSize / 1000.0 / 1000.0);
        QueryCpuCount();
        threadCount = threadCount > cpuCount ? cpuCount : threadCount;
        double[] sums = new double[threadCount + 1];
        List<Task> tasks = new List<Task>();

        for (int i = 0; i < threadCount; ++i)
        {
            int index = i;
            tasks.Add(Task.Run(() =>
            {
                jobFunc(vecA, vecB, (int)(arrSize / threadCount), arrSize / threadCount * index, ref sums[index]);
            }));
        }

        // handle quotient
        if (arrSize % threadCount != 0)
        {
            tasks.Add(Task.Run(() =>
            {
                jobFunc(vecA, vecB, (int)(arrSize % threadCount), arrSize / threadCount * threadCount, ref sums[threadCount]);
            }));
        }

        Task.WaitAll(tasks.ToArray());

        sum = sums.Sum();

        return sum;
    }

    static void Main(string[] args)
    {
        // Initialize the random number generator from the unmanaged DLL
        init_random();

        double base_ = 2;
        int exp = 0;
        if (args.Length == 1)
            exp = int.Parse(args[0]);
        if (exp <= 0)
            exp = 5;
        Console.WriteLine("exp,vector_size,result,takes(ms),result,takes(ms)");
        for (int e = 0; e < exp; ++e)
        {
            int arrSize = (int)Math.Pow(base_, e);
            Console.Write($"{e,3}, {arrSize,15}, ");
            double[] vecA = new double[arrSize];
            double[] vecB = new double[arrSize];
            for (int i = 0; i < arrSize; ++i)
            {
                vecA[i] = get_random_0_to_1();
                vecB[i] = get_random_0_to_1();
            }

            var t0 = DateTime.Now;
            var sum = JobsDispatcher(DotProductJob, vecA, vecB, arrSize);
            var t1 = DateTime.Now;
            var duration = (t1 - t0).TotalMilliseconds;
            Console.Write($"{sum,15:F5}, {duration,10:F2}");

            t0 = DateTime.Now;
            sum = JobsDispatcher(ElementWisePowJob, vecA, vecB, arrSize);
            t1 = DateTime.Now;
            duration = (t1 - t0).TotalMilliseconds;
            Console.WriteLine($"{sum,15:F5}, {duration,10:F2}");
        }
    }
}