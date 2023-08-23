# para
A simple thread pool

## why

Multithread programming, in the general sense, is incredibly hard (at least for my brain).
There are however a couple of cases in which I don't think the complexity explodes

1. **Forgettable threads**:
   For example you start a thread reading from stdin and generating events for the application.
   No direct interaction with global state and no explicit synchronization is needed, you can
   basically start the thread and forget about it.

2. **Embarrassingly parallel workload**:
   For example you're writing a raytracer and you divide the image in times and then as many
   threads as reasonable are started each one with a similar task: pick next tile, compute it.
   Synchronization, if needed at all, is trivial (e.g. acquiring a mutex to store the result
   in a shared container). Parallelism is used massively, but only around a specific task.

Anything else is for my brain very difficult to reason about (i.e. complex shared state with
multiple threads doing independent things and synchronizing when necessary) and almost impossible
to debug to any reasonable level of correctness (anything involving multithreading is inherently
non reproducible; also logic bugs in multithreading can go unnoticed for years in production).

## how

To handle use case 2 this library introduces just one class `Para`. At first use it creates
a pool of workers that are waiting for something to do.

The static method `void Para::run(std::function<void(int i, int n)>f)` just starts all workers on calling `f`;
and passing the worker index and the total number of workers (those parameters in some cases simplifiy
work subdivision without need of synchronization - like scanlines in an image).

When function return from all workers, call to `run` returns.

## example usage

    Para::run([&](int i, int n){
        for (int y=i*height/n, y1=(i+1)*height/n; y<y1; y++) {
            // process image scanline y
        }
    });

# Warning

It's important to note that while efforts have been made to ensure the correctness of this
implementation and that I don't know of any bug, it's not guaranteed to be bug-free.
The complexities of multithreading can lead to unexpected issues that might not be
immediately apparent, even when using tools like thread and address sanitizers.
