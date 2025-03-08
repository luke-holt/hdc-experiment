


**Multiplication by vector**
XOR.
A ^ B produces a vector A_B that is as far away from A as the number of 1s in B
(hamming distance)

A ^ B ^ B == A

XOR changes the state of a bit in A if the bit in B is 1.
If A is a random vector, half its bits are 1s, and so A\_B is in the part of the space that is unrelated to A in terms of the distnace criterion.
Multiplication randomizes.

Because multiplication preserves distance, it also preserves noise.

If each of the multiplied vectors contains independent random noise, the amount of noise in the product - its distance to the noise-free product-vector - is given by 2 = f + g - 2fg, where f and g are the relative amounts of noise in the two vectors being multiplied.


? If XOR == multiplication...
1 ^ 1 = 0 |  1 *  1 != -1 ->  1
0 ^ 1 = 1 | -1 *  1 !=  1 -> -1
1 ^ 0 = 1 |  1 * -1 !=  1 -> -1
0 ^ 0 = 0 | -1 * -1 != -1 ->  1
shouldn't it be nXOR?

**Permutation as multiplication**


https://www.youtube.com/watch?v=WUEb9YVas84

consider data $x_1, ..., x_2 \in {0, ..., m-1}^d

"m is number of colors you have"

binding operation maps data to a hypervector

"product over all dimensions of our data of a permutation of the hypervector, where the permutation is taken from some family and depends on the pixel location"
"the hypervector depends on the color at location i"

no floating point.
take hvs, pick one based on color, permute its entries, then product

example:
dimension = number of pixels
m = 3 colors


for the "m" things

hypervectors are chosen based on a certain covariance structure

Colors should be related by a linear kernel. As you move along the color spectrum, you are more related to the colors close to you.

how do you choose the kernel? gaussian mostly used.
what is the geometry of the embedding space?


assumption 1: permutations don't collide

permutations can't have collisions.
cyclical shifts don't have collisions.

if your families of permutations have lots of collisions, when you try to encode, it won't work.


assumption 2: affinity kernel must be admissible.
must be able to decompose it to generate HVs based on a specific covariance structure


---
https://web.stanford.edu/class/ee380/Abstracts/171025-slides.pdf

slide 21
"add" is coordinatewise majority
"multiply" is XOR
"permute" (rotate) vector coordinates: P = rA
"compare" is hamming distance

slide 24 et al
binding is just regular XOR, not nXOR

