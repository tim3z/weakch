# Weak Contraction Hierarchies

This the repository contains most of the code I wrote for my Bachelorthesis at KIT in Karlsruhe Germany.
The code is a proof-of-concept implementation of a technique called Weak Contraction Hierarchies.
Contraction Hierarchies are an accelleration technique for finding shortest paths on road networks.
Weak Contraction Hierarchies extend this approch to be able to handle arbitrary arc metrics.
If you want more details on the theoretical backgrounds of the technique you should probably dig into the existing scientific literature.
As this implmentation could demonstrate that the technique works quite well research on it continued - you can find the results [here](http://arxiv.org/abs/1402.0402).
Or, of course, you can also read my thesis - just contact me and I'll send you the PDF.

# The Code

## Warnings

* The code as is will not compile. It is missing a priority queue implementation from the graph framework of the institute at which i wrote my thesis which i'm not allowed to publish here.
* For the same reason the project history is not included in the repository.
* Sadly, the code has no adequate test suite. (Bad) excuses: Both C++ and algorithms in general are quite painfully to test. I wrote some tests during the development but they were mostly used to ensure that some implementation details work and may not work anymore so I excluded them. Sorry, guys...
* As this is a proof-of-concept implemenation there are quite some things that could be done in better ways.
* All the code is located in the header files - yeah, I know, there are reasons not to do such a thing - but as this project is reasonable small and DRYness matters somewhat more to me than compile speed I decided to do it this way. Most likely there are going to be some more things an experienced C++ programmer would not have done... I'm open for feedback.

## What to do with it

* First of all download [METIS](http://glaros.dtc.umn.edu/gkhome/metis/metis/download) and build it.
* Than the usual `cmake` stuff - (`mkdir build`, `cd build`, `cmake ..`, `make`).
* Get a graph - I used the ones from [DIMACs](http://www.dis.uniroma1.it/challenge9/download.shtml).
* Use the simplify script to remove doubled arcs - or you might get inconsistent results later on.
* Use the gr_to_graph script to convert the DIMACs graph into a METIS readable format.
* METIS requires undirected graphs, so if your graph is directed use the undirect script to add missing arcs for metis.
* Acquire a node order with `ndmetis`.
* Apply this order to the initial file using the appy_order script - the result is what you will use for all later steps.
* Contract the graph using the `contraction` programm - this is the metric independent preprocessing.
* Generate the macrocode (and the binary graph file) for the customization phase - `buildmacrocode`
* Introduce a metric with `macrocustomization` - your DIMACs file contains one but make sure to use the reordered file.
* Run superfast queries on the result of the customization (`queries`) - HOORAY

## Programms

Each programm has a (little) more documentation in its source file.

* `contraction` (`contraction/contraction.cpp`) contracts the given graph. This is the metric independent preprocessing.
* `buildmacrocode` (`contraction/macro.cpp`) generates a macroinstruction file for the customization (and the file for the graph without metric).
* `macrocustomization` (`customization/macro_customization.cpp`) introduces an arbitrary metric to a preprocessed graph (using macrocode) and generates a graph ready for fast queries.
* `builder` (`customization/builder.cpp`) converts a DIMACs .gr graph into the binary graph format of this project.
* `queries` (`query/query.cpp`) runs (a lot of) random queries on the provided graph.

## Scripts

All scripts are written in ruby and have no external dependencies.
I have used them with ruby 1.9 and 2.0 but I guess 1.8 should do as well.
Some more documentation can be found in each script.

* `simplify.rb` removes any doubled edges from a graph.
* `gr_to_graph.rb` converts a DIMACs .gr file to a METIS .graph file.
* `undirect.rb` adds missing backward arcs to a graph for METIS.
* `apply_order.rb` reorders the node ids of a graph.
* `create_dist_metric.rb` computes a distance metric from coordinates.
* `metis_extractor.rb` turnes ndmetis output into a machine-readable csv.
* `grid.rb` contains a seperate implementation of the contraction process for grid graphs. Quite usefull to get an impression of how things behave. Outputs LaTeX (with TikZ).
* `experiments.rb` is the script I used to run all sorts of different experiments with all possible combinations of different variants (which were activated through compile flags) on all avaiable graphs. HORRIBLE. And deprecated since i cleaned most of that mess away for publishing.