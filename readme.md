# Hyperdimensional Computing Experiment

Inspired by a seminar on "Computing with Hyper-Dimensional Vectors" by Pentti Kanerva, this project is an experimental implementation of a concepts described in the talk.

In the talk, Kanerva describes an application of Hyper-Dimension Computing. 27 random High Dimensional Vectors (HDVs) are used to represent the letters A-Z and a space. These are the "27 seed vectors". Given an input text, each trigram's seed vectors are combined into a *profile vector*. This profile vector accumulates the information of all the trigrams from the input text. As such, "queries" can be formed to extract information from the profile. In this case, the queries have the form of "given two characters, what is the next most likely character?". They are constructed by combining two seed vectors and multiplying it with the profile vector. This yields a query vector that is then compared to the 27 seed vectors to determine which next character is most likely.

Much was left out of this brief summary. The slide deck (2) linked below contains a light, written summary of the experiment. The slide deck supports the seminar (1). The paper (3) contains all the information required to implement the project.

## Sources

1. [P. Kanerva - Stanford Seminar - Computing with High-Dimensional Vectors](https://www.youtube.com/watch?v=zUCoxhExe0o)
2. [P. Kanerva - Slide deck - Computing with High-Dimensional Vectors](https://web.stanford.edu/class/ee380/Abstracts/171025-slides.pdf)
3. [P. Kanerva - Paper - Hyperdimensional Computing: An Introduction to Computing in Distributed Representation with High-Dimensional Random Vectors](https://link.springer.com/article/10.1007/s12559-009-9009-8)

# Usage

First, find a large text file. For example, I used the [complete works of Shakespeare](https://ocw.mit.edu/ans7870/6/6.006/s08/lecturenotes/files/t8.shakespeare.txt). Nearly 900,000 words. Then, generate the seed vectors and the profile vector from this text with `make-profile`. Depending on your hardware, this may take a minute or so. Please be patient.
```sh
$ ./hdc-experiment make-profile input.txt profile.hdv
```

Use the `interactive` mode to manually query the previously generated `output-profile.hdv` with digrams to get the next most likely characters. Queried digrams return the distance of each seed vector to the query vector. The seed vectors are sorted by lowest distance first (most likely first).
```sh
$ ./hdc-experiment interactive profile.hdv
Enter digram (q to quit): th
  'e': 0.4722
  'a': 0.4911
  'i': 0.4913
  ' ': 0.4915
...
```

Finally, generate a table (CSV) with the number of most significant likely letters predicted for each possible digram with `digram-table`.
```sh
$ ./hdc-experiment digram-table output.csv profile.hdv
```

## Tests

Run the tests to validate the functionality of the `hdvector` module.
```sh
$ ./hdc-experiment-test
```

