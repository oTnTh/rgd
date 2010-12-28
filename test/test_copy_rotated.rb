# coding: utf-8

m = RGD::Image.create_truecolor(50, 50)
m.filled_rectangle(0, 0, 50, 50, 0xFF0000)
m.transparent = 0xFF0000
m.filled_ellipse(25, 25, 30, 40, 0x50FFFFFF)

n = RGD::Image.create_truecolor(50, 50)
n.copy_rotated(m, 25, 25, 0, 0, 50, 50, 60)

basename = File.basename(__FILE__, '.rb')
t = RGD::Image.new(basename + '.png')
puts "#{basename}: #{t.compare(n) == 0 ? 'ok' : 'fail'}"