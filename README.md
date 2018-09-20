# DSM-on-Real-Network

This project implements an abstract [Distributed Shared Memory (DSM)](https://en.wikipedia.org/wiki/Distributed_shared_memory) system on the real network using [PARTOV](http://sharif.edu/~kharrazi/partov/). The *cores* are assigned to have their specific nodes in the real network (i.e., IP, MAC, etc.). The communications of cores are provided by an interconnect which consists of *routers*, as many as cores. Just like cores, routers have their specific nodes in the real network. 

Each core has its local cache, and a shared memory backs all cores. Some basic operations of cores (e.g., caching data) is implemented in the baseline version. The routers are responsible for delivering the requests from the source node to the destination node via XY routing policy. 

Being TA, I defined/implemented the project as a programming assignment for [Computer Networks course](http://ce.sharif.edu/~b_momeni/ce443/40443-941.html). However, it might outgrow its purpose, in the future. For example, the interconnect load can be easily obtained from the statistics that PARTOV reports based on measurements on the real network (Internet). As another example, one might interest in parallel simulation of many cores with vast resources available in network-based computation. 

The details of the protocol and supported core-side operations can be obtained from the associated [document](https://github.com/bakhshalipour/DSM-on-Real-Network/blob/master/doc.pdf).

