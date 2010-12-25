# coding: utf-8

m = RGD::Image.create_truecolor(11, 11)
m.filled_rectangle(0, 0, 10, 10, 0xFFFFFF)

m.thickness = 3
m.line(5, 0, 5, 11, 0x0)
m.line(0, 5, 11, 5, 0x0)
m.line(0, 0, 11, 11, 0x0)

m.thickness = 1
m.line(5, 0, 5, 11, 0xFF0000)
m.line(0, 5, 11, 5, 0xFF0000)
m.line(0, 0, 11, 11, 0xFF0000)

basename = File.basename(__FILE__, '.rb')
n = RGD::Image.new(basename + '.png')
puts "#{basename}: #{n.compare(m) == 0 ? 'ok' : 'fail'}"
