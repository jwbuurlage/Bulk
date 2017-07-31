# BSP Model

The bulk synchronous parallel (BSP) model was developed by Leslie Valiant in the 1980s. The BSP model is intended as a bridging model between parallel hardware and software. It is an elegant and simple model that has a small and easy to understand interface.

The BSP model is defined on an abstract computer called a BSP computer. This computer has three important requirements.

1. It has \(p\) processors capable of computation and communication, i.e. it allows for local memory transactions.
2. It has a network in place that allows the different processors to send and receive data.
3. It has a mechanism that allows for the synchronisation of these processors, e.g. by means of a blocking barrier.

A BSP program consists of a number of distinct blocks of computation and communication called *supersteps*. These steps are separated by a barrier synchronisation, and consist of a computation and a communication step.

An important part of a BSP algorithm is the associated cost function. To this end we introduce two important concepts: namely an \(h\)-relation, and a notion of the *work* done by a processor. Furthermore we introduce two parameters that define a BSP computer: \(g\) and \(l\).

An \(h\)-relation is a superstep in which each processor sends or receives a maximum of \(h\) words of data. We commonly denote with \(s\) the id of a processor such that we can write for the \(h\)-relation:

$$h = \max_s \left\{ \max \{ (h_s)_\text{sent}, (h_s)_\text{received} \}~|~\text{processors } s \right\}$$

Where \(h_s\) denotes the number of words received or sent by processor \(s\). Similarly we define the work \(w\) done in a superstep as the maximum number of flops, floating point operations, performed by all processors. Finally we define the latency \(l\) of a superstep as the fixed constant overhead, used primarily to account for the barrier synchronisation. The values for \(g\) and \(l\) are platform-specific constants that are found emperically. The values for \(w\) and \(h\) are superstep specific and commonly obtained analytically. The total BSP cost associated to a BSP algorithm is:

$$T = \sum_{\text{supersteps } i} (w_i + g \cdot h_i + l)$$

The BSP model has gained significant interest in the last couple of years. Most notably because Google has adopted the model and has developed some technologies based on BSP such as MapReduce and Pregel. The standard for BSP implementations is [BSPlib](http://www.bsp-worldwide.org/). Modern implementations of the BSPlib include BSPonMPI, which simulates the BSP model on top of MPI, and MulticoreBSP, which provides a BSP implementation for shared-memory multi-core computers.

For a more detailed introduction on the BSP model, as well as a large number of examples of BSP programs we refer to the [introductory textbook on BSP and MPI](http://ukcatalogue.oup.com/product/9780198529392.do) by Rob Bisseling.

A large number of algorithms have already been implemented using the BSP model. Some of them with their associated cost function are listed below:

| Problem                                       | BSP Complexity |
|-----------------------------------------------| ------------------------------------------|
| Matrix multiplication                         | \(n^3/p + (n^2/p^{2/3}) \cdot g + l\) |
| Sorting                                       | \((n \log n)/p + (n/p)\cdot g + l\) |
| Fast Fourier Transform                        | \((n \log n)/p + (n/p)\cdot g + l\) |
| LU Decomposition                              | \(n^3/p + (n^2/p^{1/2})\cdot g + p^{1/2}\cdot l\) |
| Cholesky Factorisation                        | \(n^3/p + (n^2/p^{1/2})\cdot g + p^{1/2}\cdot l\) |
| Algebraic Path Problem (Shortest Paths)       | \(n^3/p + (n^2/p^{1/2})\cdot g + p^{1/2}\cdot l\) |
| Triangular Solver                             | \(n^2/p + n\cdot g + p\cdot l\) |
| String Edit Problem                           | \(n^2/p + n\cdot g + p\cdot l\) |
| Dense Matrix-Vector Multiplication            | \(n^2/p + (n/p^{1/2})\cdot g+l\) |
| Sparse Matrix-Vector Multiplication (2D grid) | \(n/p + (n/p)^{1/2}\cdot g+l\) |
| Sparse Matrix-Vector Multiplication (3D grid) | \(n/p + (n/p)^{2/3}\cdot g+l\) |
| Sparse Matrix-Vector Multiplication (random)  | \(n/p + (n/p)\cdot g+l\) |
| List Ranking                                  | \(n/p + (n/p)\cdot g+(\log p)\cdot l\) |

*(From: McColl 1998 "Foundations of Time-Critical Scalable Computing")*
