# Lab1
ECE 353 Project 1

ECE 353 Computer Systems Laboratory I
Modeling Thermal Issues in Multicore Systems
Due: October 9, 2015
In this lab, you will write a C program to study thermal effects in computers. The electrical
power that computers consume gets transformed into heat. This raises the temperature of the
processor cores and needs to be effectively conducted away (using fans if necessary). Operating
at high temperatures can significantly reduce a processor’s lifetime.
Equivalent Circuit Approach to Thermal Modeling: In ECE 211 you studied the basics of circuits
and learned about the behavior of resistors, capacitors and inductors. You learned how the
application of a certain current flow or voltage at some point had an impact on the other parts
of the circuit.
There is a close analogy between heat and electrical flows. Table 1 indicates corresponding
items in the two domains. Heat flows from hot (high temperature) to cold (lower temperature)
nodes; the rate at which this happens is governed by Newton’s Law of Cooling. Heat flow
between two nodes which have a thermal resistance of Rth between them and a temperature
difference of ∆T is given by ∆T/Rth. Note the similarity between this and Ohm’s Law which
tells how the current flow is related to the voltage difference between two nodes and the path
resistance: I = ∆V/R. Similarly, just as one coulomb of charge raises a 1 Farad capacitor’s
voltage by 1 V, one joule of heat energy raises a 1-thermal-Farad node’s temperature by 1
oC.
It follows from this that we can write and solve heat flow equations the same way in which
we write and solve current flow equations. Let us start with the simplest condition of all. We
have a processor core (which we will model approximately as a single node) which dissipates
ω(t) watts of power at time t in running some computational task. Its heat is conducted away
to the ambient and the thermal resistance associated with this path is Rth. Then, if Cth is its
thermal capacitance, its temperature rises at a rate equal to the difference between the power
dissipation (ω(t)) and power outflow, divided by Cth. Denote the (assumed constant) ambient
temperature, Tamb. From this, we can write the differential equation
Cth
dT(t)
dt +
T(t) − Tamb
Rth
= ω(t).
Heat (Joules) Charge (Coulombs)
Temperature (Degrees) Voltage (Volts)
Heat transfer rate (Joules/sec) Current (Amperes)
Newton’s Law of Cooling Ohm’s Law
Thermal resistance Electrical resistance
Thermal capacitance Electrical capacitance
Table 1: Heat and Electrical Flow Analogy
1
This is a very simple model: just one node dissipating power. Modern processors, though,
consist of multiple cores, each of which dissipates power.
In this lab, we will develop a coarse-granularity thermal model for a four-core processor1
.
The notation is as follows. We number the cores 0 to 3; the ambient (i.e., environment) is
labeled node 4. Heat flow between cores i and j encounters a thermal resistance value of Ri,j.
The thermal capacitance of core i is given by Ci and its power dissipation at time t by ωi(t). The
temperature of core i at time t is Ti(t). The ambient temperature is constant and is unaffected
by heat flows into it (it has effectively infinite thermal capacitance).
The differential equations governing this model are simply a generalization of what we have
seen above:
C0
dT0(t)
dt +
X
j∈{1,2,3,4}
T1(t) − Tj(t)
R0,j
= ω0(t)
C1
dT1(t)
dt +
X
j∈{0,2,3,4}
T2(t) − Tj(t)
R1,j
= ω1(t)
C2
dT2(t)
dt
+
X
j∈{0,1,3,4}
T2(t) − Tj(t)
R2,j
= ω2(t)
C3
dT3(t)
dt +
X
j∈{0,1,2,4}
T4(t) − Tj(t)
R3,j
= ω3(t)
Of course, this can be written more compactly in the form:
dTi(t)
dt = − X
4
i = 0
i 6= j
Ti(t) − Tj(t)
Ri,jCi
+
ωi(t)
Ci
As you know from your studies in calculus, given the initial temperatures and the power
1This is coarse-granularity because in reality, a core consists of subunits which should be modeled separately
for greater accuracy. For example, a core consists of the integer and floating point register files and functional
units, reorder buffers, branch predictors and so on. Each of these has its own activity level. Typically, the register
files are the hottest part of a core. The coldest part of a processor is usually the on-chip cache where nothing
happens in any given region for long periods of time.
2
consumption functions, these equations can be solved. In this lab, we will write a program in C
to do so.
What are the consequences of temperature? As mentioned earlier, high operating temperatures
shorten processor lifetimes. This impact is highly nonlinear. While the exact expressions
are often complicated (since temperature-induced degradation has multiple physical causes), for
our purposes we can approximate the aging effect by the function
α(T) = λ exp 
−
Ea
kT 
where T is the device temperature (in degrees Kelvin) and λ is a constant of proportionality.
Ea is something called the activation energy and k is Boltzmann’s constant. If you have taken
courses in semiconductors, you may come across Ea; you may already have encountered k in
your physics courses. If you have not, simply treat them as constants: use Ea = 0.8eV (the
actual number varies depending on the technology) and k = 8.617 × 10−5 eV/oK. Running the
core at temperature T1 for one second ages it as much as running it for α(T2)/α(T1) seconds at
temperature T2 (all temperatures are in degrees Kelvin). Aging reduces the mean time to device
failure; we are only interested here in the relative aging, so that λ cancels out and its actual
value is unimportant.
We can now define the age acceleration factor as
β(Td(t), Tambient(t)) = α(Td(t))/α(Tambient)
where Td(t) and Tambient(t) are the device and ambient temperatures, respectively. The age
acceleration factor captures the aging that comes from running the device hotter than the
ambient. (Obviously, since we are using the ambient as a heat sink, we cannot run the processor
cooler than the ambient.)
The effective age of the device, A(t), at time t satisfies the differential equation
dA(t)
dt = β(Td(t), Tambient(t)).
Runge-Kutta Numerical Algorithm: One widely used approach to numerically solving differential
equations is the Runge-Kutta (RK) algorithm. We will use the fourth order RK algorithm
which we now describe. First, some notation. Assume we are solving a system of
differential equations
dyi(t)
dt
= fi(t, y1, y2, · · · , yn)
3
for i ∈ {1, 2, · · · , n}. Pick some small value, h (say, h = 0.01); this will be our step size. Then,
calculate the following:
k1,i = hfi(t, y1, y2, y3, y4) for i = 1, 2, 3, 4
k2,i = hfi(t + h/2, y1 + k1,1/2, · · · , y4 + k1,4/2) for i = 1, 2, 3, 4
k3,i = hfi(t + h/2, y1 + k2,1/2, · · · , y4 + k2,4/2) for i = 1, 2, 3, 4
k4,i = hfi(t + h, y1 + k3,1, · · · , y4 + k3,4) for i = 1, 2, 3, 4
Then,
yi(t + h) = yi(t) + k1,i + 2k2,i + 2k3,i + k4,i
6
for i = 1, 2, 3, 4.
So, start at t = 0 and keep stepping the time forward by h units each time until the end is
reached.
Do the Following:
Step 1: Write a double precision function rk() implementing the RK algorithm to solve a
system of n differential equations,
dyi(t)
dt = f(i, t, y1, y2, · · · , yn), i = 1, 2, · · · , n
n is an input parameter. The function f() is called by rk().
Step 2: Call the function in Step 1 to obtain the temperature of each of the four cores as a
function of time, given as input the thermal resistances and capacitances as well as the power
consumptions. Use h = 0.005 for the RK algorithm.
Step 3: Augment Step 2 to output the effective age of each core as a function of time (solving
the differential equation for age, A(t), given above). Assume the age at time 0 is 0 for each core.
The parameters of the thermal equivalent circuit (Ri,j and Ci) will be in a text file called
thermalParam.txt. The first line of this file will contain values for C0C1C2C3 (with a space
separating each entry). The second line will contain values for R0,0R0,1R0,2R0,3; the third
line R1,0R1,1R1,2R1,3; and so on. (Note that we won’t be using Ri,j if i = j, so values of
R0,0, R1,1, R2,2, R3,3 won’t make a difference.) Once again, a space will separate each doubleprecision
entry.
The power trace is to be read from a text file called powerTrace.txt. The n
′
th row of this
file contains the power consumption measured at time nτs; assume that the power consumption
is constant between samples. (For example, the first row is the power consumption measured
at time τs; you should assume that this is the (constant) power consumption throughout the
interval [0, τs]; the second row is the power consumption at time measured at time 2τs and you
should assume this is the (constant) power consumption throughout (τs, 2τs], and so on.) Each
row has 4 entries; the i
′
th entry is the power consumption of core i.
4
The output of your program will have as many rows as does the input file; The n
′
th row of
the output corresponds to the time nτs (for n = 1, 2, · · · ). It has a total of nine entries. The
first entry is the time (i.e., nτs). The second entry is the temperature of core 0, the third is the
relative age of core 0, the fourth the temperature of core 1, the fifth the relative age of core 1,
etc. This output should be written into a file called tempOutput.txt.
Submission will be on quark.ecs.umass.edu, using a script that will be posted about a week
before this project is due.
5
