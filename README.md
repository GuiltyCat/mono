Mono Music0
=====================

Introduction
---------------

This program generates wav file from text file.
The contents of text file is defined below.

Usage
-------------

```
./mono.out -i <input> -o <output> -p <standard pitch> -b <bit per minute>
```

Definition of mm0
----------------

```
<score>=<sound>|<sound>,<mm0>
<sound>={<chunk>}*
<chunk>=s<frac>|l<frac>|t<frac>|w<frac>|m<frac>|v<frac>|p<frac>|[a-g][0-9]{#b}
<frac>={+|-}{<float>}{/}{<float>}
<float>={0-9}*{.}{0-9}*
```

For example,

```
s0l0.3t.9w0m0.5v0.3p440,a4,s+c5
```

This is three sounds example.

First sound is
- start time is 0 [bps].
- length is 0.3 [bps].
- till is 90 [%] of length.
- wave type is 0 (sine wave).
- midway is 0.5 (ignored).
- volume is 0.3 (30% of full volume).
- pitch is 440 [Hz].

Second sound is
- start from the end of previous sound.
- pitch is a4.
- other parameter is same as previous sound.

Third sound is
- start from the start of previous sound.
- pitch is c5.
- other parameter is same as previous sound.

Score and sound
-----------------

Score is a text file that contains souds.
Each sound is separated by comma.

Sound is composed of chunk.
Chunk has serveral parameters.
However, each parameter do not have to be set all.
Some parameters are automatically complemented by the program.

Chunk parameter
---------------

Each parameter has two points except for scientific pitch notation.
First character means parameter name and second frac is a value for that parameter.

For example,

```
s100
```

means that set parameter s as 100.


### s

Parameter s express the start time for its chunk.
Time is counted by the number of beats.

If set bpm as 60, one beat is 1 [sec].
In this case, if you set start time as 5.
The chunk sound starts from 5[sec].

If set bpm as 120, one beat is 0.5 [sec].
In this case, if you set start time as 5.
The chunk sound starts from 2.5[sec].


If you do not set this parameter,
s is automatically setted as the end of previous chunk.
This mens that s is set as the sum of s and l in previous chunk.



If you use + in the head of frac,
the sound of its chunk start from start of previous  chunk plus frac of current chunk.
```
  previous
|------------------------------|
A       +frac        |------------------------|
count from here      A      currentt
                     start from here
```

If you use - in the head of frac,
the sound of its chunk start from end of previous chunk minus frac of current chunk.


```
  previous
|------------------------------|
               -frac           A
                               count from here
               |------------------------|
               A      current
              start from here
```

Be careful, start points of + and - are not same.

### l

Parameter l express the allocated length of sounds in its chunk.
This does not mean sound continues full of length. (See parameter t)
Time is counted by the number of beats.

If set bpm as 120 and set l as 3, the lenght of sound is counted as 1.5 [sec].


### t

Parameter t express the length of sounds that truly sound in its chunk.
The sound starts from paramter s and sounds till parameter t.
And left l - t time is rest.

This parameter is used as stacato, legato, and pulse sound like a drum.


### w

Parameter w express the sound type.

- 0 means sine wave.
- 1 means pulse wave.
- 2 means triangle wave.


pulse wave and triangle wave have paramter m. (See parameter m)

### m

If you use pulse wave or triangle wave, you can use parameter m.
m means the midway of sound.

If you use pulse wave, m means changing point of wave.

```
        period
|-----------------------|
 ---------
         |
         ---------------
|--------|--------------|
 m*period  (1-m)*period
```

If you use triangle wave, m means changing point of tilt.


```
        period
|----------------------|
        / \
      /      \
0----------------------
   /             \
 /                   \
|--------|-------------|
 m*period  (1-m)*period
```

To tell the truth, triangle wave start from 0.
Thus phase is shifted m*period/2 earlier than above.

### v

Parameter v means amplitude of wave.
Volume is expressed as a percentage from max volume.
Max is 1, minimum is 0.


### p

Parmeter p means pitch of wave.
The unit of p is [Hz].


### [a-g][0-9]{#b}

This parameter is alternative of parameter p.
This is scientific pitch notation.

Sharp is expressed as # and, Frat is expressed as b.



frac
------------------

frac means the expression of the number in this mm0.

In the head of frac, you can add + or -.

You can use these expression.

```
1
0.1
1.1
.1
+3
+
-
+3.0/1
-.1/1.3
+1/2

```

