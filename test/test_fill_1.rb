# coding: utf-8

m = RGD::Image.create_truecolor(100, 100)
m.fill(0, 0, 0xFFFFFF)
m.fill(0, 0, 0xFFFFFF)

basename = File.basename(__FILE__, '.rb')
n = RGD::Image.new(basename + '.png')
puts "#{basename}: #{n.compare(m) == 0 ? 'ok' : 'fail'}"
