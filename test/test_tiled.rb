# coding: utf-8

t = RGD::Image.create_truecolor(10, 10)
t.fill(0, 0, 0xFFFFFF)
t.line(0, 0, 9, 9, 0xFF0000)
t.transparent = 0xFFFFFF

m = RGD::Image.create_truecolor(50, 50)
m.filled_rectangle(0, 0, 25, 25, 0x00FF00)
m.tile = t
m.filled_rectangle(10, 10, 49, 49, RGD::Image::COLOR_TILED)

basename = File.basename(__FILE__, '.rb')
n = RGD::Image.new(basename + '.png')
puts "#{basename}: #{m.compare(n) == 0 ? 'ok' : 'fail'}" 
