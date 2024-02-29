This program was created for the sole purpose of testing my implementation of
the rtES-HyperNeat algorithm. The idea is to generate a set of racer-agents
that must learn to race proficiently around a track. Several tests have been
designed, each with different challenges.

- [YouTube Playlist](https://www.youtube.com/watch?v=JeXqbYXKqcs&list=PLrEmYrpTcDJb_RfwCpSnCPk1Cus310ehA)
- [Featured on Evolutionary Complexity (EPlex) Research Group](http://eplex.cs.ucf.edu/ESHyperNEAT/)

rtES-HyperNeat (Real Time Evolving Substrate Hypercube based Neuro-Evolution of
Augmenting Topologies) is a real time neuro-evolution algorithm based on the
papers written by Kenneth Stanley and other researchers at the EPLEX group
(http://eplex.cs.ucf.edu/). More specifically, rtES-HyperNeat tries to blend the
existing ES-HyperNeat (http://eplex.cs.ucf.edu/ESHyperNEAT/) algorithm with
rtNeat (http://nn.cs.utexas.edu/?rtNEAT), a previous version which allowed real
time evolution of agents on a virtual field. The main problem with trying to
make ES-HyperNeat real-timed was that the CPPNs (genotypes) had to be converted
into functional Neural Networks (phenotypes) on a single time-step. This was
proven to be impossible, so, as a solution, the library handles these
transformations on separate execution threads. This way, the algorithm flows
seamlessly.