require 'matrix'
include Math

N = 1000
tmax = 100.0
dt = tmax/N
M = 50

def f(x, xs, m1, idx)
  force = Vector[0.0, 0.0]
  0.upto(M-1) do |i|
    next if xs[i] == xs[idx]
    force += m1[i]*m1[idx]*(xs[i]-xs[idx]) / ((xs[i]-xs[idx]).r + 0.1)**3
  #xs.each do |y|
  #  next if x == y
  #  force += (y-x) / ((y-x).r + 0.1)**3
  end
  force
end

xs = []
vs = []

xs[0] = Vector[0.0, 0.0]
vs[0] = Vector[0.0, 0.0]
1.upto(M-1) do |i|
  xs[i] = Vector[6*rand-3, 6*rand-3]
  vs[i] = Vector[rand-0.5, rand-0.5]
end

m1 = []
m1[0] = 5.0
1.upto(M-1) do |i|
  m1[i] = 1.0
end

puts M

0.upto(N-1) do |i|
  0.upto(M-1) do |j|
    vs[j] += f(xs[j], xs, m1, j)*dt
  end
  0.upto(M-1) do |j|
    xs[j] += vs[j]*dt
  end

  xs.each do |x|
    puts "#{(i+1)*dt} #{x[0]} #{x[1]}"
  end
end
