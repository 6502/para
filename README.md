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

To handle use case 2 this library introduces just one class `Para`. At instantiation it creates
a pool of workers that are waiting for something to do.

The method `void Para::run(std::function<void()>f)` just starts all workers on calling `f`; when
function return from all workers, call to `run` returns.

Example usage

    std::atomic_int gy{0}
    para.run([&](){
        for (int y=gy++; y<h; y=gy++) {
            // process image scanline y
        }
    });

# Warning

It's important to note that while efforts have been made to ensure the correctness of this
implementation, it's not guaranteed to be bug-free. The complexities of multithreading can
lead to unexpected issues that might not be immediately apparent, even when using tools like
thread and address sanitizers.
