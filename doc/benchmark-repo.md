# Repository backends benchmark

There is a class, a binary and a script to benchmark backends for the
QC repository.

### runRepositoryBenchmark.cxx

It is the executable that is called to simulate a client for the
repository. It accepts a large number of options, not detailed here.
Executable is called `repositoryBenchmark`.

_Example execution :_
```
o2-qc-repository-benchmark --max-iterations 30
                    --id benchmarkTask_0
                    --mq-config ~/alice/QualityControl/Framework/alfa.json
                    --number-tasks 1
                    --delete 0
                    --control static
                    --size-objects 10
                    --number-objects 10
                    --monitoring-url influxdb-udp://aido2mon-gpn.cern.ch:8087
                    --task-name benchmarkTask_0
                    --database-backend CCDB
                    --database-name ""
                    --database-username ""
                    --database-password ""
                    --database-url ccdb-test.cern.ch:8080
                    --monitoring-threaded 0
                    --monitoring-threaded-interval 5
                    --size-objects-old-behaviour false
```

### RepositoryBenchmark

The FairMQ device that does the actual publication to the repository.
It can be configured in terms of objects' size, number of objects
published, number of iterations, etc...

### repo_benchmark.sh

A shell script to drive the whole benchmark. It iterates over the
possible values of the variables (number of tasks, number of objects
published per second, size of the objects) and remotely launches
repositoryBenchmark as many times needed on the machine(s).

## How to start using it

1. Edit the bash script `o2-qc-repo-benchmark.sh` by modifying the variables
NB_OF_TASKS, NB_OF_OBJECTS and SIZE_OBJECTS at the top. They arrays
that should contain all the possible values for these three variables.
2. Find a set of machines and assign the variable NODES accordingly.
3. Edit the other variables if needed, such as NUMBER_CYCLES, MONITORING_URL
DB_*.
4. Make sure that the QC is installed on the client nodes and that there
is the key there for a password-less ssh connection.

## Note on size-objects-old-behaviour

To test the storage of various size of objects we used histograms with varying number of bins. However, the way they are now serialized make them far smaller than before. 

Setting the parameter `size-objects-old-behaviour` to `true` will use the old histograms (useful to compare with previous results) while setting it to `false` will use objects of the correct size. 