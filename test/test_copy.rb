# coding: utf-8

m = RGD::Image.create_truecolor(5, 5)
m.filled_rectangle(0, 0, 49, 49, 0x00FFFFFF)
m.transparent = 0xFFFFFF
m.filled_rectangle(1, 1, 4, 4, 0xFF00FF)

n = RGD::Image.create_truecolor(20, 20)
n.copy(m, 2, 2, 0, 0, m.width, m.height)

basename = File.basename(__FILE__, '.rb')
t = RGD::Image.new(basename + '.png')
puts "#{basename}: #{t.compare(n) == 0 ? 'ok' : 'fail'}"