ares
====

A c++ server framework designed to support large numbers of concurrent connections, without suffering significant i/o latency or performance degradation. It was essentially my solution, in 2002, to the [C10K problem](http://www.kegel.com/c10k.html). By modern standards it is most assuredly obsolete; see [Thrift](http://thrift.apache.org/).

It's strongly in the "opinionated framework" camp; that is, it provides everything but the main function, and the programmer was expected to write what were essentially plug-ins in order to define app-level functionality. That said, it could be, and occasionally was, used merely as a library.