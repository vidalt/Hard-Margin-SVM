
# This terminal will not attempt to plot anything, but only to read variable values
set terminal unknown
plot 'solution.txt' u 0:($0==0?(x1=$1):$1),\
                '' u 0:($0==0?(x2=$2):$2),\
                '' u 0:($0==0?(x3=$3):$3)
print x1
print x2
print x3
f(x)= (-x3-x1*x)/x2
g(x)= (-x3+1-x1*x)/x2
h(x)= (-x3-1-x1*x)/x2

# This is used to take the dimensions of the data and avoid autoscaling to the functions that define the hyperplane
plot "samples-classified.txt" u 2:3:4 notitle with points pt 7 palette ps 0.5, \
     "samples-classified.txt" u 2:3:1 notitle with labels offset char 0.7,0.7, \
	 "samples-misclassified.txt" u 2:3:4 notitle with points pt 7 palette ps 1.2, \
     "samples-misclassified.txt" u 2:3:1 notitle with labels offset char 0.7,0.7
set yrange [GPVAL_Y_MIN:GPVAL_Y_MAX] 

# Now producing the final plot
set terminal pdf enhanced size 20in,15in

# Adding a timestamp to avoid file overwriting
set output sprintf("%u_solution.pdf",timestamp)
unset colorbox
set palette model RGB defined (-1 "red",1 "blue")
plot "samples-classified.txt" u 2:3:4 notitle with points pt 7 palette ps 0.5, \
     "samples-classified.txt" u 2:3:1 notitle with labels offset char 0.7,0.7, \
	 "samples-misclassified.txt" u 2:3:4 notitle with points pt 7 palette ps 1.2, \
     "samples-misclassified.txt" u 2:3:1 notitle with labels offset char 0.7,0.7, \
     f(x) lt -1 lw 1.5 notitle, \
	 g(x) lt 0 lw 1.5 notitle, \
	 h(x) lt 0 lw 1.5 notitle
